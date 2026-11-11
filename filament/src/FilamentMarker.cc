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
#include <gz/common/Console.hh>

#include "gz/rendering/Capsule.hh"
#include "gz/rendering/filament/FilamentMarker.hh"
#include "gz/rendering/filament/FilamentMaterial.hh"
#include "gz/rendering/filament/FilamentScene.hh"
#include "gz/rendering/filament/FilamentVisual.hh"
#include "FilamentUtils.hh"

using namespace gz;
using namespace rendering;

//////////////////////////////////////////////////
FilamentMarker::FilamentMarker() = default;

//////////////////////////////////////////////////
FilamentMarker::~FilamentMarker() = default;

//////////////////////////////////////////////////
void FilamentMarker::Init()
{
  FilamentGeometry::Init();
  this->markerType = MT_NONE;
  this->dynamic = this->scene->CreateDynamicMesh();
}

//////////////////////////////////////////////////
void FilamentMarker::Destroy()
{
  BaseMarker::Destroy();
  if (this->geom)
  {
    // ours to tear down: do not let it call back into the visual
    this->geom->SetParent(nullptr);
    this->geom->Destroy();
    this->geom.reset();
  }
  if (this->dynamic)
  {
    this->dynamic->SetOwnerParent(nullptr);
    this->dynamic->Destroy();
    this->dynamic.reset();
  }
  // after the renderables: filament panics on destroying a material in use
  filament_utils::ReplaceMaterial(this->Scene(), this->material,
      this->ownsMaterial, nullptr, false);
}

//////////////////////////////////////////////////
void FilamentMarker::PreRender()
{
  BaseMarker::PreRender();
  if (this->geom)
    this->geom->PreRender();
  if (this->dynamic)
    this->dynamic->PreRender();
}

//////////////////////////////////////////////////
::utils::Entity FilamentMarker::Entity() const
{
  if (this->geom)
    return this->geom->Entity();
  return this->dynamic ? this->dynamic->Entity() : ::utils::Entity();
}

//////////////////////////////////////////////////
void FilamentMarker::SetLayers(uint8_t _layers)
{
  this->layers = _layers;
  if (this->geom)
    this->geom->SetLayers(_layers);
  // the dynamic mesh stays attached when the type is none: hide it
  if (this->dynamic)
    this->dynamic->SetLayers(this->markerType == MT_NONE ? 0u : _layers);
}

//////////////////////////////////////////////////
MaterialPtr FilamentMarker::Material() const
{
  return this->material;
}

//////////////////////////////////////////////////
void FilamentMarker::SetMaterial(MaterialPtr _material, bool _unique)
{
  _material = filament_utils::MaterialToAssign(_material, _unique);
  if (!_material)
    return;
  _material->SetReceiveShadows(false);
  _material->SetCastShadows(false);
  _material->SetLightingEnabled(false);

  if (this->geom)
    this->geom->SetMaterial(_material, false);
  this->dynamic->SetLineMaterial(_material);

  filament_utils::ReplaceMaterial(this->Scene(), this->material,
      this->ownsMaterial, _material, _unique);
}

//////////////////////////////////////////////////
void FilamentMarker::SetType(MarkerType _markerType)
{
  if (_markerType == this->markerType)
    return;

  // the entity changes: re-attach like FilamentCapsule does
  FilamentVisualPtr visual = this->parent;
  auto self = std::dynamic_pointer_cast<Geometry>(this->shared_from_this());
  if (visual)
    visual->DetachGeometry(self);

  if (this->geom)
  {
    this->geom->SetParent(nullptr);
    this->geom->Destroy();
    this->geom.reset();
  }
  this->markerType = _markerType;

  GeometryPtr newGeom;
  switch (_markerType)
  {
    case MT_NONE:
      break;
    case MT_BOX:
      newGeom = this->scene->CreateBox();
      break;
    case MT_CAPSULE:
      newGeom = this->scene->CreateCapsule();
      break;
    case MT_CONE:
      newGeom = this->scene->CreateCone();
      break;
    case MT_CYLINDER:
      newGeom = this->scene->CreateCylinder();
      break;
    case MT_SPHERE:
      newGeom = this->scene->CreateSphere();
      break;
    case MT_LINE_STRIP:
    case MT_LINE_LIST:
    case MT_POINTS:
    case MT_TRIANGLE_FAN:
    case MT_TRIANGLE_LIST:
    case MT_TRIANGLE_STRIP:
      this->dynamic->SetOperationType(_markerType);
      this->dynamic->SetPointSize(this->size);
      break;
    default:
      gzerr << "Invalid Marker type [" << _markerType << "]" << std::endl;
      break;
  }

  if (newGeom)
  {
    this->geom = std::dynamic_pointer_cast<FilamentGeometry>(newGeom);
    if (!this->geom)
    {
      gzerr << "Failed to cast to [FilamentGeometry], type [" << _markerType
            << "]" << std::endl;
    }
    else if (this->material)
    {
      this->geom->SetMaterial(this->material, false);
    }
  }

  this->SetLayers(this->layers);
  if (visual)
    visual->AttachGeometry(self);
}

//////////////////////////////////////////////////
void FilamentMarker::SetSize(double _size)
{
  BaseMarker::SetSize(_size);
  this->dynamic->SetPointSize(_size);
}

//////////////////////////////////////////////////
void FilamentMarker::AddPoint(const math::Vector3d &_pt,
    const math::Color &_color)
{
  this->dynamic->AddPoint(_pt, _color);
}

//////////////////////////////////////////////////
void FilamentMarker::SetPoint(unsigned int _index,
    const math::Vector3d &_value)
{
  this->dynamic->SetPoint(_index, _value);
}

//////////////////////////////////////////////////
void FilamentMarker::ClearPoints()
{
  this->dynamic->Clear();
}

//////////////////////////////////////////////////
void FilamentMarker::SetParent(FilamentVisualPtr _parent)
{
  // sensors and picking find the visual through the renderable's mesh
  FilamentGeometry::SetParent(_parent);
  if (this->geom)
    this->geom->SetParent(_parent);
  if (this->dynamic)
    this->dynamic->SetOwnerParent(_parent);
}
