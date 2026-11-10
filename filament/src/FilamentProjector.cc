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
#include <gz/common/Filesystem.hh>
#include <gz/common/Image.hh>
#include <gz/math/Matrix4.hh>

#include "gz/rendering/Utils.hh"
#include "gz/rendering/filament/FilamentProjector.hh"
#include "gz/rendering/filament/FilamentRenderEngine.hh"
#include "gz/rendering/filament/FilamentScene.hh"

using namespace gz;
using namespace rendering;

//////////////////////////////////////////////////
FilamentProjector::FilamentProjector() = default;

//////////////////////////////////////////////////
FilamentProjector::~FilamentProjector()
{
  this->Destroy();
}

//////////////////////////////////////////////////
void FilamentProjector::Init()
{
  BaseProjector::Init();
  this->scene->RegisterProjector(this);
}

//////////////////////////////////////////////////
void FilamentProjector::Destroy()
{
  if (this->scene)
    this->scene->UnregisterProjector(this);
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
  BaseProjector::Destroy();
}

//////////////////////////////////////////////////
void FilamentProjector::PreRender()
{
  BaseProjector::PreRender();
  // like Ogre2Projector, which enables itself when first rendered
  if (!this->initialized)
  {
    this->initialized = true;
    this->SetEnabled(true);
  }
}

//////////////////////////////////////////////////
void FilamentProjector::SetEnabled(bool _enabled)
{
  BaseProjector::SetEnabled(_enabled);
  this->SetVisible(_enabled);
}

//////////////////////////////////////////////////
bool FilamentProjector::VisibleTo(uint32_t _visibilityMask) const
{
  if (!this->enabled || !this->visible)
    return false;
  const uint32_t flags = this->VisibilityFlags();
  return (flags & GZ_VISIBILITY_ALL) == GZ_VISIBILITY_ALL ||
      (flags & _visibilityMask) != 0u;
}

//////////////////////////////////////////////////
void FilamentProjector::LoadTexture()
{
  if (this->textureName == this->loadedTexture)
    return;
  this->loadedTexture = this->textureName;
  this->texture = nullptr;
  if (!common::isFile(this->textureName))
  {
    gzerr << "Unable to create projector. Projector texture not found: "
          << this->textureName << std::endl;
    return;
  }
  common::Image image(this->textureName);
  if (image.Height() == 0u)
    return;
  this->aspect = static_cast<double>(image.Width()) /
      static_cast<double>(image.Height());
  this->texture = FilamentRenderEngine::Instance()->Texture(this->textureName);
}

//////////////////////////////////////////////////
bool FilamentProjector::CreateResources()
{
  if (this->view)
    return true;
  filament::Material *mat =
      FilamentRenderEngine::Instance()->Material("gz_projector");
  if (!mat)
  {
    gzerr << "gz_projector material not available" << std::endl;
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
filament::View *FilamentProjector::DecalView(
    const filament::math::mat4f &_invViewProj, filament::Texture *_depth)
{
  this->LoadTexture();
  if (!this->texture || !this->CreateResources())
    return nullptr;

  // ogre2's decal volume: the box around the frustum, from the near to the
  // far plane along the projector's X axis
  const double hfov = this->hfov.Radian();
  const double vfov = 2.0 * std::atan(std::tan(hfov / 2.0) / this->aspect);
  const double depth = this->farClip - this->nearClip;
  const double width = 2.0 * std::tan(hfov / 2.0) * this->farClip;
  const double height = 2.0 * std::tan(vfov / 2.0) * this->farClip;
  math::Matrix4d unitFromWorld(this->WorldPose().Inverse());
  const math::Matrix4d unitFromProjector(
      1.0 / depth, 0.0, 0.0, -(this->nearClip + depth * 0.5) / depth,
      0.0, 1.0 / width, 0.0, 0.0,
      0.0, 0.0, 1.0 / height, 0.0,
      0.0, 0.0, 0.0, 1.0);
  unitFromWorld = unitFromProjector * unitFromWorld;
  filament::math::mat4f unit;
  for (int c = 0; c < 4; ++c)
  {
    for (int r = 0; r < 4; ++r)
      unit[c][r] = static_cast<float>(unitFromWorld(r, c));
  }

  auto *mi = this->instance;
  mi->setParameter("depthIn", _depth, filament::TextureSampler());
  mi->setParameter("decalMap", this->texture, filament::TextureSampler(
      filament::TextureSampler::MinFilter::LINEAR,
      filament::TextureSampler::MagFilter::LINEAR,
      filament::TextureSampler::WrapMode::CLAMP_TO_EDGE));
  mi->setParameter("invViewProj", _invViewProj);
  mi->setParameter("unitFromWorld", unit);
  return this->view;
}
