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
// #include "ignition/rendering/SceneManager.hh"
#include "ignition/rendering/ogre/OgreRenderEngine.hh"

using namespace ignition;

/*//////////////////////////////////////////////////
bool rendering::load()
{
  SceneManager::Instance()->Load();
  return true;
}

//////////////////////////////////////////////////
bool rendering::init()
{
  SceneManager::Instance()->Init();
  return true;
}

//////////////////////////////////////////////////
bool rendering::fini()
{
  SceneManager::Instance()->Fini();
  return true;
}*/

//////////////////////////////////////////////////
unsigned int rendering::get_engine_count()
{
  return RenderEngineManager::Instance()->EngineCount();
}

//////////////////////////////////////////////////
bool rendering::has_engine(const std::string &_name)
{
  return RenderEngineManager::Instance()->HasEngine(_name);
}

//////////////////////////////////////////////////
rendering::RenderEngine *rendering::get_engine(const std::string &_name)
{
  return RenderEngineManager::Instance()->Engine(_name);
}

//////////////////////////////////////////////////
rendering::RenderEngine *rendering::get_engine(unsigned int _index)
{
  return RenderEngineManager::Instance()->EngineAt(_index);
}

//////////////////////////////////////////////////
void rendering::register_engine(const std::string &_name,
    rendering::RenderEngine *_engine)
{
  RenderEngineManager::Instance()->RegisterEngine(_name, _engine);
}

//////////////////////////////////////////////////
void rendering::unregister_engine(const std::string &_name)
{
  RenderEngineManager::Instance()->UnregisterEngine(_name);
}

//////////////////////////////////////////////////
void rendering::unregister_engine(rendering::RenderEngine *_engine)
{
  RenderEngineManager::Instance()->UnregisterEngine(_engine);
}

//////////////////////////////////////////////////
void rendering::unregister_engine(unsigned int _index)
{
  RenderEngineManager::Instance()->UnregisterEngineAt(_index);
}
