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

#include <filament/Box.h>
#include <filament/Engine.h>
#include <filament/RenderableManager.h>
#include <filament/Scene.h>
#include <filament/TransformManager.h>

#include <gz/common/Console.hh>

#include "gz/rendering/filament/FilamentConversions.hh"
#include "gz/rendering/filament/FilamentGeometry.hh"
#include "gz/rendering/filament/FilamentMesh.hh"
#include "gz/rendering/filament/FilamentRenderEngine.hh"
#include "gz/rendering/filament/FilamentScene.hh"
#include "gz/rendering/filament/FilamentMesh.hh"
#include "gz/rendering/filament/FilamentVisual.hh"
#include "gz/rendering/Utils.hh"

using namespace gz;
using namespace rendering;

//////////////////////////////////////////////////
FilamentVisual::FilamentVisual()
{
}

//////////////////////////////////////////////////
FilamentVisual::~FilamentVisual()
{
}

//////////////////////////////////////////////////
void FilamentVisual::SetVisible(bool _visible)
{
  this->visible = _visible;
  this->UpdateLayers();
  for (unsigned int i = 0; i < this->ChildCount(); ++i)
  {
    auto child = std::dynamic_pointer_cast<Visual>(this->ChildByIndex(i));
    if (child)
      child->SetVisible(_visible);
  }
}

//////////////////////////////////////////////////
void FilamentVisual::SetVisibilityFlags(uint32_t _flags)
{
  BaseVisual::SetVisibilityFlags(_flags);
  this->UpdateLayers();
}

//////////////////////////////////////////////////
void FilamentVisual::SetWireframe(bool _show)
{
  // like ogre2: this visual's own geometries only
  this->wireframe = _show;
  for (auto it = this->geometries->Begin(); it != this->geometries->End(); ++it)
  {
    auto mesh = std::dynamic_pointer_cast<FilamentMesh>(*it);
    if (mesh)
      mesh->SetWireframe(_show);
  }
}

//////////////////////////////////////////////////
void FilamentVisual::UpdateLayers()
{
  const uint8_t layers = this->visible ?
      FilamentConversions::ToLayers(this->VisibilityFlags()) : 0u;
  for (auto it = this->geometries->Begin(); it != this->geometries->End(); ++it)
    (*it)->SetLayers(layers);
}

//////////////////////////////////////////////////
GeometryStorePtr FilamentVisual::Geometries() const
{
  return this->geometries;
}

//////////////////////////////////////////////////
bool FilamentVisual::AttachGeometry(GeometryPtr _geometry)
{
  if (!_geometry)
  {
    gzerr << "Cannot attach null geometry." << std::endl;
    return false;
  }
  FilamentGeometryPtr derived =
      std::dynamic_pointer_cast<FilamentGeometry>(_geometry);
  if (!derived)
  {
    gzerr << "Cannot attach geometry created by another render-engine"
          << std::endl;
    return false;
  }
  if (derived->Entity().isNull())
  {
    gzerr << "Cannot attach a geometry without a filament entity"
          << std::endl;
    return false;
  }
  derived->SetParent(this->SharedThis());
  auto &tm = FilamentRenderEngine::Instance()->FilamentEngine()
      ->getTransformManager();
  tm.setParent(tm.getInstance(derived->Entity()),
               tm.getInstance(this->entity));
  // like ogre2: drawn only once the visual is in the scene graph
  if (this->InSceneGraph())
    this->scene->Native()->addEntity(derived->Entity());
  derived->SetLayers(this->visible ?
      FilamentConversions::ToLayers(this->VisibilityFlags()) : 0u);
  return true;
}

//////////////////////////////////////////////////
bool FilamentVisual::DetachGeometry(GeometryPtr _geometry)
{
  FilamentGeometryPtr derived =
      std::dynamic_pointer_cast<FilamentGeometry>(_geometry);
  if (!derived)
  {
    gzerr << "Cannot detach geometry created by another render-engine"
          << std::endl;
    return false;
  }
  if (!derived->Entity().isNull())
  {
    auto &tm = FilamentRenderEngine::Instance()->FilamentEngine()
        ->getTransformManager();
    tm.setParent(tm.getInstance(derived->Entity()), {});
    this->scene->Native()->remove(derived->Entity());
  }
  derived->SetParent(nullptr);
  return true;
}

//////////////////////////////////////////////////
void FilamentVisual::SetInScene(bool _inScene)
{
  filament::Scene *native = this->scene ? this->scene->Native() : nullptr;
  if (!native)
    return;
  for (auto it = this->geometries->Begin(); it != this->geometries->End(); ++it)
  {
    const ::utils::Entity e = (*it)->Entity();
    if (e.isNull())
      continue;
    // a wireframe mesh's hidden surface goes with it
    auto mesh = std::dynamic_pointer_cast<FilamentMesh>(*it);
    const ::utils::Entity hidden = mesh ? mesh->HiddenSurface() :
        ::utils::Entity();
    if (_inScene)
    {
      native->addEntity(e);
      if (!hidden.isNull())
        native->addEntity(hidden);
    }
    else
    {
      native->remove(e);
      if (!hidden.isNull())
        native->remove(hidden);
    }
  }
  FilamentNode::SetInScene(_inScene);
}

//////////////////////////////////////////////////
void FilamentVisual::Init()
{
  BaseVisual::Init();
  this->geometries = FilamentGeometryStorePtr(new FilamentGeometryStore);
}

//////////////////////////////////////////////////
FilamentVisualPtr FilamentVisual::SharedThis()
{
  return std::dynamic_pointer_cast<FilamentVisual>(this->shared_from_this());
}

//////////////////////////////////////////////////
math::AxisAlignedBox FilamentVisual::LocalBoundingBox() const
{
  math::AxisAlignedBox box;
  this->BoundsHelper(box, true, this->WorldPose());
  return box;
}

//////////////////////////////////////////////////
math::AxisAlignedBox FilamentVisual::BoundingBox() const
{
  math::AxisAlignedBox box;
  this->BoundsHelper(box, false, this->WorldPose());
  return box;
}

//////////////////////////////////////////////////
void FilamentVisual::BoundsHelper(math::AxisAlignedBox &_box, bool _local,
    const math::Pose3d &_pose) const
{
  // ponytail: lights get no box (ogre2 gives attached lights a unit box)
  auto &rm = FilamentRenderEngine::Instance()->FilamentEngine()
      ->getRenderableManager();
  if (this->visible && this->VisibilityFlags() != GZ_VISIBILITY_GUI)
  {
    const math::Vector3d scale = this->WorldScale();
    math::Pose3d transform = _pose;
    if (_local)
    {
      // this visual's pose relative to _pose (the queried visual)
      const math::Pose3d world = this->WorldPose();
      const math::Quaterniond rotInv = _pose.Rot().Inverse();
      transform = math::Pose3d(rotInv * (world.Pos() - _pose.Pos()),
          rotInv * world.Rot());
    }
    for (auto it = this->geometries->Begin(); it != this->geometries->End();
        ++it)
    {
      const ::utils::Entity e = (*it)->Entity();
      if (e.isNull() || !rm.hasComponent(e))
        continue;
      const filament::Box bb = rm.getAxisAlignedBoundingBox(
          rm.getInstance(e));
      const filament::math::float3 lo = bb.getMin();
      const filament::math::float3 hi = bb.getMax();
      math::AxisAlignedBox box(scale * math::Vector3d(lo.x, lo.y, lo.z),
          scale * math::Vector3d(hi.x, hi.y, hi.z));
      _box.Merge(transformAxisAlignedBox(box, transform));
    }
  }
  for (unsigned int i = 0; i < this->ChildCount(); ++i)
  {
    auto child = std::dynamic_pointer_cast<FilamentVisual>(
        this->ChildByIndex(i));
    if (child)
      child->BoundsHelper(_box, _local, _pose);
  }
}
