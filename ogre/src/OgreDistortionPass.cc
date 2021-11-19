/*
 * Copyright (C) 2021 Open Source Robotics Foundation
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


#include <ignition/common/Console.hh>

#include "ignition/rendering/RenderPassSystem.hh"
#include "ignition/rendering/ogre/OgreIncludes.hh"
#include "ignition/rendering/ogre/OgreDistortionPass.hh"

using namespace ignition;
using namespace rendering;

//////////////////////////////////////////////////
OgreDistortionPass::OgreDistortionPass()
{
}

//////////////////////////////////////////////////
OgreDistortionPass::~OgreDistortionPass()
{
}

//////////////////////////////////////////////////
void OgreDistortionPass::PreRender()
{
}

//////////////////////////////////////////////////
void OgreDistortionPass::CreateRenderPass()
{
  if (!this->ogreCamera)
  {
    ignerr << "No camera set for applying Distortion Pass" << std::endl;
    return;
  }

  if (this->distortionInstance)
  {
    ignerr << "Distortion pass already created. " << std::endl;
    return;
  }

  // create compositor instance
  this->distortionInstance =
      Ogre::CompositorManager::getSingleton().addCompositor(
      this->ogreCamera->getViewport(), "RenderPass/Distortion");
  this->distortionInstance->setEnabled(this->enabled);
}

//////////////////////////////////////////////////
void OgreDistortionPass::Destroy()
{
}

IGN_RENDERING_REGISTER_RENDER_PASS(OgreDistortionPass, DistortionPass)
