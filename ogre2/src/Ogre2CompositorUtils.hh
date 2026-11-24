/*
 * Copyright (C) 2026 Open Source Robotics Foundation
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

#ifndef GZ_RENDERING_OGRE2_OGRE2COMPOSITORUTILS_HH_
#define GZ_RENDERING_OGRE2_OGRE2COMPOSITORUTILS_HH_

#include <string>

#include "gz/rendering/config.hh"
#include "gz/rendering/ogre2/Export.hh"

namespace Ogre
{
  class CompositorManager2;
}

namespace gz
{
namespace rendering
{
inline namespace GZ_RENDERING_VERSION_NAMESPACE {

/// \brief Remove a workspace definition and the node definitions it uses.
///
/// CompositorManager2::createBasicWorkspaceDef also creates a node
/// definition, named from a hash of the workspace name. Removing only the
/// workspace definition leaves that node behind, and creating the same
/// workspace again (e.g. a sensor recreated with the same name) throws
/// because the node already exists.
///
/// Use it only for workspaces whose nodes no other workspace uses.
/// \param[in] _compositorManager Ogre compositor manager
/// \param[in] _workspaceDefName Name of the workspace definition. Nothing
/// is done if it doesn't exist.
GZ_RENDERING_OGRE2_HIDDEN
void RemoveWorkspaceDefinitionAndNodes(
    Ogre::CompositorManager2 *_compositorManager,
    const std::string &_workspaceDefName);
}
}  // namespace rendering
}  // namespace gz

#endif  // GZ_RENDERING_OGRE2_OGRE2COMPOSITORUTILS_HH_
