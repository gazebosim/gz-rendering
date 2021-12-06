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
#ifndef IGNITION_RENDERING_OGRE_OGREDISTORTIONPASS_HH_
#define IGNITION_RENDERING_OGRE_OGREDISTORTIONPASS_HH_

#include <memory>
#include <vector>

#include <ignition/common/SuppressWarning.hh>

#include "ignition/rendering/base/BaseDistortionPass.hh"
#include "ignition/rendering/ogre/OgreIncludes.hh"
#include "ignition/rendering/ogre/OgreRenderPass.hh"
#include "ignition/rendering/ogre/Export.hh"

namespace ignition
{
  namespace rendering
  {
    inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {
    //
    // Forward declaration
    class DistortionCompositorListener;

    /* \class OgreDistortionPass OgreDistortionPass.hh \
     * ignition/rendering/ogre/OgreDistortionPass.hh
     */
    /// \brief Ogre implementation of the DistortionPass class
    class IGNITION_RENDERING_OGRE_VISIBLE OgreDistortionPass :
      public BaseDistortionPass<OgreRenderPass>
    {
      /// \brief Constructor
      public: OgreDistortionPass();

      /// \brief Destructor
      public: virtual ~OgreDistortionPass();

      // Documentation inherited
      public: void PreRender() override;

      // Documentation inherited
      public: void Destroy() override;

      // Documentation inherited
      public: void CreateRenderPass() override;

      /// \brief Apply distortion model
      /// \param[in] _in Input uv coordinate.
      /// \param[in] _center Normalized distortion center.
      /// \param[in] _k1 Radial distortion coefficient k1.
      /// \param[in] _k2 Radial distortion coefficient k2.
      /// \param[in] _k3 Radial distortion coefficient k3.
      /// \param[in] _p1 Tangential distortion coefficient p1.
      /// \param[in] _p2 Tangential distortion coefficient p2.
      /// \return Distorted coordinate.
      public: static ignition::math::Vector2d Distort(
                  const ignition::math::Vector2d &_in,
                  const ignition::math::Vector2d &_center,
                  double _k1, double _k2, double _k3,
                  double _p1, double _p2);

      /// \brief Apply distortion model using camera coordinates projection
      /// \param[in] _in Input uv coordinate.
      /// \param[in] _center Normalized distortion center.
      /// \param[in] _k1 Radial distortion coefficient k1.
      /// \param[in] _k2 Radial distortion coefficient k2.
      /// \param[in] _k3 Radial distortion coefficient k3.
      /// \param[in] _p1 Tangential distortion coefficient p1.
      /// \param[in] _p2 Tangential distortion coefficient p2.
      /// \param[in] _width Width of the image texture in pixels.
      /// \param[in] _f Focal length in pixels.
      /// \return Distorted coordinate.
      public: static ignition::math::Vector2d Distort(
                  const ignition::math::Vector2d &_in,
                  const ignition::math::Vector2d &_center,
                  double _k1, double _k2, double _k3,
                  double _p1, double _p2,
                  unsigned int _width, double _f);

      /// \brief get the distortion map value.
      /// \param[in] _x X component of map.
      /// \param[in] _y Y component of map.
      /// \return the distortion map value at the specified index.
      protected: ignition::math::Vector2d
        DistortionMapValueClamped(const int _x, const int _y) const;

      /// \brief calculate the correct scale factor to "zoom" the render,
      /// cutting off black borders caused by distortion (only if the crop
      /// flag has been set).
      protected: void CalculateAndApplyDistortionScale();

      /// \brief Scale applied to distorted image.
      public: ignition::math::Vector2d distortionScale = {1.0, 1.0};

      /// \brief True if the distorted image will be cropped to remove the
      /// black pixels at the corners of the image.
      public: bool distortionCrop = true;

      /// \brief Use the legacy distortion mode. If this is set to false, the
      /// new mode modifies how Brown's distortion equations are applied to
      /// better reflect real distortion. The image is projected from image
      /// plane to camera plane to apply distortion equations, then projected
      /// back to image plane. Note that the new distortion doesn't allow
      /// the image to be cropped.
      public: bool legacyMode = true;

      /// \brief Distortion compositor.
      public: Ogre::CompositorInstance *distortionInstance = nullptr;

      /// \brief Ogre Material that contains the distortion shader
      public: Ogre::MaterialPtr distortionMaterial;

      /// \brief Mapping of distorted to undistorted normalized pixels
      public: std::vector<ignition::math::Vector2d> distortionMap;

      /// \brief Width of distortion texture map
      public: unsigned int distortionTexWidth;

      /// \brief Height of distortion texture map
      public: unsigned int distortionTexHeight;

      /// \brief Distortion compositor listener
      /// uses <ignition/utils/ImplPtr.hh> from ign-rendering7
      IGN_COMMON_WARN_IGNORE__DLL_INTERFACE_MISSING
      public: std::shared_ptr<DistortionCompositorListener>
          distortionCompositorListener;
      IGN_COMMON_WARN_RESUME__DLL_INTERFACE_MISSING
    };
    }
  }
}
#endif
