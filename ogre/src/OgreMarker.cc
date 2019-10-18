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

#include "ignition/rendering/ogre/OgreDynamicLines.hh"
#include "ignition/rendering/ogre/OgreMarker.hh"
#include "ignition/rendering/ogre/OgreMaterial.hh"
#include "ignition/rendering/ogre/OgreMesh.hh"
#include "ignition/rendering/ogre/OgreScene.hh"

class ignition::rendering::OgreMarkerPrivate
{
  /// \brief Marker material
  public: OgreMaterialPtr material = nullptr;

  /// \brief DynamicLines Object to display
  public: std::shared_ptr<OgreDynamicLines> dynamicRenderable;

  /// \brief Mesh Object for primitive shapes
  public: OgreMeshPtr mesh = nullptr;

  /// \brief MarkerType of render operation
  public: MarkerType markerType;
};

using namespace ignition;
using namespace rendering;

//////////////////////////////////////////////////
OgreMarker::OgreMarker()
  : dataPtr(new OgreMarkerPrivate)
{
}

//////////////////////////////////////////////////
OgreMarker::~OgreMarker()
{
}

//////////////////////////////////////////////////
void OgreMarker::PreRender()
{
  this->dataPtr->dynamicRenderable->Update();
}

//////////////////////////////////////////////////
Ogre::MovableObject *OgreMarker::OgreObject() const
{
  switch (markerType)
  {
    case NONE:
    case BOX:
    case CYLINDER:
    case SPHERE:
      return this->dataPtr->mesh->OgreObject();
    case LINE_STRIP:
    case LINE_LIST:
    case POINTS:
    case TRIANGLE_FAN:
    case TRIANGLE_LIST:
    case TRIANGLE_STRIP:
      return std::dynamic_pointer_cast<Ogre::MovableObject>
        (this->dataPtr->dynamicRenderable).get();
    default:
      ignerr << "Invalid Marker type " << markerType << "\n";
      return nullptr;
  }
}

//////////////////////////////////////////////////
void OgreMarker::Init()
{
  this->Create();
}

//////////////////////////////////////////////////
void OgreMarker::Create()
{
  this->dataPtr->markerType = LINE_STRIP;
  this->dataPtr->dynamicRenderable.reset(new OgreDynamicLines(LINE_STRIP));

  if (!this->dataPtr->mesh)
  {
    this->dataPtr->mesh =
      std::dynamic_pointer_cast<OgreMesh>(this->scene->CreateBox());
  }
}

//////////////////////////////////////////////////
void OgreMarker::SetMaterial(MaterialPtr _material, bool _unique)
{
  _material = (_unique) ? _material->Clone() : _material;

  OgreMaterialPtr derived =
    std::dynamic_pointer_cast<OgreMaterial>(_material);

  if (!derived)
  {
    ignerr << "Cannot assign material created by another render-engine"
      << std::endl;

    return;
  }

  std::string materialName = derived->Name();
  Ogre::MaterialPtr ogreMaterial = derived->Material();
  this->dataPtr->material = derived;

  this->dataPtr->material->SetReceiveShadows(false);
  this->dataPtr->material->SetLightingEnabled(false);
  
  switch (markerType)
  {
    case NONE:
    case BOX:
    case CYLINDER:
    case SPHERE:
      this->dataPtr->mesh->SetMaterial(derived, _unique);
      break;
    case LINE_STRIP:
    case LINE_LIST:
    case POINTS:
    case TRIANGLE_FAN:
    case TRIANGLE_LIST:
    case TRIANGLE_STRIP:
      this->dataPtr->dynamicRenderable->setMaterial(materialName);
      break;
    default:
      ignerr << "Invalid Marker type " << markerType << "\n";
      break;
  }
}

//////////////////////////////////////////////////
MaterialPtr OgreMarker::Material() const
{
  return this->dataPtr->material;
}

void OgreMarker::SetRenderOperation(const MarkerType _markerType)
{
  markerType = _markerType;
  switch (_markerType)
  {
    case NONE:
      this->dataPtr->mesh =
        std::dynamic_pointer_cast<OgreMesh>(this->scene->CreateBox());
      break;
    case BOX:
      this->dataPtr->mesh =
        std::dynamic_pointer_cast<OgreMesh>(this->scene->CreateBox());
      break;
    case CYLINDER:
      this->dataPtr->mesh =
        std::dynamic_pointer_cast<OgreMesh>(this->scene->CreateCylinder());
      break;
    case SPHERE:
      this->dataPtr->mesh =
        std::dynamic_pointer_cast<OgreMesh>(this->scene->CreateSphere());
      break;
    case LINE_STRIP:
    case LINE_LIST:
    case POINTS:
    case TRIANGLE_FAN:
    case TRIANGLE_LIST:
    case TRIANGLE_STRIP:
      this->dataPtr->dynamicRenderable->SetOperationType(_markerType);
      break;
    default:
      ignerr << "Invalid Marker type\n";
      break;
  }
}

MarkerType OgreMarker::RenderOperation() const
{
  return this->dataPtr->dynamicRenderable->OperationType();
}

void OgreMarker::SetPoint(const unsigned int _index,
    const ignition::math::Vector3d &_value)
{
  this->dataPtr->dynamicRenderable->SetPoint(_index, _value);
}

void OgreMarker::AddPoint(const double _x, const double _y, const double _z,
    const ignition::math::Color &_color)
{
  this->dataPtr->dynamicRenderable->AddPoint(_x, _y, _z, _color);
}

void OgreMarker::AddPoint(const ignition::math::Vector3d &_pt,
    const ignition::math::Color &_color)
{
  this->dataPtr->dynamicRenderable->AddPoint(_pt, _color);
}

void OgreMarker::ClearPoints()
{
  this->dataPtr->dynamicRenderable->Clear();
}

void OgreMarker::SetType(MarkerType _markerType)
{
  this->dataPtr->markerType = _markerType;
  SetRenderOperation(_markerType);
}

MarkerType OgreMarker::Type() const
{
  return this->dataPtr->markerType;
}
