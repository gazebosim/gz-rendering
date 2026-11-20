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

#ifndef GZ_RENDERING_OGRE2_OGRE2QUADNODEPOOL_HH_
#define GZ_RENDERING_OGRE2_OGRE2QUADNODEPOOL_HH_

#include <optional>
#include <string>

#include "gz/rendering/config.hh"
#include "gz/rendering/ogre2/Export.hh"

namespace gz
{
namespace rendering
{
inline namespace GZ_RENDERING_VERSION_NAMESPACE {

/// \brief A compositor node definition made by a render pass, whose quad
/// pass draws a material the render pass cloned.
struct Ogre2QuadNode
{
  /// \brief Name of the node definition
  std::string nodeDefName;

  /// \brief Name of the cloned material the quad pass draws
  std::string materialName;
};

/// \brief Free the resources of a destroyed render pass.
///
/// The cloned material is removed. The node definition is kept for a new
/// render pass to reuse, see TakeQuadNode: it can't be removed, because a
/// workspace made before the render pass was destroyed may still use it.
/// Reusing it keeps the number of node definitions at the most render
/// passes alive at once, instead of growing with every pass created.
/// Nothing is done if Ogre is already gone.
/// \param[in] _pool Name of the pool, one per kind of render pass
/// \param[in] _node The node of the destroyed render pass
GZ_RENDERING_OGRE2_HIDDEN
void ReleaseQuadNode(const std::string &_pool, const Ogre2QuadNode &_node);

/// \brief Take a node released by ReleaseQuadNode, to reuse it.
/// The caller must clone its material again, with the same name.
/// \param[in] _pool Name of the pool, one per kind of render pass
/// \return A node whose definition still exists and whose material name is
/// free, or nullopt if there is none.
GZ_RENDERING_OGRE2_HIDDEN
std::optional<Ogre2QuadNode> TakeQuadNode(const std::string &_pool);
}
}  // namespace rendering
}  // namespace gz

#endif  // GZ_RENDERING_OGRE2_OGRE2QUADNODEPOOL_HH_
