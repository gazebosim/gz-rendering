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
#include "ignition/rendering/base/BaseObject.hh"

#include "gazebo/common/Console.hh"

using namespace ignition;
using namespace rendering;

//////////////////////////////////////////////////
BaseObject::BaseObject()
{
}

//////////////////////////////////////////////////
BaseObject::~BaseObject()
{
  this->Destroy();
}

//////////////////////////////////////////////////
unsigned int BaseObject::GetId() const
{
  return this->id;
}

//////////////////////////////////////////////////
std::string BaseObject::GetName() const
{
  return this->name;
}

//////////////////////////////////////////////////
void BaseObject::PreRender()
{
  // do nothing
}

//////////////////////////////////////////////////
void BaseObject::Destroy()
{
  // do nothing
}

//////////////////////////////////////////////////
// TODO: make pure virtual
void BaseObject::Load()
{
}

//////////////////////////////////////////////////
// TODO: make pure virtual
void BaseObject::Init()
{
}
