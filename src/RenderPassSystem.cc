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

#include "ignition/rendering/RenderPassSystem.hh"

using namespace ignition;
using namespace rendering;

/// \brief Private implementation of the RenderPassSystem class
class ignition::rendering::RenderPassSystemPrivate
{
    /// \brief A map of render pass type id name to its factory class
    public: std::map<std::string, RenderPassFactory *> renderPassMap;
};

//////////////////////////////////////////////////
// RenderPassSystem
//////////////////////////////////////////////////
RenderPassSystem::RenderPassSystem() :
  dataPtr(new RenderPassSystemPrivate)
{
}

//////////////////////////////////////////////////
RenderPassSystem::~RenderPassSystem()
{
}

//////////////////////////////////////////////////
RenderPassPtr RenderPassSystem::CreateImpl(const std::string &_type)
{
  RenderPassPtr pass;
  auto it = this->dataPtr->renderPassMap.find(_type);
  if (it != this->dataPtr->renderPassMap.end())
  {
    pass.reset(it->second->New());
  }
  else
  {
    ignerr << "RenderPass of typeid '" << _type << "' is not registered"
           << std::endl;
  }
  return pass;
}

//////////////////////////////////////////////////
void RenderPassSystem::Register(const std::string &_name,
    RenderPassFactory *_factory)
{
  ignerr << "Renderpass register: " << _name << std::endl;
  this->dataPtr->renderPassMap[_name] = _factory;
}
