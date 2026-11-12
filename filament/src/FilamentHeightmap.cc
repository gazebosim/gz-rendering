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
#include <cmath>
#include <vector>

#include <filament/Engine.h>
#include <filament/Material.h>
#include <filament/MaterialInstance.h>
#include <filament/TextureSampler.h>
#include <filament/TransformManager.h>
#include <math/mat4.h>
#include <math/vec4.h>

#include <gz/common/Console.hh>
#include <gz/common/Mesh.hh>
#include <gz/common/SubMesh.hh>
#include <gz/common/geospatial/HeightmapData.hh>
#include <gz/math/Helpers.hh>

#include "gz/rendering/MeshDescriptor.hh"
#include "gz/rendering/filament/FilamentHeightmap.hh"
#include "gz/rendering/filament/FilamentMaterial.hh"
#include "gz/rendering/filament/FilamentMesh.hh"
#include "gz/rendering/filament/FilamentRenderEngine.hh"
#include "gz/rendering/filament/FilamentScene.hh"
#include "gz/rendering/filament/FilamentVisual.hh"

using namespace gz;
using namespace rendering;

//////////////////////////////////////////////////
FilamentHeightmap::~FilamentHeightmap()
{
}

//////////////////////////////////////////////////
FilamentHeightmap::FilamentHeightmap(const HeightmapDescriptor &_desc)
  : BaseHeightmap(_desc)
{
}

//////////////////////////////////////////////////
void FilamentHeightmap::Destroy()
{
  // detaches from the visual through Entity(), so before the mesh goes
  BaseHeightmap::Destroy();
  if (this->mesh)
  {
    this->mesh->SetParent(nullptr);
    this->mesh->Destroy();
    this->mesh.reset();
  }
}

//////////////////////////////////////////////////
void FilamentHeightmap::PreRender()
{
  BaseHeightmap::PreRender();
  // not in any visual's geometry store: push its material from here
  if (!this->mesh)
    return;
  this->mesh->PreRender();
  // Terra ignores its scene node: the descriptor's position and size are
  // world coordinates. gz-sim scales heightmap visuals by their size
  // (1200 x for Harmonic World's terrain), so cancel the parent transform.
  auto &tm = FilamentRenderEngine::Instance()->FilamentEngine()
      ->getTransformManager();
  auto inst = tm.getInstance(this->mesh->Entity());
  if (inst && this->parent)
  {
    auto parentInst = tm.getInstance(this->parent->Entity());
    if (parentInst)
      tm.setTransform(inst,
          inverse(tm.getWorldTransformAccurate(parentInst)));
  }
}

//////////////////////////////////////////////////
::utils::Entity FilamentHeightmap::Entity() const
{
  return this->mesh ? this->mesh->Entity() : ::utils::Entity();
}

//////////////////////////////////////////////////
void FilamentHeightmap::SetLayers(uint8_t _layers)
{
  if (this->mesh)
    this->mesh->SetLayers(_layers);
}

//////////////////////////////////////////////////
void FilamentHeightmap::SetMaterial(MaterialPtr, bool)
{
}

//////////////////////////////////////////////////
MaterialPtr FilamentHeightmap::Material() const
{
  return nullptr;
}

//////////////////////////////////////////////////
void FilamentHeightmap::SetParent(FilamentVisualPtr _parent)
{
  FilamentGeometry::SetParent(_parent);
  // sensors find the owning visual (labels, temperature) through the mesh
  if (this->mesh)
    this->mesh->SetParent(_parent);
}

//////////////////////////////////////////////////
void FilamentHeightmap::Init()
{
  BaseHeightmap::Init();
  auto data = this->descriptor.Data();
  if (!data)
  {
    gzerr << "Failed to initialize: null heightmap data." << std::endl;
    return;
  }

  // Same sampling, size and normalization as Ogre2Heightmap, so that each
  // vertex is one Terra vertex at LOD 0.
  const unsigned int sampling = this->descriptor.Sampling();
  const unsigned int dataWidth = data->Width();
  const bool ogre1Compat = math::isPowerOfTwo(dataWidth - 1u);
  const unsigned int srcWidth = ogre1Compat ?
      dataWidth * sampling - sampling + 1u : dataWidth * sampling;
  const unsigned int w = math::isPowerOfTwo(srcWidth) ? srcWidth :
      srcWidth - 1u;
  if (w < 2u)
  {
    gzerr << "Heightmap [" << this->Name() << "] is too small" << std::endl;
    return;
  }

  const math::Vector3d size = this->descriptor.Size();
  math::Vector3d scale(size.X() / w, size.Y() / w, 1.0);
  std::vector<float> lookup;
  data->FillHeightMap(static_cast<int>(sampling), srcWidth, size, scale,
      false, lookup);

  // Terra stores (h - min) / (max - min) and scales it by size.Z, on top of
  // position.Z + min
  const double minElevation = data->MinElevation();
  const double maxElevation = data->MaxElevation();
  const double range = maxElevation - minElevation;
  const double heightScale =
      std::abs(range) < 1e-6 ? size.Z() : size.Z() / range;
  const math::Vector3d pos = this->descriptor.Position();

  // Terra's grid: vertex (x, y) at origin + (x, -y) * size / w, image row 0
  // on the +Y side; its last vertex is one step short of the far edge
  const double dx = size.X() / w;
  const double dy = size.Y() / w;
  const double x0 = pos.X() - size.X() * 0.5;
  const double y0 = pos.Y() + size.Y() * 0.5;
  std::vector<double> z(static_cast<size_t>(w) * w);
  for (unsigned int y = 0; y < w; ++y)
  {
    for (unsigned int x = 0; x < w; ++x)
    {
      double h = lookup[static_cast<size_t>(y) * srcWidth + x];
      if (!std::isfinite(h))
        h = minElevation;
      z[static_cast<size_t>(y) * w + x] =
          pos.Z() + minElevation + (h - minElevation) * heightScale;
    }
  }

  this->grid = std::make_unique<common::Mesh>();
  this->grid->SetName(this->Name());
  auto sub = std::make_unique<common::SubMesh>();
  sub->SetName(this->Name());
  sub->SetPrimitiveType(common::SubMesh::TRIANGLES);
  auto at = [&](int _x, int _y)
  {
    _x = std::clamp(_x, 0, static_cast<int>(w) - 1);
    _y = std::clamp(_y, 0, static_cast<int>(w) - 1);
    return z[static_cast<size_t>(_y) * w + _x];
  };
  for (unsigned int y = 0; y < w; ++y)
  {
    for (unsigned int x = 0; x < w; ++x)
    {
      const int ix = static_cast<int>(x);
      const int iy = static_cast<int>(y);
      sub->AddVertex(x0 + x * dx, y0 - y * dy, at(ix, iy));
      // central differences; +Y is towards row y - 1
      const int xl = std::max(ix - 1, 0);
      const int xr = std::min(ix + 1, static_cast<int>(w) - 1);
      const int yu = std::max(iy - 1, 0);
      const int yd = std::min(iy + 1, static_cast<int>(w) - 1);
      const double gx = (at(xr, iy) - at(xl, iy)) / ((xr - xl) * dx);
      const double gy = (at(ix, yu) - at(ix, yd)) / ((yd - yu) * dy);
      sub->AddNormal(math::Vector3d(-gx, -gy, 1.0).Normalize());
      // uv0: terrain coordinates, tiled per layer by gz_terrain; uv1.x:
      // height above the terrain bottom, Terra's localHeight
      sub->AddTexCoord(static_cast<double>(x) / w,
          static_cast<double>(y) / w);
      sub->AddTexCoordBySet(at(ix, iy) - pos.Z() - minElevation, 0.0, 1u);
    }
  }
  // Terra's strips split every quad along (x, y) - (x + 1, y + 1)
  for (unsigned int y = 0; y + 1u < w; ++y)
  {
    for (unsigned int x = 0; x + 1u < w; ++x)
    {
      const unsigned int a = y * w + x;
      const unsigned int b = a + 1u;
      const unsigned int c = a + w;
      const unsigned int d = c + 1u;
      sub->AddIndex(a);
      sub->AddIndex(c);
      sub->AddIndex(d);
      sub->AddIndex(a);
      sub->AddIndex(d);
      sub->AddIndex(b);
    }
  }
  this->grid->AddSubMesh(std::move(sub));

  this->mesh = std::dynamic_pointer_cast<FilamentMesh>(
      this->scene->CreateMesh(MeshDescriptor(this->grid.get())));
  if (!this->mesh || this->mesh->SubMeshCount() == 0u)
  {
    gzerr << "Failed to build heightmap [" << this->Name() << "]"
          << std::endl;
    this->mesh.reset();
    return;
  }
  // ogre2's thermal camera draws Terra flat white unless it is a heat source
  this->mesh->terrain = true;

  // The factory gave the material-less submesh an owned unlit white
  // material; Terra is lit and blends its textures by height
  auto mat = std::dynamic_pointer_cast<FilamentMaterial>(
      this->mesh->SubMeshByIndex(0u)->Material());
  mat->SetLightingEnabled(true);
  mat->SetDiffuse(1.0, 1.0, 1.0);
  // Terra is not in the shadow maps (it raymarches its own self shadows)
  mat->SetCastShadows(false);
  auto *engine = FilamentRenderEngine::Instance();
  filament::Material *terrain = engine->Material("gz_terrain");
  if (!terrain)
    return;
  filament::MaterialInstance *mi = terrain->createInstance();
  // like Ogre2Heightmap: texture i > 0 fades in over blend i - 1; Terra
  // takes four layers (five when the first is a base map without normals)
  // Four layers at most, a fifth base map is dropped
  const unsigned int layers = static_cast<unsigned int>(
      std::min<uint64_t>(this->descriptor.TextureCount(), 4u));
  filament::math::float4 tileU(1.0f), tileV(1.0f), minH(0.0f), maxH(0.0f);
  int normals = 0;
  // Terra samples with 8x anisotropy
  filament::TextureSampler sampler(
      filament::TextureSampler::MinFilter::LINEAR_MIPMAP_LINEAR,
      filament::TextureSampler::MagFilter::LINEAR,
      filament::TextureSampler::WrapMode::REPEAT);
  sampler.setAnisotropy(8.0f);
  for (unsigned int i = 0; i < 4u; ++i)
  {
    const HeightmapTexture *tex =
        i < layers ? this->descriptor.TextureByIndex(i) : nullptr;
    const std::string si = std::to_string(i);
    filament::Texture *diffuse = tex && !tex->Diffuse().empty() ?
        engine->Texture(tex->Diffuse()) : nullptr;
    filament::Texture *normal = tex && !tex->Normal().empty() ?
        engine->Texture(tex->Normal(), nullptr, false) : nullptr;
    mi->setParameter(("diffuse" + si).c_str(),
        diffuse ? diffuse : engine->Texture(""), sampler);
    mi->setParameter(("normal" + si).c_str(),
        normal ? normal : engine->Texture(""), sampler);
    if (normal)
      normals |= 1 << i;
    if (tex && tex->Size() > 0.0)
    {
      tileU[i] = static_cast<float>(size.X() / tex->Size());
      tileV[i] = static_cast<float>(size.Y() / tex->Size());
    }
    const HeightmapBlend *blend = i > 0u && i - 1u <
        this->descriptor.BlendCount() ? this->descriptor.BlendByIndex(i - 1u) :
        nullptr;
    // without a blend (or with a zero fade) Terra's weight is 0: hidden
    minH[i] = 1e30f;
    maxH[i] = 2e30f;
    if (blend && i < layers && std::abs(blend->FadeDistance()) >= 1e-6)
    {
      minH[i] = static_cast<float>(blend->MinHeight());
      maxH[i] = static_cast<float>(blend->MinHeight() +
          blend->FadeDistance());
    }
  }
  mi->setParameter("layers", static_cast<int>(layers));
  mi->setParameter("normalFlags", normals);
  mi->setParameter("tileU", tileU);
  mi->setParameter("tileV", tileV);
  mi->setParameter("minHeight", minH);
  mi->setParameter("maxHeight", maxH);
  // Deliberate deviation from ogre2: gz's Terra shader (GZ CUSTOMIZE in
  // Hlms/Terra/Any/800.PixelShader_piece_ps.any) blends roughness from 1
  // towards the layer's roughness map with weight 1, and a layer without a
  // map reads 0, so gz heightmaps (which have no roughness maps) shade as a
  // mirror there. ogre2 hides it behind a pin-point lobe (alpha floor
  // 0.001) and no sky reflections; here it shows as sun glints over the
  // grass and a sky sheen. Use the intended start value.
  mi->setParameter("roughness", 1.0f);
  // owned and destroyed by the material like a custom shader instance
  mat->custom = mi;
}
