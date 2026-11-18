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
#ifndef GZ_RENDERING_FILAMENT_FILAMENTSEGMENTATIONCAMERA_HH_
#define GZ_RENDERING_FILAMENT_FILAMENTSEGMENTATIONCAMERA_HH_

#include <string>
#include <unordered_map>

#include <gz/common/Event.hh>

#include "gz/rendering/base/BaseSegmentationCamera.hh"
#include "gz/rendering/filament/FilamentCamera.hh"
#include "gz/rendering/filament/FilamentRenderTypes.hh"
#include "gz/rendering/filament/Export.hh"

namespace filament
{
  class MaterialInstance;
}

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    //
    /// \brief Segmentation camera: every mesh is drawn with a flat label
    /// color into a linear RGB8 target.
    class GZ_RENDERING_FILAMENT_VISIBLE FilamentSegmentationCamera :
      public BaseSegmentationCamera<FilamentCamera>
    {
      /// \brief Constructor
      protected: FilamentSegmentationCamera() = default;

      /// \brief Destructor
      public: virtual ~FilamentSegmentationCamera() = default;

      /// \brief Forward to FilamentCamera, which BaseCamera<FilamentCamera>
      /// would otherwise win over (same for ProjectionMatrix()).
      /// \param[in] _matrix New projection matrix
      public: virtual void SetProjectionMatrix(const math::Matrix4d &_matrix)
          override { FilamentCamera::SetProjectionMatrix(_matrix); }

      /// \brief Forward to FilamentCamera, see SetProjectionMatrix().
      /// \return The projection matrix
      public: virtual math::Matrix4d ProjectionMatrix() const override
          { return FilamentCamera::ProjectionMatrix(); }

      // Documentation inherited
      public: virtual void CreateSegmentationTexture() override;

      // Documentation inherited
      public: virtual void Render() override;

      // Documentation inherited
      public: virtual void PostRender() override;

      // Documentation inherited
      public: virtual uint8_t *SegmentationData() const override;

      // Documentation inherited
      public: virtual common::ConnectionPtr ConnectNewSegmentationFrame(
          std::function<void(const uint8_t *, unsigned int, unsigned int,
          unsigned int, const std::string &)> _subscriber) override;

      // Documentation inherited
      public: virtual void SetBackgroundLabel(int _label) override;

      // Documentation inherited
      public: virtual void LabelMapFromColoredBuffer(
          uint8_t *_labelBuffer) const override;

      // Documentation inherited
      public: virtual void Destroy() override;

      /// \brief Initialize the camera with shadows turned off.
      protected: virtual void Init() override;

      /// \brief Get the target the label colors are rendered to.
      /// \return The camera's render texture
      protected: virtual RenderTargetPtr RenderTarget() const override;

      /// \brief gz_flat instances by color (0xRRGGBB)
      private: std::unordered_map<uint32_t, filament::MaterialInstance *>
          flatInstances;

      /// \brief True once CreateSegmentationTexture configured the render
      /// texture.
      private: bool textureCreated{false};

      /// \brief Segmentation data of the last frame, RGB8
      private: Image image;

      /// \brief Event fired by PostRender with the new segmentation image.
      private: common::EventT<void(const uint8_t *, unsigned int,
          unsigned int, unsigned int, const std::string &)>
          newSegmentationFrame;

      private: friend class FilamentScene;
    };
    }
  }
}
#endif
