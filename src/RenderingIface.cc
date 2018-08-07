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
#include "ignition/rendering/RenderingIface.hh"
#include "ignition/rendering/RenderEngineManager.hh"

namespace ignition
{
  namespace rendering
  {
    inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {
    //
    /*//////////////////////////////////////////////////
    bool load()
    {
      SceneManager::Instance()->Load();
      return true;
    }

    //////////////////////////////////////////////////
    bool init()
    {
      SceneManager::Instance()->Init();
      return true;
    }

    //////////////////////////////////////////////////
    bool fini()
    {
      SceneManager::Instance()->Fini();
      return true;
    }*/

    //////////////////////////////////////////////////
    unsigned int engineCount()
    {
      return RenderEngineManager::Instance()->EngineCount();
    }

    //////////////////////////////////////////////////
    bool hasEngine(const std::string &_name)
    {
      return RenderEngineManager::Instance()->HasEngine(_name);
    }

    //////////////////////////////////////////////////
    RenderEngine *engine(const std::string &_name)
    {
      return RenderEngineManager::Instance()->Engine(_name);
    }

    //////////////////////////////////////////////////
    RenderEngine *engine(const unsigned int _index)
    {
      return RenderEngineManager::Instance()->EngineAt(_index);
    }

    //////////////////////////////////////////////////
    void registerEngine(const std::string &_name,
        RenderEngine *_engine)
    {
      RenderEngineManager::Instance()->RegisterEngine(_name, _engine);
    }

    //////////////////////////////////////////////////
    void unregisterEngine(const std::string &_name)
    {
      RenderEngineManager::Instance()->UnregisterEngine(_name);
    }

    //////////////////////////////////////////////////
    void unregisterEngine(RenderEngine *_engine)
    {
      RenderEngineManager::Instance()->UnregisterEngine(_engine);
    }

    //////////////////////////////////////////////////
    void unregisterEngine(const unsigned int _index)
    {
      RenderEngineManager::Instance()->UnregisterEngineAt(_index);
    }
    }
  }
}
