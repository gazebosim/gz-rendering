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

#ifndef IGNITION_RENDERING_OGRE2_OGRE2SEGMENTATIONMATERIALSWITCHER_HH_
#define IGNITION_RENDERING_OGRE2_OGRE2SEGMENTATIONMATERIALSWITCHER_HH_

#include <random>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include <ignition/math/Color.hh>

#include "ignition/rendering/config.hh"
#include "ignition/rendering/ogre2/Export.hh"
#include "ignition/rendering/ogre2/Ogre2Camera.hh"
#include "ignition/rendering/ogre2/Ogre2RenderTypes.hh"
#include "ignition/rendering/SegmentationCamera.hh"

namespace ignition
{
namespace rendering
{
inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {

/// \brief Helper class to assign unique colors to renderables
/// Due to historic reasons it's called "MaterialSwitcher" although
/// there is no longer any material switching going on.
///
/// Hlms customizations take care of running custom code that outputs
/// a flat colour
class IGNITION_RENDERING_OGRE2_VISIBLE Ogre2SegmentationMaterialSwitcher :
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

  /// \brief Create a color to apply for the given visual
  /// \param[in] _visual Visual will be applying the color to
  /// \param[in,out] _prevParentName A persistent string between call
  /// to ensure multilink visuals receive the same color
  /// \return The color to apply to the visual
  private: Ogre::Vector4 ColorForVisual(const VisualPtr &_visual,
                                        std::string &_prevParentName);

  /// \brief Convert label of semantic map to a unique color for colored map and
  /// add the color of the label to the taken colors if it doesn't exist
  /// \param[in] _label id of the semantic map or encoded id of panoptic map
  /// \param[in] _isMultiLink bool used to skip the taken color check if the
  /// label is for a multi link model (all links for the same model should have
  /// the same color)
  /// \return Unique color in the colored map for that label
  private: math::Color LabelToColor(int64_t _label,
    bool _isMultiLink = false);

  /// \brief Get the top level model visual of a particular visual
  /// \param[in] _visual The visual who's top level model visual we are
  /// interested in
  /// \return The top level model visual of _visual
  private: VisualPtr TopLevelModelVisual(VisualPtr _visual) const;

  /// \brief Check if the color is already taken and add it to taken colors
  /// if it does not exist
  /// \param[in] _color Color to be checked
  /// \return True if taken, False otherwise
  private: bool IsTakenColor(const math::Color &_color);

  /// \brief A map of ogre sub item pointer to its original hlms maults to 10mK
  private: double resolution = 0.01;

  /// \brief Keep track of num of instances of the same label
  /// Key: label id, value: num of instances
  private: std::unordered_map<int, unsigned int> instancesCount;

  /// \brief keep track of the random colors (store encoded id of r,g,b)
  private: std::unordered_set<int64_t> takenColors;

  /// \brief keep track of the labels that are already colored.
  /// Useful for coloring items in semantic mode in LabelToColor()
  private: std::unordered_set<int64_t> coloredLabel;

  /// \brief Mapping from the colorId to the label id, used in converting
  /// the colored map to label ids map
  /// Key: colorId, value: label in case of semantic segmentation
  /// or composite id (8 bit label + 16 bit instances) in instance type
  private: std::unordered_map<int64_t, int64_t> colorToLabel;

  /// \brief A map of ogre datablock pointer to their original blendblocks
  private: std::unordered_map<Ogre::HlmsDatablock *,
      const Ogre::HlmsBlendblock *> datablockMap;

  /// \brief A map of ogre sub item pointer to their original low level
  /// material.
  /// Most objects don't use one so it should be almost always empty.
  private:
    std::vector<std::pair<Ogre::SubItem *, Ogre::MaterialPtr>> materialMap;

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
}  // namespace ignition

#endif  // IGNITION_RENDERING_OGRE2_OGRE2SEGMENTATIONMATERIALSWITCHER_HH_
