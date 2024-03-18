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

#include <gz/common/Console.hh>

#include <gz/utils/NeverDestroyed.hh>

#include "gz/rendering/RenderPassSystem.hh"

using namespace gz;
using namespace rendering;

/// \brief Private implementation of the RenderPassSystem class
class gz::rendering::BaseRenderPassSystem::Implementation
{
  public: std::map<std::string, RenderPassFactoryFn> factoryFns;
};

//////////////////////////////////////////////////
BaseRenderPassSystem::BaseRenderPassSystem()
  : dataPtr(gz::utils::MakeUniqueImpl<Implementation>())
{
}

//////////////////////////////////////////////////
BaseRenderPassSystem::~BaseRenderPassSystem() = default;

//////////////////////////////////////////////////
RenderPassPtr BaseRenderPassSystem::Create(const std::string &_type)
{
  RenderPassPtr pass;
  auto itFn = this->dataPtr->factoryFns.find(_type);
  if (itFn != this->dataPtr->factoryFns.end())
  {
    pass.reset(itFn->second());
  }
  else
  {
    gzwarn << "RenderPass of typeid '" << _type << "' is not registered"
           << std::endl;
  }
  return pass;
}

//////////////////////////////////////////////////
void BaseRenderPassSystem::Register(const std::string &_name,
    RenderPassFactoryFn _factoryFn)
{
  this->dataPtr->factoryFns[_name] = _factoryFn;
}

//////////////////////////////////////////////////
BaseRenderPassSystem& RenderPassSystem::Implementation()
{
  static gz::utils::NeverDestroyed<BaseRenderPassSystem> instance;
  return instance.Access();
}

//////////////////////////////////////////////////
void RenderPassSystem::Register(const std::string &_type, RenderPassFactory *_factory)
{
  Implementation().Register(_type, [_factory](){return _factory->New();});
}
