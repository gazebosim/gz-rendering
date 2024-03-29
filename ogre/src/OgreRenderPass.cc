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
#include "gz/rendering/ogre/OgreRenderPass.hh"


using namespace gz;
using namespace rendering;

//////////////////////////////////////////////////
OgreRenderPass::OgreRenderPass()
{
}

//////////////////////////////////////////////////
OgreRenderPass::~OgreRenderPass()
{
}

//////////////////////////////////////////////////
void OgreRenderPass::SetCamera(Ogre::Camera *_camera)
{
  this->ogreCamera[0] = _camera;
}

//////////////////////////////////////////////////
void OgreRenderPass::SetCameras(
  Ogre::Camera *_cameras[kMaxOgreRenderPassCameras])
{
  for (size_t i = 0u; i < kMaxOgreRenderPassCameras; ++i)
    this->ogreCamera[i] = _cameras[i];
}

//////////////////////////////////////////////////
void OgreRenderPass::Destroy()
{
}

//////////////////////////////////////////////////
void OgreRenderPass::CreateRenderPass()
{
  // To be overriden by derived render pass classes
}
