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
#ifndef GZ_RENDERING_OGRE2_OGRE2BBOXMATERIALSWITCHER_HH_
#define GZ_RENDERING_OGRE2_OGRE2BBOXMATERIALSWITCHER_HH_

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "gz/rendering/config.hh"
#include "gz/rendering/ogre2/Export.hh"
#include "gz/rendering/ogre2/Ogre2RenderTypes.hh"
#include "gz/rendering/ogre2/Ogre2BoundingBoxCamera.hh"

namespace gz
{
namespace rendering
{
inline namespace GZ_RENDERING_VERSION_NAMESPACE {

/// \brief Makes every item in view draw a flat color that encodes a
/// per-frame object index, so the rendered image tells which object covers
/// each pixel. Items outside the view are left alone.
///
/// Uses GORM_SOLID_COLOR: items keep their Hlms material, whose shaders then
/// draw custom parameter 1, so there is no material to switch and restore.
/// Only items with low level materials are switched to a flat color
/// material. Heightmaps are drawn as background.
///
/// Every labeled item gets a compact index, 1..N, in the order the items
/// are visited this frame. Index 0 is the background: the clear color, and
/// the color of unlabeled items, which still occlude. The index is written
/// in 24 bits: R = bits 0-7, G = bits 8-15, B = bits 16-23. Labels and models
/// stay on the CPU side, in Entries(), indexed by the same index.
class Ogre2BoundingBoxMaterialSwitcher : public Ogre::Camera::Listener
{
  /// \brief What the camera needs to know about an indexed item.
  public: struct Entry
  {
    /// \brief The item. Only valid during the frame it was indexed in.
    Ogre::Item *item{nullptr};

    /// \brief Label of the item's visual.
    uint32_t label{0};

    /// \brief Id of the item's top level visual (the model). Items of the
    /// same model are merged into one box.
    unsigned int modelId{0};

    /// \brief The model's node. Only valid during the frame it was indexed
    /// in.
    const Node *model{nullptr};
  };

  /// \brief Constructor
  public: explicit Ogre2BoundingBoxMaterialSwitcher(Ogre2ScenePtr _scene);

  /// \brief Destructor
  public: ~Ogre2BoundingBoxMaterialSwitcher();

  /// \brief Items indexed in the last render, by index. Entry 0 is the
  /// background and holds no item.
  public: const std::vector<Entry> &Entries() const;

  /// \brief Decode the index stored in a pixel.
  /// \param[in] _r Red channel
  /// \param[in] _g Green channel
  /// \param[in] _b Blue channel
  /// \return The index, 0 for background.
  public: static uint32_t DecodeIndex(uint8_t _r, uint8_t _g, uint8_t _b)
  {
    return static_cast<uint32_t>(_r) | (static_cast<uint32_t>(_g) << 8) |
        (static_cast<uint32_t>(_b) << 16);
  }

  /// \brief Callback when a camara is about to be rendered
  /// \param[in] _cam Ogre camera pointer which is about to render
  private: virtual void cameraPreRenderScene(
    Ogre::Camera * _cam) override;

  /// \brief Callback when a camera is finisned being rendered
  /// \param[in] _cam Ogre camera pointer which has already render
  private: virtual void cameraPostRenderScene(
    Ogre::Camera * _cam) override;

  /// \brief Whether an item can appear in the camera's image. Items that
  /// can't are left alone. Conservative: when in doubt, the item is in view.
  /// \param[in] _item The item
  /// \param[in] _planes Frustum planes of the camera, pointing inwards
  /// \param[in] _planeCount Number of planes
  /// \return False if the item is hidden or clearly outside the frustum.
  private: static bool InView(const Ogre::Item *_item,
               const Ogre::Plane *_planes, size_t _planeCount);

  /// \brief Find the visual an item belongs to, using the cache.
  /// \param[in] _item The item
  /// \return The visual, or nullptr if the item has none.
  private: VisualPtr ItemVisual(Ogre::Item *_item);

  /// \brief Sub items whose (low level) material was switched, with their
  /// original datablock, to restore after rendering.
  private: std::vector<std::pair<Ogre::SubItem *, Ogre::HlmsDatablock *>>
      datablocks;

  /// \brief Sub items given a color, to remove it after rendering.
  private: std::vector<Ogre::SubItem *> coloredSubItems;

  /// \brief Datablocks whose blending was disabled, with their original
  /// blendblock, to restore after rendering.
  private: std::unordered_map<Ogre::HlmsDatablock *,
      const Ogre::HlmsBlendblock *> blendblocks;

  /// \brief Ogre v1 material consisting of a shader that changes the
  /// appearance of item to use a unique color for mouse picking
  private: Ogre::MaterialPtr plainMaterial;

  /// \brief Ogre v1 material consisting of a shader that changes the
  /// appearance of item to use a unique color for mouse picking. In
  /// addition, the depth check and depth write properties disabled.
  private: Ogre::MaterialPtr plainOverlayMaterial;

  /// \brief User Data Key to set the label
  private: const std::string labelKey = "label";

  /// \brief Labels must be lower than this to be reported. Kept for
  /// compatibility with the previous 8 bit label encoding, where 255 was
  /// the background.
  private: static constexpr int kLabelLimit = 255;

  /// \brief Items indexed in the last render, by index.
  private: std::vector<Entry> entries;

  /// \brief Cached visual of each item, keyed by Ogre item id. Ogre ids are
  /// never reused, so an entry can't point to another item's visual.
  /// Looking visuals up by id in the scene is slow for nested visuals, so
  /// it is only done the first time an item is seen.
  private: struct CachedVisual
  {
    /// \brief The visual
    std::weak_ptr<Visual> visual;

    /// \brief Last frame the item was seen, used to drop stale entries.
    uint64_t lastSeen{0};
  };

  /// \brief Visual cache, see CachedVisual.
  private: std::unordered_map<uint32_t, CachedVisual> visualCache;

  /// \brief Number of renders so far.
  private: uint64_t frame{0};

  /// \brief Ogre2 Scene
  private: Ogre2ScenePtr scene;
};
}
}  // namespace rendering
}  // namespace gz
#endif
