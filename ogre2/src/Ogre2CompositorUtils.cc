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

#include "Ogre2CompositorUtils.hh"

#include <algorithm>
#include <iterator>
#include <vector>

#ifdef _MSC_VER
  #pragma warning(push, 0)
#endif
#include <Compositor/OgreCompositorManager2.h>
#include <Compositor/OgreCompositorWorkspaceDef.h>
#ifdef _MSC_VER
  #pragma warning(pop)
#endif

namespace gz
{
namespace rendering
{
inline namespace GZ_RENDERING_VERSION_NAMESPACE {

//////////////////////////////////////////////////
void RemoveWorkspaceDefinitionAndNodes(
    Ogre::CompositorManager2 *_compositorManager,
    const std::string &_workspaceDefName)
{
  if (!_compositorManager || _workspaceDefName.empty())
    return;

  Ogre::CompositorWorkspaceDef *workspaceDef =
      _compositorManager->getWorkspaceDefinitionNoThrow(_workspaceDefName);
  if (!workspaceDef)
    return;

  // Copied: the map goes away with the workspace definition
  const auto &aliases = workspaceDef->getNodeAliasMap();
  std::vector<Ogre::IdString> nodeDefNames;
  std::transform(aliases.begin(), aliases.end(),
      std::back_inserter(nodeDefNames),
      [](const auto &_alias) { return _alias.second; });

  _compositorManager->removeWorkspaceDefinition(_workspaceDefName);

  for (const Ogre::IdString &nodeDefName : nodeDefNames)
  {
    if (_compositorManager->hasNodeDefinition(nodeDefName))
      _compositorManager->removeNodeDefinition(nodeDefName);
  }
}
}
}  // namespace rendering
}  // namespace gz
