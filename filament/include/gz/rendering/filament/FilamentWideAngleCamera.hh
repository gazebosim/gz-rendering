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
#include <vector>

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
      protected: FilamentWideAngleCamera();

      public: virtual ~FilamentWideAngleCamera();

      public: virtual void Render() override;

      public: virtual void PostRender() override;

      public: virtual void Destroy() override;

      /// \brief Like ogre2, L8 / L16 hold the linear red channel of the
      /// sRGB image
      public: virtual void Copy(Image &_image) const override;

      public: virtual math::Vector3d Project3d(const math::Vector3d &_pt) const
                  override;

      public: virtual common::ConnectionPtr ConnectNewWideAngleFrame(
                  std::function<void(const unsigned char *, unsigned int,
                  unsigned int, unsigned int, const std::string &)>
                  _subscriber) override;

      /// \brief Cube face resolution (ogre2 default: 512)
      public: uint32_t EnvTextureSize() const;

      public: void SetEnvTextureSize(uint32_t _size);

      protected: virtual void Init() override;

      protected: virtual RenderTargetPtr RenderTarget() const override;

      /// \brief Focal length the shader uses (rescaled when ScaleToHFOV)
      private: float FocalLength() const;

      /// \brief Recompute which cube faces the lens samples when the image
      /// size, HFOV, lens or face resolution changed
      private: void UpdateFaces();

      private: uint32_t envTextureSize{512u};

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

      private: common::EventT<void(const unsigned char *, unsigned int,
          unsigned int, unsigned int, const std::string &)> newImageFrame;

      private: friend class FilamentScene;
    };
    }
  }
}
#endif
