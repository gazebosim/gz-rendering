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
      /// \brief Constructor
      public: FilamentGaussianNoisePass();

      /// \brief Destructor
      public: virtual ~FilamentGaussianNoisePass();

      // Documentation inherited
      public: virtual void Destroy() override;

      /// \brief View drawing _colorIn plus noise (ogre2's
      /// gaussian_noise_fs.glsl) into whatever target it is rendered to,
      /// which must have _colorIn's size. A new pattern every call.
      /// \param[in] _colorIn Color texture to add the noise to
      /// \return View to render, or nullptr if the pass could not be
      /// created
      public: filament::View *ColorView(filament::Texture *_colorIn);

      /// \brief ogre2's depth camera noise (gaussian_noise_depth_fs.glsl)
      /// applied to the clamped outputs of a depth camera, for each of its
      /// enabled noise passes: the same sample is added to x, y, z (meters)
      /// and, as sign(z) |z|^2.1, to the cloud's packed sRGB color.
      /// \param[in] _camera Depth camera whose render passes are applied
      /// \param[in,out] _depth ImageWidth x ImageHeight depth image
      /// \param[in,out] _cloud x, y, z, rgba per pixel, or nullptr
      public: static void ApplyDepth(const Camera &_camera, float *_depth,
          float *_cloud);

      /// \brief Draw and read back _width x _height noise samples (mean
      /// and stddev applied), a new pattern every call.
      /// \param[in] _scene Scene that creates the render texture
      /// \param[in] _width Number of samples per row
      /// \param[in] _height Number of rows of samples
      /// \param[out] _out The samples, resized to _width x _height
      private: void Samples(const ScenePtr &_scene, unsigned int _width,
          unsigned int _height, std::vector<float> &_out);

      /// \brief Set the per draw uniforms.
      /// \param[in] _colorIn Texture the pass samples
      /// \param[in] _mode 0 draws _colorIn plus noise (ColorView), 1 draws
      /// the noise samples (Samples)
      private: void SetParameters(filament::Texture *_colorIn, int _mode);

      /// \brief Fullscreen pass drawing gz_noise, created on first use
      private: std::unique_ptr<FilamentFullscreenPass> pass;

      /// \brief RGBA8 target for Samples(): the float bits of z
      private: FilamentRenderTexturePtr samples;

      /// \brief Samples() read back for ApplyDepth, reused
      private: std::vector<float> depthSamples;
    };
    }
  }
}
#endif
