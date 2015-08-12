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
#include "ignition/rendering/optix/OptixRenderEngine.hh"

#include "ignition/rendering/optix/OptixIncludes.hh"
#include "ignition/rendering/optix/OptixScene.hh"
#include "ignition/rendering/optix/OptixStorage.hh"

using namespace ignition;
using namespace rendering;

//////////////////////////////////////////////////
const std::string OptixRenderEngine::PTX_PREFIX("cuda_compile_ptx_generated_");

const std::string OptixRenderEngine::PTX_SUFFIX(".cu.ptx");

//////////////////////////////////////////////////
OptixRenderEngine::OptixRenderEngine()
{
}

//////////////////////////////////////////////////
OptixRenderEngine::~OptixRenderEngine()
{
  this->Fini();
}

//////////////////////////////////////////////////
bool OptixRenderEngine::Fini()
{
  this->scenes->RemoveAll();
  return true;
}

//////////////////////////////////////////////////
std::string OptixRenderEngine::GetPtxFile(const std::string& _fileBase) const
{
  return PTX_PREFIX + _fileBase + PTX_SUFFIX;
}

//////////////////////////////////////////////////
ScenePtr OptixRenderEngine::CreateSceneImpl(unsigned int _id,
    const std::string &_name)
{
  return OptixScenePtr(new OptixScene(_id, _name));
}

//////////////////////////////////////////////////
SceneStorePtr OptixRenderEngine::GetScenes() const
{
  return this->scenes;
}

//////////////////////////////////////////////////
bool OptixRenderEngine::LoadImpl()
{
  return true;
}

//////////////////////////////////////////////////
bool OptixRenderEngine::InitImpl()
{
  this->scenes = OptixSceneStorePtr(new OptixSceneStore);
  return true;
}
