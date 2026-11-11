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
#ifndef GZ_RENDERING_FILAMENT_FILAMENTLENSFLAREPASS_HH_
#define GZ_RENDERING_FILAMENT_FILAMENTLENSFLAREPASS_HH_

#include <cstdint>
#include <memory>

#include <gz/math/Vector3.hh>

#include "gz/rendering/config.hh"
#include "gz/rendering/base/BaseLensFlarePass.hh"
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

    /// \brief Lens flare with ogre2's semantics: the light is projected with
    /// the camera, occluded by sparse RayQuery casts around it and drawn
    /// over the image by gz_lens_flare.mat (ogre2's lens_flare_fs.glsl).
    /// RGB and wide-angle cameras. A wide-angle camera always gets it on
    /// the stitched image (WideAngleCameraAfterStitching), seen as a 90 degree
    /// pinhole like ogre2; ogre2's per cube face mode is not reproduced.
    class GZ_RENDERING_FILAMENT_VISIBLE FilamentLensFlarePass :
      public BaseLensFlarePass<BaseRenderPass<FilamentObject>>
    {
      public: FilamentLensFlarePass();

      public: virtual ~FilamentLensFlarePass();

      // Documentation inherited
      public: virtual void Destroy() override;

      // Documentation inherited
      public: virtual void Init(ScenePtr _scene) override;

      // Documentation inherited
      public: virtual void PreRender(const CameraPtr &_camera) override;

      // Documentation inherited
      public: virtual void SetScale(double _scale) override;

      // Documentation inherited
      public: virtual double Scale() const override;

      // Documentation inherited
      public: virtual void SetColor(const math::Vector3d &_color) override;

      // Documentation inherited
      public: virtual const math::Vector3d &Color() const override;

      // Documentation inherited
      public: virtual void SetOcclusionSteps(uint32_t _steps) override;

      // Documentation inherited
      public: virtual uint32_t OcclusionSteps() const override;

      /// \brief View drawing _colorIn plus the flare into a target of the
      /// same size, or nullptr when there is nothing to draw (light behind
      /// the camera, fully occluded, no light). Needs PreRender(camera).
      public: filament::View *ColorView(filament::Texture *_colorIn);

      /// \brief Share of the flare left after the occlusion casts, times
      /// the scale. _viewProj maps world to clip space, _ndc is the light
      /// in normalized device coordinates, _lightPos its world position.
      private: double OcclusionScale(const math::Matrix4d &_viewProj,
          const math::Vector3d &_ndc, const math::Vector3d &_lightPos);

      /// \brief Lazily create the material instance, fullscreen triangle,
      /// scene and view; false if the material is missing
      private: bool CreateResources();

      private: math::Vector3d color{1.0, 1.0, 1.0};

      private: double scale{1.0};

      private: double occlusionSteps{10.0};

      /// \brief Camera of the last PreRender
      private: CameraPtr currentCamera;

      /// \brief World position of the light of the last PreRender (a
      /// directional light: far away along its direction)
      private: math::Vector3d lightWorldPos;

      private: RayQueryPtr rayQuery;

      /// \brief Fullscreen pass drawing gz_lens_flare, created on first use
      private: std::unique_ptr<FilamentFullscreenPass> pass;
    };
    }
  }
}
#endif
