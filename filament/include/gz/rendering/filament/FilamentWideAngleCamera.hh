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
#ifndef GZ_RENDERING_FILAMENT_FILAMENTWIDEANGLECAMERA_HH_
#define GZ_RENDERING_FILAMENT_FILAMENTWIDEANGLECAMERA_HH_

#include <array>
#include <limits>
#include <memory>
#include <string>

#include <gz/common/Event.hh>

#include "gz/rendering/config.hh"
#include "gz/rendering/base/BaseWideAngleCamera.hh"
#include "gz/rendering/filament/FilamentCamera.hh"
#include "gz/rendering/filament/FilamentRenderTypes.hh"
#include "gz/rendering/filament/Export.hh"

namespace filament
{
  class View;
}

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    //
    class FilamentFullscreenPass;

    /// \brief Wide-angle camera: renders the cube faces the lens sees side by
    /// side into one atlas, then a fullscreen pass maps every output pixel to
    /// a direction with ogre2's lens shader and samples the atlas. Only the
    /// final image is read back.
    class GZ_RENDERING_FILAMENT_VISIBLE FilamentWideAngleCamera :
      public BaseWideAngleCamera<FilamentCamera>
    {
      /// \brief Constructor
      protected: FilamentWideAngleCamera();

      /// \brief Destructor
      public: virtual ~FilamentWideAngleCamera();

      // Documentation inherited
      public: virtual void Render() override;

      // Documentation inherited
      public: virtual void PostRender() override;

      // Documentation inherited
      public: virtual void Destroy() override;

      /// \brief Copy the rendered image into _image.
      /// Like ogre2, L8 / L16 hold the linear red channel of the sRGB
      /// image.
      /// \param[out] _image Image to fill
      public: virtual void Copy(Image &_image) const override;

      // Documentation inherited
      public: virtual math::Vector3d Project3d(const math::Vector3d &_pt) const
                  override;

      // Documentation inherited
      public: virtual common::ConnectionPtr ConnectNewWideAngleFrame(
                  std::function<void(const unsigned char *, unsigned int,
                  unsigned int, unsigned int, const std::string &)>
                  _subscriber) override;

      /// \brief Get the resolution of one cube face in pixels (ogre2
      /// default: 512).
      /// \return Cube face resolution in pixels
      public: uint32_t EnvTextureSize() const;

      /// \brief Set the resolution of one cube face in pixels.
      /// \param[in] _size Resolution in pixels, at least 1
      public: void SetEnvTextureSize(uint32_t _size);

      /// \brief Initialize the camera and create the cube face atlas and the
      /// pass that stitches it into the image.
      protected: virtual void Init() override;

      /// \brief Get the target of the stitched image, not the face atlas.
      /// \return The camera's render texture
      protected: virtual RenderTargetPtr RenderTarget() const override;

      /// \brief Get the focal length the lens shader uses.
      /// \return The lens focal length, rescaled when the lens scales to
      /// the HFOV
      private: float FocalLength() const;

      /// \brief Recompute which cube faces the lens samples when the image
      /// size, HFOV, lens or face resolution changed
      private: void UpdateFaces();

      /// \brief Resolution of one cube face in pixels.
      private: uint32_t envTextureSize{512u};

      /// \brief Per cube face, true when the lens samples it; only those
      /// faces are rendered.
      private: std::array<bool, 6> faceUsed{};

      /// \brief Configuration faceUsed was computed for (NaN: none yet)
      private: std::array<double, 11> faceKey{
          std::numeric_limits<double>::quiet_NaN()};

      /// \brief The used faces, each in its slot (face index * size)
      private: FilamentRenderTexturePtr atlas;

      /// \brief Second pass: samples the atlas (gz_wide_angle)
      private: std::unique_ptr<FilamentFullscreenPass> samplePass;

      /// \brief Readback image of PostRender, reused
      private: Image image;

      /// \brief Event fired by PostRender with the new image.
      private: common::EventT<void(const unsigned char *, unsigned int,
          unsigned int, unsigned int, const std::string &)> newImageFrame;

      private: friend class FilamentScene;
    };
    }
  }
}
#endif
