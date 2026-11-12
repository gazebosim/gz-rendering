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
#include <cmath>
#include <limits>

#include <filament/Engine.h>
#include <filament/RenderableManager.h>
#include <filament/TransformManager.h>
#include <math/mat4.h>

#include <gz/common/Console.hh>
#include <gz/common/Mesh.hh>
#include <gz/common/SubMesh.hh>

#include "gz/rendering/MeshDescriptor.hh"
#include "gz/rendering/filament/FilamentMesh.hh"
#include "gz/rendering/filament/FilamentMeshFactory.hh"
#include "gz/rendering/filament/FilamentRayQuery.hh"
#include "gz/rendering/filament/FilamentRenderEngine.hh"
#include "gz/rendering/filament/FilamentScene.hh"

using namespace gz;
using namespace rendering;

namespace
{
  using filament::math::double3;

  /// \brief Distance along the ray to the front face of triangle abc, or
  /// a negative value: Ogre::Math::intersects(ray, a, b, c, true, false)
  double TriangleHit(const double3 &_o, const double3 &_d, const double3 &_a,
      const double3 &_b, const double3 &_c)
  {
    const double3 normal = normalize(cross(_b - _a, _c - _a));
    const double denom = dot(normal, _d);
    // only the side the normal points to, not parallel
    if (!(denom < -std::numeric_limits<double>::epsilon()))
      return -1.0;
    const double t = dot(normal, _a - _o) / denom;
    if (t < 0.0)
      return -1.0;
    // the hit has to be inside the three edges
    const double3 p = _o + _d * t;
    if (dot(cross(_b - _a, p - _a), normal) < 0.0 ||
        dot(cross(_c - _b, p - _b), normal) < 0.0 ||
        dot(cross(_a - _c, p - _c), normal) < 0.0)
    {
      return -1.0;
    }
    return t;
  }

  /// \brief Does the ray reach the box before _best
  bool HitsBox(const double3 &_o, const double3 &_d, const double3 &_lo,
      const double3 &_hi, double _best)
  {
    double tmin = 0.0;
    double tmax = _best;
    for (int i = 0; i < 3; ++i)
    {
      if (std::abs(_d[i]) < 1e-300)
      {
        if (_o[i] < _lo[i] || _o[i] > _hi[i])
          return false;
        continue;
      }
      double t0 = (_lo[i] - _o[i]) / _d[i];
      double t1 = (_hi[i] - _o[i]) / _d[i];
      if (t0 > t1)
        std::swap(t0, t1);
      tmin = std::max(tmin, t0);
      tmax = std::min(tmax, t1);
      if (tmin > tmax)
        return false;
    }
    return true;
  }
}

//////////////////////////////////////////////////
FilamentRayQuery::FilamentRayQuery() = default;

//////////////////////////////////////////////////
FilamentRayQuery::~FilamentRayQuery() = default;

//////////////////////////////////////////////////
void FilamentRayQuery::SetFromCamera(const WideAngleCameraPtr &,
    uint32_t, const math::Vector2d &)
{
  // TODO(anyone): ogre2 shoots through one face of the wide angle camera's
  // cube; add it when something picks through that sensor
  gzwarn << "FilamentRayQuery does not support wide angle cameras"
         << std::endl;
}

//////////////////////////////////////////////////
RayQueryResult FilamentRayQuery::ClosestPoint(
    [[maybe_unused]] bool _forceSceneUpdate)
{
  RayQueryResult result;
  auto *engine = FilamentRenderEngine::Instance()->FilamentEngine();
  if (!engine || !this->scene)
    return result;
  const math::Vector3d dirN = this->direction.Normalized();
  if (!dirN.IsFinite() || !this->origin.IsFinite() ||
      dirN == math::Vector3d::Zero)
  {
    return result;
  }
  auto &tm = engine->getTransformManager();
  auto &rm = engine->getRenderableManager();
  const double3 worldO{this->origin.X(), this->origin.Y(), this->origin.Z()};
  const double3 worldD{dirN.X(), dirN.Y(), dirN.Z()};

  double best = std::numeric_limits<double>::max();
  bool hit = false;
  for (FilamentMesh *mesh : this->scene->Meshes())
  {
    const VisualPtr visual = mesh->Parent();
    const ::utils::Entity entity = mesh->Entity();
    if (!visual || entity.isNull() || !rm.hasComponent(entity) ||
        !tm.hasComponent(entity))
    {
      continue;
    }
    const auto renderable = rm.getInstance(entity);
    // hidden by its visual
    if (rm.getLayerMask(renderable) == 0u)
      continue;

    // the ray in the mesh frame (the direction is not renormalized, so the
    // parameter stays a world distance)
    const filament::math::mat4 world =
        tm.getWorldTransformAccurate(tm.getInstance(entity));
    const filament::math::mat4 inv = inverse(world);
    const double3 o = (inv * filament::math::double4(worldO, 1.0)).xyz;
    const double3 d = (inv * filament::math::double4(worldD, 0.0)).xyz;

    const filament::Box &box = rm.getAxisAlignedBoundingBox(renderable);
    const double3 lo = double3(box.getMin());
    const double3 hi = double3(box.getMax());
    if (!HitsBox(o, d, lo, hi, best))
      continue;

    // dynamic meshes (markers, grids) have no source mesh: not pickable
    MeshDescriptor desc = mesh->Descriptor();
    if (!desc.mesh && desc.meshName.empty())
      continue;
    desc.Load();
    if (!desc.mesh)
      continue;

    for (unsigned int j = 0; j < desc.mesh->SubMeshCount(); ++j)
    {
      auto sub = desc.mesh->SubMeshByIndex(j).lock();
      if (!sub || sub->VertexCount() < 3u ||
          sub->SubMeshPrimitiveType() != common::SubMesh::TRIANGLES)
      {
        continue;
      }
      if (!desc.subMeshName.empty() && sub->Name() != desc.subMeshName)
        continue;
      const math::Vector3d *vertices = sub->VertexPtr();
      const unsigned int *indices = sub->IndexPtr();
      const math::Vector3d center =
          FilamentMeshFactory::SubMeshCenter(*sub, desc);
      auto vertex = [&](unsigned int _i)
      {
        const math::Vector3d v = vertices[indices[_i]] - center;
        return double3{v.X(), v.Y(), v.Z()};
      };
      for (unsigned int k = 0; k + 2 < sub->IndexCount(); k += 3)
      {
        const double t = TriangleHit(o, d, vertex(k), vertex(k + 1),
            vertex(k + 2));
        if (t >= 0.0 && t < best)
        {
          best = t;
          hit = true;
          result.distance = t;
          result.objectId = visual->Id();
        }
      }
    }
  }
  if (hit)
    result.point = this->origin + dirN * best;
  return result;
}
