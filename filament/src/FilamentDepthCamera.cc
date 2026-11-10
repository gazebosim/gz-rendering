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

#include <cmath>
#include <cstring>
#include <limits>
#include <random>
#include <string>
#include <type_traits>
#include <vector>
#if defined(__SSE2__)
#include <emmintrin.h>
#endif

#include <backend/PixelBufferDescriptor.h>
#include <filament/Camera.h>
#include <filament/Engine.h>
#include <filament/Material.h>
#include <filament/MaterialInstance.h>
#include <filament/Renderer.h>
#include <filament/View.h>

#include <gz/common/Console.hh>
#include <gz/math/Helpers.hh>
#include <gz/math/Rand.hh>

#include "gz/rendering/Image.hh"
#include "gz/rendering/filament/FilamentConversions.hh"
#include "gz/rendering/filament/FilamentDepthCamera.hh"
#include "gz/rendering/filament/FilamentGaussianNoisePass.hh"
#include "gz/rendering/filament/FilamentMesh.hh"
#include "gz/rendering/filament/FilamentRenderEngine.hh"
#include "gz/rendering/filament/FilamentRenderTarget.hh"
#include "gz/rendering/filament/FilamentScene.hh"
#include "FilamentParticles.hh"

using namespace gz;
using namespace rendering;

//////////////////////////////////////////////////
FilamentDepthCamera::FilamentDepthCamera()
{
}

//////////////////////////////////////////////////
FilamentDepthCamera::~FilamentDepthCamera()
{
}

//////////////////////////////////////////////////
void FilamentDepthCamera::Init()
{
  FilamentCamera::Init();
  // no shadow maps for the depth pass, nor any screen-space effect
  this->view->setShadowingEnabled(false);
  this->view->setPostProcessingEnabled(false);
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
  if (this->colorTexture)
  {
    this->colorTexture->Destroy();
    this->colorTexture.reset();
  }
  FilamentCamera::Destroy();
}

namespace
{
  /// \brief Whether the depth target holds the depth's float bits as RGBA8
  /// (gz_depth_packed) instead of R32F. Vulkan readPixels converts R32F
  /// texel by texel but copies RGBA8 rows as they are; OpenGL reads both
  /// directly, so it keeps R32F.
  // ponytail: RenderTarget()->Copy() of a depth camera then reads the packed
  // bytes on Vulkan; nothing (gz-sensors, tests) copies it, unpack if needed
  bool PackedDepth()
  {
    return FilamentRenderEngine::Instance()->FilamentEngine()->getBackend()
        == filament::Engine::Backend::VULKAN;
  }
}

//////////////////////////////////////////////////
void FilamentDepthCamera::CreateDepthTexture()
{
  if (!this->depthInstance)
  {
    const std::string matName = PackedDepth() ? "gz_depth_packed" :
        "gz_depth";
    filament::Material *mat =
        FilamentRenderEngine::Instance()->Material(matName);
    if (!mat)
    {
      gzerr << matName << " material not available" << std::endl;
      return;
    }
    this->depthInstance = mat->createInstance();
  }
  if (PackedDepth())
  {
    this->renderTexture->SetFormat(PF_R8G8B8A8);
    this->renderTexture->SetSrgb(false);
  }
  else
  {
    this->renderTexture->SetFormat(PF_FLOAT32_R);
  }
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
}

namespace
{
  /// \brief ogre2's color of clamped points: the gamma-encoded background
  /// as R << 24 | G << 16 | B << 8 | 255
  uint32_t BgPacked(const math::Color &_bg)
  {
    return
      (static_cast<uint32_t>(std::round(FilamentConversions::LinearToSrgb(_bg.R()) * 255.0)) << 24u) |
      (static_cast<uint32_t>(std::round(FilamentConversions::LinearToSrgb(_bg.G()) * 255.0)) << 16u) |
      (static_cast<uint32_t>(std::round(FilamentConversions::LinearToSrgb(_bg.B()) * 255.0)) << 8u) |
      255u;
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
void FilamentDepthCamera::Render()
{
  if (!this->depthInstance)
    this->CreateDepthTexture();

  this->SetClampedProjection(this->NearClipPlane() * kNearScale,
      this->FarClipPlane() * kFarScale);

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
  // packed: the bytes of +inf (0x7f800000), least significant first
  const float inf = std::numeric_limits<float>::infinity();
  if (PackedDepth())
    this->renderTexture->SetClearColor({0.0f, 0.0f, 128.0f / 255.0f, 127.0f / 255.0f});
  else
    this->renderTexture->SetClearColor({inf, inf, inf, 1.0f});
  const auto &meshes = this->scene->Meshes();
  bool alphaTested = false;
  for (FilamentMesh *m : meshes)
  {
    m->OverrideMaterial(this->depthInstance);
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
  // gz image layout)
  const bool wantCloud = this->newRgbPointCloud.ConnectionCount() > 0u;
  auto *renderer = FilamentRenderEngine::Instance()->Renderer();
  this->depthRaw.resize(len);
  const bool packedDepth = PackedDepth();
  renderer->readPixels(this->renderTexture->Native(), 0, 0, width, height,
      filament::backend::PixelBufferDescriptor(this->depthRaw.data(),
      len * sizeof(float),
      packedDepth ? filament::backend::PixelDataFormat::RGBA :
          filament::backend::PixelDataFormat::R,
      packedDepth ? filament::backend::PixelDataType::UBYTE :
                    filament::backend::PixelDataType::FLOAT));
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
  const float *src = this->depthRaw.data();
  if (wantCloud)
    this->pointCloud.resize(len * 4u);

  // Same clamping rules as ogre2's depth_camera_fs.glsl: beyond far is
  // judged on the euclidean range, nearer than near on the forward distance.
  const float minVal = -std::numeric_limits<float>::infinity();
  const float maxVal = std::numeric_limits<float>::infinity();
  const double nearClip = this->NearClipPlane();
  const double farClip = this->FarClipPlane();
  const double tol = 1e-6;
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
  // ogre2 also writes the background color for clamped points; it
  // gamma-encodes it, the color pass target here is sRGB as well
  const uint32_t bgPacked = BgPacked(this->scene->BackgroundColor());
  // particles: half resolution depth of the nearest particle, like ogre2's
  // particle pass, sampled nearest
  namespace fp = filament_particles;
  fp::Noise noise;
  const unsigned int pw = std::max(width / 2u, 1u);
  const unsigned int ph = std::max(height / 2u, 1u);
  bool particles = fp::FindNoise(*this->scene, noise);
  if (particles)
  {
    this->particleDepth.resize(static_cast<size_t>(pw) * ph);
    const math::Pose3d world = this->WorldPose();
    particles = fp::Raster(*this->scene, world.Pos(),
        world.Rot().XAxis(), world.Rot().YAxis(), world.Rot().ZAxis(),
        fx * pw / width, fy * ph / height, pw, ph,
        nearClip * kNearScale, this->particleDepth.data());
    // ponytail: particles assume a centered principal point, off-center
    // custom matrices shift them by (cx, cy) - center; pass both if needed
  }
  // per frame seed, like ogre2's rnd uniform (gz::math::Rand stays seedable)
  std::minstd_rand rng;
  if (particles)
  {
    rng.seed(static_cast<unsigned int>(
        math::Rand::IntUniform(1, std::numeric_limits<int>::max())));
  }
  std::uniform_real_distribution<double> uniform(0.0, 1.0);

  // ray slopes per column and row: -(u + 0.5 - cx) / fx * d below is
  // colK[u] * d, the same double operations in the same order
  std::vector<double> colK(width), rowK(height);
  for (unsigned int u = 0; u < width; ++u)
    colK[u] = -(u + 0.5 - cx) / fx;
  for (unsigned int v = 0; v < height; ++v)
    rowK[v] = -(v + 0.5 - cy) / fy;

  // sqrt(r2) > farClip - tol, without the square root
  const double farSq = FarRangeSq(farClip - tol);

  // Fast path without particles: the same clamping and packing as the loop
  // below, with no per pixel divisions or square roots.
  auto convert = [&](auto _cloud)
  {
    constexpr bool cloud = decltype(_cloud)::value;
    const double nearT = nearClip + tol;
#if defined(__SSE2__)
    // the least float >= nearT: for a float f, f < nearT <=> f < nearF
    float nearFl = static_cast<float>(nearT);
    if (static_cast<double>(nearFl) < nearT)
      nearFl = std::nextafter(nearFl, maxVal);
    const __m128 nearF = _mm_set1_ps(nearFl);
    const __m128d farSqD = _mm_set1_pd(farSq);
    const __m128 absF = _mm_castsi128_ps(_mm_set1_epi32(0x7fffffff));
    const __m128 maxF = _mm_set1_ps(maxVal);
    const __m128 minF = _mm_set1_ps(minVal);
#endif
    float *depthOut = this->depthImage.data();
    float *cloudOut = this->pointCloud.data();
    for (unsigned int v = 0; v < height; ++v)
    {
      const double kb = rowK[v];
      const size_t row = static_cast<size_t>(v) * width;
      unsigned int u = 0;
#if defined(__SSE2__)
      // depth only, four pixels per step: the same double operations as the
      // scalar loop, two lanes at a time; the unclamped output is the input
      // float itself, like the scalar loop's float -> double -> float
      if constexpr (!cloud)
      {
        const __m128d kbD = _mm_set1_pd(kb);
        for (; u + 4u <= width; u += 4u)
        {
          const size_t i = row + u;
          const __m128 f = _mm_loadu_ps(src + i);
          const __m128d d0 = _mm_cvtps_pd(f);
          const __m128d d1 = _mm_cvtps_pd(_mm_movehl_ps(f, f));
          const __m128d q0 = _mm_mul_pd(_mm_loadu_pd(&colK[u]), d0);
          const __m128d q1 = _mm_mul_pd(_mm_loadu_pd(&colK[u + 2u]), d1);
          const __m128d r0 = _mm_mul_pd(kbD, d0);
          const __m128d r1 = _mm_mul_pd(kbD, d1);
          // (d * d + p1 * p1) + p2 * p2 > farSq
          const __m128d s0 = _mm_add_pd(_mm_add_pd(_mm_mul_pd(d0, d0),
              _mm_mul_pd(q0, q0)), _mm_mul_pd(r0, r0));
          const __m128d s1 = _mm_add_pd(_mm_add_pd(_mm_mul_pd(d1, d1),
              _mm_mul_pd(q1, q1)), _mm_mul_pd(r1, r1));
          const __m128 beyond = _mm_shuffle_ps(
              _mm_castpd_ps(_mm_cmpgt_pd(s0, farSqD)),
              _mm_castpd_ps(_mm_cmpgt_pd(s1, farSqD)), _MM_SHUFFLE(2, 0, 2, 0));
          const __m128 far = _mm_or_ps(beyond,
              _mm_cmpeq_ps(_mm_and_ps(f, absF), maxF));
          const __m128 clamped = _mm_or_ps(far, _mm_cmplt_ps(f, nearF));
          const __m128 c = _mm_or_ps(_mm_and_ps(far, maxF),
              _mm_andnot_ps(far, minF));
          _mm_storeu_ps(depthOut + i, _mm_or_ps(_mm_and_ps(clamped, c),
              _mm_andnot_ps(clamped, f)));
        }
      }
#endif
      for (; u < width; ++u)
      {
        const size_t i = row + u;
        const double d = src[i];
        const double p1 = colK[u] * d;
        const double p2 = kb * d;
        // converted unconditionally (exact, or unused when clamped) so the
        // selects below need no branches
        const float x = static_cast<float>(d);
        const bool far = std::isinf(d) | (d * d + p1 * p1 + p2 * p2 > farSq);
        const bool clamped = far | (d < nearT);
        const float c = far ? maxVal : minVal;
        depthOut[i] = clamped ? c : x;
        if constexpr (cloud)
        {
          const float y = static_cast<float>(p1);
          const float z = static_cast<float>(p2);
          float *out = cloudOut + i * 4u;
          out[0] = clamped ? c : x;
          out[1] = clamped ? c : y;
          out[2] = clamped ? c : z;
          const uint32_t packed = (clamped || !rgba) ? bgPacked :
              (static_cast<uint32_t>(rgba[i * 4u]) << 24u) |
              (static_cast<uint32_t>(rgba[i * 4u + 1u]) << 16u) |
              (static_cast<uint32_t>(rgba[i * 4u + 2u]) << 8u) | 255u;
          std::memcpy(&out[3], &packed, sizeof(float));
        }
      }
    }
  };
  if (!particles)
  {
    if (wantCloud)
      convert(std::true_type{});
    else
      convert(std::false_type{});
  }

  for (unsigned int v = 0; particles && v < height; ++v)
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
      double p[3] = {d, -(u + 0.5 - cx) / fx * d, -(v + 0.5 - cy) / fy * d};
      bool clamped = false;
      if (std::isinf(d) || p[0] * p[0] + p[1] * p[1] + p[2] * p[2] > farSq)
      {
        p[0] = p[1] = p[2] = maxVal;
        clamped = true;
      }
      else if (d < nearClip + tol)
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
      const uint32_t packed = ((clamped && !particlePixel) || !rgba) ?
          bgPacked :
          (static_cast<uint32_t>(rgba[i * 4u]) << 24u) |
          (static_cast<uint32_t>(rgba[i * 4u + 1u]) << 16u) |
          (static_cast<uint32_t>(rgba[i * 4u + 2u]) << 8u) | 255u;
      std::memcpy(&out[3], &packed, sizeof(float));
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
