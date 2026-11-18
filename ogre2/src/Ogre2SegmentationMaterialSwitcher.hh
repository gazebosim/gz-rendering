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

#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "gz/rendering/config.hh"
#include "gz/rendering/ogre2/Export.hh"
#include "gz/rendering/ogre2/Ogre2Camera.hh"
#include "gz/rendering/ogre2/Ogre2RenderTypes.hh"

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
              Ogre2SegmentationCamera *_camera);

  /// \brief Destructor
  public: ~Ogre2SegmentationMaterialSwitcher();

  /// \brief Ogre's pre render update callback
  /// \param[in] _cam Ogre camera
  public: virtual void cameraPreRenderScene(Ogre::Camera *_cam) override;

  /// \brief Ogre's postrender update callback
  /// \param[in] _cam Ogre camera
  public: virtual void cameraPostRenderScene(Ogre::Camera *_cam) override;

  /// \brief A map of ogre sub item pointer to its original hlms maults to 10mK
  private: double resolution = 0.01;

  /// \brief A map of ogre datablock pointer to their original blendblocks
  private: std::unordered_map<Ogre::HlmsDatablock *,
      const Ogre::HlmsBlendblock *> datablockMap;

  /// \brief A map of ogre sub item pointer to their original low level
  /// material.
  /// Most objects don't use one so it should be almost always empty.
  private:
    std::vector<std::pair<Ogre::SubItem *, Ogre::MaterialPtr>> materialMap;

  /// \brief Ogre2 Scene
  private: Ogre2ScenePtr scene = nullptr;

  /// \brief Pointer to segmentation camera that gives the material switcher
  /// access to things like the segmentation type, background color, background
  /// label, and if colored map is enabled
  private: Ogre2SegmentationCamera *segmentationCamera {nullptr};
};
}
}  // namespace rendering
}  // namespace gz

#endif  // GZ_RENDERING_OGRE2_OGRE2SEGMENTATIONMATERIALSWITCHER_HH_
