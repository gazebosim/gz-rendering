/*
 * Copyright (C) 2019 Open Source Robotics Foundation
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

#include <ignition/common/Console.hh>

#include "ignition/rendering/ogre2/Ogre2DynamicRenderable.hh"
#include "ignition/rendering/ogre2/Ogre2Marker.hh"
#include "ignition/rendering/ogre2/Ogre2Material.hh"
#include "ignition/rendering/ogre2/Ogre2Mesh.hh"
#include "ignition/rendering/ogre2/Ogre2Scene.hh"
#include "ignition/rendering/ogre2/Ogre2Visual.hh"

class ignition::rendering::Ogre2MarkerPrivate
{
  /// \brief Marker material
  public: Ogre2MaterialPtr material = nullptr;

  /// \brief Flag to indicate whether or not this mesh should be
  /// responsible for destroying the material
  public: bool ownsMaterial = false;

  /// \brief Mesh Object for primitive shapes
  public: Ogre2MeshPtr mesh = nullptr;

  /// \brief DynamicLines Object to display
  public: std::shared_ptr<Ogre2DynamicRenderable> dynamicRenderable;
};

using namespace ignition;
using namespace rendering;

//////////////////////////////////////////////////
Ogre2Marker::Ogre2Marker()
  : dataPtr(new Ogre2MarkerPrivate)
{
}

//////////////////////////////////////////////////
Ogre2Marker::~Ogre2Marker()
{
  this->Destroy();
}

//////////////////////////////////////////////////
void Ogre2Marker::PreRender()
{
  this->dataPtr->dynamicRenderable->Update();
}

//////////////////////////////////////////////////
void Ogre2Marker::Destroy()
{
  if (!this->Scene())
    return;

  if (this->dataPtr->mesh)
  {
    this->dataPtr->mesh->Destroy();
    this->dataPtr->mesh.reset();
  }

  if (this->dataPtr->dynamicRenderable)
  {
    this->dataPtr->dynamicRenderable->Destroy();
    this->dataPtr->dynamicRenderable.reset();
  }

  if (this->dataPtr->material && this->dataPtr->ownsMaterial)
  {
    this->Scene()->DestroyMaterial(this->dataPtr->material);
    this->dataPtr->material.reset();
  }
}

//////////////////////////////////////////////////
Ogre::MovableObject *Ogre2Marker::OgreObject() const
{
  switch (this->markerType)
  {
    case MT_NONE:
      return nullptr;
    case MT_BOX:
    case MT_CYLINDER:
    case MT_ELLIPSOID:
    case MT_SPHERE:
      return this->dataPtr->mesh->OgreObject();
    case MT_LINE_STRIP:
    case MT_LINE_LIST:
    case MT_POINTS:
    case MT_TRIANGLE_FAN:
    case MT_TRIANGLE_LIST:
    case MT_TRIANGLE_STRIP:
    {
      return this->dataPtr->dynamicRenderable->OgreObject();
    }
    default:
      ignerr << "Invalid Marker type " << this->markerType << "\n";
      return nullptr;
  }
}

//////////////////////////////////////////////////
void Ogre2Marker::Init()
{
  this->Create();
}

//////////////////////////////////////////////////
void Ogre2Marker::Create()
{
  this->markerType = MT_NONE;
  this->dataPtr->dynamicRenderable.reset(new Ogre2DynamicRenderable(
      this->scene));
  if (!this->dataPtr->mesh)
  {
    this->dataPtr->mesh =
      std::dynamic_pointer_cast<Ogre2Mesh>(this->scene->CreateBox());
  }
}

//////////////////////////////////////////////////
void Ogre2Marker::SetMaterial(MaterialPtr _material, bool _unique)
{
  _material = (_unique) ? _material->Clone() : _material;

  Ogre2MaterialPtr derived =
    std::dynamic_pointer_cast<Ogre2Material>(_material);

  if (!derived)
  {
    ignerr << "Cannot assign material created by another render-engine"
      << std::endl;

    return;
  }

  std::string materialName = derived->Name();

  derived->SetReceiveShadows(false);
  derived->SetCastShadows(false);
  derived->SetLightingEnabled(false);

  switch (this->markerType)
  {
    case MT_NONE:
      break;
    case MT_BOX:
    case MT_CYLINDER:
    case MT_ELLIPSOID:
    case MT_SPHERE:
      this->dataPtr->mesh->SetMaterial(derived, false);
      break;
    case MT_LINE_STRIP:
    case MT_LINE_LIST:
    case MT_POINTS:
    case MT_TRIANGLE_FAN:
    case MT_TRIANGLE_LIST:
    case MT_TRIANGLE_STRIP:
      this->dataPtr->dynamicRenderable->SetMaterial(derived, false);
      break;
    default:
      ignerr << "Invalid Marker type " << this->markerType << "\n";
      break;
  }

  if (this->dataPtr->material && this->dataPtr->ownsMaterial)
    this->Scene()->DestroyMaterial(this->dataPtr->material);

  this->dataPtr->material = derived;
  this->dataPtr->ownsMaterial = _unique;
}

//////////////////////////////////////////////////
MaterialPtr Ogre2Marker::Material() const
{
  return this->dataPtr->material;
}

//////////////////////////////////////////////////
void Ogre2Marker::SetPoint(unsigned int _index,
    const ignition::math::Vector3d &_value)
{
  this->dataPtr->dynamicRenderable->SetPoint(_index, _value);
}

//////////////////////////////////////////////////
void Ogre2Marker::AddPoint(const ignition::math::Vector3d &_pt,
    const ignition::math::Color &_color)
{
  this->dataPtr->dynamicRenderable->AddPoint(_pt, _color);
}

//////////////////////////////////////////////////
void Ogre2Marker::ClearPoints()
{
  this->dataPtr->dynamicRenderable->Clear();
}

//////////////////////////////////////////////////
void Ogre2Marker::SetType(MarkerType _markerType)
{
  if (_markerType == this->markerType)
    return;

  this->markerType = _markerType;

  auto visual = std::dynamic_pointer_cast<Ogre2Visual>(this->Parent());

  // clear geom if needed
  if (this->dataPtr->mesh)
  {
    if (visual)
    {
      visual->RemoveGeometry(
          std::dynamic_pointer_cast<Geometry>(shared_from_this()));
    }
    this->dataPtr->mesh->Destroy();
  }


  GeometryPtr newMesh;
  switch (_markerType)
  {
    case MT_NONE:
      break;
    case MT_BOX:
      newMesh = this->scene->CreateBox();
      break;
    case MT_CYLINDER:
      newMesh = this->scene->CreateCylinder();
      break;
    case MT_ELLIPSOID:
      newMesh = this->scene->CreateEllipsoid();
      break;
    case MT_SPHERE:
      newMesh = this->scene->CreateSphere();
      break;
    case MT_LINE_STRIP:
    case MT_LINE_LIST:
    case MT_POINTS:
    case MT_TRIANGLE_FAN:
    case MT_TRIANGLE_LIST:
    case MT_TRIANGLE_STRIP:
      this->dataPtr->dynamicRenderable->SetOperationType(_markerType);
      break;
    default:
      ignerr << "Invalid Marker type\n";
      break;
  }

  if (newMesh)
  {
    this->dataPtr->mesh = std::dynamic_pointer_cast<Ogre2Mesh>(newMesh);
    if (visual)
    {
      visual->AddGeometry(
          std::dynamic_pointer_cast<Geometry>(shared_from_this()));
    }
  }
}

//////////////////////////////////////////////////
MarkerType Ogre2Marker::Type() const
{
  return this->markerType;
}
