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
#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <limits>
#include <random>
#include <string>
#include <vector>
#if defined(__SSE2__)
#include <emmintrin.h>
#endif

#include <backend/PixelBufferDescriptor.h>
#include <filament/Engine.h>
#include <filament/Material.h>
#include <filament/MaterialInstance.h>
#include <filament/Renderer.h>
#include <filament/Texture.h>
#include <filament/TextureSampler.h>
#include <filament/View.h>

#include <gz/math/Rand.hh>

#include "gz/rendering/filament/FilamentConversions.hh"
#include "gz/rendering/filament/FilamentDepthCamera.hh"
#include "gz/rendering/filament/FilamentGaussianNoisePass.hh"
#include "gz/rendering/filament/FilamentMesh.hh"
#include "gz/rendering/filament/FilamentRenderEngine.hh"
#include "gz/rendering/filament/FilamentRenderTarget.hh"
#include "gz/rendering/filament/FilamentScene.hh"
#include "FilamentParticles.hh"
#include "FilamentUtils.hh"

using namespace gz;
using namespace rendering;

//////////////////////////////////////////////////
void FilamentDepthCamera::Init()
{
  FilamentCamera::Init();
  // no shadow maps for the depth pass (FilamentCamera::Init already turns
  // off the screen-space effects)
  this->view->setShadowingEnabled(false);
}

//////////////////////////////////////////////////
RenderTargetPtr FilamentDepthCamera::RenderTarget() const
{
  return this->renderTexture;
}

//////////////////////////////////////////////////
void FilamentDepthCamera::Destroy()
{
  auto *engine = FilamentRenderEngine::Instance()->FilamentEngine();
  if (engine && this->depthInstance)
    engine->destroy(this->depthInstance);
  this->depthInstance = nullptr;
  if (engine && this->clampInstance)
    engine->destroy(this->clampInstance);
  this->clampInstance = nullptr;
  if (engine && this->farMap)
    engine->destroy(this->farMap);
  this->farMap = nullptr;
  if (this->colorTexture)
  {
    this->colorTexture->Destroy();
    this->colorTexture.reset();
  }
  FilamentCamera::Destroy();
}

//////////////////////////////////////////////////
void FilamentDepthCamera::CreateDepthTexture()
{
  // the depth's float bits as RGBA8, see filament_utils::kPackedInf.
  // RenderTarget()->Copy() of a depth camera then reads the packed bytes;
  // nothing (gz-sensors, tests) copies it.
  if (!this->depthInstance)
  {
    filament::Material *mat =
        FilamentRenderEngine::Instance()->Material("gz_depth");
    filament::Material *clampMat =
        FilamentRenderEngine::Instance()->Material("gz_depth_clamp");
    if (!mat || !clampMat)
      return;
    this->depthInstance = mat->createInstance();
    this->clampInstance = clampMat->createInstance();
  }
  this->renderTexture->SetFormat(PF_R8G8B8A8);
  this->renderTexture->SetSrgb(false);
  this->renderTexture->SetWidth(this->ImageWidth());
  this->renderTexture->SetHeight(this->ImageHeight());
}

namespace
{
  /// \brief The depth pass renders with a closer near plane and a farther far
  /// plane than requested, so geometry just outside [near, far] is still
  /// rasterized and can be reported as -inf / +inf instead of being clipped
  /// (ogre2 gets the near case from depth clamping, which filament only
  /// enables for shadow passes).
  constexpr double kNearScale = 0.1;
  constexpr double kFarScale = 1.1;

  /// \brief ogre2's clamping tolerance: beyond far - tol, nearer than
  /// near + tol
  constexpr double kTol = 1e-6;

  /// \brief The largest r2 with sqrt(r2) <= _far, so that
  /// sqrt(r2) > _far exactly when r2 > FarRangeSq(_far). sqrt is correctly
  /// rounded and monotonic, so this threshold gives the same decision as
  /// the square root, bit for bit, without one per pixel.
  double FarRangeSq(double _far)
  {
    const double maxD = std::numeric_limits<double>::max();
    // NaN: never beyond; negative: every non-NaN r2 is beyond
    if (std::isnan(_far) || _far < 0.0)
      return _far < 0.0 ? -1.0 : _far;
    double s = _far * _far;
    // overflow: only r2 = inf has a root beyond a finite _far
    if (std::isinf(s))
      return std::isinf(_far) ? s : maxD;
    while (s > 0.0 && std::sqrt(s) > _far)
      s = std::nextafter(s, 0.0);
    while (s < maxD && std::sqrt(std::nextafter(s, maxD)) <= _far)
      s = std::nextafter(s, maxD);
    return s;
  }

  /// \brief PostRender's range test of a depth _d on the pixel ray with
  /// slopes _ku, _kv: the same double operations in the same order
  bool BeyondFar(float _d, double _ku, double _kv, double _farSq)
  {
    const double d = _d;
    const double p1 = _ku * d;
    const double p2 = _kv * d;
    return d * d + p1 * p1 + p2 * p2 > _farSq;
  }

  /// \brief The least float depth >= 0 that BeyondFar(_ku, _kv, _farSq)
  /// holds for, +inf if none. The test is monotonic in the depth (every
  /// operation is), so a depth d >= 0 is beyond far exactly when
  /// d >= FarThreshold: gz_depth_clamp decides like PostRender, bit for bit.
  float FarThreshold(double _ku, double _kv, double _farSq)
  {
    const float maxF = std::numeric_limits<float>::max();
    if (!BeyondFar(maxF, _ku, _kv, _farSq))
      return std::numeric_limits<float>::infinity();
    if (BeyondFar(0.0f, _ku, _kv, _farSq))
      return 0.0f;
    // bisect the bits of the non-negative floats (ordered like the floats):
    // BeyondFar(lo) is false, BeyondFar(hi) true. Start next to the root of
    // d^2 (1 + ku^2 + kv^2) = farSq when it brackets the answer.
    auto bits = [](float _f) { uint32_t b; std::memcpy(&b, &_f, 4); return b; };
    auto value = [](uint32_t _b)
        { float f; std::memcpy(&f, &_b, 4); return f; };
    uint32_t lo = 0u;
    uint32_t hi = bits(maxF);
    const float guess = static_cast<float>(
        std::sqrt(_farSq / (1.0 + _ku * _ku + _kv * _kv)));
    if (guess > 0.0f && guess < maxF)
    {
      const uint32_t g = bits(guess);
      const uint32_t a = g > 16u ? g - 16u : 0u;
      const uint32_t b = std::min(g + 16u, hi);
      if (!BeyondFar(value(a), _ku, _kv, _farSq))
        lo = a;
      if (BeyondFar(value(b), _ku, _kv, _farSq))
        hi = b;
    }
    while (hi - lo > 1u)
    {
      const uint32_t mid = lo + (hi - lo) / 2u;
      if (BeyondFar(value(mid), _ku, _kv, _farSq))
        hi = mid;
      else
        lo = mid;
    }
    return value(hi);
  }
}

namespace
{
  /// \brief ogre2's color of clamped points: the gamma-encoded background
  /// as R << 24 | G << 16 | B << 8 | 255
  uint32_t BgPacked(const math::Color &_bg)
  {
    auto channel = [](float _c)
    {
      return static_cast<uint32_t>(
          std::round(FilamentConversions::LinearToSrgb(_c) * 255.0));
    };
    return (channel(_bg.R()) << 24u) | (channel(_bg.G()) << 16u) |
        (channel(_bg.B()) << 8u) | 255u;
  }
}

//////////////////////////////////////////////////
void FilamentDepthCamera::PreRender()
{
  if (!this->depthInstance)
    this->CreateDepthTexture();
  FilamentCamera::PreRender();
}

//////////////////////////////////////////////////
std::array<double, 4> FilamentDepthCamera::UpdateRays()
{
  const unsigned int width = this->ImageWidth();
  const unsigned int height = this->ImageHeight();
  double fx = (width * 0.5) / std::tan(this->HFOV().Radian() * 0.5);
  double fy = fx;
  double cx = width * 0.5;
  double cy = height * 0.5;
  if (this->customProjection)
  {
    // pinhole of the custom matrix (ogre2 interpolates the frustum corners
    // of its inverse): x_ndc = (P00 x + P02 z) / -z, same for y
    const math::Matrix4d proj = FilamentCamera::ProjectionMatrix();
    fx = proj(0, 0) * width * 0.5;
    fy = proj(1, 1) * height * 0.5;
    cx = width * 0.5 * (1.0 - proj(0, 2));
    cy = height * 0.5 * (1.0 + proj(1, 2));
  }
  // ray slopes per column and row: -(u + 0.5 - cx) / fx * d is
  // colK[u] * d, the same double operations in the same order
  const std::array<double, 6> rayKey{static_cast<double>(width),
      static_cast<double>(height), cx, cy, fx, fy};
  if (rayKey != this->cachedRayKey)
  {
    this->cachedRayKey = rayKey;
    this->cachedColK.resize(width);
    this->cachedRowK.resize(height);
    for (unsigned int u = 0; u < width; ++u)
      this->cachedColK[u] = -(u + 0.5 - cx) / fx;
    for (unsigned int v = 0; v < height; ++v)
      this->cachedRowK[v] = -(v + 0.5 - cy) / fy;
  }
  return {fx, fy, cx, cy};
}

//////////////////////////////////////////////////
void FilamentDepthCamera::Render()
{
  if (!this->depthInstance)
    this->CreateDepthTexture();

  this->SetClampedProjection(this->NearClipPlane() * kNearScale,
      this->FarClipPlane() * kFarScale);

  // Particles in view: half resolution depth of the nearest particle, like
  // ogre2's particle pass. PostRender adds their effect to the raw depth.
  // Otherwise the depth pass clamps (gz_depth_clamp). Chosen before the
  // color pass begins the frame: the engine commits material parameters
  // when a frame begins.
  const unsigned int width = this->ImageWidth();
  const unsigned int height = this->ImageHeight();
  const std::array<double, 4> pinhole = this->UpdateRays();
  namespace fp = filament_particles;
  fp::Noise noise;
  bool particles = fp::FindNoise(*this->scene, noise);
  if (particles)
  {
    const unsigned int pw = std::max(width / 2u, 1u);
    const unsigned int ph = std::max(height / 2u, 1u);
    this->particleDepth.resize(static_cast<size_t>(pw) * ph);
    const math::Pose3d world = this->WorldPose();
    particles = fp::Raster(*this->scene, world.Pos(),
        world.Rot().XAxis(), world.Rot().YAxis(), world.Rot().ZAxis(),
        pinhole[0] * pw / width, pinhole[1] * ph / height, pw, ph,
        this->NearClipPlane() * kNearScale, this->particleDepth.data());
    // TODO(anyone): particles assume a centered principal point, off-center
    // custom matrices shift them by (cx, cy) - center; pass both if needed
  }
  const float inf = std::numeric_limits<float>::infinity();
  this->gpuClamped = !particles && this->clampInstance;
  if (this->gpuClamped)
  {
    // per pixel far thresholds, texture rows like gl_FragCoord's: bottom
    // first on OpenGL, top first on Vulkan (readbacks are top first on both)
    const double farSq = FarRangeSq(this->FarClipPlane() - kTol);
    const auto &k = this->cachedRayKey;
    const std::array<double, 7> key{k[0], k[1], k[2], k[3], k[4], k[5], farSq};
    if (key != this->farMapKey)
    {
      this->farMapKey = key;
      auto *engine = FilamentRenderEngine::Instance()->FilamentEngine();
      if (this->farMap)
        engine->destroy(this->farMap);
      this->farMap = filament::Texture::Builder()
          .width(width).height(height).levels(1)
          .format(filament::Texture::InternalFormat::R32F)
          .usage(filament::Texture::Usage::SAMPLEABLE |
                 filament::Texture::Usage::UPLOADABLE)
          .build(*engine);
      const bool flip =
          engine->getBackend() == filament::Engine::Backend::OPENGL;
      auto *data = new float[static_cast<size_t>(width) * height];
      for (unsigned int v = 0; v < height; ++v)
      {
        float *row = data + static_cast<size_t>(flip ? height - 1u - v : v) *
            width;
        for (unsigned int u = 0; u < width; ++u)
        {
          row[u] = FarThreshold(this->cachedColK[u], this->cachedRowK[v],
              farSq);
        }
      }
      this->farMap->setImage(*engine, 0,
          filament::Texture::PixelBufferDescriptor(data,
          static_cast<size_t>(width) * height * sizeof(float),
          filament::Texture::Format::R, filament::Texture::Type::FLOAT,
          &filament_utils::FreeBuffer<float>));
      this->clampInstance->setParameter("farMap", this->farMap,
          filament::TextureSampler(filament::TextureSampler::MinFilter::NEAREST,
          filament::TextureSampler::MagFilter::NEAREST));
    }
    // the least float >= near + tol: for a float d, d < near + tol (in
    // double, PostRender's test) <=> d < nearT
    const double nearT = this->NearClipPlane() + kTol;
    float nearFl = static_cast<float>(nearT);
    if (static_cast<double>(nearFl) < nearT)
      nearFl = std::nextafter(nearFl, inf);
    this->clampInstance->setParameter("nearT", nearFl);
    this->clampInstance->setParameter("inf", inf);
  }

  this->view->setVisibleLayers(0xff,
      FilamentConversions::MaskToLayers(this->VisibilityMask()));

  // Color pass, only needed for the colored point cloud
  if (this->newRgbPointCloud.ConnectionCount() > 0u)
  {
    if (!this->colorTexture)
    {
      this->colorTexture = std::dynamic_pointer_cast<FilamentRenderTexture>(
          this->scene->CreateRenderTexture());
      this->colorTexture->SetFormat(PF_R8G8B8A8);
    }
    // SetWidth/SetHeight mark the target dirty (rebuilt in PreRender)
    if (this->colorTexture->Width() != this->ImageWidth() ||
        this->colorTexture->Height() != this->ImageHeight())
    {
      this->colorTexture->SetWidth(this->ImageWidth());
      this->colorTexture->SetHeight(this->ImageHeight());
    }
    this->colorTexture->PreRender();
    this->colorTexture->SetClearColor(
        FilamentConversions::ToFloat4(this->scene->BackgroundColor()));
    // the color and the depth pass share one frame (one command buffer,
    // one submit), closed after the depth pass
    FilamentRenderEngine::Instance()->BeginFrame();
    // ogre2 renders this pass with its shadow node, like a camera
    this->view->setShadowingEnabled(true);
    this->colorTexture->Render(this->view);
    this->view->setShadowingEnabled(false);
  }

  // Depth pass: every mesh drawn with the linear-depth material, no
  // particles (PostRender adds their effect). Background reads as "beyond
  // far".
  this->view->setVisibleLayers(0xff,
      FilamentConversions::ToSensorLayers(this->VisibilityMask()));
  this->renderTexture->SetClearColor(filament_utils::kPackedInf);
  const auto &meshes = this->scene->Meshes();
  bool alphaTested = false;
  for (FilamentMesh *m : meshes)
  {
    m->OverrideMaterial(this->gpuClamped ? this->clampInstance :
        this->depthInstance);
    alphaTested = alphaTested || m->HasAlphaTestedOverride();
  }
  // alpha tested copies are committed when a frame begins: submit the
  // color pass on its own then
  if (alphaTested)
    FilamentRenderEngine::Instance()->EndFrame();
  this->renderTexture->Render(this->view);
  FilamentRenderEngine::Instance()->EndFrame();
  for (FilamentMesh *m : meshes)
    m->RestoreMaterials();
}

//////////////////////////////////////////////////
void FilamentDepthCamera::PostRender()
{
  FilamentCamera::PostRender();

  const unsigned int width = this->ImageWidth();
  const unsigned int height = this->ImageHeight();
  const size_t len = static_cast<size_t>(width) * height;
  this->depthImage.resize(len);

  // queue both readbacks, wait once (rows come back top row first, the
  // gz image layout). A clamped target is the depth image as is.
  const bool wantCloud = this->newRgbPointCloud.ConnectionCount() > 0u;
  auto *renderer = FilamentRenderEngine::Instance()->Renderer();
  if (!this->gpuClamped)
    this->depthRaw.resize(len);
  renderer->readPixels(this->renderTexture->Native(), 0, 0, width, height,
      filament::backend::PixelBufferDescriptor(this->gpuClamped ?
      this->depthImage.data() : this->depthRaw.data(),
      len * sizeof(float), filament::backend::PixelDataFormat::RGBA,
      filament::backend::PixelDataType::UBYTE));
  const unsigned char *rgba = nullptr;
  if (wantCloud && this->colorTexture)
  {
    this->colorRaw.resize(len * 4u);
    renderer->readPixels(this->colorTexture->Native(), 0, 0, width, height,
        filament::backend::PixelBufferDescriptor(this->colorRaw.data(),
        this->colorRaw.size(), filament::backend::PixelDataFormat::RGBA,
        filament::backend::PixelDataType::UBYTE));
    rgba = this->colorRaw.data();
  }
  FilamentRenderEngine::Instance()->FilamentEngine()->flushAndWait();
  if (wantCloud)
    this->pointCloud.resize(len * 4u);

  const auto [fx, fy, cx, cy] = this->UpdateRays();
  const std::vector<double> &colK = this->cachedColK;
  const std::vector<double> &rowK = this->cachedRowK;
  // ogre2 also writes the background color for clamped points; it
  // gamma-encodes it, the color pass target here is sRGB as well
  const uint32_t bgPacked = BgPacked(this->scene->BackgroundColor());
  auto color = [&](size_t _i, bool _bg)
  {
    const uint32_t packed = (_bg || !rgba) ? bgPacked :
        (static_cast<uint32_t>(rgba[_i * 4u]) << 24u) |
        (static_cast<uint32_t>(rgba[_i * 4u + 1u]) << 16u) |
        (static_cast<uint32_t>(rgba[_i * 4u + 2u]) << 8u) | 255u;
    float f;
    std::memcpy(&f, &packed, sizeof(float));
    return f;
  };

  if (this->gpuClamped && wantCloud)
  {
    // the shader's clamped pixels hold +-inf, the others the depth: the
    // cloud of the loop below without its range tests
    for (unsigned int v = 0; v < height; ++v)
    {
      const double kb = rowK[v];
      unsigned int u = 0;
#if defined(__SSE2__)
      // four points per step, the same double operations as the scalar
      // loop (cvtpd_ps rounds like the cast), transposed to x y z rgb
      const __m128 absF = _mm_castsi128_ps(_mm_set1_epi32(0x7fffffff));
      const __m128 infF = _mm_set1_ps(std::numeric_limits<float>::infinity());
      const __m128i bg = _mm_set1_epi32(static_cast<int>(bgPacked));
      const __m128d kbD = _mm_set1_pd(kb);
      for (; u + 4u <= width; u += 4u)
      {
        const size_t i = static_cast<size_t>(v) * width + u;
        __m128 x = _mm_loadu_ps(&this->depthImage[i]);
        const __m128 clamped = _mm_cmpeq_ps(_mm_and_ps(x, absF), infF);
        const __m128d d0 = _mm_cvtps_pd(x);
        const __m128d d1 = _mm_cvtps_pd(_mm_movehl_ps(x, x));
        const __m128 y = _mm_movelh_ps(
            _mm_cvtpd_ps(_mm_mul_pd(_mm_loadu_pd(&colK[u]), d0)),
            _mm_cvtpd_ps(_mm_mul_pd(_mm_loadu_pd(&colK[u + 2u]), d1)));
        const __m128 z = _mm_movelh_ps(_mm_cvtpd_ps(_mm_mul_pd(kbD, d0)),
            _mm_cvtpd_ps(_mm_mul_pd(kbD, d1)));
        __m128 yc = _mm_or_ps(_mm_and_ps(clamped, x),
            _mm_andnot_ps(clamped, y));
        __m128 zc = _mm_or_ps(_mm_and_ps(clamped, x),
            _mm_andnot_ps(clamped, z));
        __m128i c = bg;
        if (rgba)
        {
          // bytes R G B A -> R << 24 | G << 16 | B << 8 | 255
          const __m128i p = _mm_loadu_si128(
              reinterpret_cast<const __m128i *>(rgba + i * 4u));
          const __m128i packed = _mm_or_si128(
              _mm_or_si128(_mm_slli_epi32(p, 24), _mm_and_si128(
              _mm_slli_epi32(p, 8), _mm_set1_epi32(0x00ff0000))),
              _mm_or_si128(_mm_and_si128(_mm_srli_epi32(p, 8),
              _mm_set1_epi32(0x0000ff00)), _mm_set1_epi32(0xff)));
          const __m128i m = _mm_castps_si128(clamped);
          c = _mm_or_si128(_mm_and_si128(m, bg), _mm_andnot_si128(m, packed));
        }
        __m128 cf = _mm_castsi128_ps(c);
        _MM_TRANSPOSE4_PS(x, yc, zc, cf);
        float *out = &this->pointCloud[i * 4u];
        _mm_storeu_ps(out, x);
        _mm_storeu_ps(out + 4, yc);
        _mm_storeu_ps(out + 8, zc);
        _mm_storeu_ps(out + 12, cf);
      }
#endif
      for (; u < width; ++u)
      {
        const size_t i = static_cast<size_t>(v) * width + u;
        const float x = this->depthImage[i];
        const bool clamped = std::isinf(x);
        const double d = x;
        float *out = &this->pointCloud[i * 4u];
        out[0] = x;
        out[1] = clamped ? x : static_cast<float>(colK[u] * d);
        out[2] = clamped ? x : static_cast<float>(kb * d);
        out[3] = color(i, clamped);
      }
    }
  }
  else if (!this->gpuClamped)
  {
    // Same clamping rules as ogre2's depth_camera_fs.glsl: beyond far is
    // judged on the euclidean range, nearer than near on the forward
    // distance.
    const float minVal = -std::numeric_limits<float>::infinity();
    const float maxVal = std::numeric_limits<float>::infinity();
    const double nearClip = this->NearClipPlane();
    // sqrt(r2) > farClip - tol, without the square root
    const double farSq = FarRangeSq(this->FarClipPlane() - kTol);
    const float *src = this->depthRaw.data();
    // particles rasterized in Render
    namespace fp = filament_particles;
    fp::Noise noise;
    const bool particles = fp::FindNoise(*this->scene, noise);
    const unsigned int pw = std::max(width / 2u, 1u);
    const unsigned int ph = std::max(height / 2u, 1u);
    // per frame seed, like ogre2's rnd uniform (gz::math::Rand stays
    // seedable)
    std::minstd_rand rng;
    if (particles)
    {
      rng.seed(static_cast<unsigned int>(
          math::Rand::IntUniform(1, std::numeric_limits<int>::max())));
    }
    std::uniform_real_distribution<double> uniform(0.0, 1.0);
    for (unsigned int v = 0; v < height; ++v)
    {
      for (unsigned int u = 0; u < width; ++u)
      {
        const size_t i = static_cast<size_t>(v) * width + u;
        double d = src[i];
        bool particlePixel = false;
        if (particles)
        {
          const float pd = this->particleDepth[
              std::min(ph - 1u, (2u * v + 1u) * ph / (2u * height)) * pw +
              std::min(pw - 1u, (2u * u + 1u) * pw / (2u * width))];
          particlePixel = std::isfinite(pd);
          if (particlePixel && pd < d)
          {
            // ranges along the pixel ray
            const double a = (u + 0.5 - cx) / fx;
            const double b = (v + 0.5 - cy) / fy;
            const double rayLen = std::sqrt(1.0 + a * a + b * b);
            d = fp::Scatter(uniform(rng), noise, pd * rayLen, d * rayLen) /
                rayLen;
          }
        }
        // camera frame: X forward, Y left, Z up
        double p[3] = {d, colK[u] * d, rowK[v] * d};
        bool clamped = false;
        if (std::isinf(d) || p[0] * p[0] + p[1] * p[1] + p[2] * p[2] > farSq)
        {
          p[0] = p[1] = p[2] = maxVal;
          clamped = true;
        }
        else if (d < nearClip + kTol)
        {
          p[0] = p[1] = p[2] = minVal;
          clamped = true;
        }
        this->depthImage[i] = static_cast<float>(p[0]);

        if (!wantCloud)
          continue;
        float *out = &this->pointCloud[i * 4u];
        out[0] = static_cast<float>(p[0]);
        out[1] = static_cast<float>(p[1]);
        out[2] = static_cast<float>(p[2]);
        // ogre2 keeps the color of particle pixels
        out[3] = color(i, clamped && !particlePixel);
      }
    }
  }

  FilamentGaussianNoisePass::ApplyDepth(*this, this->depthImage.data(),
      wantCloud ? this->pointCloud.data() : nullptr);

  this->newDepthFrame(this->depthImage.data(), width, height, 1u, "FLOAT32");
  if (wantCloud)
  {
    this->newRgbPointCloud(this->pointCloud.data(), width, height, 4u,
        "PF_FLOAT32_RGBA");
  }
}

//////////////////////////////////////////////////
math::Matrix4d FilamentDepthCamera::ProjectionMatrix() const
{
  if (!this->customProjection)
    return FilamentCamera::ProjectionMatrix();
  return WithDepthRows(FilamentCamera::ProjectionMatrix(),
      this->NearClipPlane(), this->FarClipPlane());
}

//////////////////////////////////////////////////
const float *FilamentDepthCamera::DepthData() const
{
  return this->depthImage.data();
}

//////////////////////////////////////////////////
common::ConnectionPtr FilamentDepthCamera::ConnectNewDepthFrame(
    std::function<void(const float *, unsigned int, unsigned int,
    unsigned int, const std::string &)> _subscriber)
{
  return this->newDepthFrame.Connect(_subscriber);
}

//////////////////////////////////////////////////
common::ConnectionPtr FilamentDepthCamera::ConnectNewRgbPointCloud(
    std::function<void(const float *, unsigned int, unsigned int,
    unsigned int, const std::string &)> _subscriber)
{
  return this->newRgbPointCloud.Connect(_subscriber);
}
