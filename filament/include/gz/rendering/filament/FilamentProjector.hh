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
#ifndef GZ_RENDERING_FILAMENT_FILAMENTPROJECTOR_HH_
#define GZ_RENDERING_FILAMENT_FILAMENTPROJECTOR_HH_

#include <memory>

#include <math/mat4.h>

#include "gz/rendering/config.hh"
#include "gz/rendering/base/BaseProjector.hh"
#include "gz/rendering/filament/FilamentRenderTypes.hh"
#include "gz/rendering/filament/FilamentVisual.hh"
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

    /// \brief Screen space decal like ogre2's: the texture is projected onto
    /// whatever lies inside the box that bounds the projector's frustum
    /// (x from the near to the far plane, as wide and tall as the frustum
    /// at the far plane), seen by the cameras whose visibility mask matches.
    /// Filament has no decals: FilamentRenderTarget::ApplyProjectors blends
    /// the texture over the camera image, rebuilding each pixel's world
    /// position from the scene depth (gz_projector.mat).
    class GZ_RENDERING_FILAMENT_VISIBLE FilamentProjector :
      public BaseProjector<FilamentVisual>
    {
      protected: FilamentProjector();

      public: virtual ~FilamentProjector();

      public: virtual void Init() override;

      public: virtual void PreRender() override;

      public: virtual void Destroy() override;

      public: virtual void SetEnabled(bool _enabled) override;

      /// \brief Is the projector's texture drawn on a camera with
      /// _visibilityMask
      public: bool VisibleTo(uint32_t _visibilityMask) const;

      /// \brief View blending the texture over a target the size of the
      /// camera image, or nullptr if there is nothing to draw.
      /// \param[in] _invViewProj Camera clip space to world
      /// \param[in] _depth Scene depth of the target
      public: filament::View *DecalView(const filament::math::mat4f
          &_invViewProj, filament::Texture *_depth);

      /// \brief (Re)load the texture if its name changed
      private: void LoadTexture();

      /// \brief Fullscreen pass drawing gz_projector, created on first use
      private: std::unique_ptr<FilamentFullscreenPass> pass;

      private: filament::Texture *texture{nullptr};

      private: std::string loadedTexture;

      /// \brief Width over height of the texture
      private: double aspect{1.0};

      private: bool initialized{false};

      private: friend class FilamentScene;
    };
    }
  }
}
#endif
