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
#include "ignition/rendering/ogre2/Ogre2Geometry.hh"

#include "ignition/rendering/ogre2/Ogre2Scene.hh"
#include "ignition/rendering/ogre2/Ogre2Visual.hh"

using namespace ignition;
using namespace rendering;

//////////////////////////////////////////////////
Ogre2Geometry::Ogre2Geometry()
{
}

//////////////////////////////////////////////////
Ogre2Geometry::~Ogre2Geometry()
{
}

//////////////////////////////////////////////////
bool Ogre2Geometry::HasParent() const
{
  return this->parent != nullptr;
}

//////////////////////////////////////////////////
VisualPtr Ogre2Geometry::Parent() const
{
  return this->parent;
}

//////////////////////////////////////////////////
void Ogre2Geometry::SetParent(Ogre2VisualPtr _parent)
{
  this->parent = _parent;
}

//////////////////////////////////////////////////
GeometryPtr Ogre2Geometry::Clone(const std::string &_name,
    VisualPtr _newParent) const
{
  if (nullptr == _newParent)
  {
    ignerr << "Cloning a geometry failed because the parent of the clone is "
      << "null" << std::endl;
    return nullptr;
  }

  GeometryPtr result;

  if (this->descriptorName == "unit_box")
    result = this->Scene()->CreateBox();
  else if (this->descriptorName == "unit_cone")
    result = this->Scene()->CreateCone();
  else if (this->descriptorName == "unit_cylinder")
    result = this->Scene()->CreateCylinder();
  else if (this->descriptorName == "unit_plane")
    result = this->Scene()->CreatePlane();
  else if (this->descriptorName == "unit_sphere")
    result = this->Scene()->CreateSphere();
  else
  {
    ignerr << "Attempted to clone a geometry with a descriptor name of ["
      << this->descriptorName << "], which is invalid." << std::endl;
    return nullptr;
  }

  // TODO(adlarkin) need to also set the name of the cloned geometry, the
  // parent, and check if anything else needs to be set

  return result;
}
