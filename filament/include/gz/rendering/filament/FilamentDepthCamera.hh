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
}

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    //
    /// \brief Depth camera: renders every mesh with the gz_depth material
    /// (linear view depth) into an R32F target, then reads it back.
    class GZ_RENDERING_FILAMENT_VISIBLE FilamentDepthCamera :
      public BaseDepthCamera<FilamentCamera>
    {
      protected: FilamentDepthCamera();

      public: virtual ~FilamentDepthCamera();

      // BaseCamera<FilamentCamera> would otherwise win over FilamentCamera
      public: virtual void SetProjectionMatrix(const math::Matrix4d &_matrix)
          override { FilamentCamera::SetProjectionMatrix(_matrix); }

      public: virtual void CreateDepthTexture() override;

      public: virtual void PreRender() override;

      public: virtual void Render() override;

      public: virtual void PostRender() override;

      public: virtual const float *DepthData() const override;

      /// \brief Like ogre2: a custom matrix reports the configured clip
      /// planes in its depth rows
      public: virtual math::Matrix4d ProjectionMatrix() const override;

      public: virtual common::ConnectionPtr ConnectNewDepthFrame(
          std::function<void(const float *, unsigned int, unsigned int,
          unsigned int, const std::string &)> _subscriber) override;

      public: virtual common::ConnectionPtr ConnectNewRgbPointCloud(
          std::function<void(const float *, unsigned int, unsigned int,
          unsigned int, const std::string &)> _subscriber) override;

      public: virtual void Destroy() override;

      protected: virtual void Init() override;

      protected: virtual RenderTargetPtr RenderTarget() const override;

      protected: filament::MaterialInstance *depthInstance{nullptr};

      /// \brief Color pass target, created on demand for the point cloud
      protected: FilamentRenderTexturePtr colorTexture;

      protected: std::vector<float> depthImage;

      protected: std::vector<float> pointCloud;

      /// \brief Readback buffers (depth pass, color pass), reused
      protected: std::vector<float> depthRaw;

      protected: std::vector<unsigned char> colorRaw;

      /// \brief Half resolution particle depth, reused
      protected: std::vector<float> particleDepth;

      /// \brief Ray slope per column and row of PostRender, and the
      /// width, height, cx, cy, fx, fy they were computed for (NaN: none)
      protected: std::vector<double> cachedColK;

      protected: std::vector<double> cachedRowK;

      protected: std::array<double, 6> cachedRayKey{
          std::numeric_limits<double>::quiet_NaN()};

      protected: common::EventT<void(const float *, unsigned int,
          unsigned int, unsigned int, const std::string &)> newDepthFrame;

      protected: common::EventT<void(const float *, unsigned int,
          unsigned int, unsigned int, const std::string &)> newRgbPointCloud;

      private: friend class FilamentScene;
    };
    }
  }
}
#endif
