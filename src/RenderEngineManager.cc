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

#include <map>

#include <ignition/common/Console.hh>

#include "ignition/rendering/config.hh"
#include "ignition/rendering/RenderEngine.hh"
#include "ignition/rendering/RenderEngineManager.hh"

#if HAVE_OGRE
#include "ignition/rendering/ogre/OgreRenderEngine.hh"
#endif

#if HAVE_OPTIX
#include "ignition/rendering/optix/OptixRenderEngine.hh"
#endif

namespace ignition
{
  namespace rendering
  {
    class RenderEngine;

    class RenderEngineManagerPrivate
    {
      typedef std::map<std::string, RenderEngine *> EngineMap;

      typedef EngineMap::iterator EngineIter;

      public: RenderEngine *Engine(EngineIter _iter) const;

      public: void RegisterDefaultEngines();

      public: void UnregisterEngine(EngineIter _iter);

      public: EngineMap engines;
    };
  }
}

using namespace ignition;
using namespace rendering;

//////////////////////////////////////////////////
// RenderEngineManager
//////////////////////////////////////////////////
RenderEngineManager::RenderEngineManager() :
  dataPtr(new RenderEngineManagerPrivate)
{
  this->dataPtr->RegisterDefaultEngines();
}

//////////////////////////////////////////////////
RenderEngineManager::~RenderEngineManager()
{
}

//////////////////////////////////////////////////
unsigned int RenderEngineManager::EngineCount() const
{
  return this->dataPtr->engines.size();
}

//////////////////////////////////////////////////
bool RenderEngineManager::HasEngine(const std::string &_name) const
{
  auto iter = this->dataPtr->engines.find(_name);
  return iter != this->dataPtr->engines.end();
}

//////////////////////////////////////////////////
RenderEngine *RenderEngineManager::Engine(const std::string &_name) const
{
  auto iter = this->dataPtr->engines.find(_name);

  if (iter == this->dataPtr->engines.end())
  {
    ignerr << "No render-engine registered with name: " << _name << std::endl;
    return NULL;
  }

  return this->dataPtr->Engine(iter);
}

//////////////////////////////////////////////////
RenderEngine *RenderEngineManager::EngineAt(unsigned int _index) const
{
  if (_index >= this->EngineCount())
  {
    ignerr << "Invalid render-engine index: " << _index << std::endl;
    return NULL;
  }

  auto iter = this->dataPtr->engines.begin();
  std::advance(iter, _index);
  return this->dataPtr->Engine(iter);
}

//////////////////////////////////////////////////
void RenderEngineManager::RegisterEngine(const std::string &_name,
    RenderEngine *_engine)
{
  if (!_engine)
  {
    ignerr << "Render-engine cannot be null" << std::endl;
    return;
  }

  if (this->HasEngine(_name))
  {
    ignerr << "Render-engine already registered with name: "
          << _name << std::endl;

    return;
  }

  this->dataPtr->engines[_name] = _engine;
}

//////////////////////////////////////////////////
void RenderEngineManager::UnregisterEngine(const std::string &_name)
{
  auto iter = this->dataPtr->engines.find(_name);

  if (iter != this->dataPtr->engines.end())
  {
    this->dataPtr->UnregisterEngine(iter);
  }
}

//////////////////////////////////////////////////
void RenderEngineManager::UnregisterEngine(RenderEngine *_engine)
{
  auto begin = this->dataPtr->engines.begin();
  auto end = this->dataPtr->engines.end();

  for (auto iter = begin; iter != end; ++iter)
  {
    if (iter->second == _engine)
    {
      this->dataPtr->UnregisterEngine(iter);
      return;
    }
  }
}

//////////////////////////////////////////////////
void RenderEngineManager::UnregisterEngineAt(unsigned int _index)
{
  if (_index >= this->EngineCount())
  {
    ignerr << "Invalid render-engine index: " << _index << std::endl;
    return;
  }

  auto iter = this->dataPtr->engines.begin();
  std::advance(iter, _index);
  this->dataPtr->UnregisterEngine(iter);
}

//////////////////////////////////////////////////
// RenderEngineManagerPrivate
//////////////////////////////////////////////////
RenderEngine *RenderEngineManagerPrivate::Engine(EngineIter _iter) const
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
#if HAVE_OGRE
  this->engines["ogre"] = OgreRenderEngine::Instance();
#endif
#if HAVE_OPTIX
  this->engines["optix"] = OptixRenderEngine::Instance();
#endif
}

//////////////////////////////////////////////////
void RenderEngineManagerPrivate::UnregisterEngine(EngineIter _iter)
{
  _iter->second->Destroy();
  this->engines.erase(_iter);
}
