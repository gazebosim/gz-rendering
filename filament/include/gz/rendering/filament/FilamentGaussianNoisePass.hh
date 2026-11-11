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
#ifndef GZ_RENDERING_FILAMENT_FILAMENTGAUSSIANNOISEPASS_HH_
#define GZ_RENDERING_FILAMENT_FILAMENTGAUSSIANNOISEPASS_HH_

#include <cstdint>
#include <memory>
#include <vector>

#include "gz/rendering/config.hh"
#include "gz/rendering/base/BaseGaussianNoisePass.hh"
#include "gz/rendering/base/BaseRenderPass.hh"
#include "gz/rendering/filament/FilamentObject.hh"
#include "gz/rendering/filament/FilamentRenderTypes.hh"
#include "gz/rendering/filament/Export.hh"

namespace filament
{
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

    /// \brief Gaussian noise with ogre2's semantics (gz_noise.mat).
    /// RGB and wide-angle cameras: FilamentRenderTarget::ApplyRenderPasses
    /// draws ColorView over the color attachment on the GPU. Depth cameras:
    /// the samples are drawn on the GPU and added by ApplyDepth in the CPU
    /// post-step that already clamps and builds the point cloud. Thermal,
    /// segmentation, bounding box cameras and GPU rays keep the pass without
    /// applying it, like ogre2.
    class GZ_RENDERING_FILAMENT_VISIBLE FilamentGaussianNoisePass :
      public BaseGaussianNoisePass<BaseRenderPass<FilamentObject>>
    {
      public: FilamentGaussianNoisePass();

      public: virtual ~FilamentGaussianNoisePass();

      public: virtual void Destroy() override;

      /// \brief View drawing _colorIn plus noise (ogre2's
      /// gaussian_noise_fs.glsl) into whatever target it is rendered to,
      /// which must have _colorIn's size. A new pattern every call.
      public: filament::View *ColorView(filament::Texture *_colorIn);

      /// \brief ogre2's depth camera noise (gaussian_noise_depth_fs.glsl)
      /// applied to the clamped outputs of a depth camera, for each of its
      /// enabled noise passes: the same sample is added to x, y, z (meters)
      /// and, as sign(z) |z|^2.1, to the cloud's packed sRGB color.
      /// \param[in,out] _depth ImageWidth x ImageHeight depth image
      /// \param[in,out] _cloud x, y, z, rgba per pixel, or nullptr
      public: static void ApplyDepth(const Camera &_camera, float *_depth,
          float *_cloud);

      /// \brief Draw and read back w x h noise samples (mean and stddev
      /// applied), a new pattern every call
      private: void Samples(const ScenePtr &_scene, unsigned int _width,
          unsigned int _height, std::vector<float> &_out);

      /// \brief Lazily create the material instance, fullscreen triangle,
      /// scene and view; false if the material is missing
      private: bool CreateResources();

      /// \brief Set the per draw uniforms
      private: void SetParameters(filament::Texture *_colorIn, int _mode);

      /// \brief Fullscreen pass drawing gz_noise, created on first use
      private: std::unique_ptr<FilamentFullscreenPass> pass;

      /// \brief R32F target for Samples()
      private: FilamentRenderTexturePtr samples;

      /// \brief Samples() read back for ApplyDepth, reused
      private: std::vector<float> depthSamples;
    };
    }
  }
}
#endif
