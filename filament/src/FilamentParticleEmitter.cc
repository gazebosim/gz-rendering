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

#include <filament/Engine.h>
#include <filament/IndexBuffer.h>
#include <filament/Material.h>
#include <filament/MaterialInstance.h>
#include <filament/RenderableManager.h>
#include <filament/Scene.h>
#include <filament/TextureSampler.h>
#include <filament/VertexBuffer.h>
#include <utils/EntityManager.h>

#include <gz/common/Console.hh>
#include <gz/common/Filesystem.hh>
#include <gz/common/Image.hh>
#include <gz/math/Rand.hh>

#include "gz/rendering/Utils.hh"
#include "gz/rendering/filament/FilamentConversions.hh"
#include "gz/rendering/filament/FilamentMaterial.hh"
#include "gz/rendering/filament/FilamentParticleEmitter.hh"
#include "gz/rendering/filament/FilamentRenderEngine.hh"
#include "gz/rendering/filament/FilamentScene.hh"
#include "FilamentUtils.hh"

using namespace gz;
using namespace rendering;

namespace
{
  /// \brief ogre2's particle quota
  constexpr size_t kQuota = 5000u;

  /// \brief One billboard corner
  struct Vertex
  {
    float center[3];
    float uv[2];
    /// \brief Offset from the center in camera right / up units
    float corner[2];
    /// \brief Particle colour, packed like ogre2 packs it (truncated)
    uint8_t rgba[4];
  };

  /// \brief ogre2's ColourImage affector: the first image row, left to right
  /// over the lifetime fraction _t, linearly interpolated; white without one.
  math::Color ColorAt(const std::vector<math::Color> &_row, double _t)
  {
    if (_row.empty())
      return math::Color::White;
    const float last = static_cast<float>(_row.size() - 1u);
    const float f = static_cast<float>(std::clamp(_t, 0.0, 1.0)) * last;
    const size_t i = static_cast<size_t>(f);
    if (i >= _row.size() - 1u)
      return _row.back();
    const float k = f - static_cast<float>(i);
    const math::Color &a = _row[i];
    const math::Color &b = _row[i + 1u];
    return math::Color(a.R() * (1.0f - k) + b.R() * k,
        a.G() * (1.0f - k) + b.G() * k, a.B() * (1.0f - k) + b.B() * k,
        a.A() * (1.0f - k) + b.A() * k);
  }
}

//////////////////////////////////////////////////
FilamentParticleEmitter::FilamentParticleEmitter()
{
}

//////////////////////////////////////////////////
FilamentParticleEmitter::~FilamentParticleEmitter()
{
  // never leave a dangling pointer in the registry sensors iterate
  if (this->scene)
    this->scene->UnregisterParticleEmitter(this);
}

//////////////////////////////////////////////////
void FilamentParticleEmitter::Init()
{
  BaseParticleEmitter<FilamentVisual>::Init();
  auto *engine = FilamentRenderEngine::Instance()->FilamentEngine();
  filament::Material *mat =
      FilamentRenderEngine::Instance()->Material("gz_particle");
  if (!engine || !mat)
  {
    gzerr << "gz_particle material not available" << std::endl;
    return;
  }
  const uint32_t stride = sizeof(Vertex);
  this->vb = filament::VertexBuffer::Builder()
      .vertexCount(static_cast<uint32_t>(kQuota * 4u)).bufferCount(1)
      .attribute(filament::VertexAttribute::POSITION, 0,
          filament::VertexBuffer::AttributeType::FLOAT3, 0, stride)
      .attribute(filament::VertexAttribute::UV0, 0,
          filament::VertexBuffer::AttributeType::FLOAT2, 12, stride)
      .attribute(filament::VertexAttribute::CUSTOM0, 0,
          filament::VertexBuffer::AttributeType::FLOAT2, 20, stride)
      .attribute(filament::VertexAttribute::COLOR, 0,
          filament::VertexBuffer::AttributeType::UBYTE4, 28, stride)
      .normalized(filament::VertexAttribute::COLOR)
      .build(*engine);
  auto *indices = new uint16_t[kQuota * 6u];
  for (size_t q = 0; q < kQuota; ++q)
  {
    const uint16_t v = static_cast<uint16_t>(q * 4u);
    const uint16_t quad[6] = {v, static_cast<uint16_t>(v + 1u),
        static_cast<uint16_t>(v + 2u), v, static_cast<uint16_t>(v + 2u),
        static_cast<uint16_t>(v + 3u)};
    std::copy(quad, quad + 6, indices + q * 6u);
  }
  this->ib = filament::IndexBuffer::Builder()
      .indexCount(static_cast<uint32_t>(kQuota * 6u))
      .bufferType(filament::IndexBuffer::IndexType::USHORT)
      .build(*engine);
  this->ib->setBuffer(*engine, filament::IndexBuffer::BufferDescriptor(
      indices, kQuota * 6u * sizeof(uint16_t),
      &filament_utils::FreeBuffer<uint16_t>));
  this->mi = mat->createInstance();

  // world space particles: no transform component, hidden until emitted
  this->renderable = ::utils::EntityManager::get().create();
  filament::RenderableManager::Builder(1)
      .boundingBox({{-1.0f, -1.0f, -1.0f}, {1.0f, 1.0f, 1.0f}})
      .culling(false)
      .castShadows(false)
      .receiveShadows(false)
      .layerMask(0xff, 0u)
      .material(0, this->mi)
      .geometry(0, filament::RenderableManager::PrimitiveType::TRIANGLES,
          this->vb, this->ib, 0, 6)
      .build(*engine, this->renderable);
  this->scene->Native()->addEntity(this->renderable);
  this->scene->RegisterParticleEmitter(this);
}

//////////////////////////////////////////////////
void FilamentParticleEmitter::Destroy()
{
  if (this->scene)
    this->scene->UnregisterParticleEmitter(this);
  auto *engine = FilamentRenderEngine::Instance()->FilamentEngine();
  if (engine && !this->renderable.isNull())
  {
    if (this->scene && this->scene->Native())
      this->scene->Native()->remove(this->renderable);
    // the renderable first: it references the buffers and the instance
    engine->getRenderableManager().destroy(this->renderable);
    ::utils::EntityManager::get().destroy(this->renderable);
    engine->destroy(this->vb);
    engine->destroy(this->ib);
    engine->destroy(this->mi);
  }
  this->renderable = ::utils::Entity();
  this->vb = nullptr;
  this->ib = nullptr;
  this->mi = nullptr;
  this->particles.clear();
  BaseParticleEmitter<FilamentVisual>::Destroy();
}

//////////////////////////////////////////////////
void FilamentParticleEmitter::SetColorRange(const math::Color &,
    const math::Color &)
{
  // see https://github.com/gazebosim/gz-rendering/issues/902
  gzwarn << "ParticleEmitter SetColorRange is currently disabled." << std::endl;
}

//////////////////////////////////////////////////
void FilamentParticleEmitter::SetColorRangeImage(const std::string &_image)
{
  if (!common::exists(_image) || !common::isFile(_image))
  {
    gzerr << "SetColorRangeImage() error: Texture [" << _image << "] not found"
          << std::endl;
    return;
  }
  const common::Image img(_image);
  if (!img.Valid())
  {
    gzerr << "Ignoring SetColorRangeImage(): unable to read [" << _image
          << "]" << std::endl;
    return;
  }
  // only the horizontal dimension is used, like ogre2
  this->colorRow.clear();
  for (unsigned int x = 0; x < img.Width(); ++x)
    this->colorRow.push_back(img.Pixel(x, 0));
  BaseParticleEmitter<FilamentVisual>::SetColorRangeImage(_image);
}

//////////////////////////////////////////////////
const std::vector<FilamentParticleEmitter::Particle> &
    FilamentParticleEmitter::Particles() const
{
  static const std::vector<Particle> kNone;
  return this->visible ? this->particles : kNone;
}

//////////////////////////////////////////////////
math::AxisAlignedBox FilamentParticleEmitter::ParticleBounds() const
{
  math::AxisAlignedBox box;
  for (const Particle &p : this->Particles())
  {
    const double pad = 0.5 * std::max(p.width, p.height);
    box.Merge(math::AxisAlignedBox(p.pos - pad, p.pos + pad));
  }
  if (box.Min().X() > box.Max().X())
    return box;
  // ogre2 stores a local box and returns its world box
  const math::Pose3d world = this->WorldPose();
  return transformAxisAlignedBox(
      transformAxisAlignedBox(box, world.Inverse()), world);
}

//////////////////////////////////////////////////
void FilamentParticleEmitter::PreRender()
{
  FilamentVisual::PreRender();
  // ogre2 advances its particle systems by the scene time elapsed since
  // the last rendered frame
  const auto now = this->scene->Time();
  if (!this->stepped)
  {
    this->lastTime = now;
    this->stepped = true;
  }
  const double dt = std::chrono::duration<double>(now - this->lastTime).count();
  this->lastTime = now;
  if (dt > 0.0)
    this->Step(dt);
  this->UpdateRenderable();
}

//////////////////////////////////////////////////
void FilamentParticleEmitter::Step(double _dt)
{
  // expire
  this->particles.erase(std::remove_if(this->particles.begin(),
      this->particles.end(), [_dt](const Particle &_p)
      { return _p.ttl < _dt; }), this->particles.end());
  // grow (Scaler affector), then move
  const double ds = this->scaleRate * _dt;
  for (Particle &p : this->particles)
  {
    p.ttl -= _dt;
    // ogre2 runs the colour affector after the expiry has aged the particle
    p.color = ColorAt(this->colorRow, 1.0 - p.ttl / p.life);
    p.width += ds;
    p.height += ds;
    if (p.width < 0.0 || p.height < 0.0)
      p.width = p.height = 0.0;
    p.pos += p.vel * _dt;
  }

  if (!this->emitting ||
      (this->duration > 0.0 && this->emitTime >= this->duration))
    return;
  this->emitTime += _dt;
  this->remainder += this->rate * _dt;
  const unsigned int count = static_cast<unsigned int>(this->remainder);
  this->remainder -= count;

  // ogre2's emitters: direction is the emitter's X axis; area emitters
  // spread width (size X) along Y, height (size Y) along Z and depth
  // (size Z) along X
  const math::Pose3d world = this->WorldPose();
  const math::Vector3d half = this->emitterSize * 0.5;
  auto sym = [] { return math::Rand::DblUniform(-1.0, 1.0); };
  for (unsigned int j = 0; j < count && this->particles.size() < kQuota; ++j)
  {
    double x = 0.0, y = 0.0, z = 0.0;
    switch (this->type)
    {
      case EM_BOX:
        x = sym(); y = sym(); z = sym();
        break;
      case EM_CYLINDER:
        do { x = sym(); y = sym(); } while (x * x + y * y > 1.0);
        z = sym();
        break;
      case EM_ELLIPSOID:
        do { x = sym(); y = sym(); z = sym(); }
        while (x * x + y * y + z * z > 1.0);
        break;
      default:
        break;
    }
    const math::Vector3d local(z * half.Z(), x * half.X(), y * half.Y());
    const double speed = this->maxVelocity > this->minVelocity ?
        math::Rand::DblUniform(this->minVelocity, this->maxVelocity) :
        this->minVelocity;
    Particle p;
    p.vel = world.Rot() * math::Vector3d(speed, 0.0, 0.0);
    // ogre2 spreads the emissions of one update over its time step
    p.pos = world.Pos() + world.Rot() * local + p.vel * (j * _dt / count);
    p.ttl = p.life = this->lifetime;
    p.color = ColorAt(this->colorRow, 0.0);
    p.width = this->particleSize.X();
    p.height = this->particleSize.Y();
    this->particles.push_back(p);
  }
}

//////////////////////////////////////////////////
void FilamentParticleEmitter::UpdateRenderable()
{
  if (this->renderable.isNull())
    return;
  auto *engine = FilamentRenderEngine::Instance()->FilamentEngine();
  auto &rm = engine->getRenderableManager();
  const auto inst = rm.getInstance(this->renderable);
  const size_t count = std::min(this->particles.size(), kQuota);
  rm.setLayerMask(inst, 0xff, (this->visible && count > 0u) ?
      FilamentConversions::kParticleLayer : 0u);
  if (count == 0u)
    return;
  this->Upload();

  // the material's diffuse and texture; the particle colours are the vertices'
  math::Color color = math::Color::White;
  filament::Texture *tex = nullptr;
  auto fm = std::dynamic_pointer_cast<FilamentMaterial>(this->material);
  if (fm)
  {
    fm->PreRender();
    color = fm->Diffuse();
    color.A(static_cast<float>(1.0 - fm->Transparency()));
    tex = fm->FilamentTexture();
  }
  this->mi->setParameter("baseColor", FilamentConversions::ToFloat4(color));
  this->mi->setParameter("hasTexture", tex ? 1 : 0);
  this->mi->setParameter("baseColorMap",
      tex ? tex : FilamentRenderEngine::Instance()->Texture(""),
      filament::TextureSampler(filament::TextureSampler::MinFilter::LINEAR,
      filament::TextureSampler::MagFilter::LINEAR));
}

//////////////////////////////////////////////////
void FilamentParticleEmitter::SortFor(const math::Vector3d &_dir)
{
  if (_dir == this->sortDir)
    return;
  this->sortDir = _dir;
  this->Upload();
}

//////////////////////////////////////////////////
void FilamentParticleEmitter::Upload()
{
  const size_t count = std::min(this->particles.size(), kQuota);
  if (this->renderable.isNull() || count == 0u)
    return;
  auto *engine = FilamentRenderEngine::Instance()->FilamentEngine();
  auto &rm = engine->getRenderableManager();

  // ogre2 draws the particles back to front along the camera direction
  // (billboard sort mode SM_DIRECTION), in emission order if equal
  std::vector<std::pair<double, size_t>> order(count);
  for (size_t i = 0; i < count; ++i)
    order[i] = {this->sortDir.Dot(this->particles[i].pos), i};
  std::stable_sort(order.begin(), order.end(),
      [](const auto &_a, const auto &_b) { return _a.first > _b.first; });

  auto *data = new Vertex[count * 4u];
  Vertex *v = data;
  static const float kCorners[4][2] = {{-1, -1}, {1, -1}, {1, 1}, {-1, 1}};
  for (const auto &o : order)
  {
    const Particle &p = this->particles[o.second];
    const float rgba[4] = {p.color.R(), p.color.G(), p.color.B(), p.color.A()};
    for (const auto &c : kCorners)
    {
      *v = {{static_cast<float>(p.pos.X()), static_cast<float>(p.pos.Y()),
          static_cast<float>(p.pos.Z())},
          {0.5f * (c[0] + 1.0f), 0.5f * (1.0f - c[1])},
          {static_cast<float>(0.5 * c[0] * p.width),
          static_cast<float>(0.5 * c[1] * p.height)}, {}};
      for (int k = 0; k < 4; ++k)
        v->rgba[k] = static_cast<uint8_t>(std::clamp(rgba[k], 0.0f, 1.0f) *
            255.0f);
      ++v;
    }
  }
  this->vb->setBufferAt(*engine, 0, filament::VertexBuffer::BufferDescriptor(
      data, count * 4u * sizeof(Vertex),
      &filament_utils::FreeBuffer<Vertex>));
  rm.setGeometryAt(rm.getInstance(this->renderable), 0,
      filament::RenderableManager::PrimitiveType::TRIANGLES, this->vb,
      this->ib, 0, count * 6u);
}
