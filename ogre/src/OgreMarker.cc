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
#include "ignition/rendering/ogre/OgreScene.hh"

class ignition::rendering::OgreMarkerPrivate
{
  /// \brief Grid materal
  public: OgreMaterialPtr material;

  public: std::shared_ptr<OgreDynamicLines> dynamicRenderable;

  public: Type type;

  public: Visibility visibility;

  public: Action action;
};

using namespace ignition;
using namespace rendering;

//////////////////////////////////////////////////
OgreMarker::OgreMarker()
    : dataPtr(new OgreMarkerPrivate)
{
}
// TODO(jshep1): add Destroy implementation
//////////////////////////////////////////////////
OgreMarker::~OgreMarker()
{
}

//////////////////////////////////////////////////
void OgreMarker::PreRender()
{
  this->dataPtr->dynamicRenderable->Update();
}

// TODO(jshep1): Add actual implementation for ogreobject
//////////////////////////////////////////////////
Ogre::MovableObject *OgreMarker::OgreObject() const
{
  Ogre::ManualObject *manualObject = nullptr;
  return manualObject;
}

//////////////////////////////////////////////////
void OgreMarker::Init()
{
  this->Create();
}

//////////////////////////////////////////////////
void OgreMarker::Create()
{
  // TODO(jshep1): init dynamicRenderable to default rendertype
  this->dataPtr->type = LINE_STRIP;
  this->dataPtr->visibility = GUI;
  this->dataPtr->action = ADD_MODIFY;
  this->dataPtr->dynamicRenderable.reset(new OgreDynamicLines(LINE_STRIP));
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

  this->SetMaterialImpl(derived);
}

//////////////////////////////////////////////////
void OgreMarker::SetMaterialImpl(OgreMaterialPtr _material)
{
  std::string materialName = _material->Name();
  Ogre::MaterialPtr ogreMaterial = _material->Material();
  this->dataPtr->material = _material;

  this->dataPtr->material->SetReceiveShadows(false);
  this->dataPtr->material->SetLightingEnabled(false);
}

//////////////////////////////////////////////////
MaterialPtr OgreMarker::Material() const
{
  return this->dataPtr->material;
}

void OgreMarker::SetRenderOperation(const Type _type)
{
  this->dataPtr->dynamicRenderable->SetOperationType(_type);
}

Type OgreMarker::RenderOperation() const
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

void OgreMarker::SetType(Type _type)
{
  this->dataPtr->type = _type;
  SetRenderOperation(_type);
}

void OgreMarker::SetAction(Action _action)
{
  this->dataPtr->action = _action;
}

void OgreMarker::SetVisibility(Visibility _visibility)
{
  this->dataPtr->visibility = _visibility;
}

Type OgreMarker::getType() const
{
  return this->dataPtr->type;
}

Action OgreMarker::getAction() const
{
  return this->dataPtr->action;
}

Visibility OgreMarker::getVisibility() const
{
  return this->dataPtr->visibility;
}

