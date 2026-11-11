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

#include <algorithm>
#include <cstring>
#include <utility>
#include <vector>

#include <filament/Camera.h>
#include <filament/Engine.h>
#include <filament/LightManager.h>
#include <filament/RenderTarget.h>
#include <filament/Renderer.h>
#include <filament/Texture.h>
#include <filament/TextureSampler.h>
#include <filament/View.h>
#include <filament/Viewport.h>
#include <backend/PixelBufferDescriptor.h>

#include <EGL/egl.h>
#include <GL/gl.h>
#include <GL/glext.h>

#include <gz/common/Console.hh>

#include "gz/rendering/Image.hh"
#include "gz/rendering/PixelFormat.hh"
#include "gz/rendering/filament/FilamentGaussianNoisePass.hh"
#include "gz/rendering/filament/FilamentLensFlarePass.hh"
#include "gz/rendering/filament/FilamentProjector.hh"
#include "gz/rendering/filament/FilamentRenderEngine.hh"
#include "gz/rendering/Utils.hh"
#include "gz/rendering/filament/FilamentRenderTarget.hh"
#include "FilamentFullscreenPass.hh"

using namespace gz;
using namespace rendering;

namespace
{
  filament::Engine *Engine()
  {
    return FilamentRenderEngine::Instance()->FilamentEngine();
  }

  /// \brief GL entry point of the caller's context, current on this thread
  template <typename T>
  T CallerGL(const char *_name)
  {
    return reinterpret_cast<T>(eglGetProcAddress(_name));
  }
}

//////////////////////////////////////////////////
FilamentRenderTarget::FilamentRenderTarget() = default;

//////////////////////////////////////////////////
FilamentRenderTarget::~FilamentRenderTarget() = default;

//////////////////////////////////////////////////
void FilamentRenderTarget::Destroy()
{
  this->DestroyTarget();
  this->flipPass.reset();
  BaseRenderTarget::Destroy();
}

//////////////////////////////////////////////////
void FilamentRenderTarget::DestroyTarget()
{
  auto *engine = Engine();
  if (!engine)
    return;
  for (auto *t : {this->target, this->scratchTarget, this->glColorTarget})
  {
    if (t)
      engine->destroy(t);
  }
  for (auto *t : {this->colorTexture, this->depthTexture,
                  this->scratchColor, this->scratchDepth, this->glColor})
  {
    if (t)
      engine->destroy(t);
  }
  for (auto &[texture, t] : this->colorTargets)
    engine->destroy(t);
  this->colorTargets.clear();
  if (this->glColorId)
  {
    // filament releases the import on its driver thread: wait for that
    engine->flushAndWait();
    CallerGL<decltype(&glDeleteTextures)>("glDeleteTextures")(
        1, &this->glColorId);
    this->glColorId = 0;
    this->glColor = nullptr;
  }
  this->target = nullptr;
  this->glColorTarget = nullptr;
  this->colorTexture = nullptr;
  this->depthTexture = nullptr;
  this->scratchTarget = nullptr;
  this->swapped = false;
  this->scratchColor = nullptr;
  this->scratchDepth = nullptr;
}

//////////////////////////////////////////////////
void FilamentRenderTarget::SetSamples(uint8_t _samples)
{
  // the scene renders into the (multisampled) main attachments: undo the
  // last frame's ping-pong, the scene clears them anyway
  if (this->swapped)
  {
    std::swap(this->target, this->scratchTarget);
    std::swap(this->colorTexture, this->scratchColor);
    std::swap(this->depthTexture, this->scratchDepth);
    this->swapped = false;
  }
  _samples = std::max<uint8_t>(_samples, 1u);
  if (this->samples == _samples)
    return;
  this->samples = _samples;
  this->targetDirty = true;
}

//////////////////////////////////////////////////
filament::Texture *FilamentRenderTarget::ColorTexture() const
{
  return this->colorTexture;
}

//////////////////////////////////////////////////
void FilamentRenderTarget::RebuildImpl()
{
  this->DestroyTarget();
  if (this->width == 0u || this->height == 0u)
    return;
  this->BuildAttachments(this->target, this->colorTexture,
      this->depthTexture, true);
  this->BuildSharedTexture();
  // readPixels must not read the multisampled target: on GL with
  // EXT_multisampled_render_to_texture (NVIDIA) filament resolves the colour
  // implicitly but the depth through a sidecar FBO, and readPixels reads
  // that FBO, which has no colour attachment (black/garbage images). Read
  // the resolved colour through a single-sample colour-only target.
  if (this->samples > 1u && this->colorTexture)
  {
    this->colorTargets.emplace(this->colorTexture,
        filament::RenderTarget::Builder()
            .texture(filament::RenderTarget::AttachmentPoint::COLOR,
                this->colorTexture)
            .build(*Engine()));
  }
}

//////////////////////////////////////////////////
unsigned int FilamentRenderTarget::SharedGLId() const
{
  return this->glColorId;
}

//////////////////////////////////////////////////
void FilamentRenderTarget::BuildSharedTexture()
{
  auto *gzEngine = FilamentRenderEngine::Instance();
  const bool shared = gzEngine->SharesCallerGLContext() &&
      eglGetCurrentContext() != EGL_NO_CONTEXT;
  // Not shared, e.g. Qt on GLX: the texture is filled from a readback, in
  // the context current on this thread (any API: the entry points dispatch
  // to it)
  const bool copied = !shared && gzEngine->UsesCallerGLContext() &&
      CallerGL<decltype(&glGetString)>("glGetString")(GL_VERSION) != nullptr;
  if (!this->colorTexture || (!shared && !copied))
    return;
  const auto format = this->colorTexture->getFormat();
  if (format != filament::Texture::InternalFormat::RGBA8 &&
      format != filament::Texture::InternalFormat::SRGB8_A8)
  {
    return;
  }
  // The caller's context is current here: allocate the texture in it, so
  // the caller can sample it, and import it into filament to draw into
  GLuint id = 0;
  CallerGL<decltype(&glGenTextures)>("glGenTextures")(1, &id);
  CallerGL<decltype(&glBindTexture)>("glBindTexture")(GL_TEXTURE_2D, id);
  CallerGL<PFNGLTEXSTORAGE2DPROC>("glTexStorage2D")(GL_TEXTURE_2D, 1,
      format == filament::Texture::InternalFormat::RGBA8 ?
          GL_RGBA8 : GL_SRGB8_ALPHA8,
      static_cast<GLsizei>(this->width), static_cast<GLsizei>(this->height));
  CallerGL<decltype(&glBindTexture)>("glBindTexture")(GL_TEXTURE_2D, 0);
  // filament's context must see the storage before it imports the name
  CallerGL<decltype(&glFlush)>("glFlush")();
  this->glColorId = id;
  if (copied)
  {
    gzdbg << "Filament colour target copied to the caller's GL context: "
          << "texture " << id << " (" << this->width << "x" << this->height
          << ")" << std::endl;
    return;
  }
  auto *engine = Engine();
  this->glColor = filament::Texture::Builder()
      .width(this->width).height(this->height).levels(1)
      .format(format)
      .usage(filament::Texture::Usage::COLOR_ATTACHMENT |
             filament::Texture::Usage::SAMPLEABLE)
      .import(static_cast<intptr_t>(id))
      .build(*engine);
  this->glColorTarget = filament::RenderTarget::Builder()
      .texture(filament::RenderTarget::AttachmentPoint::COLOR, this->glColor)
      .build(*engine);
  gzdbg << "Filament colour target in the caller's GL context: texture "
        << id << " (" << this->width << "x" << this->height << ")"
        << std::endl;
}

//////////////////////////////////////////////////
void FilamentRenderTarget::UpdateSharedTexture()
{
  if (!this->glColorId || !this->target)
    return;
  if (!this->glColorTarget)
  {
    // not shared: read back (top row first) and upload
    if (this->glColorImage.Width() != this->width ||
        this->glColorImage.Height() != this->height)
    {
      this->glColorImage = Image(this->width, this->height, PF_R8G8B8A8);
    }
    this->Copy(this->glColorImage);
    CallerGL<decltype(&glBindTexture)>("glBindTexture")(GL_TEXTURE_2D,
        this->glColorId);
    CallerGL<PFNGLBINDBUFFERPROC>("glBindBuffer")(GL_PIXEL_UNPACK_BUFFER, 0);
    CallerGL<decltype(&glPixelStorei)>("glPixelStorei")(
        GL_UNPACK_ALIGNMENT, 4);
    CallerGL<decltype(&glTexSubImage2D)>("glTexSubImage2D")(GL_TEXTURE_2D,
        0, 0, 0, static_cast<GLsizei>(this->width),
        static_cast<GLsizei>(this->height), GL_RGBA, GL_UNSIGNED_BYTE,
        this->glColorImage.Data<unsigned char>());
    CallerGL<decltype(&glBindTexture)>("glBindTexture")(GL_TEXTURE_2D, 0);
    return;
  }
  if (!FilamentFullscreenPass::Create(this->flipPass, "gz_flip"))
    return;
  // Filament renders bottom row first, like any GL render; the caller
  // shows row 0 at the top. Flipping the scene's projection instead would
  // break filament's screen-space effects (ambient occlusion, shadow
  // filtering, front faces), so copy the finished image upside down.
  this->flipPass->instance->setParameter("colorIn", this->colorTexture,
      filament::TextureSampler());
  filament::RenderTarget *scene = this->target;
  this->target = this->glColorTarget;
  this->Render(this->flipPass->view);
  this->target = scene;
  // the caller samples the texture next
  Engine()->flushAndWait();
}

//////////////////////////////////////////////////
void FilamentRenderTarget::BuildAttachments(filament::RenderTarget *&_target,
    filament::Texture *&_color, filament::Texture *&_depth, bool _main)
{
  auto *engine = Engine();

  filament::Texture::InternalFormat colorFormat;
  switch (this->format)
  {
    case PF_FLOAT32_R:
      colorFormat = filament::Texture::InternalFormat::R32F;
      break;
    case PF_FLOAT32_RGBA:
      colorFormat = filament::Texture::InternalFormat::RGBA32F;
      break;
    case PF_L8:
      // like ogre2, which renders grey scale natively into R8 unorm: the
      // linear red channel, no sRGB encoding
      colorFormat = filament::Texture::InternalFormat::R8;
      break;
    default:
      colorFormat = this->srgb ? filament::Texture::InternalFormat::SRGB8_A8
                               : filament::Texture::InternalFormat::RGBA8;
      break;
  }

  filament::Texture::Builder colorBuilder;
  colorBuilder.width(this->width).height(this->height).levels(1)
      .format(colorFormat)
      .usage(filament::Texture::Usage::COLOR_ATTACHMENT |
             filament::Texture::Usage::SAMPLEABLE |
             filament::Texture::Usage::BLIT_SRC);
  _color = colorBuilder.build(*engine);
  _depth = filament::Texture::Builder()
      .width(this->width).height(this->height).levels(1)
      .format(filament::Texture::InternalFormat::DEPTH32F)
      // sampled by the projectors
      .usage(filament::Texture::Usage::DEPTH_ATTACHMENT |
             filament::Texture::Usage::SAMPLEABLE)
      .build(*engine);
  // Only the main target is multisampled: filament renders straight into
  // it and resolves once into _color/_depth (like ogre2's fsaa target),
  // instead of view MSAA's intermediate buffer + resolve + blit. Scratch
  // targets (full screen passes) stay single sample.
  _target = filament::RenderTarget::Builder()
      .texture(filament::RenderTarget::AttachmentPoint::COLOR, _color)
      .texture(filament::RenderTarget::AttachmentPoint::DEPTH, _depth)
      .samples(_main ? this->samples : 1u)
      .build(*engine);
}

//////////////////////////////////////////////////
const std::vector<RenderPassPtr> &FilamentRenderTarget::RenderPasses() const
{
  return this->renderPasses;
}

//////////////////////////////////////////////////
void FilamentRenderTarget::ApplyRenderPasses(
    const std::vector<RenderPassPtr> &_passes)
{
  for (const RenderPassPtr &pass : _passes)
  {
    if (!pass->IsEnabled() || !this->target)
      continue;
    filament::View *view = nullptr;
    if (auto *noise = dynamic_cast<FilamentGaussianNoisePass *>(pass.get()))
      view = noise->ColorView(this->colorTexture);
    else if (auto *flare = dynamic_cast<FilamentLensFlarePass *>(pass.get()))
      view = flare->ColorView(this->colorTexture);
    if (!view)
      continue;
    // ping-pong: draw from the current attachments into a second set, which
    // then becomes the current one
    if (!this->scratchTarget)
    {
      this->BuildAttachments(this->scratchTarget, this->scratchColor,
          this->scratchDepth, false);
    }
    std::swap(this->target, this->scratchTarget);
    std::swap(this->colorTexture, this->scratchColor);
    std::swap(this->depthTexture, this->scratchDepth);
    this->swapped = !this->swapped;
    this->Render(view);
  }
}

//////////////////////////////////////////////////
void FilamentRenderTarget::RenderDepth(filament::View *_view)
{
  if (!this->target)
    return;
  auto it = this->colorTargets.find(this->depthTexture);
  if (it == this->colorTargets.end())
  {
    it = this->colorTargets.emplace(this->depthTexture,
        filament::RenderTarget::Builder()
            .texture(filament::RenderTarget::AttachmentPoint::DEPTH,
                this->depthTexture)
            .build(*Engine())).first;
  }
  filament::RenderTarget *scene = this->target;
  this->target = it->second;
  this->Render(_view);
  this->target = scene;
}

//////////////////////////////////////////////////
void FilamentRenderTarget::ApplyProjectors(
    const std::set<FilamentProjector *> &_projectors,
    const filament::math::mat4f &_invViewProj, uint32_t _visibilityMask)
{
  if (!this->target)
    return;
  filament::RenderTarget *colorOnly = nullptr;
  for (FilamentProjector *projector : _projectors)
  {
    if (!projector->VisibleTo(_visibilityMask))
      continue;
    filament::View *view = projector->DecalView(_invViewProj,
        this->depthTexture);
    if (!view)
      continue;
    if (!colorOnly)
    {
      auto it = this->colorTargets.find(this->colorTexture);
      if (it == this->colorTargets.end())
      {
        it = this->colorTargets.emplace(this->colorTexture,
            filament::RenderTarget::Builder()
                .texture(filament::RenderTarget::AttachmentPoint::COLOR,
                    this->colorTexture)
                .build(*Engine())).first;
      }
      colorOnly = it->second;
    }
    // blend over the scene: no clear, the depth is only sampled
    filament::RenderTarget *scene = this->target;
    this->target = colorOnly;
    this->Render(view, nullptr, false);
    this->target = scene;
  }
}

//////////////////////////////////////////////////
void FilamentRenderTarget::SetClearColor(const filament::math::float4 &_color)
{
  this->clearColor = _color;
}

//////////////////////////////////////////////////
filament::RenderTarget *FilamentRenderTarget::Native() const
{
  if (this->samples > 1u)
  {
    // the colour-only target of the resolved colour (RebuildImpl), unless
    // a render pass swapped in the single-sample scratch target
    auto it = this->colorTargets.find(this->colorTexture);
    if (it != this->colorTargets.end())
      return it->second;
  }
  return this->target;
}

namespace
{
  /// \brief ogre2 ends directional shadows at 500 m (PSSM, 3 splits,
  /// lambda 0.95 over near..min(far, 500)). Filament takes shadowFar per
  /// light and otherwise spreads the cascades to the camera far plane (50 km
  /// in gz-sim's GUI: no visible shadows). Fit every directional light to
  /// the camera about to render; cameras within 500 m keep FilamentLight's
  /// defaults (shadowFar 0, splits for 0.1..100 m).
  void FitDirectionalShadows(filament::Engine &_engine,
      const filament::Camera &_camera)
  {
    constexpr float kShadowFar = 500.0f;
    const bool clipped = _camera.getCullingFar() > kShadowFar;
    filament::LightManager::ShadowOptions fit;
    fit.shadowFar = clipped ? kShadowFar : 0.0f;
    filament::LightManager::ShadowCascades::computePracticalSplits(
        fit.cascadeSplitPositions, 3u,
        clipped ? static_cast<float>(_camera.getNear()) : 0.1f,
        clipped ? kShadowFar : 100.0f, 0.95f);
    auto &lm = _engine.getLightManager();
    const ::utils::Entity *lights = lm.getEntities();
    for (size_t i = 0; i < lm.getComponentCount(); ++i)
    {
      auto inst = lm.getInstance(lights[i]);
      if (!lm.isDirectional(inst))
        continue;
      filament::LightManager::ShadowOptions o = lm.getShadowOptions(inst);
      const bool sameFar = !(o.shadowFar < fit.shadowFar) &&
          !(o.shadowFar > fit.shadowFar);
      if (sameFar && std::equal(o.cascadeSplitPositions,
              o.cascadeSplitPositions + 3, fit.cascadeSplitPositions))
        continue;
      o.shadowFar = fit.shadowFar;
      std::copy(fit.cascadeSplitPositions, fit.cascadeSplitPositions + 3,
          o.cascadeSplitPositions);
      lm.setShadowOptions(inst, o);
    }
  }
}

//////////////////////////////////////////////////
void FilamentRenderTarget::Render(filament::View *_view,
    const filament::Viewport *_viewport, bool _clear)
{
  if (!this->target || !_view)
    return;
  // the caller may render from another thread than the one that created
  // the engine (gz-gui's render thread); no-op once adopted
  FilamentRenderEngine::Instance()->AdoptThread();
  auto *renderer = FilamentRenderEngine::Instance()->Renderer();
  filament::Renderer::ClearOptions clear;
  clear.clearColor = this->clearColor;
  clear.clear = _clear;
  clear.discard = _clear;
  renderer->setClearOptions(clear);
  _view->setRenderTarget(this->target);
  _view->setViewport(_viewport ? *_viewport :
      filament::Viewport{0, 0, this->width, this->height});
  auto *engine = FilamentRenderEngine::Instance();
  if (_view->isShadowingEnabled() && _view->hasCamera())
    FitDirectionalShadows(*engine->FilamentEngine(), _view->getCamera());
  if (engine->InFrame())
    renderer->render(_view);
  else
    renderer->renderStandaloneView(_view);
}

//////////////////////////////////////////////////
void FilamentRenderTarget::SetSrgb(bool _srgb)
{
  if (this->srgb == _srgb)
    return;
  this->srgb = _srgb;
  this->targetDirty = true;
}

//////////////////////////////////////////////////
void FilamentRenderTarget::Copy(Image &_image) const
{
  if (!this->target)
  {
    gzerr << "Render target not built" << std::endl;
    return;
  }
  if (_image.Width() != this->width || _image.Height() != this->height)
  {
    gzerr << "Invalid image dimensions" << std::endl;
    return;
  }

  const PixelFormat dstFormat = _image.Format();
  if (dstFormat == PF_BAYER_RGGB8 || dstFormat == PF_BAYER_BGGR8 ||
      dstFormat == PF_BAYER_GBRG8 || dstFormat == PF_BAYER_GRBG8)
  {
    // render target holds RGB: read that back, then mosaic on the CPU
    if (this->bayerSource.Width() != this->width ||
        this->bayerSource.Height() != this->height)
    {
      this->bayerSource = Image(this->width, this->height, PF_R8G8B8);
    }
    this->Copy(this->bayerSource);
    convertRGBToBayer(this->bayerSource, PixelBuffer(_image));
    return;
  }

  using PF = filament::backend::PixelDataFormat;
  using PT = filament::backend::PixelDataType;
  PF readFormat;
  PT readType;
  size_t readBpp;
  switch (this->format)
  {
    case PF_FLOAT32_R:
      readFormat = PF::R; readType = PT::FLOAT; readBpp = 4u;
      break;
    case PF_FLOAT32_RGBA:
      readFormat = PF::RGBA; readType = PT::FLOAT; readBpp = 16u;
      break;
    case PF_L8:
      readFormat = PF::R; readType = PT::UBYTE; readBpp = 1u;
      break;
    default:
      readFormat = PF::RGBA; readType = PT::UBYTE; readBpp = 4u;
      break;
  }

  // Offscreen render targets read back top row first on both the Vulkan and
  // OpenGL backends (verified empirically), matching gz image layout.
  const size_t rowBytes = readBpp * this->width;
  unsigned char *dst = _image.Data<unsigned char>();
  const PixelFormat imgFormat = _image.Format();
  const size_t dstBpp = PixelUtil::BytesPerPixel(imgFormat);
  // same layout: straight into the image
  const bool direct = dstBpp == readBpp && _image.Width() == this->width &&
      _image.Height() == this->height;
  // resize keeps the allocation (and skips zeroing) from frame to frame
  std::vector<unsigned char> &buf = this->readBuffer;
  if (!direct)
    buf.resize(rowBytes * this->height);
  filament::backend::PixelBufferDescriptor desc(direct ? dst : buf.data(),
      rowBytes * this->height, readFormat, readType);
  auto *renderer = FilamentRenderEngine::Instance()->Renderer();
  renderer->readPixels(this->Native(), 0, 0, this->width, this->height,
      std::move(desc));
  Engine()->flushAndWait();
  if (direct)
    return;
  for (unsigned int y = 0; y < this->height; ++y)
  {
    const unsigned char *src =
        buf.data() + static_cast<size_t>(y) * rowBytes;
    unsigned char *row = dst + static_cast<size_t>(y) * this->width * dstBpp;
    if (imgFormat == PF_R8G8B8 && readBpp == 4u)
    {
      // one 4-byte store per pixel; the next pixel overwrites the alpha
      for (unsigned int x = 0; x + 1u < this->width; ++x)
        std::memcpy(row + x * 3u, src + x * 4u, 4u);
      std::memcpy(row + (this->width - 1u) * 3u,
          src + (this->width - 1u) * 4u, 3u);
    }
    else if (imgFormat == PF_B8G8R8 && readBpp == 4u)
    {
      for (unsigned int x = 0; x < this->width; ++x)
      {
        row[x * 3u + 0u] = src[x * 4u + 2u];
        row[x * 3u + 1u] = src[x * 4u + 1u];
        row[x * 3u + 2u] = src[x * 4u + 0u];
      }
    }
    else if (dstBpp == readBpp)
    {
      std::memcpy(row, src, rowBytes);
    }
    else
    {
      gzerr << "Unsupported image format for copy: " << imgFormat
            << std::endl;
      return;
    }
  }
}

//////////////////////////////////////////////////
unsigned int FilamentRenderTexture::GLId() const
{
  return this->SharedGLId();
}
