/*
 * Copyright (C) 2026 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <algorithm>
#include <functional>
#include <limits>
#include <map>
#include <memory>
#include <utility>
#include <vector>

#include <filament/Box.h>
#include <filament/Engine.h>
#include <filament/IndexBuffer.h>
#include <filament/RenderableManager.h>
#include <filament/VertexBuffer.h>
#include <geometry/SurfaceOrientation.h>
#include <math/quat.h>
#include <math/vec2.h>
#include <math/vec3.h>
#include <math/vec4.h>

#include <gz/common/Console.hh>
#include <gz/common/Material.hh>
#include <gz/common/Mesh.hh>
#include <gz/common/Skeleton.hh>
#include <gz/common/SubMesh.hh>

#include "gz/rendering/Material.hh"
#include "gz/rendering/filament/FilamentConversions.hh"
#include "gz/rendering/filament/FilamentMesh.hh"
#include "gz/rendering/filament/FilamentMeshFactory.hh"
#include "gz/rendering/filament/FilamentRenderEngine.hh"
#include "gz/rendering/filament/FilamentScene.hh"
#include "FilamentUtils.hh"

using namespace gz;
using namespace rendering;

namespace
{
  /// \brief GPU buffers of one common::SubMesh, shared by all the
  /// visuals that use it
  struct SharedGeometry
  {
    /// \brief Pins the source so its address (the cache key) is not reused
    std::shared_ptr<common::SubMesh> source;
    filament::VertexBuffer *vb{nullptr};
    filament::IndexBuffer *ib{nullptr};
    filament::math::float3 bmin;
    filament::math::float3 bmax;
    ~SharedGeometry()
    {
      auto *engine = FilamentRenderEngine::Instance()->FilamentEngine();
      if (engine)
      {
        engine->destroy(this->vb);
        engine->destroy(this->ib);
      }
    }
  };

  // TODO(anyone): keyed by SubMesh address (and centering, which changes the
  // vertices), so editing a common::SubMesh in place after a visual was built
  // from it is not picked up by later visuals; key on a content hash if that
  // ever matters
  using CacheKey = std::pair<const common::SubMesh *, bool>;
  std::map<CacheKey, std::weak_ptr<SharedGeometry>> &Cache()
  {
    static std::map<CacheKey, std::weak_ptr<SharedGeometry>> cache;
    return cache;
  }

  using filament::math::float2;
  using filament::math::float3;
  using filament::math::float4;
  using filament::math::quatf;
  using filament::math::uint3;
  using filament::math::ushort4;
  using filament_utils::FreeBuffer;

  bool PrimitiveType(common::SubMesh::PrimitiveType _in,
      filament::RenderableManager::PrimitiveType &_out)
  {
    switch (_in)
    {
      case common::SubMesh::TRIANGLES:
        _out = filament::RenderableManager::PrimitiveType::TRIANGLES;
        return true;
      case common::SubMesh::LINES:
        _out = filament::RenderableManager::PrimitiveType::LINES;
        return true;
      case common::SubMesh::POINTS:
        _out = filament::RenderableManager::PrimitiveType::POINTS;
        return true;
      default:
        return false;
    }
  }
}

//////////////////////////////////////////////////
FilamentMeshFactory::FilamentMeshFactory(FilamentScenePtr _scene)
  : scene(_scene)
{
}

//////////////////////////////////////////////////
math::Vector3d FilamentMeshFactory::SubMeshCenter(const common::SubMesh &_sub,
    const MeshDescriptor &_desc)
{
  if (!_desc.centerSubMesh)
    return math::Vector3d::Zero;
  // the same arithmetic as common::SubMesh::Center(Zero)
  const math::Vector3d min = _sub.Min();
  return min + (_sub.Max() - min) * 0.5;
}

//////////////////////////////////////////////////
bool FilamentMeshFactory::Load(FilamentMeshPtr _mesh,
    const MeshDescriptor &_desc)
{
  MeshDescriptor desc = _desc;
  desc.Load();
  if (!desc.mesh)
  {
    gzerr << "Cannot load null mesh [" << _desc.meshName << "]" << std::endl;
    return false;
  }
  const common::Mesh &common = *desc.mesh;
  auto *engine = FilamentRenderEngine::Instance()->FilamentEngine();

  struct Prim
  {
    FilamentSubMeshPtr sub;
    filament::RenderableManager::PrimitiveType type;
    MaterialPtr material;
  };
  std::vector<Prim> prims;

  float3 bmin{std::numeric_limits<float>::max()};
  float3 bmax{std::numeric_limits<float>::lowest()};

  // skinned mesh: GPU skinning for up to filament's 255 bones
  const common::SkeletonPtr skel =
      common.HasSkeleton() ? common.MeshSkeleton() : nullptr;
  const bool skinned = skel && skel->NodeCount() > 0u &&
      skel->NodeCount() <= 255u;
  if (skel && !skinned)
  {
    gzwarn << "Mesh [" << _desc.meshName << "] has " << skel->NodeCount()
           << " bones, more than filament skins: drawn in bind pose"
           << std::endl;
  }

  for (unsigned int i = 0; i < common.SubMeshCount(); ++i)
  {
    auto s = common.SubMeshByIndex(i).lock();
    if (!s)
      continue;
    if (!desc.subMeshName.empty() && s->Name() != desc.subMeshName)
      continue;

    filament::RenderableManager::PrimitiveType type;
    if (!PrimitiveType(s->SubMeshPrimitiveType(), type))
    {
      gzwarn << "Skipping submesh [" << s->Name()
             << "] with unsupported primitive type" << std::endl;
      continue;
    }
    const unsigned int nv = s->VertexCount();
    const unsigned int ni = s->IndexCount();
    if (nv == 0u || ni == 0u)
      continue;

    const CacheKey key{s.get(), desc.centerSubMesh};
    std::shared_ptr<SharedGeometry> geom = Cache()[key].lock();
    if (!geom)
    {
      geom = std::make_shared<SharedGeometry>();
      geom->source = s;
      geom->bmin = float3{std::numeric_limits<float>::max()};
      geom->bmax = float3{std::numeric_limits<float>::lowest()};
      const math::Vector3d center = SubMeshCenter(*s, desc);

      // positions
      auto *pos = new float3[nv];
      for (unsigned int j = 0; j < nv; ++j)
      {
        pos[j] = FilamentConversions::Convert(s->Vertex(j) - center);
        geom->bmin = min(geom->bmin, pos[j]);
        geom->bmax = max(geom->bmax, pos[j]);
      }

      // indices
      auto *idx = new uint32_t[ni];
      for (unsigned int j = 0; j < ni; ++j)
        idx[j] = static_cast<uint32_t>(s->Index(j));

      // texture coordinates, interleaved: uv0 (set 0) and uv1 (set 1, for
      // light maps; a copy of set 0 when the mesh has a single set)
      auto *uv = new float4[nv];
      // by set: TexCoord()/TexCoordCount() warn per call on multi-set meshes
      const bool hasUv = s->TexCoordCountBySet(0u) == nv;
      const bool hasUv1 = s->TexCoordCountBySet(1u) == nv;
      for (unsigned int j = 0; j < nv; ++j)
      {
        if (hasUv)
        {
          // gz UVs have a top-left origin (like Ogre). Textures are
          // uploaded bottom row first and matc flips uv0.y in every material
          // (flipUV, on by default), so the UVs go in as they are.
          math::Vector2d t = s->TexCoordBySet(j, 0u);
          math::Vector2d t1 = hasUv1 ? s->TexCoordBySet(j, 1u) : t;
          uv[j] = {static_cast<float>(t.X()), static_cast<float>(t.Y()),
                   static_cast<float>(t1.X()), static_cast<float>(t1.Y())};
        }
        else
        {
          uv[j] = {0.0f, 0.0f, 0.0f, 0.0f};
        }
      }

      // tangent frames (quaternions) from normals, or from geometry
      auto *tangents = new float4[nv];
      {
        filament::geometry::SurfaceOrientation::Builder builder;
        builder.vertexCount(nv);
        std::vector<float3> normals;
        std::vector<float2> uv0;
        if (s->NormalCount() == nv)
        {
          normals.resize(nv);
          for (unsigned int j = 0; j < nv; ++j)
            normals[j] = FilamentConversions::Convert(s->Normal(j));
          builder.normals(normals.data());
          // with UVs, tangents follow them (normal maps)
          if (hasUv &&
              type == filament::RenderableManager::PrimitiveType::TRIANGLES)
          {
            // (the builder takes packed UVs only)
            uv0.resize(nv);
            for (unsigned int j = 0; j < nv; ++j)
              uv0[j] = uv[j].xy;
            builder.uvs(uv0.data());
            builder.positions(pos);
            builder.triangleCount(ni / 3u);
            builder.triangles(reinterpret_cast<const uint3 *>(idx));
          }
        }
        else if (type == filament::RenderableManager::PrimitiveType::TRIANGLES)
        {
          builder.positions(pos);
          builder.triangleCount(ni / 3u);
          builder.triangles(reinterpret_cast<const uint3 *>(idx));
        }
        else
        {
          // lines/points: an arbitrary frame is fine
          normals.assign(nv, float3{0.0f, 0.0f, 1.0f});
          builder.normals(normals.data());
        }
        auto *orientation = builder.build();
        std::vector<quatf> quats(nv);
        orientation->getQuats(quats.data(), nv);
        delete orientation;
        for (unsigned int j = 0; j < nv; ++j)
          tangents[j] = {quats[j].x, quats[j].y, quats[j].z, quats[j].w};
      }

      using AT = filament::VertexBuffer::AttributeType;
      using VA = filament::VertexAttribute;
      filament::VertexBuffer::Builder vbb;
      vbb.vertexCount(nv)
          .bufferCount(skinned ? 5 : 3)
          .attribute(VA::POSITION, 0, AT::FLOAT3)
          .attribute(VA::TANGENTS, 1, AT::FLOAT4)
          .attribute(VA::UV0, 2, AT::FLOAT2, 0, sizeof(float4))
          .attribute(VA::UV1, 2, AT::FLOAT2, sizeof(float2), sizeof(float4));
      if (skinned)
      {
        vbb.attribute(VA::BONE_INDICES, 3, AT::USHORT4)
            .attribute(VA::BONE_WEIGHTS, 4, AT::FLOAT4);
      }
      geom->vb = vbb.build(*engine);
      geom->vb->setBufferAt(*engine, 0,
          filament::VertexBuffer::BufferDescriptor(
          pos, sizeof(float3) * nv, &FreeBuffer<float3>));
      geom->vb->setBufferAt(*engine, 1,
          filament::VertexBuffer::BufferDescriptor(
          tangents, sizeof(float4) * nv, &FreeBuffer<float4>));
      geom->vb->setBufferAt(*engine, 2,
          filament::VertexBuffer::BufferDescriptor(
          uv, sizeof(float4) * nv, &FreeBuffer<float4>));
      if (skinned)
      {
        // like ogre2 (v1 _rationaliseBoneAssignments): the 4 largest
        // influences, renormalized; a vertex without any follows bone 0.
        // Bone index = skeleton node handle, as in FilamentMesh::InitSkeleton
        std::vector<std::vector<std::pair<float, uint16_t>>> infl(nv);
        for (unsigned int j = 0; j < s->NodeAssignmentsCount(); ++j)
        {
          const common::NodeAssignment na = s->NodeAssignmentByIndex(j);
          if (na.vertexIndex < nv && na.nodeIndex < skel->NodeCount())
            infl[na.vertexIndex].push_back(
                {na.weight, static_cast<uint16_t>(na.nodeIndex)});
        }
        auto *bones = new ushort4[nv];
        auto *weights = new float4[nv];
        for (unsigned int j = 0; j < nv; ++j)
        {
          auto &l = infl[j];
          std::sort(l.begin(), l.end(), std::greater<>());
          l.resize(std::min<size_t>(l.size(), 4u));
          float total = 0.0f;
          for (const auto &w : l)
            total += w.first;
          bones[j] = ushort4{0u};
          weights[j] = l.empty() || total <= 0.0f ?
              float4{1.0f, 0.0f, 0.0f, 0.0f} : float4{0.0f};
          for (size_t k = 0; k < l.size() && total > 0.0f; ++k)
          {
            bones[j][k] = l[k].second;
            weights[j][k] = l[k].first / total;
          }
        }
        geom->vb->setBufferAt(*engine, 3,
            filament::VertexBuffer::BufferDescriptor(
            bones, sizeof(ushort4) * nv, &FreeBuffer<ushort4>));
        geom->vb->setBufferAt(*engine, 4,
            filament::VertexBuffer::BufferDescriptor(
            weights, sizeof(float4) * nv, &FreeBuffer<float4>));
      }

      geom->ib = filament::IndexBuffer::Builder()
          .indexCount(ni)
          .bufferType(filament::IndexBuffer::IndexType::UINT)
          .build(*engine);
      geom->ib->setBuffer(*engine, filament::IndexBuffer::BufferDescriptor(
          idx, sizeof(uint32_t) * ni, &FreeBuffer<uint32_t>));
      Cache()[key] = geom;
    }
    bmin = min(bmin, geom->bmin);
    bmax = max(bmax, geom->bmax);

    FilamentSubMeshPtr sub(new FilamentSubMesh);
    sub->vertexBuffer = geom->vb;
    sub->indexBuffer = geom->ib;
    sub->geometry = geom;
    sub->mesh = _mesh.get();
    sub->index = prims.size();
    sub->subMeshIndex = i;

    // material: from the imported mesh if any, else a default one
    MaterialPtr material;
    if (const auto matIdx = s->GetMaterialIndex())
    {
      common::MaterialPtr commonMat = common.MaterialByIndex(matIdx.value());
      if (commonMat)
      {
        material = this->scene->CreateMaterial();
        material->CopyFrom(*commonMat);
      }
    }
    if (!material)
    {
      // ogre2 draws material-less meshes with its default datablock: flat
      // unlit white
      material = this->scene->CreateMaterial();
      material->SetLightingEnabled(false);
    }

    prims.push_back({sub, type, material});
  }

  if (prims.empty())
  {
    gzerr << "Mesh [" << _desc.meshName << "] has no usable submeshes"
          << std::endl;
    return false;
  }

  // ogre2 bounds animated skinned meshes by a fixed -1..1 box
  if (skel && skel->AnimationCount() != 0u)
  {
    bmin = float3{-1.0f};
    bmax = float3{1.0f};
  }
  filament::Box box;
  box.set(bmin, bmax);
  filament::RenderableManager::Builder builder(prims.size());
  builder.boundingBox(box)
      .culling(true)
      .castShadows(true)
      .receiveShadows(true);
  if (skinned)
    builder.skinning(skel->NodeCount());
  for (size_t i = 0; i < prims.size(); ++i)
  {
    builder.geometry(i, prims[i].type, prims[i].sub->vertexBuffer,
        prims[i].sub->indexBuffer);
  }
  builder.build(*engine, _mesh->entity);
  // BaseMesh::Clone recreates the mesh from it
  _mesh->meshDescriptor = _desc;

  for (auto &p : prims)
  {
    _mesh->AddSubMesh(p.sub);
    // created for this submesh only: owned (destroyed when replaced), like
    // the clone ogre2's SetMaterial(mat) makes
    p.sub->SetMaterial(p.material, false);
    p.sub->ownsMaterial = true;
  }
  if (skel)
    _mesh->InitSkeleton(skel, skinned);
  return true;
}
