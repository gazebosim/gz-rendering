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
      /// \brief Constructor
      public: FilamentLensFlarePass();

      /// \brief Destructor
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

      /// \brief Draw _colorIn plus the flare into a target of the same size.
      /// Needs PreRender(camera).
      /// \param[in] _colorIn Color texture of the camera image
      /// \return View to render, or nullptr when there is nothing to draw
      /// (light behind the camera, fully occluded, no light)
      public: filament::View *ColorView(filament::Texture *_colorIn);

      /// \brief Share of the flare left after the occlusion casts, times
      /// the scale.
      /// \param[in] _viewProj Matrix from world to clip space
      /// \param[in] _ndc Light in normalized device coordinates, z is
      /// positive in front of the near plane
      /// \param[in] _lightPos Light position in the world frame
      /// \return Scale to draw the flare with, 0 when the light itself is
      /// hidden
      private: double OcclusionScale(const math::Matrix4d &_viewProj,
          const math::Vector3d &_ndc, const math::Vector3d &_lightPos);

      /// \brief Color of the flare (RGB).
      private: math::Vector3d color{1.0, 1.0, 1.0};

      /// \brief Scale of the flare, also scales the area of the occlusion
      /// casts.
      private: double scale{1.0};

      /// \brief Occlusion cast steps along each axis around the light, 0
      /// disables the casts.
      private: double occlusionSteps{10.0};

      /// \brief Camera of the last PreRender
      private: CameraPtr currentCamera;

      /// \brief World position of the light of the last PreRender (a
      /// directional light: far away along its direction)
      private: math::Vector3d lightWorldPos;

      /// \brief CPU ray query for the occlusion casts.
      private: RayQueryPtr rayQuery;

      /// \brief Fullscreen pass drawing gz_lens_flare, created on first use
      private: std::unique_ptr<FilamentFullscreenPass> pass;
    };
    }
  }
}
#endif
