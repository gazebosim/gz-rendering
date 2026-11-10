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
#include <filament/IndexBuffer.h>
#include <filament/Material.h>
#include <filament/MaterialInstance.h>
#include <filament/RenderableManager.h>
#include <filament/Scene.h>
#include <filament/TextureSampler.h>
#include <filament/VertexBuffer.h>
#include <filament/View.h>
#include <utils/EntityManager.h>

#include <gz/common/Console.hh>
#include <gz/math/Matrix4.hh>
#include <gz/math/Vector4.hh>

#include "gz/rendering/Camera.hh"
#include "gz/rendering/Light.hh"
#include "gz/rendering/RayQuery.hh"
#include "gz/rendering/RenderPassSystem.hh"
#include "gz/rendering/Scene.hh"
#include "gz/rendering/WideAngleCamera.hh"
#include "gz/rendering/filament/FilamentLensFlarePass.hh"
#include "gz/rendering/filament/FilamentRenderEngine.hh"
#include "gz/rendering/filament/FilamentScene.hh"

using namespace gz;
using namespace rendering;

namespace
{
  /// \brief a * v, with the homogeneous w that gz-math leaves out
  math::Vector4d Transform(const math::Matrix4d &_m, const math::Vector3d &_v)
  {
    return {
        _m(0, 0) * _v.X() + _m(0, 1) * _v.Y() + _m(0, 2) * _v.Z() + _m(0, 3),
        _m(1, 0) * _v.X() + _m(1, 1) * _v.Y() + _m(1, 2) * _v.Z() + _m(1, 3),
        _m(2, 0) * _v.X() + _m(2, 1) * _v.Y() + _m(2, 2) * _v.Z() + _m(2, 3),
        _m(3, 0) * _v.X() + _m(3, 1) * _v.Y() + _m(3, 2) * _v.Z() + _m(3, 3)};
  }
}

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
  auto *engine = FilamentRenderEngine::Instance()->FilamentEngine();
  if (engine)
  {
    if (this->view)
      engine->destroy(this->view);
    // the renderable goes before its material instance
    if (!this->triangle.isNull())
    {
      engine->getRenderableManager().destroy(this->triangle);
      ::utils::EntityManager::get().destroy(this->triangle);
    }
    if (this->instance)
      engine->destroy(this->instance);
    if (this->triangleVb)
      engine->destroy(this->triangleVb);
    if (this->triangleIb)
      engine->destroy(this->triangleIb);
    if (this->passScene)
      engine->destroy(this->passScene);
    if (!this->cameraEntity.isNull())
    {
      engine->destroyCameraComponent(this->cameraEntity);
      ::utils::EntityManager::get().destroy(this->cameraEntity);
    }
  }
  this->view = nullptr;
  this->triangle = ::utils::Entity();
  this->instance = nullptr;
  this->triangleVb = nullptr;
  this->triangleIb = nullptr;
  this->passScene = nullptr;
  this->cameraEntity = ::utils::Entity();
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
    const math::Vector4d s = Transform(inv, nearPt);
    const math::Vector4d e = Transform(inv, farPt);
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
bool FilamentLensFlarePass::CreateResources()
{
  if (this->view)
    return true;
  filament::Material *mat =
      FilamentRenderEngine::Instance()->Material("gz_lens_flare");
  if (!mat)
  {
    gzerr << "gz_lens_flare material not available" << std::endl;
    return false;
  }
  auto *engine = FilamentRenderEngine::Instance()->FilamentEngine();
  this->instance = mat->createInstance();
  this->passScene = engine->createScene();
  this->view = engine->createView();
  this->view->setScene(this->passScene);
  this->view->setPostProcessingEnabled(false);
  this->view->setShadowingEnabled(false);
  // a view needs a camera; device-domain vertices ignore it
  this->cameraEntity = ::utils::EntityManager::get().create();
  this->view->setCamera(engine->createCamera(this->cameraEntity));

  // one triangle covering clip space [-1, 1]^2
  static const filament::math::float4 kVerts[3] = {
      {-1.0f, -1.0f, 0.5f, 1.0f}, {3.0f, -1.0f, 0.5f, 1.0f},
      {-1.0f, 3.0f, 0.5f, 1.0f}};
  static const uint16_t kIndices[3] = {0, 1, 2};
  this->triangleVb = filament::VertexBuffer::Builder()
      .vertexCount(3).bufferCount(1)
      .attribute(filament::VertexAttribute::POSITION, 0,
          filament::VertexBuffer::AttributeType::FLOAT4)
      .build(*engine);
  this->triangleVb->setBufferAt(*engine, 0,
      filament::VertexBuffer::BufferDescriptor(kVerts, sizeof(kVerts)));
  this->triangleIb = filament::IndexBuffer::Builder()
      .indexCount(3).bufferType(filament::IndexBuffer::IndexType::USHORT)
      .build(*engine);
  this->triangleIb->setBuffer(*engine,
      filament::IndexBuffer::BufferDescriptor(kIndices, sizeof(kIndices)));
  this->triangle = ::utils::EntityManager::get().create();
  filament::RenderableManager::Builder(1)
      .boundingBox({{-1.0f, -1.0f, -1.0f}, {1.0f, 1.0f, 1.0f}})
      .culling(false)
      .castShadows(false)
      .receiveShadows(false)
      .material(0, this->instance)
      .geometry(0, filament::RenderableManager::PrimitiveType::TRIANGLES,
          this->triangleVb, this->triangleIb)
      .build(*engine, this->triangle);
  this->passScene->addEntity(this->triangle);
  return true;
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
  const math::Vector4d pos = Transform(viewProj, this->lightWorldPos);
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
  if (flareScale <= 0.0 || !this->CreateResources())
    return nullptr;

  auto *mi = this->instance;
  mi->setParameter("colorIn", _colorIn, filament::TextureSampler());
  mi->setParameter("vpAspectRatio", static_cast<float>(aspect));
  mi->setParameter("lightPos", filament::math::float2{
      static_cast<float>(ndc.X()), static_cast<float>(ndc.Y())});
  mi->setParameter("scale", static_cast<float>(flareScale));
  mi->setParameter("color", filament::math::float3{
      static_cast<float>(this->color.X()),
      static_cast<float>(this->color.Y()),
      static_cast<float>(this->color.Z())});
  return this->view;
}

GZ_RENDERING_REGISTER_RENDER_PASS(FilamentLensFlarePass, LensFlarePass)
