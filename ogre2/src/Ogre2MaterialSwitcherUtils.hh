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

#ifndef GZ_RENDERING_OGRE2_OGRE2MATERIALSWITCHERUTILS_HH_
#define GZ_RENDERING_OGRE2_OGRE2MATERIALSWITCHERUTILS_HH_

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <unordered_map>

#include "gz/rendering/config.hh"
#include "gz/rendering/RenderTypes.hh"
#include "gz/rendering/ogre2/Export.hh"
#include "gz/rendering/ogre2/Ogre2RenderTypes.hh"

#ifdef _MSC_VER
  #pragma warning(push, 0)
#endif
#include <OgreCamera.h>
#include <OgreItem.h>
#include <OgrePlane.h>
#ifdef _MSC_VER
  #pragma warning(pop)
#endif

namespace gz
{
namespace rendering
{
inline namespace GZ_RENDERING_VERSION_NAMESPACE {

/// \brief Frustum planes of a camera, pointing inwards. The far plane is
/// left out when the far clip distance is infinite.
struct GZ_RENDERING_OGRE2_HIDDEN Ogre2FrustumPlanes
{
  /// \brief The planes; only the first `count` are valid.
  std::array<Ogre::Plane, 6> planes;

  /// \brief Number of valid planes. 0 means no culling: every item is in
  /// view.
  std::size_t count{0u};
};

/// \brief Planes of a camera's frustum, with its current pose.
/// \param[in] _camera The camera; nullptr gives no planes.
/// \return The planes
GZ_RENDERING_OGRE2_HIDDEN
Ogre2FrustumPlanes CollectFrustumPlanes(const Ogre::Camera *_camera);

/// \brief Whether a camera with the given frustum may render an item: the
/// item is visible and its world bounding box, grown by 1% + 1e-3 m, isn't
/// fully outside the frustum. Infinite or invalid boxes count as in view.
///
/// Material switchers use this to skip items the camera won't render. An
/// item that is rendered without its custom parameters set would make the
/// shaders throw, so the test only says "out of view" when it clearly is.
/// \param[in] _item The item
/// \param[in] _planes The frustum planes
/// \return False only if the item is clearly out of view.
GZ_RENDERING_OGRE2_HIDDEN
bool ItemInView(const Ogre::Item &_item, const Ogre2FrustumPlanes &_planes);

/// \brief The id of the visual an item belongs to, stored in the item's
/// user any.
/// \param[in] _item The item
/// \param[out] _id The visual id, set only when the function returns true
/// \return False if the item has no visual id.
GZ_RENDERING_OGRE2_HIDDEN
bool ItemVisualId(const Ogre::Item &_item, unsigned int &_id);

/// \brief Remembers the visual each Ogre item belongs to. Looking a visual
/// up by id in the scene is slow (it searches the visual tree), and
/// material switchers do it for every item, every frame.
/// Only use from the render thread.
class GZ_RENDERING_OGRE2_HIDDEN Ogre2ItemVisualCache
{
  /// \brief The visual with id _visualId, which _item belongs to. A cached
  /// visual is used if it's still alive and has that id; otherwise the
  /// visual is looked up in the scene.
  /// \param[in] _scene Scene to look the visual up in
  /// \param[in] _item The item
  /// \param[in] _visualId Id of the visual, see ItemVisualId
  /// \return The visual, or nullptr if the scene has none with that id
  /// (e.g. a visual that isn't attached to the scene)
  public: VisualPtr Lookup(Ogre2Scene &_scene, const Ogre::Item &_item,
                           unsigned int _visualId);

  /// \brief Call once per frame, after the lookups. Every kPruneInterval
  /// frames, drops the items not looked up since the last prune.
  public: void EndFrame();

  /// \brief Number of cached items
  /// \return Number of items
  public: std::size_t Size() const;

  /// \brief Frames between prunes
  public: static constexpr uint64_t kPruneInterval = 120u;

  /// \brief A cached visual
  private: struct Entry
  {
    /// \brief Id of the visual. Checked against the item's visual id, in
    /// case the item moved to another visual.
    unsigned int visualId{0u};

    /// \brief The visual
    std::weak_ptr<Visual> visual;

    /// \brief Last frame the item was looked up
    uint64_t lastSeen{0u};
  };

  /// \brief Visual of each item, by Ogre item id (never reused)
  private: std::unordered_map<Ogre::IdType, Entry> entries;

  /// \brief Frames ended so far
  private: uint64_t frame{0u};
};
}
}  // namespace rendering
}  // namespace gz

#endif  // GZ_RENDERING_OGRE2_OGRE2MATERIALSWITCHERUTILS_HH_
