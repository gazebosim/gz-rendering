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
#ifndef GZ_RENDERING_FILAMENT_FILAMENTDEPTHCAMERA_HH_
#define GZ_RENDERING_FILAMENT_FILAMENTDEPTHCAMERA_HH_

#include <array>
#include <limits>
#include <string>
#include <vector>

#include <gz/common/Event.hh>

#include "gz/rendering/config.hh"
#include "gz/rendering/base/BaseDepthCamera.hh"
#include "gz/rendering/filament/FilamentCamera.hh"
#include "gz/rendering/filament/FilamentRenderTypes.hh"
#include "gz/rendering/filament/Export.hh"

namespace filament
{
  class MaterialInstance;
  class Texture;
}

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    //
    /// \brief Depth camera: renders every mesh with the gz_depth_clamp
    /// material (linear view depth, clamped like ogre2's) into an RGBA8
    /// target that holds the float bits, then reads it back. With particles
    /// in view it renders the raw depth (gz_depth) and PostRender clamps.
    class GZ_RENDERING_FILAMENT_VISIBLE FilamentDepthCamera :
      public BaseDepthCamera<FilamentCamera>
    {
      /// \brief Constructor
      protected: FilamentDepthCamera() = default;

      /// \brief Destructor
      public: virtual ~FilamentDepthCamera() = default;

      /// \brief Forwards to FilamentCamera::SetProjectionMatrix, which
      /// BaseCamera<FilamentCamera> would otherwise override.
      /// \param[in] _matrix Custom projection matrix.
      public: virtual void SetProjectionMatrix(const math::Matrix4d &_matrix)
          override { FilamentCamera::SetProjectionMatrix(_matrix); }

      /// \brief Creates the depth material instances and sets up the RGBA8
      /// target, which holds the depth's float bits.
      public: virtual void CreateDepthTexture() override;

      // Documentation inherited
      public: virtual void PreRender() override;

      /// \brief Renders the depth pass, after a color pass when the colored
      /// point cloud has subscribers.
      public: virtual void Render() override;

      /// \brief Reads the images back, clamps the depth to near / far unless
      /// the GPU did it, applies particles and noise and sends the results
      /// to the new frame subscribers.
      public: virtual void PostRender() override;

      // Documentation inherited
      public: virtual const float *DepthData() const override;

      /// \brief Like ogre2: a custom matrix reports the configured clip
      /// planes in its depth rows
      /// \return The projection matrix.
      public: virtual math::Matrix4d ProjectionMatrix() const override;

      // Documentation inherited
      public: virtual common::ConnectionPtr ConnectNewDepthFrame(
          std::function<void(const float *, unsigned int, unsigned int,
          unsigned int, const std::string &)> _subscriber) override;

      /// \brief Connect to the new colored point cloud event.
      /// \param[in] _subscriber Callback called with every new cloud (x, y,
      /// z and the packed color, as float bits, for each pixel), its width,
      /// height and channel count, and the pixel format name.
      /// \return The connection, which must be kept in scope.
      public: virtual common::ConnectionPtr ConnectNewRgbPointCloud(
          std::function<void(const float *, unsigned int, unsigned int,
          unsigned int, const std::string &)> _subscriber) override;

      // Documentation inherited
      public: virtual void Destroy() override;

      // Documentation inherited
      protected: virtual void Init() override;

      // Documentation inherited
      protected: virtual RenderTargetPtr RenderTarget() const override;

      /// \brief Update cachedColK / cachedRowK to the current pinhole
      /// \return The pinhole: fx, fy, cx, cy
      private: std::array<double, 4> UpdateRays();

      /// \brief gz_depth: the raw depth, for particles
      protected: filament::MaterialInstance *depthInstance{nullptr};

      /// \brief gz_depth_clamp: the depth clamped in the shader
      protected: filament::MaterialInstance *clampInstance{nullptr};

      /// \brief clampInstance's per pixel far thresholds (R32F), the
      /// least depth whose range along the pixel ray is beyond far
      protected: filament::Texture *farMap{nullptr};

      /// \brief cachedRayKey and the far range farMap was computed for
      protected: std::array<double, 7> farMapKey{
          std::numeric_limits<double>::quiet_NaN()};

      /// \brief This frame's target holds the clamped depth (set in Render;
      /// false while particles are in view)
      protected: bool gpuClamped{false};

      /// \brief Color pass target, created on demand for the point cloud
      protected: FilamentRenderTexturePtr colorTexture;

      /// \brief Depth image sent to subscribers, one value per pixel: +inf
      /// beyond far, -inf nearer than near.
      protected: std::vector<float> depthImage;

      /// \brief Colored point cloud sent to subscribers: x, y, z and the
      /// packed color (as float bits) per pixel.
      protected: std::vector<float> pointCloud;

      /// \brief Depth pass readback when the GPU did not clamp, reused
      protected: std::vector<float> depthRaw;

      /// \brief Color pass readback (RGBA8), reused
      protected: std::vector<unsigned char> colorRaw;

      /// \brief Half resolution particle depth, reused
      protected: std::vector<float> particleDepth;

      /// \brief Ray slope per column, used by PostRender
      protected: std::vector<double> cachedColK;

      /// \brief Ray slope per row, used by PostRender
      protected: std::vector<double> cachedRowK;

      /// \brief The width, height, cx, cy, fx, fy that cachedColK and
      /// cachedRowK were computed for (NaN: none)
      protected: std::array<double, 6> cachedRayKey{
          std::numeric_limits<double>::quiet_NaN()};

      /// \brief Event triggered with every new depth image.
      protected: common::EventT<void(const float *, unsigned int,
          unsigned int, unsigned int, const std::string &)> newDepthFrame;

      /// \brief Event triggered with every new colored point cloud.
      protected: common::EventT<void(const float *, unsigned int,
          unsigned int, unsigned int, const std::string &)> newRgbPointCloud;

      /// \brief Only the scene can create this class.
      private: friend class FilamentScene;
    };
    }
  }
}
#endif
