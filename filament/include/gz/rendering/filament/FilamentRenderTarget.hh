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
#include <memory>
#include <set>
#include <vector>

#include <math/mat4.h>
#include <math/vec4.h>

#include "gz/rendering/config.hh"
#include "gz/rendering/Image.hh"
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
    class FilamentFullscreenPass;
    class FilamentProjector;

    /// \brief Offscreen color+depth attachment pair. Rendering goes through
    /// Renderer::renderStandaloneView (Renderer::render inside
    /// FilamentRenderEngine::BeginFrame/EndFrame), readback through
    /// readPixels.
    class GZ_RENDERING_FILAMENT_VISIBLE FilamentRenderTarget :
      public virtual BaseRenderTarget<FilamentObject>
    {
      /// \brief Constructor
      protected: FilamentRenderTarget();

      /// \brief Destructor
      public: virtual ~FilamentRenderTarget();

      // Documentation inherited
      public: virtual void Destroy() override;

      // Documentation inherited
      public: virtual void Copy(Image &_image) const override;

      /// \brief Render _view into this target: a standalone frame (its own
      /// submit), or a pass of the frame opened by
      /// FilamentRenderEngine::BeginFrame
      /// \param[in] _view View to render
      /// \param[in] _viewport Region to draw into; nullptr: whole target
      /// \param[in] _clear Clear the color attachment first (the whole
      /// attachment, on every backend); false keeps what is there
      public: void Render(filament::View *_view,
          const filament::Viewport *_viewport = nullptr, bool _clear = true);

      /// \brief Set the raw clear color written to the attachment before
      /// rendering.
      /// \param[in] _color RGBA clear color
      public: void SetClearColor(const filament::math::float4 &_color);

      /// \brief Single-sample target holding the colour, for readPixels
      /// (the main target when it is not multisampled)
      /// \return The target to read the colour from
      public: filament::RenderTarget *Native() const;

      /// \brief Color attachment, e.g. to sample it in a later pass
      /// \return The current color attachment
      public: filament::Texture *ColorTexture() const;

      /// \brief Run the enabled filament passes of _passes (Gaussian noise
      /// and lens flare; other passes are skipped) over the color
      /// attachment, in order. Afterwards Native() and ColorTexture() name
      /// the attachments holding the result.
      /// \param[in] _passes Passes to run
      public: void ApplyRenderPasses(const std::vector<RenderPassPtr> &_passes);

      /// \brief Blend the projectors' textures over the color attachment,
      /// for the projectors visible to _visibilityMask. Call right after
      /// rendering the scene: the scene depth is still in the depth
      /// attachment.
      /// \param[in] _projectors Projectors to blend
      /// \param[in] _invViewProj Camera clip space to world
      /// \param[in] _visibilityMask Visibility mask of the camera, only the
      /// projectors visible to it are blended
      public: void ApplyProjectors(
          const std::set<FilamentProjector *> &_projectors,
          const filament::math::mat4f &_invViewProj,
          uint32_t _visibilityMask);

      /// \brief Render _view's depth alone into the depth attachment, for
      /// ApplyProjectors after a post-processed render (which leaves it
      /// untouched)
      /// \param[in] _view View to render
      public: void RenderDepth(filament::View *_view);

      /// \brief Passes added with AddRenderPass
      /// \return The passes, in the order they were added
      public: const std::vector<RenderPassPtr> &RenderPasses() const;

      /// \brief Set whether 8 bit formats are sRGB encoded. They are by
      /// default (camera images); false stores values as is (e.g.
      /// segmentation labels). Takes effect on the next Rebuild.
      /// \param[in] _srgb True for sRGB encoded 8 bit formats
      public: void SetSrgb(bool _srgb);

      /// \brief GL name of a texture in the caller's GL context holding the
      /// image top row first (row 0 at the top, like ogre2's). There is one
      /// only if the engine shares the caller's context or renders for it
      /// (see FilamentRenderEngine::SharesCallerGLContext and
      /// UsesCallerGLContext), the target is 8 bit colour and the caller
      /// shows it (UpdateSharedTexture creates and fills it; sensor targets
      /// never get one).
      /// \return The GL texture name, 0 if there is none
      public: unsigned int SharedGLId() const;

      /// \brief Copy the rendered image into SharedGLId's texture, top row
      /// first, and wait for the GPU, so the caller can sample it: upside
      /// down with a last pass (filament renders bottom row first) if the
      /// context is shared, else read back and uploaded in the caller's
      /// context, which must be current. Does nothing without the texture.
      public: void UpdateSharedTexture();

      /// \brief Implementation of the Rebuild function: destroy the
      /// attachments and build them again for the current size, format,
      /// sample count and sRGB setting. A target that was presented
      /// (UpdateSharedTexture) gets its caller's GL context texture again.
      protected: virtual void RebuildImpl() override;

      /// \brief Destroy the attachments, the filament targets built on them
      /// and the texture in the caller's GL context.
      protected: void DestroyTarget();

      /// \brief MSAA sample count of the main colour + depth target (the
      /// driver clamps it to what the device supports; 1 = off). Also
      /// makes the main attachments current again after a render pass
      /// swapped in the scratch ones. Takes effect on the next Rebuild.
      /// \param[in] _samples Sample count, at least 1
      public: void SetSamples(uint8_t _samples);

      /// \brief Create a color + depth attachment pair for the current
      /// size and format
      /// \param[out] _target Render target on the new attachments
      /// \param[out] _color Color attachment
      /// \param[out] _depth Depth attachment, sampleable
      /// \param[in] _main The main attachments (multisampled), else the
      /// scratch ones of render passes
      private: void BuildAttachments(filament::RenderTarget *&_target,
          filament::Texture *&_color, filament::Texture *&_depth, bool _main);

      /// \brief Create SharedGLId's texture in the caller's GL context, if
      /// the engine shares it or renders for it, the context is current and
      /// the colour is 8 bit
      private: void BuildSharedTexture();

      /// \brief Color attachment of target. Swapped with scratchColor by
      /// ApplyRenderPasses.
      protected: filament::Texture *colorTexture{nullptr};

      /// \brief Depth attachment of target, sampled by the projectors.
      /// Swapped with scratchDepth by ApplyRenderPasses.
      protected: filament::Texture *depthTexture{nullptr};

      /// \brief Target the scene is rendered into. Swapped with
      /// scratchTarget by ApplyRenderPasses.
      protected: filament::RenderTarget *target{nullptr};

      /// \brief Second attachment set for ApplyRenderPasses, built on
      /// first use
      private: filament::RenderTarget *scratchTarget{nullptr};

      /// \brief Color attachment of scratchTarget
      private: filament::Texture *scratchColor{nullptr};

      /// \brief Depth attachment of scratchTarget
      private: filament::Texture *scratchDepth{nullptr};

      /// \brief True while ApplyRenderPasses left the scratch attachments
      /// current
      private: bool swapped{false};

      /// \brief See SetSamples
      private: uint8_t samples{1};

      /// \brief UpdateSharedTexture was called: the caller shows this
      /// target, RebuildImpl recreates glColorId
      private: bool presented{false};

      /// \brief Colour texture created in the caller's GL context (see
      /// SharedGLId), 0 if none; imported into filament if shared
      private: unsigned int glColorId{0};

      /// \brief The filament texture wrapping glColorId
      private: filament::Texture *glColor{nullptr};

      /// \brief Colour-only target on glColor, for UpdateSharedTexture
      private: filament::RenderTarget *glColorTarget{nullptr};

      /// \brief Draws the colour attachment upside down into glColorTarget
      private: std::unique_ptr<FilamentFullscreenPass> flipPass;

      /// \brief Readback uploaded to glColorId when the caller's context
      /// is not shared, kept across frames
      private: Image glColorImage;

      /// \brief Targets with just a color attachment, for drawing over what
      /// the scene rendered while sampling its depth (ApplyProjectors), or
      /// just the depth one (RenderDepth); by texture
      private: std::map<filament::Texture *, filament::RenderTarget *>
          colorTargets;

      /// \brief Color the color attachment is cleared to before rendering,
      /// see SetClearColor
      protected: filament::math::float4 clearColor{0.0f, 0.0f, 0.0f, 1.0f};

      /// \brief True when 8 bit formats are sRGB encoded, see SetSrgb
      protected: bool srgb{true};

      /// \brief Readback staging buffer of Copy(), kept across frames
      private: mutable std::vector<unsigned char> readBuffer;

      /// \brief RGB readback of Copy() into a Bayer image, kept across
      /// frames
      private: mutable Image bayerSource;
    };

    /// \brief Filament implementation of the render texture class
    class GZ_RENDERING_FILAMENT_VISIBLE FilamentRenderTexture :
      public virtual BaseRenderTexture<FilamentRenderTarget>
    {
      /// \brief Constructor
      protected: FilamentRenderTexture() = default;

      /// \brief Destructor
      public: virtual ~FilamentRenderTexture() = default;

      /// \brief Same as SharedGLId
      /// \return The GL texture name, 0 if there is none
      public: virtual unsigned int GLId() const override;

      private: friend class FilamentScene;
    };
    }
  }
}
#endif
