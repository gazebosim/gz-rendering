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

#include <filament/Camera.h>
#include <filament/Engine.h>
#include <filament/Material.h>
#include <filament/MaterialInstance.h>
#include <filament/TextureSampler.h>
#include <filament/View.h>

#include <gz/common/Console.hh>
#include <gz/math/Matrix4.hh>
#include <gz/math/Vector4.hh>

#include "gz/rendering/Camera.hh"
#include "gz/rendering/Light.hh"
#include "gz/rendering/RayQuery.hh"
#include "gz/rendering/RenderPassSystem.hh"
#include "gz/rendering/Scene.hh"
#include "gz/rendering/WideAngleCamera.hh"
#include "gz/rendering/filament/FilamentConversions.hh"
#include "gz/rendering/filament/FilamentLensFlarePass.hh"
#include "gz/rendering/filament/FilamentRenderEngine.hh"
#include "gz/rendering/filament/FilamentScene.hh"
#include "FilamentFullscreenPass.hh"

using namespace gz;
using namespace rendering;

//////////////////////////////////////////////////
FilamentLensFlarePass::FilamentLensFlarePass()
{
  // like ogre2, the flare goes on the stitched wide-angle image
  this->afterStitching = true;
}

//////////////////////////////////////////////////
FilamentLensFlarePass::~FilamentLensFlarePass()
{
  this->Destroy();
}

//////////////////////////////////////////////////
void FilamentLensFlarePass::Destroy()
{
  this->rayQuery.reset();
  this->currentCamera.reset();
  this->pass.reset();
}

//////////////////////////////////////////////////
void FilamentLensFlarePass::Init(ScenePtr _scene)
{
  this->scene = std::dynamic_pointer_cast<FilamentScene>(_scene);
  this->rayQuery = _scene->CreateRayQuery();
  this->rayQuery->SetPreferGpu(false);
}

//////////////////////////////////////////////////
void FilamentLensFlarePass::PreRender(const CameraPtr &_camera)
{
  BaseRenderPass::PreRender(_camera);
  if (!this->enabled || !this->light)
    return;
  auto dirLight = std::dynamic_pointer_cast<DirectionalLight>(this->light);
  if (dirLight)
  {
    // ogre2: a directional light is a point far along its direction, which
    // the occlusion casts compare hit distances with
    this->lightWorldPos =
        -(this->light->WorldPose().Rot() * dirLight->Direction()) * 100000.0;
  }
  else
  {
    this->lightWorldPos = this->light->WorldPose().Pos();
  }
  this->currentCamera = _camera;
}

//////////////////////////////////////////////////
void FilamentLensFlarePass::SetScale(double _scale)
{
  this->scale = _scale;
}

//////////////////////////////////////////////////
double FilamentLensFlarePass::Scale() const
{
  return this->scale;
}

//////////////////////////////////////////////////
void FilamentLensFlarePass::SetColor(const math::Vector3d &_color)
{
  this->color = _color;
}

//////////////////////////////////////////////////
const math::Vector3d &FilamentLensFlarePass::Color() const
{
  return this->color;
}

//////////////////////////////////////////////////
void FilamentLensFlarePass::SetOcclusionSteps(uint32_t _steps)
{
  this->occlusionSteps = static_cast<double>(_steps);
}

//////////////////////////////////////////////////
uint32_t FilamentLensFlarePass::OcclusionSteps() const
{
  return static_cast<uint32_t>(this->occlusionSteps);
}

//////////////////////////////////////////////////
double FilamentLensFlarePass::OcclusionScale(const math::Matrix4d &_viewProj,
    const math::Vector3d &_ndc, const math::Vector3d &_lightPos)
{
  if (std::abs(this->occlusionSteps) <= 1e-7 || !this->rayQuery)
    return this->scale;

  // like RayQuery::SetFromCamera, with a matrix of our own
  const math::Matrix4d inv = _viewProj.Inverse();
  auto occluded = [&](double _x, double _y)
  {
    const math::Vector3d nearPt(_x, _y, -1.0);
    const math::Vector3d farPt(_x, _y, 0.0);
    const math::Vector4d s = FilamentConversions::Transform(inv, nearPt);
    const math::Vector4d e = FilamentConversions::Transform(inv, farPt);
    const math::Vector3d start = math::Vector3d(s.X(), s.Y(), s.Z()) / s.W();
    const math::Vector3d end = math::Vector3d(e.X(), e.Y(), e.Z()) / e.W();
    this->rayQuery->SetOrigin(start);
    this->rayQuery->SetDirection((end - start).Normalized());
    const RayQueryResult result = this->rayQuery->ClosestPoint(false);
    return result.distance >= 0.0 &&
        result.point.SquaredLength() < _lightPos.SquaredLength();
  };

  // the light itself hidden: no flare
  if (occluded(_ndc.X(), _ndc.Y()))
    return 0.0;

  // sparse casts around it, in normalized device coordinates; the flare's
  // half size is an approximation
  unsigned int rays = 0u;
  unsigned int hidden = 0u;
  const double halfSize = 0.05 * this->scale;
  const double stepSize = halfSize * 2.0 / this->occlusionSteps;
  const double endX = _ndc.X() + halfSize;
  const double endY = _ndc.Y() + halfSize;
  for (double i = _ndc.Y() - halfSize; i < endY; i += stepSize)
  {
    for (double j = _ndc.X() - halfSize; j < endX; j += stepSize)
    {
      if (occluded(j, i))
        ++hidden;
      ++rays;
    }
  }
  if (rays == 0u)
    return this->scale;
  return static_cast<double>(rays - hidden) / static_cast<double>(rays) *
      this->scale;
}

//////////////////////////////////////////////////
filament::View *FilamentLensFlarePass::ColorView(filament::Texture *_colorIn)
{
  if (!this->enabled || !this->light || !this->currentCamera)
    return nullptr;

  // ogre2 projects with the camera's matrices; a wide-angle camera on the
  // stitched image as a 90 degree, square pinhole
  const CameraPtr &camera = this->currentCamera;
  math::Matrix4d proj;
  double aspect;
  if (std::dynamic_pointer_cast<WideAngleCamera>(camera))
  {
    const double n = camera->NearClipPlane();
    const double f = camera->FarClipPlane();
    proj = math::Matrix4d::Zero;
    proj(0, 0) = 1.0;
    proj(1, 1) = 1.0;
    proj(2, 2) = -(f + n) / (f - n);
    proj(2, 3) = -2.0 * f * n / (f - n);
    proj(3, 2) = -1.0;
    aspect = 1.0;
  }
  else
  {
    proj = camera->ProjectionMatrix();
    aspect = camera->AspectRatio();
  }
  const math::Matrix4d viewProj = proj * camera->ViewMatrix();
  const math::Vector4d pos =
      FilamentConversions::Transform(viewProj, this->lightWorldPos);
  if (!std::isfinite(pos.W()) || pos.W() == 0.0)
    return nullptr;
  // xy in normalized device coordinates; z > 0: in front of the near plane
  const math::Vector3d ndc(pos.X() / pos.W(), pos.Y() / pos.W(),
      pos.Z() + std::abs(pos.W()));
  if (ndc.Z() < 0.0)
    return nullptr;

  const double flareScale =
      this->OcclusionScale(viewProj, ndc, this->lightWorldPos);
  // fully occluded: the shader would add nothing
  if (flareScale <= 0.0 ||
      !FilamentFullscreenPass::Create(this->pass, "gz_lens_flare"))
    return nullptr;

  auto *mi = this->pass->instance;
  mi->setParameter("colorIn", _colorIn, filament::TextureSampler());
  mi->setParameter("vpAspectRatio", static_cast<float>(aspect));
  mi->setParameter("lightPos", filament::math::float2{
      static_cast<float>(ndc.X()), static_cast<float>(ndc.Y())});
  mi->setParameter("scale", static_cast<float>(flareScale));
  mi->setParameter("color", filament::math::float3{
      static_cast<float>(this->color.X()),
      static_cast<float>(this->color.Y()),
      static_cast<float>(this->color.Z())});
  return this->pass->view;
}

GZ_RENDERING_REGISTER_RENDER_PASS(FilamentLensFlarePass, LensFlarePass)
