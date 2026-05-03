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
#include "gz/rendering/ogre_next/OgreNextRenderPass.hh"

/// \brief Private data for the OgreNextRenderPass class
class gz::rendering::OgreNextRenderPassPrivate
{
};

using namespace gz;
using namespace rendering;

//////////////////////////////////////////////////
OgreNextRenderPass::OgreNextRenderPass()
  : dataPtr(std::make_unique<OgreNextRenderPassPrivate>())
{
}

//////////////////////////////////////////////////
OgreNextRenderPass::~OgreNextRenderPass()
{
}

//////////////////////////////////////////////////
void OgreNextRenderPass::Destroy()
{
}

//////////////////////////////////////////////////
void OgreNextRenderPass::WorkspaceAdded(Ogre::CompositorWorkspace *)
{
  // To be overridden by derived render pass classes
}

//////////////////////////////////////////////////
void OgreNextRenderPass::WorkspaceRemoved(Ogre::CompositorWorkspace *)
{
  // To be overridden by derived render pass classes
}

//////////////////////////////////////////////////
void OgreNextRenderPass::CreateRenderPass()
{
  // To be overridden by derived render pass classes
}

//////////////////////////////////////////////////
std::string OgreNextRenderPass::OgreCompositorNodeDefinitionName() const
{
  return this->ogreCompositorNodeDefName;
}
