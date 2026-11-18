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

#ifndef GZ_RENDERING_OGRE2_OGRE2SEGMENTATIONMATERIALSWITCHER_HH_
#define GZ_RENDERING_OGRE2_OGRE2SEGMENTATIONMATERIALSWITCHER_HH_

#include <memory>
#include <random>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include <gz/math/Color.hh>

#include "gz/rendering/config.hh"
#include "gz/rendering/ogre2/Export.hh"
#include "gz/rendering/ogre2/Ogre2Camera.hh"
#include "gz/rendering/ogre2/Ogre2RenderTypes.hh"
#include "gz/rendering/SegmentationCamera.hh"

namespace gz
{
namespace rendering
{
inline namespace GZ_RENDERING_VERSION_NAMESPACE {

/// \brief Helper class to assign unique colors to renderables
/// Due to historic reasons it's called "MaterialSwitcher" although
/// there is no longer any material switching going on.
///
/// Hlms customizations take care of running custom code that outputs
/// a flat colour
class GZ_RENDERING_OGRE2_VISIBLE Ogre2SegmentationMaterialSwitcher :
  public Ogre::Camera::Listener
{
  /// \brief Constructor
  /// \param[in] _scene The scene associated with the material switcher
  /// \param[in] _camera The canera associated with the material switcher
  public: Ogre2SegmentationMaterialSwitcher(Ogre2ScenePtr _scene,
              SegmentationCamera *_camera);

  /// \brief Destructor
  public: ~Ogre2SegmentationMaterialSwitcher();

  /// \brief Ogre's pre render update callback
  /// \param[in] _cam Ogre camera
  public: virtual void cameraPreRenderScene(Ogre::Camera *_cam) override;

  /// \brief Ogre's postrender update callback
  /// \param[in] _cam Ogre camera
  public: virtual void cameraPostRenderScene(Ogre::Camera *_cam) override;

  /// \brief Get the map between color IDs and label IDs
  /// \return The map between color and label IDs
  public: const std::unordered_map<int64_t, int64_t> &ColorToLabel() const;

  /// \brief Label of a visual, or the background label if it has none.
  /// Warns once about labels that don't fit in a label map.
  /// \param[in] _visual The visual
  /// \return The label
  private: int VisualLabel(const Visual &_visual);

  /// \brief The top level visual (the model) a node belongs to: its
  /// ancestor just below the root visual.
  /// \param[in] _node The node
  /// \return The model
  private: const Node *TopLevelModel(const Node *_node) const;

  /// \brief The instance group of a model and label, created if needed.
  /// \param[in] _model The model
  /// \param[in] _label The label
  /// \return Index in groups
  private: uint32_t Group(const Node *_model, int _label);

  /// \brief Number the instance of each group and set its color.
  private: void NumberInstances();

  /// \brief Color of a key (a label, or a label and instance), the same
  /// every time. Unique among keys, and never the background color.
  /// \param[in] _key The key
  /// \return The color
  private: Ogre::Vector4 KeyColor(int64_t _key);

  /// \brief Color to draw an item with, for semantic segmentation.
  /// \param[in] _label The item's label
  /// \return The color
  private: Ogre::Vector4 SemanticColor(int _label);

  /// \brief Color to draw an item with, for panoptic segmentation.
  /// \param[in] _label The item's label
  /// \param[in] _instance The item's instance
  /// \return The color
  private: Ogre::Vector4 PanopticColor(int _label, unsigned int _instance);

  /// \brief Read the camera's mode and background color; drop the colors
  /// if they changed.
  private: void UpdateColorState();

  /// \brief The visual an item belongs to. Cached: looking a visual up by
  /// id in the scene is slow for nested visuals.
  /// \param[in] _item The item
  /// \param[in] _visualId Id of the visual, from the item's user data
  /// \return The visual, or nullptr if there is none.
  private: VisualPtr ItemVisual(const Ogre::Item *_item,
                                unsigned int _visualId);

  /// \brief Group of an item that has no visual: drawn as background.
  private: static constexpr uint32_t kBackgroundGroup = 0xFFFFFFFFu;

  /// \brief An item to color this frame.
  private: struct ItemEntry
  {
    /// \brief The item
    Ogre::Item *item;

    /// \brief Label of its visual
    int label;

    /// \brief Instance group (panoptic only), or kBackgroundGroup
    uint32_t group;
  };

  /// \brief Items to color this frame.
  private: std::vector<ItemEntry> items;

  /// \brief A heightmap to color this frame.
  private: struct HeightmapEntry
  {
    /// \brief The heightmap
    Ogre2Heightmap *heightmap;

    /// \brief Label of its visual
    int label;

    /// \brief Instance group (panoptic only), or kBackgroundGroup
    uint32_t group;
  };

  /// \brief Heightmaps to color this frame.
  private: std::vector<HeightmapEntry> heightmapGroups;

  /// \brief The items of a model with the same label: one instance.
  private: struct InstanceGroup
  {
    /// \brief Model id (high 32 bits) and label (low 32 bits)
    uint64_t key;

    /// \brief The model. Only valid this frame.
    const Node *model;

    /// \brief The label
    int label;

    /// \brief Instance number, from 1, among the groups of the label
    unsigned int instance;

    /// \brief Color to draw the group with
    Ogre::Vector4 color;
  };

  /// \brief Instance groups this frame: those of items first, then those
  /// only heightmaps belong to.
  private: std::vector<InstanceGroup> groups;

  /// \brief Number of groups items belong to; see groups.
  private: size_t itemGroupCount{0};

  /// \brief Index in groups of each key.
  private: std::unordered_map<uint64_t, uint32_t> groupIndex;

  /// \brief Keys of the groups, this frame and when groupOrder was
  /// computed.
  private: std::vector<uint64_t> groupKeys;
  private: std::vector<uint64_t> sortedGroupKeys;
  private: size_t sortedItemGroupCount{0};

  /// \brief Order to number the groups in.
  private: std::vector<uint32_t> groupOrder;

  /// \brief Number of instances of each label so far this frame.
  private: std::unordered_map<int, unsigned int> instanceCounts;

  /// \brief Color of each key, see KeyColor. Kept between frames.
  private: std::unordered_map<int64_t, Ogre::Vector4> keyColors;

  /// \brief Upper bound on the keys used in the last frame, to know when
  /// keyColors holds too many stale keys.
  private: size_t keysInUse{0};

  /// \brief The camera's background color, and its 24 bit id.
  private: Ogre::Vector4 backgroundColor;
  private: int64_t backgroundColorId{-1};

  /// \brief Mode the colors in keyColors were made for.
  private: SegmentationType colorType{SegmentationType::ST_SEMANTIC};
  private: bool colorColored{false};

  /// \brief Labels already warned about, see VisualLabel.
  private: std::unordered_set<int> warnedLabels;

  /// \brief Whether the too many instances warning was given.
  private: bool warnedInstances{false};

  /// \brief Id of the scene's root visual.
  private: unsigned int rootId{0};

  /// \brief Mapping from the colorId to the label id, used in converting
  /// the colored map to label ids map. Kept between frames.
  /// Key: colorId, value: label in case of semantic segmentation
  /// or composite id (label * 65536 + instance) in panoptic segmentation
  private: std::unordered_map<int64_t, int64_t> colorToLabel;

  /// \brief A map of ogre datablock pointer to their original blendblocks
  private: std::unordered_map<Ogre::HlmsDatablock *,
      const Ogre::HlmsBlendblock *> datablockMap;

  /// \brief A map of ogre sub item pointer to their original low level
  /// material.
  /// Most objects don't use one so it should be almost always empty.
  private:
    std::vector<std::pair<Ogre::SubItem *, Ogre::MaterialPtr>> materialMap;

  /// \brief Sub items colored this frame, to remove their color after
  /// rendering.
  private: std::vector<Ogre::SubItem *> coloredSubItems;

  /// \brief Visual of an item, see ItemVisual.
  private: struct CachedVisual
  {
    /// \brief Id of the visual. The item's user data is checked against
    /// it, in case the item moved to another visual.
    unsigned int visualId{0};

    /// \brief The visual
    std::weak_ptr<Visual> visual;

    /// \brief Last frame the item was seen, used to drop stale entries.
    uint64_t lastSeen{0};
  };

  /// \brief Visual of each item, keyed by Ogre item id (never reused).
  private: std::unordered_map<Ogre::IdType, CachedVisual> visualCache;

  /// \brief Number of frames rendered so far.
  private: uint64_t frame{0};

  /// \brief Pseudo num generator to generate colors from label id
  private: std::default_random_engine generator;

  /// \brief Ogre2 Scene
  private: Ogre2ScenePtr scene = nullptr;

  /// \brief Pointer to segmentation camera that gives the material switcher
  /// access to things like the segmentation type, background color, background
  /// label, and if colored map is enabled
  private: SegmentationCamera *segmentationCamera {nullptr};
};
}
}  // namespace rendering
}  // namespace gz

#endif  // GZ_RENDERING_OGRE2_OGRE2SEGMENTATIONMATERIALSWITCHER_HH_
