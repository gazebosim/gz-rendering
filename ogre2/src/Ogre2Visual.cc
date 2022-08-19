/*
 * Copyright (C) 2018 Open Source Robotics Foundation
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

#include "gz/rendering/ogre2/Ogre2Conversions.hh"
#include "gz/rendering/ogre2/Ogre2Geometry.hh"
#include "gz/rendering/ogre2/Ogre2RenderTypes.hh"
#include "gz/rendering/ogre2/Ogre2Storage.hh"
#include "gz/rendering/ogre2/Ogre2Visual.hh"

using namespace gz;
using namespace rendering;

//////////////////////////////////////////////////
Ogre2Visual::Ogre2Visual()
{
}

//////////////////////////////////////////////////
Ogre2Visual::~Ogre2Visual()
{
}

//////////////////////////////////////////////////
void Ogre2Visual::SetVisible(bool _visible)
{
  this->ogreNode->setVisible(_visible);
}

//////////////////////////////////////////////////
GeometryStorePtr Ogre2Visual::Geometries() const
{
  return this->geometries;
}

//////////////////////////////////////////////////
bool Ogre2Visual::AttachGeometry(GeometryPtr _geometry)
{
  if (!_geometry)
  {
    ignerr << "Cannot attach null geometry." << std::endl;

    return false;
  }

  Ogre2GeometryPtr derived =
      std::dynamic_pointer_cast<Ogre2Geometry>(_geometry);

  if (!derived)
  {
    ignerr << "Cannot attach geometry created by another render-engine"
          << std::endl;

    return false;
  }

  Ogre::MovableObject *ogreObj = derived->OgreObject();
  if (!ogreObj)
  {
    ignerr << "Cannot attach a null geometry object" << std::endl;
    return false;
  }

  // set user data for mouse queries
  ogreObj->getUserObjectBindings().setUserAny(
      Ogre::Any(this->Id()));
  ogreObj->setName(this->Name() + "_" + _geometry->Name());
  ogreObj->setVisibilityFlags(IGN_VISIBILITY_ALL);

  derived->SetParent(this->SharedThis());
  this->ogreNode->attachObject(ogreObj);

  return true;
}

//////////////////////////////////////////////////
bool Ogre2Visual::DetachGeometry(GeometryPtr _geometry)
{
  Ogre2GeometryPtr derived =
      std::dynamic_pointer_cast<Ogre2Geometry>(_geometry);

  if (!derived)
  {
    ignerr << "Cannot detach geometry created by another render-engine"
          << std::endl;

    return false;
  }

  this->ogreNode->detachObject(derived->OgreObject());
  derived->SetParent(nullptr);
  return true;
}

//////////////////////////////////////////////////
void Ogre2Visual::Init()
{
  BaseVisual::Init();
  this->geometries = Ogre2GeometryStorePtr(new Ogre2GeometryStore);
}

//////////////////////////////////////////////////
Ogre2VisualPtr Ogre2Visual::SharedThis()
{
  ObjectPtr object = shared_from_this();
  return std::dynamic_pointer_cast<Ogre2Visual>(object);
}
