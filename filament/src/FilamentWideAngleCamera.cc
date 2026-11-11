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
#include <vector>

#include <filament/Camera.h>
#include <filament/Engine.h>
#include <filament/Material.h>
#include <filament/MaterialInstance.h>
#include <filament/TextureSampler.h>
#include <filament/TransformManager.h>
#include <filament/View.h>
#include <filament/Viewport.h>

#include <gz/common/Console.hh>
#include <gz/math/Helpers.hh>

#include "gz/rendering/Image.hh"
#include "gz/rendering/PixelFormat.hh"
#include "gz/rendering/LensFlarePass.hh"
#include "gz/rendering/filament/FilamentConversions.hh"
#include "gz/rendering/filament/FilamentRenderEngine.hh"
#include "gz/rendering/filament/FilamentRenderTarget.hh"
#include "gz/rendering/filament/FilamentScene.hh"
#include "gz/rendering/filament/FilamentWideAngleCamera.hh"
#include "FilamentCubeFaces.hh"
#include "FilamentFullscreenPass.hh"

using namespace gz;
using namespace rendering;

namespace
{
  using filament_cube::FaceTransform;
  using filament_cube::kFaces;
}

//////////////////////////////////////////////////
FilamentWideAngleCamera::FilamentWideAngleCamera()
{
}

//////////////////////////////////////////////////
FilamentWideAngleCamera::~FilamentWideAngleCamera()
{
}

//////////////////////////////////////////////////
void FilamentWideAngleCamera::Init()
{
  FilamentCamera::Init();

  this->atlas = std::dynamic_pointer_cast<FilamentRenderTexture>(
      this->scene->CreateRenderTexture());
  this->atlas->SetFormat(PF_R8G8B8);

  filament::Material *mat =
      FilamentRenderEngine::Instance()->Material("gz_wide_angle");
  if (mat)
    this->samplePass = std::make_unique<FilamentFullscreenPass>(*mat);
}

//////////////////////////////////////////////////
void FilamentWideAngleCamera::Destroy()
{
  this->samplePass.reset();
  if (this->atlas)
  {
    this->atlas->Destroy();
    this->atlas.reset();
  }
  FilamentCamera::Destroy();
}

//////////////////////////////////////////////////
RenderTargetPtr FilamentWideAngleCamera::RenderTarget() const
{
  return this->renderTexture;
}

//////////////////////////////////////////////////
uint32_t FilamentWideAngleCamera::EnvTextureSize() const
{
  return this->envTextureSize;
}

//////////////////////////////////////////////////
void FilamentWideAngleCamera::SetEnvTextureSize(uint32_t _size)
{
  this->envTextureSize = std::max(_size, 1u);
}

//////////////////////////////////////////////////
float FilamentWideAngleCamera::FocalLength() const
{
  // same float math as Ogre2WideAngleCamera::PrepareForFinalPass
  if (!this->Lens().ScaleToHFOV())
    return static_cast<float>(this->Lens().F());
  const float localHfov = static_cast<float>(this->HFOV().Radian());
  float param = (localHfov / 2) / this->Lens().C2() + this->Lens().C3();
  float funRes = this->Lens().ApplyMappingFunction(param);
  return 1.0f / (this->Lens().C1() * funRes);
}

//////////////////////////////////////////////////
void FilamentWideAngleCamera::UpdateFaces()
{
  const CameraLens &cl = this->Lens();
  const math::Vector3d fun = cl.MappingFunctionAsVector3d();
  const unsigned int w = this->ImageWidth();
  const unsigned int h = this->ImageHeight();
  const float f = this->FocalLength();
  const std::array<double, 11> key = {static_cast<double>(w),
      static_cast<double>(h), f, cl.C1(), cl.C2(), cl.C3(),
      fun.X(), fun.Y(), fun.Z(), cl.CutOffAngle(),
      static_cast<double>(this->envTextureSize)};
  if (key == this->faceKey)
    return;
  this->faceKey = key;

  // Run the shader's lens math for every pixel and mark the faces it
  // samples, plus the neighbour face when within 2 texels of an edge
  // (bilinear filtering). Pixels outside the cutoff circle come out black.
  this->faceUsed.fill(false);
  const double c1 = cl.C1(), c2 = cl.C2(), c3 = cl.C3();
  const double param2 = cl.CutOffAngle() / c2 + c3;
  const double cutRadius = c1 * f * (fun.X() * std::sin(param2) +
      fun.Y() * std::tan(param2) + fun.Z() * param2);
  const double ratio = static_cast<double>(w) / h;
  const double edge = 1.0 - 4.0 / this->envTextureSize;
  for (unsigned int py = 0; py < h; ++py)
  {
    for (unsigned int px = 0; px < w; ++px)
    {
      const double fx = -((px + 0.5) / w * 2.0 - 1.0);
      const double fy = -(1.0 - (py + 0.5) / h * 2.0) / ratio;
      const double r = std::hypot(fx, fy);
      if (r >= cutRadius || r <= 0.0)
      {
        if (r <= 0.0)
          this->faceUsed[0] = true;
        continue;
      }
      const double param = r / (c1 * f);
      double theta = 0.0;
      if (fun.X() > 0)
        theta = std::asin(param);
      else if (fun.Y() > 0)
        theta = std::atan(param);
      else if (fun.Z() > 0)
        theta = param;
      theta = (theta - c3) * c2;
      if (!std::isfinite(theta))
        continue;
      // sensor frame direction, see gz_wide_angle.mat
      const double s = std::sin(theta) / r;
      const double d[3] = {std::cos(theta), s * fx, -s * fy};
      const double a[3] = {std::abs(d[0]), std::abs(d[1]), std::abs(d[2])};
      const int major = (a[0] >= a[1] && a[0] >= a[2]) ? 0 :
          (a[2] >= a[1] ? 2 : 1);
      for (int k = 0; k < 3; ++k)
      {
        if (k == major || a[k] > edge * a[major])
          this->faceUsed[2 * k + (d[k] < 0.0 ? 1 : 0)] = true;
      }
    }
  }
}

//////////////////////////////////////////////////
void FilamentWideAngleCamera::Render()
{
  if (!this->samplePass)
    return;
  this->UpdateFaces();
  const unsigned int n = this->envTextureSize;

  // cube faces: 90 degree square frustums
  this->camera->setProjection(90.0, 1.0, this->NearClipPlane(),
      this->FarClipPlane(), filament::Camera::Fov::HORIZONTAL);
  this->view->setVisibleLayers(0xff,
      FilamentConversions::MaskToLayers(this->VisibilityMask()));
  if (this->atlas->Width() != 6u * n || this->atlas->Height() != n)
  {
    this->atlas->SetWidth(6u * n);
    this->atlas->SetHeight(n);
  }
  this->atlas->PreRender();
  this->atlas->SetClearColor(
      FilamentConversions::ToFloat4(this->scene->BackgroundColor()));

  auto *engine = FilamentRenderEngine::Instance()->FilamentEngine();
  auto &tm = engine->getTransformManager();
  const auto camInst = tm.getInstance(this->cameraEntity);
  // each face into its slot; the first render clears the whole atlas
  bool first = true;
  for (unsigned int f = 0; f < 6u; ++f)
  {
    if (!this->faceUsed[f])
      continue;
    tm.setTransform(camInst, FaceTransform(kFaces[f]));
    const filament::Viewport vp{static_cast<int32_t>(f * n), 0, n, n};
    this->atlas->Render(this->view, &vp, first);
    first = false;
  }
  tm.setTransform(camInst, FilamentConversions::CameraFix());

  // render passes, like ogre2: on the cube faces, or with
  // WideAngleCameraAfterStitching on the final image
  std::vector<RenderPassPtr> facePasses, finalPasses;
  for (const RenderPassPtr &pass : this->renderTexture->RenderPasses())
  {
    // The lens flare always goes on the stitched image
    const bool final = pass->WideAngleCameraAfterStitching() ||
        std::dynamic_pointer_cast<LensFlarePass>(pass);
    (final ? finalPasses : facePasses).push_back(pass);
  }
  this->atlas->ApplyRenderPasses(facePasses);

  // second pass: lens mapping into the output image
  const CameraLens &cl = this->Lens();
  auto *mi = this->samplePass->instance;
  mi->setParameter("atlas", this->atlas->ColorTexture(),
      filament::TextureSampler(filament::TextureSampler::MinFilter::LINEAR,
      filament::TextureSampler::MagFilter::LINEAR));
  mi->setParameter("cutOffAngle", static_cast<float>(cl.CutOffAngle()));
  mi->setParameter("focal", this->FocalLength());
  mi->setParameter("c1", static_cast<float>(cl.C1()));
  mi->setParameter("c2", static_cast<float>(cl.C2()));
  mi->setParameter("c3", static_cast<float>(cl.C3()));
  mi->setParameter("fun",
      FilamentConversions::Convert(cl.MappingFunctionAsVector3d()));
  mi->setParameter("ratio", static_cast<float>(
      static_cast<double>(this->ImageWidth()) / this->ImageHeight()));
  mi->setParameter("faceRes", static_cast<float>(n));
  // readback rows start at the top on every backend, texture rows at the
  // bottom on OpenGL (see FilamentGpuRays::UploadRays)
  mi->setParameter("flip",
      engine->getBackend() == filament::Engine::Backend::OPENGL ? 1.0f : 0.0f);
  this->renderTexture->Render(this->samplePass->view);
  this->renderTexture->ApplyRenderPasses(finalPasses);
}

//////////////////////////////////////////////////
void FilamentWideAngleCamera::PostRender()
{
  FilamentCamera::PostRender();
  if (this->newImageFrame.ConnectionCount() <= 0u)
    return;

  Image &image = this->image;
  if (image.Width() != this->ImageWidth() ||
      image.Height() != this->ImageHeight() ||
      image.Format() != this->ImageFormat())
  {
    image = this->CreateImage();
  }
  this->Copy(image);
  const PixelFormat format = image.Format();
  const unsigned char *data = image.Data<unsigned char>();
  this->imageData.assign(data, data + image.MemorySize());
  this->newImageFrame(this->imageData.data(), image.Width(), image.Height(),
      PixelUtil::ChannelCount(format), PixelUtil::Name(format));
}

//////////////////////////////////////////////////
void FilamentWideAngleCamera::Copy(Image &_image) const
{
  const PixelFormat format = _image.Format();
  // L8 renders natively into linear R8 (FilamentRenderTarget, like ogre2),
  // so the stitch pass already writes the sRGB-decoded red channel
  if (format != PF_L16)
  {
    this->renderTexture->Copy(_image);
    return;
  }
  if (_image.Width() != this->ImageWidth() ||
      _image.Height() != this->ImageHeight())
  {
    gzerr << "Invalid image dimensions" << std::endl;
    return;
  }
  // ogre2 reads its sRGB RGBA8 image back into R16 unorm with
  // Ogre::PixelFormatGpuUtils::bulkPixelConversion: red, sRGB decoded
  Image rgb(_image.Width(), _image.Height(), PF_R8G8B8);
  this->renderTexture->Copy(rgb);
  static const std::array<uint16_t, 256> lut = []
  {
    std::array<uint16_t, 256> l;
    for (int i = 0; i < 256; ++i)
    {
      const float x = i / 255.0f;
      const float lin = x <= 0.040449907f ? x / 12.92f :
          std::pow((x + 0.055f) / 1.055f, 2.4f);
      l[i] = static_cast<uint16_t>(std::round(lin * 65535.0f));
    }
    return l;
  }();
  const unsigned char *src = rgb.Data<unsigned char>();
  const size_t count = static_cast<size_t>(_image.Width()) * _image.Height();
  uint16_t *dst = _image.Data<uint16_t>();
  for (size_t i = 0; i < count; ++i)
    dst[i] = lut[src[i * 3u]];
}

//////////////////////////////////////////////////
math::Vector3d FilamentWideAngleCamera::Project3d(
    const math::Vector3d &_pt) const
{
  // Ogre2WideAngleCamera::Project3d without the cube face round trip: the
  // point is visible when inside [near, far] along the axis of the face that
  // sees it, its direction in (right, up, forward) is that face's clip-space
  // ray rotated back into the camera.
  const math::Vector3d p =
      this->WorldRotation().Inverse() * (_pt - this->WorldPosition());
  const double depth = std::max({std::abs(p.X()), std::abs(p.Y()),
      std::abs(p.Z())});
  math::Vector3d screenPos;
  if (depth < this->NearClipPlane() || depth > this->FarClipPlane())
    return screenPos;
  math::Vector3d dir(-p.Y(), p.Z(), p.X());
  dir.Normalize();

  // theta is angle to dir vector from z (forward)
  // phi is angle from x in x-y plane
  double theta =
    std::atan2(std::sqrt(dir.X() * dir.X() + dir.Y() * dir.Y()), dir.Z());
  double phi = std::atan2(dir.Y(), dir.X());

  double f = this->Lens().F();
  double fov = this->HFOV().Radian();
  // recompute f if scale to HFOV is true
  if (this->Lens().ScaleToHFOV())
  {
    double param = (fov / 2.0) / this->Lens().C2() + this->Lens().C3();
    double funRes =
      this->Lens().ApplyMappingFunction(static_cast<float>(param));
    f = 1.0 / (this->Lens().C1() * funRes);
  }

  // Apply fisheye lens mapping function
  // r is distance of point from image center
  double r = this->Lens().C1() * f *
             this->Lens().ApplyMappingFunction(theta / this->Lens().C2() +
                                               this->Lens().C3());

  // compute projected x and y in clip space
  double x = cos(phi) * r;
  double y = sin(phi) * r;

  const uint32_t vpWidth = this->ImageWidth();
  const uint32_t vpHeight = this->ImageHeight();
  // adjust projected pos based on aspect ratio
  double asp = static_cast<double>(vpWidth) / static_cast<double>(vpHeight);
  y *= asp;

  // convert to screen space
  screenPos.X() = ((x / 2.0) + 0.5) * vpWidth;
  screenPos.Y() = (1 - ((y / 2.0) + 0.5)) * vpHeight;

  // r will be > 1.0 if point is not visible (outside of image)
  screenPos.Z() = r;
  return screenPos;
}

//////////////////////////////////////////////////
common::ConnectionPtr FilamentWideAngleCamera::ConnectNewWideAngleFrame(
    std::function<void(const unsigned char *, unsigned int, unsigned int,
    unsigned int, const std::string &)> _subscriber)
{
  return this->newImageFrame.Connect(_subscriber);
}
