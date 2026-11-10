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
#ifndef GZ_RENDERING_FILAMENT_FILAMENTRENDERTARGET_HH_
#define GZ_RENDERING_FILAMENT_FILAMENTRENDERTARGET_HH_

#include <map>
#include <set>
#include <vector>

#include <math/mat4.h>
#include <math/vec4.h>

#include "gz/rendering/config.hh"
#include "gz/rendering/base/BaseRenderTarget.hh"
#include "gz/rendering/filament/FilamentObject.hh"
#include "gz/rendering/filament/FilamentRenderTypes.hh"
#include "gz/rendering/filament/Export.hh"

namespace filament
{
  class RenderTarget;
  class Texture;
  class View;
  class Viewport;
}

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    //
    class FilamentProjector;

    /// \brief Offscreen color+depth attachment pair. Rendering goes through
    /// Renderer::renderStandaloneView (Renderer::render inside
    /// FilamentRenderEngine::BeginFrame/EndFrame), readback through
    /// readPixels.
    class GZ_RENDERING_FILAMENT_VISIBLE FilamentRenderTarget :
      public virtual BaseRenderTarget<FilamentObject>
    {
      protected: FilamentRenderTarget();

      public: virtual ~FilamentRenderTarget();

      public: virtual void Destroy() override;

      public: virtual void Copy(Image &_image) const override;

      /// \brief Render _view into this target: a standalone frame (its own
      /// submit), or a pass of the frame opened by
      /// FilamentRenderEngine::BeginFrame
      /// \param[in] _viewport Region to draw into; nullptr: whole target
      /// \param[in] _clear Clear the color attachment first (the whole
      /// attachment, on every backend); false keeps what is there
      public: void Render(filament::View *_view,
          const filament::Viewport *_viewport = nullptr, bool _clear = true);

      /// \brief Raw clear color written to the attachment before rendering
      public: void SetClearColor(const filament::math::float4 &_color);

      /// \brief Single-sample target holding the colour, for readPixels
      /// (the main target when it is not multisampled)
      public: filament::RenderTarget *Native() const;

      /// \brief Color attachment, e.g. to sample it in a later pass
      public: filament::Texture *ColorTexture() const;

      /// \brief Run the enabled filament passes of _passes (Gaussian noise)
      /// over the color attachment, in order. Afterwards Native() and
      /// ColorTexture() name the attachments holding the result.
      public: void ApplyRenderPasses(const std::vector<RenderPassPtr> &_passes);

      /// \brief Blend the projectors' textures over the color attachment,
      /// for the projectors visible to _visibilityMask. Call right after
      /// rendering the scene: the scene depth is still in the depth
      /// attachment.
      /// \param[in] _invViewProj Camera clip space to world
      public: void ApplyProjectors(
          const std::set<FilamentProjector *> &_projectors,
          const filament::math::mat4f &_invViewProj,
          uint32_t _visibilityMask);

      /// \brief Passes added with AddRenderPass
      public: const std::vector<RenderPassPtr> &RenderPasses() const;

      /// \brief 8 bit formats are sRGB encoded by default (camera images);
      /// false stores values as is (e.g. segmentation labels)
      public: void SetSrgb(bool _srgb);

      /// \brief GL name of the colour texture in the caller's GL context, 0
      /// unless the engine shares it (FilamentRenderEngine::
      /// SharesCallerGLContext) and the target is 8 bit colour
      public: unsigned int SharedGLId() const;

      protected: virtual void RebuildImpl() override;

      protected: void DestroyTarget();

      /// \brief MSAA sample count of the main colour + depth target (the
      /// driver clamps it to what the device supports; 1 = off). Also
      /// makes the main attachments current again after a render pass
      /// swapped in the scratch ones. Takes effect on the next Rebuild.
      public: void SetSamples(uint8_t _samples);

      /// \brief Create a color + depth attachment pair for the current
      /// size and format
      private: void BuildAttachments(filament::RenderTarget *&_target,
          filament::Texture *&_color, filament::Texture *&_depth);

      protected: filament::Texture *colorTexture{nullptr};

      protected: filament::Texture *depthTexture{nullptr};

      protected: filament::RenderTarget *target{nullptr};

      /// \brief Second attachment set for ApplyRenderPasses, built on
      /// first use
      private: filament::RenderTarget *scratchTarget{nullptr};

      private: filament::Texture *scratchColor{nullptr};

      private: filament::Texture *scratchDepth{nullptr};

      /// \brief True while ApplyRenderPasses left the scratch attachments
      /// current
      private: bool swapped{false};

      /// \brief See SetSamples
      private: uint8_t samples{1};

      /// \brief Colour texture created in the caller's GL context and
      /// imported into filament (see SharedGLId), 0 if none
      private: unsigned int glColorId{0};

      /// \brief The filament texture wrapping glColorId
      private: filament::Texture *glColor{nullptr};

      /// \brief Targets with just a color attachment, for drawing over what
      /// the scene rendered while sampling its depth (ApplyProjectors); by
      /// color texture
      private: std::map<filament::Texture *, filament::RenderTarget *>
          colorTargets;

      protected: filament::math::float4 clearColor{0.0f, 0.0f, 0.0f, 1.0f};

      protected: bool srgb{true};

      /// \brief Readback staging buffer of Copy(), kept across frames
      private: mutable std::vector<unsigned char> readBuffer;
    };

    class GZ_RENDERING_FILAMENT_VISIBLE FilamentRenderTexture :
      public virtual BaseRenderTexture<FilamentRenderTarget>
    {
      protected: FilamentRenderTexture();

      public: virtual ~FilamentRenderTexture();

      /// \brief SharedGLId()
      public: virtual unsigned int GLId() const override;

      private: friend class FilamentScene;
    };
    }
  }
}
#endif
