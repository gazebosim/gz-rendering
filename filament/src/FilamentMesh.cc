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

#include <vector>

#include <filament/Engine.h>
#include <filament/Material.h>
#include <filament/MaterialInstance.h>
#include <filament/IndexBuffer.h>
#include <filament/RenderableManager.h>
#include <filament/Scene.h>
#include <filament/TextureSampler.h>
#include <filament/TransformManager.h>
#include <filament/VertexBuffer.h>
#include <utils/EntityManager.h>

#include <gz/common/Console.hh>
#include <gz/common/Mesh.hh>
#include <gz/common/SubMesh.hh>

#include "gz/rendering/MeshDescriptor.hh"

#include "gz/rendering/filament/FilamentMaterial.hh"
#include "gz/rendering/filament/FilamentMesh.hh"
#include "gz/rendering/filament/FilamentRenderEngine.hh"
#include "gz/rendering/filament/FilamentScene.hh"
#include "FilamentUtils.hh"

using namespace gz;
using namespace rendering;

namespace
{
  filament::Engine *Engine()
  {
    return FilamentRenderEngine::Instance()->FilamentEngine();
  }
}

//////////////////////////////////////////////////
FilamentMesh::FilamentMesh()
{
  this->subMeshes = FilamentSubMeshStorePtr(new FilamentSubMeshStore);
}

//////////////////////////////////////////////////
FilamentMesh::~FilamentMesh()
{
  // freed without Destroy(): never leave a dangling pointer in the scene
  // registry that sensors iterate every frame
  if (this->scene)
    this->scene->UnregisterMesh(this);
}

//////////////////////////////////////////////////
void FilamentMesh::Destroy()
{
  if (this->entity.isNull())
    return;
  // Destroy the submeshes first so the renderable component (which owns
  // references to vertex/index buffers) goes away before the buffers do.
  auto *engine = Engine();
  this->DestroyHiddenSurface();
  if (engine)
  {
    if (this->scene && this->scene->Native())
      this->scene->Native()->remove(this->entity);
    engine->getRenderableManager().destroy(this->entity);
  }
  BaseMesh::Destroy();
  this->subMeshes->DestroyAll();
  if (engine)
  {
    engine->getTransformManager().destroy(this->entity);
    ::utils::EntityManager::get().destroy(this->entity);
  }
  if (this->scene)
    this->scene->UnregisterMesh(this);
  this->entity = ::utils::Entity();
}

//////////////////////////////////////////////////
::utils::Entity FilamentMesh::Entity() const
{
  return this->entity;
}

//////////////////////////////////////////////////
void FilamentMesh::SetLayers(uint8_t _layers)
{
  auto &rm = Engine()->getRenderableManager();
  if (!rm.hasComponent(this->entity))
    return;
  rm.setLayerMask(rm.getInstance(this->entity), 0xff, _layers);
  if (rm.hasComponent(this->hiddenSurface))
    rm.setLayerMask(rm.getInstance(this->hiddenSurface), 0xff, _layers);
}

//////////////////////////////////////////////////
::utils::Entity FilamentMesh::HiddenSurface() const
{
  return this->hiddenSurface;
}

//////////////////////////////////////////////////
void FilamentMesh::DestroyHiddenSurface()
{
  auto *engine = Engine();
  if (this->hiddenSurface.isNull() || !engine)
    return;
  if (this->scene && this->scene->Native())
    this->scene->Native()->remove(this->hiddenSurface);
  engine->getRenderableManager().destroy(this->hiddenSurface);
  engine->getTransformManager().destroy(this->hiddenSurface);
  ::utils::EntityManager::get().destroy(this->hiddenSurface);
  this->hiddenSurface = ::utils::Entity();
}

//////////////////////////////////////////////////
void FilamentMesh::OverrideMaterial(filament::MaterialInstance *_instance,
    bool _alphaTest)
{
  for (auto it = this->subMeshes->Begin(); it != this->subMeshes->End(); ++it)
    (*it)->ApplyInstance(_instance, _alphaTest);
}

//////////////////////////////////////////////////
void FilamentMesh::OverrideMaterial(unsigned int _index,
    filament::MaterialInstance *_instance)
{
  auto sub = std::dynamic_pointer_cast<FilamentSubMesh>(
      this->subMeshes->GetByIndex(_index));
  if (sub)
    sub->ApplyInstance(_instance);
}

//////////////////////////////////////////////////
bool FilamentMesh::HasAlphaTestedOverride() const
{
  for (auto it = this->subMeshes->Begin(); it != this->subMeshes->End(); ++it)
  {
    if ((*it)->alphaCopy && (*it)->bound == (*it)->alphaCopy)
      return true;
  }
  return false;
}

//////////////////////////////////////////////////
void FilamentMesh::RestoreMaterials()
{
  for (auto it = this->subMeshes->Begin(); it != this->subMeshes->End(); ++it)
  {
    FilamentMaterial *mat = (*it)->filamentMaterial;
    (*it)->ApplyInstance(mat ? mat->Instance() : nullptr);
  }
}

//////////////////////////////////////////////////
void FilamentMesh::AddSubMesh(FilamentSubMeshPtr _subMesh)
{
  _subMesh->id = this->scene->CreateObjectId();
  _subMesh->name = this->scene->CreateObjectName(_subMesh->id,
      this->Name() + "_SubMesh");
  _subMesh->scene = this->scene;
  _subMesh->Load();
  _subMesh->Init();
  this->subMeshes->Add(_subMesh);
}

//////////////////////////////////////////////////
void FilamentMesh::ReplaceInstance(filament::MaterialInstance *_old,
    filament::MaterialInstance *_new)
{
  for (auto it = this->subMeshes->Begin(); it != this->subMeshes->End(); ++it)
  {
    if ((*it)->bound == _old)
      (*it)->ApplyInstance(_new);
  }
}

//////////////////////////////////////////////////
SubMeshStorePtr FilamentMesh::SubMeshes() const
{
  return this->subMeshes;
}

//////////////////////////////////////////////////
void FilamentMesh::Init()
{
  BaseMesh::Init();
  this->entity = ::utils::EntityManager::get().create();
  Engine()->getTransformManager().create(this->entity);
  this->scene->RegisterMesh(this);
}

//////////////////////////////////////////////////
void FilamentSubMesh::Destroy()
{
  BaseSubMesh::Destroy();
  // the renderable is gone already (FilamentMesh::Destroy)
  if (this->lineBuffer && Engine())
    Engine()->destroy(this->lineBuffer);
  this->lineBuffer = nullptr;
  if (this->alphaCopy && Engine())
    Engine()->destroy(this->alphaCopy);
  this->alphaCopy = nullptr;
  this->geometry.reset();
  this->vertexBuffer = nullptr;
  this->indexBuffer = nullptr;
  this->mesh = nullptr;
}

//////////////////////////////////////////////////
void FilamentMesh::PreRender()
{
  BaseMesh::PreRender();
  this->UpdateBones();
  // filament casts shadows per renderable, gz per material: cast if any
  // submesh material does
  bool cast = false;
  for (auto it = this->subMeshes->Begin(); it != this->subMeshes->End(); ++it)
  {
    MaterialPtr mat = (*it)->Material();
    cast = cast || (mat && mat->CastShadows());
  }
  auto &rm = Engine()->getRenderableManager();
  if (rm.hasComponent(this->entity))
    rm.setCastShadows(rm.getInstance(this->entity), cast);
}

//////////////////////////////////////////////////
void FilamentMesh::SetWireframe(bool _wireframe)
{
  auto *engine = Engine();
  auto &rm = engine->getRenderableManager();
  if (!rm.hasComponent(this->entity))
    return;
  auto inst = rm.getInstance(this->entity);
  MeshDescriptor desc = this->meshDescriptor;
  desc.Load();
  using PT = filament::RenderableManager::PrimitiveType;
  if (!_wireframe)
  {
    this->DestroyHiddenSurface();
    rm.setChannel(inst, 2u);
  }
  for (auto it = this->subMeshes->Begin(); it != this->subMeshes->End(); ++it)
  {
    FilamentSubMeshPtr sub = *it;
    if (!sub->indexBuffer)
      continue;
    if (!_wireframe)
    {
      if (!sub->lineBuffer)
        continue;
      rm.setGeometryAt(inst, sub->index, PT::TRIANGLES, sub->vertexBuffer,
          sub->indexBuffer, 0, sub->indexBuffer->getIndexCount());
      engine->destroy(sub->lineBuffer);
      sub->lineBuffer = nullptr;
      continue;
    }
    if (sub->lineBuffer || !desc.mesh)
      continue;
    auto s = desc.mesh->SubMeshByIndex(sub->subMeshIndex).lock();
    if (!s || s->SubMeshPrimitiveType() != common::SubMesh::TRIANGLES)
      continue;
    // each triangle's three edges (shared edges drawn twice)
    const unsigned int tris = s->IndexCount() / 3u;
    auto *idx = new uint32_t[tris * 6u];
    for (unsigned int t = 0; t < tris; ++t)
    {
      for (unsigned int e = 0; e < 3u; ++e)
      {
        idx[t * 6u + e * 2u] = static_cast<uint32_t>(s->Index(t * 3u + e));
        idx[t * 6u + e * 2u + 1u] =
            static_cast<uint32_t>(s->Index(t * 3u + (e + 1u) % 3u));
      }
    }
    sub->lineBuffer = filament::IndexBuffer::Builder()
        .indexCount(tris * 6u)
        .bufferType(filament::IndexBuffer::IndexType::UINT)
        .build(*engine);
    sub->lineBuffer->setBuffer(*engine,
        filament::IndexBuffer::BufferDescriptor(idx, tris * 6u * 4u,
        &filament_utils::FreeBuffer<uint32_t>));
    rm.setGeometryAt(inst, sub->index, PT::LINES, sub->vertexBuffer,
        sub->lineBuffer, 0, tris * 6u);
  }

  // ogre2 culls the back faces in wireframe mode. Lines have no facing:
  // draw the triangles depth only (pushed back, before the lines) so the
  // back edges are hidden behind the surface. Both go to channel 3, after
  // everything else (sky included, channel 2): the surface hides only the
  // mesh's own edges, the rest stays visible through the gaps like ogre2.
  // Not for skinned meshes (static pose), and open meshes keep the edges
  // seen through holes.
  filament::MaterialInstance *depthOnly = nullptr;
  if (auto *m = FilamentRenderEngine::Instance()->Material("gz_depth_only"))
    depthOnly = m->getDefaultInstance();
  if (!_wireframe || !this->hiddenSurface.isNull() || this->skeleton ||
      !depthOnly)
    return;
  std::vector<FilamentSubMeshPtr> subs;
  for (auto it = this->subMeshes->Begin(); it != this->subMeshes->End(); ++it)
  {
    if ((*it)->lineBuffer)
      subs.push_back(*it);
  }
  if (subs.empty())
    return;
  depthOnly->setPolygonOffset(1.0f, 1.0f);
  this->hiddenSurface = ::utils::EntityManager::get().create();
  filament::RenderableManager::Builder builder(subs.size());
  for (size_t i = 0; i < subs.size(); ++i)
  {
    builder.geometry(i, PT::TRIANGLES, subs[i]->vertexBuffer,
        subs[i]->indexBuffer, 0, subs[i]->indexBuffer->getIndexCount());
    builder.material(i, depthOnly);
  }
  builder.boundingBox(rm.getAxisAlignedBoundingBox(inst))
      .castShadows(false)
      .receiveShadows(false)
      .priority(0)
      .channel(3u)
      .layerMask(0xff, rm.getLayerMask(inst))
      .build(*engine, this->hiddenSurface);
  auto &tm = engine->getTransformManager();
  rm.setChannel(inst, 3u);
  tm.create(this->hiddenSurface, tm.getInstance(this->entity));
  filament::Scene *native = this->scene ? this->scene->Native() : nullptr;
  if (native && native->hasEntity(this->entity))
    native->addEntity(this->hiddenSurface);
}

//////////////////////////////////////////////////
void FilamentSubMesh::PreRender()
{
  BaseSubMesh::PreRender();
  // The gz material may have switched between its lit/unlit instances
  FilamentMaterial *mat = this->filamentMaterial;
  if (mat && mat->Instance() != this->bound)
    this->ApplyInstance(mat->Instance());
}

//////////////////////////////////////////////////
void FilamentSubMesh::SetMaterialImpl(MaterialPtr _material)
{
  FilamentMaterialPtr derived =
      std::dynamic_pointer_cast<FilamentMaterial>(_material);
  if (!derived)
  {
    gzerr << "Cannot assign material created by another render-engine"
          << std::endl;
    this->filamentMaterial = nullptr;
    return;
  }
  // ApplyInstance still sees the previous material, like Material() does
  // until BaseSubMesh::SetMaterial stores _material
  this->ApplyInstance(derived->Instance());
  this->filamentMaterial = derived.get();
}

//////////////////////////////////////////////////
void FilamentSubMesh::ApplyInstance(filament::MaterialInstance *_instance,
    bool _alphaTest)
{
  if (!this->mesh || !_instance)
    return;
  auto &rm = Engine()->getRenderableManager();
  if (!rm.hasComponent(this->mesh->entity))
    return;
  // alpha from texture: ogre2's depth, lidar, segmentation and thermal
  // passes keep the datablock's alpha test, so sensor overrides discard the
  // cut-out texels too
  filament::MaterialInstance *copy = nullptr;
  FilamentMaterial *mat = this->filamentMaterial;
  if (_alphaTest && mat && mat->AlphaTested() && _instance != mat->Instance()
      && _instance->getMaterial()->hasParameter("alphaMap"))
  {
    // a fresh copy per override: sensors change their instances'
    // parameters every frame.
    // TODO: one copy per alpha tested submesh and sensor pass, cache per
    // override instance if forests of them show up in sensor profiles
    copy = filament::MaterialInstance::duplicate(_instance);
    copy->setParameter("alphaMap", mat->FilamentTexture(),
        filament::TextureSampler(
            filament::TextureSampler::MinFilter::LINEAR_MIPMAP_LINEAR,
            filament::TextureSampler::MagFilter::LINEAR,
            filament::TextureSampler::WrapMode::REPEAT));
    copy->setParameter("alphaThreshold",
        static_cast<float>(mat->AlphaThreshold()));
    if (mat->TwoSidedEnabled())
      copy->setCullingMode(filament::MaterialInstance::CullingMode::NONE);
    _instance = copy;
  }
  rm.setMaterialInstanceAt(rm.getInstance(this->mesh->entity), this->index,
      _instance);
  this->bound = _instance;
  if (this->alphaCopy)
    Engine()->destroy(this->alphaCopy);
  this->alphaCopy = copy;
}
