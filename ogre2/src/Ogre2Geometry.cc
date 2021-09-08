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
