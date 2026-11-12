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
#ifndef GZ_RENDERING_FILAMENT_FILAMENTGPURAYS_HH_
#define GZ_RENDERING_FILAMENT_FILAMENTGPURAYS_HH_

#include <array>
#include <memory>
#include <string>
#include <vector>

#include <gz/common/Event.hh>
#include <utils/Entity.h>

#include "gz/rendering/config.hh"
#include "gz/rendering/base/BaseGpuRays.hh"
#include "gz/rendering/filament/FilamentRenderTypes.hh"
#include "gz/rendering/filament/FilamentSensor.hh"
#include "gz/rendering/filament/Export.hh"

namespace filament
{
  class Camera;
  class MaterialInstance;
  class Texture;
  class View;
}

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    //
    class FilamentFullscreenPass;

    /// \brief GPU lidar: renders the cube faces the rays need with the
    /// linear-depth material into one atlas, then a second pass samples one
    /// texel per ray on the GPU so only the scan is read back (like ogre2).
    /// Channels: range, retro (always 0), 0.
    class GZ_RENDERING_FILAMENT_VISIBLE FilamentGpuRays :
      public BaseGpuRays<FilamentSensor>
    {
      /// \brief Constructor
      protected: FilamentGpuRays();

      /// \brief Destructor
      public: virtual ~FilamentGpuRays();

      /// \brief Renders the cube faces the rays need into the atlas, samples
      /// one texel per ray on the GPU and reads the ranges back.
      public: virtual void Render() override;

      /// \brief Applies the scatter of particles in view, replaces ranges
      /// outside near / far with the data min / max values and sends the scan
      /// to the new frame subscribers.
      public: virtual void PostRender() override;

      // Documentation inherited
      public: virtual void Destroy() override;

      // Documentation inherited
      public: virtual const float *Data() const override;

      // Documentation inherited
      public: virtual void Copy(float *_dataDest) override;

      /// \brief Connect to the new scan event.
      /// \param[in] _subscriber Callback called with every new scan (range,
      /// retro and 0 for each ray), its width, height and channel count, and
      /// the pixel format name.
      /// \return The connection, which must be kept in scope.
      public: virtual common::ConnectionPtr ConnectNewGpuRaysFrame(
                  std::function<void(const float *_frame, unsigned int _width,
                  unsigned int _height, unsigned int _depth,
                  const std::string &_format)> _subscriber) override;

      // Documentation inherited
      public: virtual RenderTargetPtr RenderTarget() const override;

      // Documentation inherited
      protected: virtual void Init() override;

      /// \brief Recompute ray directions and the faces they hit when the
      /// scan configuration changed
      private: void UpdateRays();

      /// \brief Per ray: cube face, texel index in the atlas, 1 / cos(angle
      /// between the texel center direction and the face axis): depth to
      /// range
      private: struct Ray
      {
        /// \brief Cube face the ray falls on
        int face;

        /// \brief Texel index of the ray in the atlas
        unsigned int texel;

        /// \brief 1 / cos(angle between the texel center direction and the
        /// face axis), to convert depth to range
        float invCos;
      };

      /// \brief Rays of the scan, row by row (vertical angle, then
      /// horizontal).
      private: std::vector<Ray> rays;

      /// \brief Configuration the rays were computed for
      private: std::array<double, 8> rayKey{};

      /// \brief Side of a cube face, in texels.
      private: unsigned int faceRes{0u};

      /// \brief Whether some ray samples each cube face.
      private: std::array<bool, 6> faceUsed{};

      /// \brief Create the second-pass objects (scene, view, fullscreen
      /// triangle, material)
      private: void InitSamplePass();

      /// \brief Upload the per-ray atlas texel and 1/cos to rayTexture
      private: void UploadRays();

      /// \brief Range per ray, read back from the second pass
      private: std::vector<float> ranges;

      /// \brief Half resolution particle depth of the six faces, reused
      private: std::vector<float> particleDepth;

      /// \brief Second pass: one fragment per ray (gz_lidar_sample)
      private: std::unique_ptr<FilamentFullscreenPass> samplePass;

      /// \brief RGBA32F, one texel per ray: atlas x, atlas y, 1/cos
      private: filament::Texture *rayTexture{nullptr};

      /// \brief RGBA8, one texel per ray: the range's float bits
      private: FilamentRenderTexturePtr scanTexture;

      /// \brief Number of used cube faces, side by side in the atlas.
      private: unsigned int faceCount{0u};

      /// \brief Scan sent to subscribers, one entry per ray and channel.
      private: std::vector<float> scan;

      /// \brief Entity of the camera that renders the cube faces.
      private: ::utils::Entity cameraEntity;

      /// \brief Camera turned to each cube face in turn, 90 degree field of
      /// view.
      private: filament::Camera *camera{nullptr};

      /// \brief View of the scene through camera.
      private: filament::View *view{nullptr};

      /// \brief Instance of the gz_depth material, applied to every mesh
      /// while rendering.
      private: filament::MaterialInstance *depthInstance{nullptr};

      /// \brief Atlas of the used cube faces, RGBA8 holding float depth
      /// bits.
      private: FilamentRenderTexturePtr renderTexture;

      /// \brief Event triggered with every new scan.
      private: common::EventT<void(const float *, unsigned int, unsigned int,
          unsigned int, const std::string &)> newFrame;

      /// \brief Only the scene can create this class.
      private: friend class FilamentScene;
    };
    }
  }
}
#endif
