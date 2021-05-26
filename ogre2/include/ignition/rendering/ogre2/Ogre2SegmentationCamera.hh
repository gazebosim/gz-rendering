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

#ifndef IGNITION_RENDERING_OGRE2_OGRE2SEGMENTATIONCAMERA_HH_
#define IGNITION_RENDERING_OGRE2_OGRE2SEGMENTATIONCAMERA_HH_

#ifdef _WIN32
  // Ensure that Winsock2.h is included before Windows.h, which can get
  // pulled in by anybody (e.g., Boost).
  #include <Winsock2.h>
#endif

#include <memory>
#include <string>

#include "ignition/rendering/base/BaseSegmentationCamera.hh"
#include "ignition/rendering/ogre2/Ogre2Includes.hh"
#include "ignition/rendering/ogre2/Ogre2Sensor.hh"

#include "ignition/common/Event.hh"
#include "ignition/common/Console.hh"

namespace Ogre
{
  class Material;
  class RenderTarget;
  class Texture;
  class Viewport;
}

namespace ignition
{
  namespace rendering
  {
    inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {
    //
    // Forward declaration
    class Ogre2SegmentationCameraPrivate;

    /// \brief Segmentation camera used to label each pixel with a label id
    /// supports Semantic / Panoptic Segmentation
    class IGNITION_RENDERING_OGRE2_VISIBLE Ogre2SegmentationCamera :
      public BaseSegmentationCamera<Ogre2Sensor>
    {
      /// \brief Constructor
      protected: Ogre2SegmentationCamera();

      /// \brief Destructor
      public: virtual ~Ogre2SegmentationCamera();

      /// \brief Initialize the camera
      public: virtual void Init() override;

      /// \brief Destroy the camera
      public: virtual void Destroy() override;

      /// \brief Create a texture which will hold the Segmentation data
      /// \brief Set up render texture and compositor
      public: virtual void CreateSegmentationTexture() override;

      /// \brief Create the camera.
      protected: void CreateCamera();

      /// \brief Get the segmentation image buffer
      /// \return buffer that holds segmentation data
      public: virtual uint8_t *SegmentationData() const;

      // Documentation inherited
      public: virtual void PreRender() override;

      // Documentation inherited
      public: virtual void Render() override;

      // Documentation inherited
      public: virtual void PostRender() override;

      /// \brief Subscribe to new segmentation frame event
      /// \param[in] _subscriber callback listener, called on each new frame
      public: virtual ignition::common::ConnectionPtr
        ConnectNewSegmentationFrame(
        std::function<void(const uint8_t *, unsigned int, unsigned int,
        unsigned int, const std::string &)>  _subscriber);

      /// \brief Create dummy render texture. Needed to satisfy inheritance
      /// and to set image's dims
      public: virtual void CreateRenderTexture();

      /// \brief Set Segmentation Type (Semantic / Panoptic)
      /// \param[in] _type Segmentation Type (Semantic / Panoptic)
      public: void SetSegmentationType(SegmentationType _type);

      /// \brief Enable Color map mode to generated colored semantics
      /// \param[in] _enable True to generate colored map, False to generate
      /// label id map
      public: void EnableColoredMap(bool _enable);

      /// \brief Set color for background & unlabeled items in the colored map
      /// \param[in] _color Color of background & unlabeled items
      public: void SetBackgroundColor(math::Color _color);

      /// \brief Set label for background & unlabeled items in the semantic map
      /// \param[in] _label label of background & unlabeled items
      public: void SetBackgroundLabel(int _label);

      /// \brief Get color for background & unlabeled items in the colored map
      /// \return Color of background & unlabeled items
      public: math::Color BackgroundColor();

      /// \brief Get label for background & unlabeled items in the semantic map
      /// \return label of background & unlabeled items
      public: int BackgroundLabel();

      /// \brief Get a pointer to the render target.
      /// \return Pointer to the render target
      protected: virtual RenderTargetPtr RenderTarget() const override;

      /// \brief Pointer to the ogre camera
      protected: Ogre::Camera *ogreCamera;

      /// \internal
      /// \brief Pointer to private data.
      private: std::unique_ptr<Ogre2SegmentationCameraPrivate> dataPtr;

      private: friend class Ogre2Scene;
    };
    }
  }
}
#endif
