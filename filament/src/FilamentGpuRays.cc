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
#include <cmath>
#include <cstring>
#include <limits>
#include <random>
#include <string>

#include <filament/Camera.h>
#include <filament/Engine.h>
#include <filament/Texture.h>
#include <filament/TextureSampler.h>
#include <filament/Material.h>
#include <filament/MaterialInstance.h>
#include <backend/PixelBufferDescriptor.h>
#include <filament/Renderer.h>
#include <filament/TransformManager.h>
#include <filament/View.h>
#include <filament/Viewport.h>
#include <utils/EntityManager.h>

#include <gz/math/Helpers.hh>
#include <gz/math/Quaternion.hh>
#include <gz/math/Rand.hh>
#include <gz/math/Vector3.hh>

#include "gz/rendering/filament/FilamentConversions.hh"
#include "gz/rendering/filament/FilamentGpuRays.hh"
#include "gz/rendering/filament/FilamentMesh.hh"
#include "gz/rendering/filament/FilamentRenderEngine.hh"
#include "gz/rendering/filament/FilamentRenderTarget.hh"
#include "gz/rendering/filament/FilamentScene.hh"
#include "FilamentCubeFaces.hh"
#include "FilamentFullscreenPass.hh"
#include "FilamentParticles.hh"
#include "FilamentUtils.hh"

using namespace gz;
using namespace rendering;

namespace
{
  using filament::math::float3;
  using filament_cube::Face;
  using filament_cube::FaceTransform;
  using filament_cube::kFaces;
}

//////////////////////////////////////////////////
FilamentGpuRays::~FilamentGpuRays()
{
}

//////////////////////////////////////////////////
FilamentGpuRays::FilamentGpuRays()
{
  // range, retro, unused (same layout as ogre2)
  this->channels = 3u;
}

//////////////////////////////////////////////////
void FilamentGpuRays::Init()
{
  BaseGpuRays::Init();
  auto *engine = FilamentRenderEngine::Instance()->FilamentEngine();
  auto &tm = engine->getTransformManager();
  this->cameraEntity = ::utils::EntityManager::get().create();
  tm.create(this->cameraEntity, tm.getInstance(this->entity),
      FaceTransform(kFaces[0]));
  this->camera = engine->createCamera(this->cameraEntity);
  this->view = engine->createView();
  this->view->setScene(this->scene->Native());
  this->view->setCamera(this->camera);
  this->view->setPostProcessingEnabled(false);
  this->view->setShadowingEnabled(false);

  filament::Material *mat =
      FilamentRenderEngine::Instance()->Material("gz_depth");
  if (mat)
    this->depthInstance = mat->createInstance();

  this->renderTexture = std::dynamic_pointer_cast<FilamentRenderTexture>(
      this->scene->CreateRenderTexture());
  // depths and ranges as float bits in RGBA8, see filament_utils::kPackedInf
  this->renderTexture->SetFormat(PF_R8G8B8A8);
  this->renderTexture->SetSrgb(false);
  this->InitSamplePass();
}

//////////////////////////////////////////////////
void FilamentGpuRays::InitSamplePass()
{
  filament::Material *mat =
      FilamentRenderEngine::Instance()->Material("gz_lidar_sample");
  if (!mat)
    return;
  this->samplePass = std::make_unique<FilamentFullscreenPass>(*mat);

  this->scanTexture = std::dynamic_pointer_cast<FilamentRenderTexture>(
      this->scene->CreateRenderTexture());
  this->scanTexture->SetFormat(PF_R8G8B8A8);
  this->scanTexture->SetSrgb(false);
}

//////////////////////////////////////////////////
void FilamentGpuRays::UploadRays()
{
  auto *engine = FilamentRenderEngine::Instance()->FilamentEngine();
  const unsigned int hCount = std::max(this->RangeCount(), 1);
  const unsigned int vCount = std::max(this->VerticalRangeCount(), 1);
  if (this->rayTexture)
    engine->destroy(this->rayTexture);
  this->rayTexture = filament::Texture::Builder()
      .width(hCount).height(vCount).levels(1)
      .format(filament::Texture::InternalFormat::RGBA32F)
      .usage(filament::Texture::Usage::SAMPLEABLE |
             filament::Texture::Usage::UPLOADABLE)
      .build(*engine);
  // readPixels returns render targets top row first on both backends, but
  // texture rows (texelFetch, gl_FragCoord) start at the bottom on OpenGL and
  // at the top on Vulkan. The CPU side indexes readback rows, so flip on GL,
  // for the atlas and for the scan alike (verified against CPU sampling).
  const bool flip = engine->getBackend() == filament::Engine::Backend::OPENGL;
  const unsigned int stride = this->faceRes * this->faceCount;
  const size_t count = this->rays.size() * 4u;
  auto *data = new float[count];
  for (size_t k = 0; k < this->rays.size(); ++k)
  {
    const Ray &ray = this->rays[k];
    const size_t row = flip ? vCount - 1u - k / hCount : k / hCount;
    const unsigned int atlasRow = ray.texel / stride;
    float *t = data + (row * hCount + k % hCount) * 4u;
    t[0] = static_cast<float>(ray.texel % stride);
    t[1] = static_cast<float>(flip ? this->faceRes - 1u - atlasRow : atlasRow);
    t[2] = ray.invCos;
    t[3] = 0.0f;
  }
  this->rayTexture->setImage(*engine, 0,
      filament::Texture::PixelBufferDescriptor(data, count * sizeof(float),
      filament::Texture::Format::RGBA, filament::Texture::Type::FLOAT,
      &filament_utils::FreeBuffer<float>));
  const filament::TextureSampler nearest(
      filament::TextureSampler::MinFilter::NEAREST,
      filament::TextureSampler::MagFilter::NEAREST);
  this->samplePass->instance->setParameter("rays", this->rayTexture, nearest);
}

//////////////////////////////////////////////////
void FilamentGpuRays::Destroy()
{
  auto *engine = FilamentRenderEngine::Instance()->FilamentEngine();
  if (engine)
  {
    if (this->view)
      engine->destroy(this->view);
    if (this->depthInstance)
      engine->destroy(this->depthInstance);
    if (!this->cameraEntity.isNull())
    {
      engine->destroyCameraComponent(this->cameraEntity);
      engine->getTransformManager().destroy(this->cameraEntity);
      ::utils::EntityManager::get().destroy(this->cameraEntity);
    }
  }
  this->samplePass.reset();
  if (engine && this->rayTexture)
    engine->destroy(this->rayTexture);
  this->rayTexture = nullptr;
  if (this->scanTexture)
  {
    this->scanTexture->Destroy();
    this->scanTexture.reset();
  }
  this->view = nullptr;
  this->depthInstance = nullptr;
  this->camera = nullptr;
  this->cameraEntity = ::utils::Entity();
  if (this->renderTexture)
  {
    this->renderTexture->Destroy();
    this->renderTexture.reset();
  }
  BaseGpuRays::Destroy();
}

//////////////////////////////////////////////////
RenderTargetPtr FilamentGpuRays::RenderTarget() const
{
  return this->renderTexture;
}

//////////////////////////////////////////////////
void FilamentGpuRays::UpdateRays()
{
  const unsigned int hCount = std::max(this->RangeCount(), 1);
  const unsigned int vCount = std::max(this->VerticalRangeCount(), 1);
  const double hMin = this->AngleMin().Radian();
  const double hMax = this->AngleMax().Radian();
  const double vMin = this->VerticalAngleMin().Radian();
  const double vMax = this->VerticalAngleMax().Radian();
  const std::array<double, 8> key = {static_cast<double>(hCount),
      static_cast<double>(vCount), hMin, hMax, vMin, vMax, 0.0, 0.0};
  if (key == this->rayKey && !this->rays.empty())
    return;
  this->rayKey = key;

  // Face resolution: same rule as ogre2 (rays per 90 degrees, next power
  // of two, clamped to [128, 1024]) so both engines sample the same texels
  const double hFov = std::max(hMax - hMin, 1e-6);
  const double vFov = vCount > 1 ? std::max(vMax - vMin, 1e-6) : 0.0;
  const unsigned int hs = hFov < GZ_PI_2 ? hCount :
      static_cast<unsigned int>(GZ_PI_2 / hFov * hCount);
  const unsigned int vs = vFov < GZ_PI_2 ? vCount :
      static_cast<unsigned int>(GZ_PI_2 / vFov * vCount);
  unsigned int pot = 1u;
  while (pot < std::max(hs, vs))
    pot <<= 1;
  this->faceRes = std::clamp(pot, 128u, 1024u);
  const unsigned int n = this->faceRes;
  const double hStep = hCount > 1 ? (hMax - hMin) / (hCount - 1) : 0.0;
  const double vStep = vCount > 1 ? (vMax - vMin) / (vCount - 1) : 0.0;

  this->rays.resize(static_cast<size_t>(hCount) * vCount);
  this->faceUsed.fill(false);
  // Directions, face choice and tie-breaks replicate ogre2 (double math,
  // accumulated angle steps) so rays on face edges pick the same face.
  double v = vMin;
  for (unsigned int j = 0; j < vCount; ++j)
  {
    const math::Quaterniond pitch(math::Vector3d(1, 0, 0), -v);
    double h = hMin;
    for (unsigned int i = 0; i < hCount; ++i)
    {
      // ogre2 Y-up cubemap frame (x, y, z) is (-Y, Z, X) of the sensor
      const math::Vector3d o = math::Quaterniond(math::Vector3d(0, 1, 0), -h)
          * pitch * math::Vector3d(0, 0, 1);
      const math::Vector3d d(o.Z(), -o.X(), o.Y());
      const math::Vector3d da = d.Abs();
      int face;
      if (da.X() >= da.Y() && da.X() >= da.Z())
        face = d.X() < 0.0 ? 1 : 0;
      else if (da.Z() >= da.Y())
        face = d.Z() < 0.0 ? 5 : 4;
      else
        face = d.Y() < 0.0 ? 3 : 2;
      const Face &fc = kFaces[face];
      const float3 r = cross(fc.fwd, fc.up);
      const math::Vector3d fwd(fc.fwd.x, fc.fwd.y, fc.fwd.z);
      const double best = d.Dot(fwd);
      // texel coordinates in [0, 1], rows stored top row first
      const float u = static_cast<float>(
          d.Dot(math::Vector3d(r.x, r.y, r.z)) / best * 0.5 + 0.5);
      const float w = static_cast<float>(
          0.5 - d.Dot(math::Vector3d(fc.up.x, fc.up.y, fc.up.z)) / best * 0.5);
      const unsigned int px = std::min(n - 1,
          static_cast<unsigned int>(std::max(0.0f, u * n)));
      const unsigned int py = std::min(n - 1,
          static_cast<unsigned int>(std::max(0.0f, w * n)));
      // like ogre2, the range is the one of the texel center direction
      const float ac = (px + 0.5f) / n * 2.0f - 1.0f;
      const float bc = 1.0f - (py + 0.5f) / n * 2.0f;
      this->rays[static_cast<size_t>(j) * hCount + i] =
          {face, py * n + px, std::sqrt(ac * ac + bc * bc + 1.0f)};
      this->faceUsed[face] = true;
      h += hStep;
    }
    v += vStep;
  }

  // face texel (py * n + px) -> atlas texel, used faces left to right
  std::array<unsigned int, 6> slot{};
  this->faceCount = 0u;
  for (int f = 0; f < 6; ++f)
    if (this->faceUsed[f])
      slot[f] = this->faceCount++;
  const unsigned int stride = n * this->faceCount;
  for (Ray &ray : this->rays)
    ray.texel = ray.texel / n * stride + slot[ray.face] * n + ray.texel % n;
  if (this->samplePass)
    this->UploadRays();
}

//////////////////////////////////////////////////
void FilamentGpuRays::Render()
{
  if (!this->depthInstance || !this->samplePass)
    return;
  this->UpdateRays();
  const unsigned int n = this->faceRes;

  // near: half the side of the cube inscribed in the near sphere (ogre2
  // does the same), so no ray closer than near gets clipped by a corner;
  // ranges are then clamped against near/far in PostRender
  this->camera->setProjection(90.0, 1.0,
      this->NearClipPlane() / std::sqrt(3.0), this->FarClipPlane(),
      filament::Camera::Fov::HORIZONTAL);
  this->view->setVisibleLayers(0xff,
      FilamentConversions::ToSensorLayers(this->VisibilityMask()));

  // SetWidth/SetHeight mark the target dirty (rebuilt in PreRender)
  const unsigned int atlasWidth = n * this->faceCount;
  if (this->renderTexture->Width() != atlasWidth ||
      this->renderTexture->Height() != n)
  {
    this->renderTexture->SetWidth(atlasWidth);
    this->renderTexture->SetHeight(n);
  }
  this->renderTexture->PreRender();
  this->renderTexture->SetClearColor(filament_utils::kPackedInf);

  auto &tm = FilamentRenderEngine::Instance()->FilamentEngine()
      ->getTransformManager();
  const auto camInst = tm.getInstance(this->cameraEntity);
  const auto &meshes = this->scene->Meshes();
  for (FilamentMesh *m : meshes)
    m->OverrideMaterial(this->depthInstance);
  auto *engine = FilamentRenderEngine::Instance()->FilamentEngine();
  auto *renderer = FilamentRenderEngine::Instance()->Renderer();
  // second pass setup: one fragment per ray samples the atlas, only the
  // scan is read back (the atlas is up to 6 MB, a VLP-16 scan 115 KB).
  // Parameters are set before the frame opens: a frame commits material
  // instances once, at its start.
  const unsigned int hCount = std::max(this->RangeCount(), 1);
  const unsigned int vCount = std::max(this->VerticalRangeCount(), 1);
  this->samplePass->instance->setParameter("atlas",
      this->renderTexture->ColorTexture(), filament::TextureSampler(
      filament::TextureSampler::MinFilter::NEAREST,
      filament::TextureSampler::MagFilter::NEAREST));
  if (this->scanTexture->Width() != hCount ||
      this->scanTexture->Height() != vCount)
  {
    this->scanTexture->SetWidth(hCount);
    this->scanTexture->SetHeight(vCount);
  }
  this->scanTexture->PreRender();

  // every used face into its own atlas slot, then the sample pass, all in
  // one frame: one submit instead of one standalone frame per pass, which
  // costs about 0.6 ms each on NVIDIA Vulkan. The first face pass clears
  // the whole attachment, the others keep it.
  FilamentRenderEngine::Instance()->BeginFrame();
  unsigned int slot = 0u;
  for (int f = 0; f < 6; ++f)
  {
    if (!this->faceUsed[f])
      continue;
    tm.setTransform(camInst, FaceTransform(kFaces[f]));
    const filament::Viewport vp{static_cast<int32_t>(slot * n), 0, n, n};
    this->renderTexture->Render(this->view, &vp, slot == 0u);
    ++slot;
  }
  this->scanTexture->Render(this->samplePass->view);
  FilamentRenderEngine::Instance()->EndFrame();
  this->ranges.resize(static_cast<size_t>(hCount) * vCount);
  // 4 bytes per ray that are the float range, little endian
  renderer->readPixels(this->scanTexture->Native(), 0, 0, hCount, vCount,
      filament::backend::PixelBufferDescriptor(this->ranges.data(),
      this->ranges.size() * sizeof(float),
      filament::backend::PixelDataFormat::RGBA,
      filament::backend::PixelDataType::UBYTE));
  engine->flushAndWait();
  for (FilamentMesh *m : meshes)
    m->RestoreMaterials();
}

//////////////////////////////////////////////////
void FilamentGpuRays::PostRender()
{
  const unsigned int hCount = std::max(this->RangeCount(), 1);
  const unsigned int vCount = std::max(this->VerticalRangeCount(), 1);
  const unsigned int channels = this->Channels();
  // only the range channel changes from frame to frame: zero the scan
  // when its size changes, then write ranges over the old ones
  const size_t scanSize = static_cast<size_t>(hCount) * vCount * channels;
  if (this->scan.size() != scanSize)
    this->scan.assign(scanSize, 0.0f);
  if (this->rays.size() != static_cast<size_t>(hCount) * vCount)
  {
    std::fill(this->scan.begin(), this->scan.end(), 0.0f);
    return;
  }

  // particles: half resolution depth of the nearest particle per used cube
  // face, like ogre2's particle pass, sampled at the ray's texel
  namespace fp = filament_particles;
  fp::Noise noise;
  const unsigned int n = this->faceRes;
  const unsigned int pn = n / 2u;
  const size_t faceSize = static_cast<size_t>(pn) * pn;
  bool particles = fp::FindNoise(*this->scene, noise);
  std::minstd_rand rng;
  if (particles)
  {
    // per frame seed, like ogre2's rnd uniform
    rng.seed(static_cast<unsigned int>(
        math::Rand::IntUniform(1, std::numeric_limits<int>::max())));
    this->particleDepth.resize(6u * faceSize);
    const math::Pose3d world = this->WorldPose();
    particles = false;
    for (int f = 0; f < 6; ++f)
    {
      if (!this->faceUsed[f])
        continue;
      const Face &fc = kFaces[f];
      const float3 r = cross(fc.fwd, fc.up);
      particles |= fp::Raster(*this->scene, world.Pos(),
          world.Rot() * math::Vector3d(fc.fwd.x, fc.fwd.y, fc.fwd.z),
          world.Rot() * math::Vector3d(-r.x, -r.y, -r.z),
          world.Rot() * math::Vector3d(fc.up.x, fc.up.y, fc.up.z),
          pn * 0.5, pn * 0.5, pn, pn, this->NearClipPlane() / std::sqrt(3.0),
          this->particleDepth.data() + f * faceSize);
    }
  }
  std::uniform_real_distribution<double> uniform(0.0, 1.0);
  const unsigned int stride = n * this->faceCount;

  const float nearClip = static_cast<float>(this->NearClipPlane());
  const float farClip = static_cast<float>(this->FarClipPlane());
  const float maxVal = this->dataMaxVal;
  const float minVal = this->dataMinVal;
  if (!particles && this->ranges.size() >= this->rays.size())
  {
    // same clamping as the loop below, without its per-ray particle and
    // bounds checks: non finite or beyond far -> max, nearer than near -> min
    const float *src = this->ranges.data();
    float *dst = this->scan.data();
    const size_t count = this->rays.size();
    for (size_t k = 0; k < count; ++k)
    {
      const float range = src[k];
      dst[k * channels] = !std::isfinite(range) || range > farClip ?
          maxVal : (range < nearClip ? minVal : range);
    }
    this->newFrame(this->scan.data(), hCount, vCount, channels,
        "PF_FLOAT32_RGB");
    return;
  }
  for (size_t k = 0; k < this->rays.size(); ++k)
  {
    float range = k < this->ranges.size() ? this->ranges[k] :
        std::numeric_limits<float>::infinity();
    if (particles)
    {
      const Ray &ray = this->rays[k];
      const unsigned int px = ray.texel % stride % n;
      const unsigned int py = ray.texel / stride;
      const float pd = this->particleDepth[ray.face * faceSize +
          (py / 2u) * pn + px / 2u];
      if (pd < range / ray.invCos)
      {
        range = static_cast<float>(fp::Scatter(uniform(rng), noise,
            pd * ray.invCos, range));
      }
    }
    if (!std::isfinite(range) || range > farClip)
      range = this->dataMaxVal;
    else if (range < nearClip)
      range = this->dataMinVal;
    this->scan[k * channels] = range;
  }
  this->newFrame(this->scan.data(), hCount, vCount, channels,
      "PF_FLOAT32_RGB");
}

//////////////////////////////////////////////////
const float *FilamentGpuRays::Data() const
{
  return this->scan.data();
}

//////////////////////////////////////////////////
void FilamentGpuRays::Copy(float *_dataDest)
{
  std::memcpy(_dataDest, this->scan.data(), this->scan.size() * sizeof(float));
}

//////////////////////////////////////////////////
common::ConnectionPtr FilamentGpuRays::ConnectNewGpuRaysFrame(
    std::function<void(const float *, unsigned int, unsigned int,
    unsigned int, const std::string &)> _subscriber)
{
  return this->newFrame.Connect(_subscriber);
}
