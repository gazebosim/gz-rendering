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

#include "Ogre2QuadNodePool.hh"

#include <map>
#include <mutex>
#include <vector>

#include "gz/rendering/ogre2/Ogre2RenderEngine.hh"

#ifdef _MSC_VER
  #pragma warning(push, 0)
#endif
#include <Compositor/OgreCompositorManager2.h>
#include <OgreMaterialManager.h>
#include <OgreRoot.h>
#ifdef _MSC_VER
  #pragma warning(pop)
#endif

namespace gz
{
namespace rendering
{
inline namespace GZ_RENDERING_VERSION_NAMESPACE {

namespace
{
/// \brief Released nodes, per pool
std::map<std::string, std::vector<Ogre2QuadNode>> g_pools;

/// \brief Protects g_pools
std::mutex g_poolsMutex;

/// \brief The Ogre compositor manager, or null if Ogre is gone
Ogre::CompositorManager2 *CompositorManager()
{
  Ogre::Root *root = Ogre2RenderEngine::Instance()->OgreRoot();
  return root ? root->getCompositorManager2() : nullptr;
}
}  // namespace

//////////////////////////////////////////////////
void ReleaseQuadNode(const std::string &_pool, const Ogre2QuadNode &_node)
{
  // Ogre may already be gone, e.g. the render engine was unloaded before
  // the render pass was released. Its definitions and materials are too.
  Ogre::MaterialManager *materialManager =
      Ogre::MaterialManager::getSingletonPtr();
  if (!materialManager)
    return;

  if (!_node.materialName.empty())
    materialManager->remove(_node.materialName);

  if (!_node.nodeDefName.empty())
  {
    std::lock_guard<std::mutex> lock(g_poolsMutex);
    g_pools[_pool].push_back(_node);
  }
}

//////////////////////////////////////////////////
std::optional<Ogre2QuadNode> TakeQuadNode(const std::string &_pool)
{
  Ogre::MaterialManager *materialManager =
      Ogre::MaterialManager::getSingletonPtr();
  Ogre::CompositorManager2 *compositorManager = CompositorManager();
  if (!materialManager || !compositorManager)
    return std::nullopt;

  std::lock_guard<std::mutex> lock(g_poolsMutex);
  std::vector<Ogre2QuadNode> &released = g_pools[_pool];
  while (!released.empty())
  {
    Ogre2QuadNode node = released.back();
    released.pop_back();
    // Nodes released before the render engine was reloaded are gone
    if (compositorManager->hasNodeDefinition(node.nodeDefName) &&
        materialManager->getByName(node.materialName).isNull())
    {
      return node;
    }
  }
  return std::nullopt;
}
}
}  // namespace rendering
}  // namespace gz
