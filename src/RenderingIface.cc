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

// using namespace ignition;

/*//////////////////////////////////////////////////
bool ignition::rendering::load()
{
  SceneManager::Instance()->Load();
  return true;
}

//////////////////////////////////////////////////
bool ignition::rendering::init()
{
  SceneManager::Instance()->Init();
  return true;
}

//////////////////////////////////////////////////
bool ignition::rendering::fini()
{
  SceneManager::Instance()->Fini();
  return true;
}*/

//////////////////////////////////////////////////
unsigned int ignition::rendering::engineCount()
{
  return RenderEngineManager::Instance()->EngineCount();
}

//////////////////////////////////////////////////
bool ignition::rendering::hasEngine(const std::string &_name)
{
  return RenderEngineManager::Instance()->HasEngine(_name);
}

//////////////////////////////////////////////////
ignition::rendering::RenderEngine *ignition::rendering::engine(const std::string &_name)
{
  return RenderEngineManager::Instance()->Engine(_name);
}

//////////////////////////////////////////////////
ignition::rendering::RenderEngine *ignition::rendering::engine(const unsigned int _index)
{
  return RenderEngineManager::Instance()->EngineAt(_index);
}

//////////////////////////////////////////////////
void ignition::rendering::registerEngine(const std::string &_name,
    ignition::rendering::RenderEngine *_engine)
{
  RenderEngineManager::Instance()->RegisterEngine(_name, _engine);
}

//////////////////////////////////////////////////
void ignition::rendering::unregisterEngine(const std::string &_name)
{
  RenderEngineManager::Instance()->UnregisterEngine(_name);
}

//////////////////////////////////////////////////
void ignition::rendering::unregisterEngine(ignition::rendering::RenderEngine *_engine)
{
  RenderEngineManager::Instance()->UnregisterEngine(_engine);
}

//////////////////////////////////////////////////
void ignition::rendering::unregisterEngine(const unsigned int _index)
{
  RenderEngineManager::Instance()->UnregisterEngineAt(_index);
}
