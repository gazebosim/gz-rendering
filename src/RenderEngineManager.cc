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
#include "ignition/rendering/RenderEngineManager.hh"
#include "ignition/rendering/RenderEngineManagerPrivate.hh"

#include "gazebo/common/Console.hh"
#include "ignition/rendering/ogre/OgreRenderEngine.hh"
#include "ignition/rendering/optix/OptixRenderEngine.hh"

using namespace ignition;
using namespace rendering;

//////////////////////////////////////////////////
// RenderEngineManager
//////////////////////////////////////////////////
RenderEngineManager::RenderEngineManager() :
  pimpl(new RenderEngineManagerPrivate)
{
  this->pimpl->RegisterDefaultEngines();
}

//////////////////////////////////////////////////
RenderEngineManager::~RenderEngineManager()
{
  delete this->pimpl;
  this->pimpl = NULL;
}

//////////////////////////////////////////////////
unsigned int RenderEngineManager::GetEngineCount() const
{
  return this->pimpl->engines.size();
}

//////////////////////////////////////////////////
bool RenderEngineManager::HasEngine(const std::string &_name) const
{
  auto iter = this->pimpl->engines.find(_name);
  return iter != this->pimpl->engines.end();
}

//////////////////////////////////////////////////
RenderEngine *RenderEngineManager::GetEngine(const std::string &_name) const
{
  auto iter = this->pimpl->engines.find(_name);

  if (iter == this->pimpl->engines.end())
  {
    gzerr << "No render-engine registered with name: " << _name << std::endl;
    return NULL;
  }

  return this->pimpl->GetEngine(iter);
}

//////////////////////////////////////////////////
RenderEngine *RenderEngineManager::GetEngineAt(unsigned int _index) const
{
  if (_index >= this->GetEngineCount())
  {
    gzerr << "Invalid render-engine index: " << _index << std::endl;
    return NULL;
  }

  auto iter = this->pimpl->engines.begin();
  std::advance(iter, _index);
  return this->pimpl->GetEngine(iter);
}

//////////////////////////////////////////////////
void RenderEngineManager::RegisterEngine(const std::string &_name,
    RenderEngine *_engine)
{
  if (!_engine)
  {
    gzerr << "Render-engine cannot be null" << std::endl;
    return;
  }

  if (this->HasEngine(_name))
  {
    gzerr << "Render-engine already registered with name: "
          << _name << std::endl;

    return;
  }

  this->pimpl->engines[_name] = _engine;
}

//////////////////////////////////////////////////
void RenderEngineManager::UnregisterEngine(const std::string &_name)
{
  auto iter = this->pimpl->engines.find(_name);

  if (iter != this->pimpl->engines.end())
  {
    this->pimpl->UnregisterEngine(iter);
  }
}

//////////////////////////////////////////////////
void RenderEngineManager::UnregisterEngine(RenderEngine *_engine)
{
  auto begin = this->pimpl->engines.begin();
  auto end = this->pimpl->engines.end();

  for (auto iter = begin; iter != end; ++iter)
  {
    if (iter->second == _engine)
    {
      this->pimpl->UnregisterEngine(iter);
      return;
    }
  }
}

//////////////////////////////////////////////////
void RenderEngineManager::UnregisterEngineAt(unsigned int _index)
{
  if (_index >= this->GetEngineCount())
  {
    gzerr << "Invalid render-engine index: " << _index << std::endl;
    return;
  }

  auto iter = this->pimpl->engines.begin();
  std::advance(iter, _index);
  this->pimpl->UnregisterEngine(iter);
}

//////////////////////////////////////////////////
// RenderEngineManagerPrivate
//////////////////////////////////////////////////
RenderEngine *RenderEngineManagerPrivate::GetEngine(EngineIter _iter) const
{
  RenderEngine *engine = _iter->second;
  
  if (!engine->IsInitialized())
  {
    engine->Load();
    engine->Init();
  }

  return engine;
}

//////////////////////////////////////////////////
void RenderEngineManagerPrivate::RegisterDefaultEngines()
{
  this->engines["ogre"] = OgreRenderEngine::Instance();
  this->engines["optix"] = OptixRenderEngine::Instance();
}

//////////////////////////////////////////////////
void RenderEngineManagerPrivate::UnregisterEngine(EngineIter _iter)
{
  _iter->second->Destroy();
  this->engines.erase(_iter);
}
