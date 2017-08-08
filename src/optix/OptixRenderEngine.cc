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

#include <vector>
#include "ignition/rendering/optix/OptixIncludes.hh"
#include "ignition/rendering/optix/OptixScene.hh"
#include "ignition/rendering/optix/OptixStorage.hh"
#include "ignition/rendering/optix/OptixRenderEngine.hh"

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
}

//////////////////////////////////////////////////
bool OptixRenderEngine::Fini()
{
  return true;
}

//////////////////////////////////////////////////
std::string OptixRenderEngine::Name() const
{
  return "optix";
}

//////////////////////////////////////////////////
std::string OptixRenderEngine::PtxFile(const std::string& _fileBase) const
{
  // TODO: actual implement system path system

  std::vector<std::string> folders;
  folders.push_back("./src/optix/");

  std::string home(std::getenv("HOME"));
  folders.push_back(home + "/local/share/ignition/rendering/ptx/");

  const char *cstr = std::getenv("IGN_RENDERING_INSTALL_DIR");

  if (cstr)
    folders.push_back(std::string(cstr) + "/share/ignition/rendering/ptx/");

  std::string file = PTX_PREFIX + _fileBase + PTX_SUFFIX;

  for (auto folder : folders)
  {
    std::string uri = folder + file;

    if (common::exists(uri))
    {
      return uri;
    }
  }

  return file;
}

//////////////////////////////////////////////////
ScenePtr OptixRenderEngine::CreateSceneImpl(unsigned int _id,
    const std::string &_name)
{
  return OptixScenePtr(new OptixScene(_id, _name));
}

//////////////////////////////////////////////////
SceneStorePtr OptixRenderEngine::Scenes() const
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
