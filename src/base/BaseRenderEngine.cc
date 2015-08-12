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
#include "ignition/rendering/ogre/OgreRenderEngine.hh"
#include "gazebo/common/Console.hh"

using namespace ignition;
using namespace rendering;

//////////////////////////////////////////////////
BaseRenderEngine::BaseRenderEngine()
{
}

//////////////////////////////////////////////////
BaseRenderEngine::~BaseRenderEngine()
{
}

//////////////////////////////////////////////////
bool BaseRenderEngine::Load()
{
  if (this->loaded)
  {
    gzwarn << "Render-engine has already been loaded" << std::endl;
    return true;
  }

  this->loaded = this->LoadImpl();
  return this->loaded;
}

//////////////////////////////////////////////////
bool BaseRenderEngine::Init()
{
  if (!this->loaded)
  {
    gzerr << "Render-engine must be loaded first" << std::endl;
    return false;
  }

  if (this->initialized)
  {
    gzwarn << "Render-engine has already been initialized" << std::endl;
    return true;
  }

  this->initialized = this->InitImpl();
  return this->initialized;
}

//////////////////////////////////////////////////
bool BaseRenderEngine::Fini()
{
  return true;
}

//////////////////////////////////////////////////
bool BaseRenderEngine::IsLoaded() const
{
  return this->loaded;
}

//////////////////////////////////////////////////
bool BaseRenderEngine::IsInitialized() const
{
  return this->initialized;
}

//////////////////////////////////////////////////
bool BaseRenderEngine::IsEnabled() const
{
  return this->initialized;
}

//////////////////////////////////////////////////
unsigned int BaseRenderEngine::GetSceneCount() const
{
  return this->GetScenes()->Size();
}

//////////////////////////////////////////////////
bool BaseRenderEngine::HasScene(ConstScenePtr _scene) const
{
  return this->GetScenes()->Contains(_scene);
}

//////////////////////////////////////////////////
bool BaseRenderEngine::HasSceneId(unsigned int _id) const
{
  return this->GetScenes()->ContainsId(_id);
}

//////////////////////////////////////////////////
bool BaseRenderEngine::HasSceneName(const std::string &_name) const
{
  return this->GetScenes()->ContainsName(_name);
}

//////////////////////////////////////////////////
ScenePtr BaseRenderEngine::GetSceneById(unsigned int _id) const
{
  return this->GetScenes()->GetById(_id);
}

//////////////////////////////////////////////////
ScenePtr BaseRenderEngine::GetSceneByName(const std::string &_name) const
{
  return this->GetScenes()->GetByName(_name);
}

//////////////////////////////////////////////////
ScenePtr BaseRenderEngine::GetSceneByIndex(unsigned int _index) const
{
  return this->GetScenes()->GetByIndex(_index);
}

//////////////////////////////////////////////////
void BaseRenderEngine::DestroyScene(ScenePtr _scene)
{
  this->GetScenes()->Destroy(_scene);
}

//////////////////////////////////////////////////
void BaseRenderEngine::DestroySceneById(unsigned int _id)
{
  this->GetScenes()->DestroyById(_id);
}

//////////////////////////////////////////////////
void BaseRenderEngine::DestroySceneByName(const std::string &_name)
{
  this->GetScenes()->DestroyByName(_name);
}

//////////////////////////////////////////////////
void BaseRenderEngine::DestroySceneByIndex(unsigned int _index)
{
  this->GetScenes()->DestroyByIndex(_index);
}

//////////////////////////////////////////////////
void BaseRenderEngine::DestroyScenes()
{
  this->GetScenes()->DestroyAll();
}

//////////////////////////////////////////////////
ScenePtr BaseRenderEngine::CreateScene(const std::string &_name)
{
  unsigned int sceneId = this->GetNextSceneId();
  return this->CreateScene(sceneId, _name);
}

//////////////////////////////////////////////////
ScenePtr BaseRenderEngine::CreateScene(unsigned int _id,
    const std::string &_name)
{
  if (!this->IsInitialized())
  {
    gzerr << "Render-engine has not been initialized" << std::endl;
    return NULL;
  }

  if (this->HasSceneId(_id))
  {
    gzerr << "Scene already exists with id: " << _id << std::endl;
    return NULL;
  }

  if (this->HasSceneName(_name))
  {
    gzerr << "Scene already exists with id: " << _id << std::endl;
    return NULL;
  }

  ScenePtr scene = this->CreateSceneImpl(_id, _name);
  this->PrepareScene(scene);
  return scene;
}

//////////////////////////////////////////////////
void BaseRenderEngine::Destroy()
{
}

//////////////////////////////////////////////////
void BaseRenderEngine::PrepareScene(ScenePtr _scene)
{
  if (_scene)
  {
    _scene->Load();
    _scene->Init();
  }
}

//////////////////////////////////////////////////
unsigned int BaseRenderEngine::GetNextSceneId()
{
  return this->nextSceneId--;
}
