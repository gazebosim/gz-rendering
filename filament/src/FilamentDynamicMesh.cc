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
#include <limits>

#include <filament/Box.h>
#include <filament/Engine.h>
#include <filament/IndexBuffer.h>
#include <filament/Material.h>
#include <filament/MaterialInstance.h>
#include <filament/RenderableManager.h>
#include <filament/VertexBuffer.h>

#include <gz/common/Console.hh>

#include "gz/rendering/filament/FilamentConversions.hh"
#include "gz/rendering/filament/FilamentDynamicMesh.hh"
#include "gz/rendering/filament/FilamentMaterial.hh"
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

  // position(3) color(4) uv0(2) custom0(2): the point corner
  constexpr size_t kFloatsPerVertex = 11u;
  constexpr uint8_t kStride = kFloatsPerVertex * sizeof(float);

  using filament_utils::FreeBuffer;
}

//////////////////////////////////////////////////
FilamentDynamicSubMesh::FilamentDynamicSubMesh()
{
}

//////////////////////////////////////////////////
void FilamentDynamicSubMesh::Setup(FilamentMesh *_mesh)
{
  this->mesh = _mesh;
  this->index = 0u;
}

//////////////////////////////////////////////////
void FilamentDynamicSubMesh::Apply(filament::MaterialInstance *_instance)
{
  this->ApplyInstance(_instance);
}

//////////////////////////////////////////////////
void FilamentDynamicSubMesh::PreRender()
{
  if (this->custom)
    BaseSubMesh<FilamentObject>::PreRender();
  else
    FilamentSubMesh::PreRender();
}

//////////////////////////////////////////////////
FilamentDynamicMesh::FilamentDynamicMesh()
{
}

//////////////////////////////////////////////////
FilamentDynamicMesh::~FilamentDynamicMesh()
{
}

//////////////////////////////////////////////////
void FilamentDynamicMesh::Init()
{
  FilamentMesh::Init();
  this->sub = std::make_shared<FilamentDynamicSubMesh>();
  this->sub->Setup(this);
  this->AddSubMesh(this->sub);
  // ogre2 draws material-less meshes flat unlit white
  MaterialPtr flat = this->scene->CreateMaterial();
  flat->SetLightingEnabled(false);
  this->sub->SetMaterial(flat, false);
  this->sub->OwnMaterial();
}

//////////////////////////////////////////////////
void FilamentDynamicMesh::Release()
{
  auto *engine = Engine();
  if (!engine)
    return;
  // the renderable holds the buffers: it goes first
  auto &rm = engine->getRenderableManager();
  if (rm.hasComponent(this->entity))
    rm.destroy(this->entity);
  if (this->vb)
    engine->destroy(this->vb);
  if (this->ib)
    engine->destroy(this->ib);
  if (this->dotIb)
    engine->destroy(this->dotIb);
  this->vb = nullptr;
  this->ib = nullptr;
  this->dotIb = nullptr;
}

//////////////////////////////////////////////////
void FilamentDynamicMesh::Destroy()
{
  if (this->entity.isNull())
    return;
  this->Release();
  auto *engine = Engine();
  FilamentMesh::Destroy();
  if (engine && this->pointInstance)
    engine->destroy(this->pointInstance);
  this->pointInstance = nullptr;
  this->sub.reset();
}

//////////////////////////////////////////////////
void FilamentDynamicMesh::SetOwnerParent(FilamentVisualPtr _parent)
{
  this->SetParent(_parent);
}

//////////////////////////////////////////////////
void FilamentDynamicMesh::SetOperationType(MarkerType _type)
{
  switch (_type)
  {
    case MT_POINTS:
    case MT_LINE_LIST:
    case MT_LINE_STRIP:
    case MT_TRIANGLE_LIST:
    case MT_TRIANGLE_STRIP:
    case MT_TRIANGLE_FAN:
      break;
    default:
      gzerr << "Unknown render operation type[" << _type << "]" << std::endl;
      return;
  }
  if (_type != this->opType)
  {
    this->opType = _type;
    this->dirty = true;
  }
}

//////////////////////////////////////////////////
MarkerType FilamentDynamicMesh::OperationType() const
{
  return this->opType;
}

//////////////////////////////////////////////////
void FilamentDynamicMesh::AddPoint(const math::Vector3d &_pt,
    const math::Color &_color)
{
  this->vertices.push_back(_pt);
  this->colors.push_back(_color);
  this->dirty = true;
}

//////////////////////////////////////////////////
void FilamentDynamicMesh::SetPoint(unsigned int _index,
    const math::Vector3d &_pt)
{
  if (_index >= this->vertices.size())
  {
    gzerr << "Point index[" << _index << "] is out of bounds[0-"
          << this->vertices.size() - 1 << "]\n";
    return;
  }
  this->vertices[_index] = _pt;
  this->dirty = true;
}

//////////////////////////////////////////////////
void FilamentDynamicMesh::SetColor(unsigned int _index,
    const math::Color &_color)
{
  if (_index >= this->colors.size())
  {
    gzerr << "Point color index[" << _index << "] is out of bounds[0-"
          << this->colors.size() - 1 << "]\n";
    return;
  }
  this->colors[_index] = _color;
  this->dirty = true;
}

//////////////////////////////////////////////////
math::Vector3d FilamentDynamicMesh::Point(unsigned int _index) const
{
  if (_index >= this->vertices.size())
  {
    gzerr << "Point index[" << _index << "] is out of bounds[0-"
          << this->vertices.size() - 1 << "]\n";
    return math::Vector3d(math::INF_D, math::INF_D, math::INF_D);
  }
  return this->vertices[_index];
}

//////////////////////////////////////////////////
unsigned int FilamentDynamicMesh::PointCount() const
{
  return static_cast<unsigned int>(this->vertices.size());
}

//////////////////////////////////////////////////
void FilamentDynamicMesh::Clear()
{
  if (this->vertices.empty() && this->colors.empty())
    return;
  this->vertices.clear();
  this->colors.clear();
  this->dirty = true;
}

//////////////////////////////////////////////////
void FilamentDynamicMesh::SetPointSize(double _size)
{
  this->pointSize = _size;
  if (this->pointInstance)
  {
    this->pointInstance->setParameter("pointSize",
        static_cast<float>(this->pointSize));
  }
}

//////////////////////////////////////////////////
void FilamentDynamicMesh::SetLineMaterial(MaterialPtr _material)
{
  if (_material && this->sub)
  {
    this->sub->SetMaterial(_material, false);
    this->ApplyMaterial();
  }
}

//////////////////////////////////////////////////
void FilamentDynamicMesh::SetMaterial(MaterialPtr _material, bool _unique)
{
  FilamentMesh::SetMaterial(_material, _unique);
  this->ApplyMaterial();
}

//////////////////////////////////////////////////
void FilamentDynamicMesh::SetLayers(uint8_t _layers)
{
  this->layers = _layers;
  FilamentMesh::SetLayers(_layers);
}

//////////////////////////////////////////////////
void FilamentDynamicMesh::OverrideMaterial(
    filament::MaterialInstance *_instance, bool _alphaTest)
{
  FilamentMesh::OverrideMaterial(_instance, _alphaTest);
  this->UseDots(true);
}

//////////////////////////////////////////////////
void FilamentDynamicMesh::OverrideMaterial(unsigned int _index,
    filament::MaterialInstance *_instance)
{
  FilamentMesh::OverrideMaterial(_index, _instance);
  this->UseDots(true);
}

//////////////////////////////////////////////////
void FilamentDynamicMesh::RestoreMaterials()
{
  this->UseDots(false);
  this->ApplyMaterial();
}

//////////////////////////////////////////////////
void FilamentDynamicMesh::UseDots(bool _dots)
{
  if (!this->dotIb || this->entity.isNull())
    return;
  auto &rm = Engine()->getRenderableManager();
  if (!rm.hasComponent(this->entity))
    return;
  using PT = filament::RenderableManager::PrimitiveType;
  rm.setGeometryAt(rm.getInstance(this->entity), 0,
      _dots ? PT::POINTS : PT::TRIANGLES, this->vb,
      _dots ? this->dotIb : this->ib, 0,
      _dots ? this->indexCount / 6u : this->indexCount);  // 6 per square
}

//////////////////////////////////////////////////
void FilamentDynamicMesh::ApplyMaterial()
{
  if (!this->sub)
    return;
  filament::MaterialInstance *instance = this->sub->custom;
  if (!instance)
  {
    auto gzMaterial = std::dynamic_pointer_cast<FilamentMaterial>(
        this->sub->Material());
    if (gzMaterial)
      instance = gzMaterial->Instance();
  }
  if (!instance)
  {
    instance = FilamentRenderEngine::Instance()->Material("gz_unlit")
        ->getDefaultInstance();
  }
  this->sub->Apply(instance);
}

//////////////////////////////////////////////////
void FilamentDynamicMesh::PreRender()
{
  this->Update();
  FilamentMesh::PreRender();
}

//////////////////////////////////////////////////
void FilamentDynamicMesh::Update()
{
  if (!this->dirty || this->entity.isNull())
    return;
  this->dirty = false;

  this->Release();

  const bool points = this->opType == MT_POINTS;
  const size_t n = this->vertices.size();

  // indices into the n vertices (or the 4n corners of the point squares)
  std::vector<uint32_t> idx;
  auto tri = [&idx](uint32_t a, uint32_t b, uint32_t c)
  {
    idx.push_back(a);
    idx.push_back(b);
    idx.push_back(c);
  };
  filament::RenderableManager::PrimitiveType type =
      filament::RenderableManager::PrimitiveType::TRIANGLES;
  switch (this->opType)
  {
    case MT_POINTS:
      for (uint32_t i = 0; i < n; ++i)
      {
        tri(4u * i, 4u * i + 1u, 4u * i + 2u);
        tri(4u * i + 2u, 4u * i + 1u, 4u * i + 3u);
      }
      break;
    case MT_LINE_LIST:
      type = filament::RenderableManager::PrimitiveType::LINES;
      for (uint32_t i = 0; i + 1u < n; i += 2u)
      {
        idx.push_back(i);
        idx.push_back(i + 1u);
      }
      break;
    case MT_LINE_STRIP:
      type = filament::RenderableManager::PrimitiveType::LINES;
      for (uint32_t i = 0; i + 1u < n; ++i)
      {
        idx.push_back(i);
        idx.push_back(i + 1u);
      }
      break;
    case MT_TRIANGLE_LIST:
      for (uint32_t i = 0; i + 2u < n; i += 3u)
        tri(i, i + 1u, i + 2u);
      break;
    case MT_TRIANGLE_STRIP:
      for (uint32_t i = 0; i + 2u < n; ++i)
      {
        if (i % 2u == 0u)
          tri(i, i + 1u, i + 2u);
        else
          tri(i + 1u, i, i + 2u);
      }
      break;
    case MT_TRIANGLE_FAN:
      for (uint32_t i = 0; i + 2u < n; ++i)
        tri(0u, i + 1u, i + 2u);
      break;
    default:
      break;
  }

  if (points && n > 0u && !this->pointInstance)
  {
    filament::Material *pointMaterial =
        FilamentRenderEngine::Instance()->Material("gz_marker_points");
    if (pointMaterial)
      this->pointInstance = pointMaterial->createInstance();
  }
  if (this->pointInstance)
  {
    this->pointInstance->setParameter("pointSize",
        static_cast<float>(this->pointSize));
  }
  this->sub->custom = points ? this->pointInstance : nullptr;

  if (idx.empty() || (points && !this->pointInstance))
    return;

  const size_t vertexCount = points ? 4u * n : n;
  auto *data = new float[vertexCount * kFloatsPerVertex];
  filament::math::float3 lo{std::numeric_limits<float>::max()};
  filament::math::float3 hi{std::numeric_limits<float>::lowest()};
  for (size_t i = 0; i < vertexCount; ++i)
  {
    const size_t src = points ? i / 4u : i;
    const math::Color c = src < this->colors.size() ? this->colors[src] :
        math::Color::White;
    const filament::math::float3 pf =
        FilamentConversions::Convert(this->vertices[src]);
    lo = min(lo, pf);
    hi = max(hi, pf);
    // corner of the square: (-1,-1) (1,-1) (-1,1) (1,1)
    const float cx = (i % 4u) % 2u == 0u ? -1.0f : 1.0f;
    const float cy = (i % 4u) < 2u ? -1.0f : 1.0f;
    float *v = data + i * kFloatsPerVertex;
    v[0] = pf.x; v[1] = pf.y; v[2] = pf.z;
    v[3] = c.R(); v[4] = c.G(); v[5] = c.B(); v[6] = c.A();
    v[7] = 0.0f; v[8] = 0.0f;
    v[9] = points ? cx : 0.0f; v[10] = points ? cy : 0.0f;
  }
  auto *indices = new uint32_t[idx.size()];
  std::copy(idx.begin(), idx.end(), indices);

  auto *engine = Engine();
  using AT = filament::VertexBuffer::AttributeType;
  using VA = filament::VertexAttribute;
  this->vb = filament::VertexBuffer::Builder()
      .vertexCount(static_cast<uint32_t>(vertexCount))
      .bufferCount(1)
      .attribute(VA::POSITION, 0, AT::FLOAT3, 0, kStride)
      .attribute(VA::COLOR, 0, AT::FLOAT4, 12, kStride)
      .attribute(VA::UV0, 0, AT::FLOAT2, 28, kStride)
      .attribute(VA::CUSTOM0, 0, AT::FLOAT2, 36, kStride)
      .build(*engine);
  this->vb->setBufferAt(*engine, 0, filament::VertexBuffer::BufferDescriptor(
      data, vertexCount * kStride, &FreeBuffer<float>));
  this->ib = filament::IndexBuffer::Builder()
      .indexCount(static_cast<uint32_t>(idx.size()))
      .bufferType(filament::IndexBuffer::IndexType::UINT)
      .build(*engine);
  this->ib->setBuffer(*engine, filament::IndexBuffer::BufferDescriptor(
      indices, idx.size() * sizeof(uint32_t), &FreeBuffer<uint32_t>));
  this->indexCount = idx.size();
  if (points)
  {
    // the first corner of each square sits on the point
    auto *dots = new uint32_t[n];
    for (uint32_t i = 0; i < n; ++i)
      dots[i] = 4u * i;
    this->dotIb = filament::IndexBuffer::Builder()
        .indexCount(static_cast<uint32_t>(n))
        .bufferType(filament::IndexBuffer::IndexType::UINT)
        .build(*engine);
    this->dotIb->setBuffer(*engine, filament::IndexBuffer::BufferDescriptor(
        dots, n * sizeof(uint32_t), &FreeBuffer<uint32_t>));
  }

  // keep a flat box from culling away
  const filament::math::float3 pad{1e-3f};
  filament::Box box;
  box.set(lo - pad, hi + pad);
  filament::RenderableManager::Builder(1)
      .boundingBox(box)
      // TODO: the point squares grow past the box, so points are never
      // culled; a pixel-size margin would need the camera
      .culling(!points)
      .castShadows(false)
      .receiveShadows(false)
      .geometry(0, type, this->vb, this->ib, 0, idx.size())
      .build(*engine, this->entity);

  auto &rm = engine->getRenderableManager();
  rm.setLayerMask(rm.getInstance(this->entity), 0xff, this->layers);
  this->ApplyMaterial();
}
