/*
 * Copyright (C) 2015 Open Source Robotics Foundation
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

#include "ignition/rendering/ogre/OgreVisual.hh"
#include "ignition/rendering/ogre/OgreConversions.hh"
#include "ignition/rendering/ogre/OgreStorage.hh"

using namespace ignition;
using namespace rendering;

//////////////////////////////////////////////////
OgreVisual::OgreVisual()
{
}

//////////////////////////////////////////////////
OgreVisual::~OgreVisual()
{
}

//////////////////////////////////////////////////
void OgreVisual::SetVisible(bool _visible)
{
  this->ogreNode->setVisible(_visible);
}

//////////////////////////////////////////////////
GeometryStorePtr OgreVisual::Geometries() const
{
  return this->geometries;
}

//////////////////////////////////////////////////
bool OgreVisual::AttachGeometry(GeometryPtr _geometry)
{
  OgreGeometryPtr derived =
      std::dynamic_pointer_cast<OgreGeometry>(_geometry);

  if (!derived)
  {
    ignerr << "Cannot attach geometry created by another render-engine"
          << std::endl;

    return false;
  }

  // set user data for mouse queries
  derived->OgreObject()->getUserObjectBindings().setUserAny(
      Ogre::Any(this->Id()));

  derived->SetParent(this->SharedThis());
  this->ogreNode->attachObject(derived->OgreObject());
  return true;
}

//////////////////////////////////////////////////
bool OgreVisual::DetachGeometry(GeometryPtr _geometry)
{
  if (!this->ogreNode)
    return true;

  OgreGeometryPtr derived =
      std::dynamic_pointer_cast<OgreGeometry>(_geometry);

  if (!derived)
  {
    ignerr << "Cannot detach geometry created by another render-engine"
          << std::endl;

    return false;
  }

  this->ogreNode->detachObject(derived->OgreObject());
  return true;
}

//////////////////////////////////////////////////
void OgreVisual::Init()
{
  BaseVisual::Init();
  this->geometries = OgreGeometryStorePtr(new OgreGeometryStore);
}

//////////////////////////////////////////////////
OgreVisualPtr OgreVisual::SharedThis()
{
  ObjectPtr object = shared_from_this();
  return std::dynamic_pointer_cast<OgreVisual>(object);
}
