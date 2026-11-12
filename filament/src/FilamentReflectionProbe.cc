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

#include <filament/Camera.h>
#include <filament/Engine.h>
#include <filament/IndirectLight.h>
#include <filament/RenderTarget.h>
#include <filament/Renderer.h>
#include <filament/Texture.h>
#include <filament/View.h>
#include <filament/Viewport.h>
#include <math/mat4.h>
#include <utils/EntityManager.h>

#include "gz/rendering/filament/FilamentRenderEngine.hh"
#include "FilamentReflectionProbe.hh"

using namespace gz::rendering::filament_probe;
using filament::math::double3;
using filament::math::double4;
using filament::math::float3;

namespace
{
  /// \brief Camera basis per cube face, in filament's cube map convention
  /// (face order +X -X +Y -Y +Z -Z; a face texel (u, v), row 0 on top,
  /// looks along fwd + u * right + v * up, as libibl's getDirectionFor).
  /// Rendered images have row 0 at the bottom, so the camera's up is the
  /// face's -up; with the cube map's left-handedness the basis stays a
  /// rotation.
  struct FaceBasis { double3 fwd; double3 right; double3 up; };
  const FaceBasis kFaceBases[6] = {
    {{1, 0, 0}, {0, 0, -1}, {0, 1, 0}}, {{-1, 0, 0}, {0, 0, 1}, {0, 1, 0}},
    {{0, 1, 0}, {1, 0, 0}, {0, 0, -1}}, {{0, -1, 0}, {1, 0, 0}, {0, 0, 1}},
    {{0, 0, 1}, {1, 0, 0}, {0, 1, 0}}, {{0, 0, -1}, {-1, 0, 0}, {0, 1, 0}}};
}

//////////////////////////////////////////////////
ReflectionProbe::ReflectionProbe(filament::Engine &_engine,
    filament::Scene *_scene, uint32_t _size, uint16_t _samples)
  : engine(_engine), size(_size)
{
  using filament::Texture;
  uint8_t levels = 1u;
  while ((this->size >> levels) > 0u)
    ++levels;
  auto format = Texture::InternalFormat::R11F_G11F_B10F;
  if (!Texture::isTextureFormatMipmappable(this->engine, format))
    format = Texture::InternalFormat::RGBA16F;
  this->cube = Texture::Builder()
      .width(this->size).height(this->size).levels(levels)
      .sampler(Texture::Sampler::SAMPLER_CUBEMAP)
      .format(format)
      .usage(Texture::Usage::COLOR_ATTACHMENT | Texture::Usage::SAMPLEABLE |
             Texture::Usage::GEN_MIPMAPPABLE)
      .build(this->engine);
  this->depth = Texture::Builder()
      .width(this->size).height(this->size).levels(1)
      .format(Texture::InternalFormat::DEPTH32F)
      .usage(Texture::Usage::DEPTH_ATTACHMENT)
      .build(this->engine);
  for (int f = 0; f < 6; ++f)
  {
    this->targets[f] = filament::RenderTarget::Builder()
        .texture(filament::RenderTarget::AttachmentPoint::COLOR, this->cube)
        .face(filament::RenderTarget::AttachmentPoint::COLOR,
            static_cast<Texture::CubemapFace>(f))
        .texture(filament::RenderTarget::AttachmentPoint::DEPTH, this->depth)
        .build(this->engine);
  }

  this->cameraEntity = ::utils::EntityManager::get().create();
  this->camera = this->engine.createCamera(this->cameraEntity);
  this->camera->setExposure(1.0f);
  this->view = this->engine.createView();
  this->view->setScene(_scene);
  this->view->setCamera(this->camera);
  // linear radiance, like the sensors; the GUI camera tone maps the
  // reflections with everything else
  this->view->setPostProcessingEnabled(false);
  this->view->setViewport({0, 0, this->size, this->size});

  this->context = std::make_unique<IBLPrefilterContext>(this->engine);
  IBLPrefilterContext::SpecularFilter::Config config;
  config.sampleCount = _samples;
  this->filter = std::make_unique<IBLPrefilterContext::SpecularFilter>(
      *this->context, config);
}

//////////////////////////////////////////////////
ReflectionProbe::~ReflectionProbe()
{
  this->filter.reset();
  this->context.reset();
  if (this->light)
    this->engine.destroy(this->light);
  if (this->reflections)
    this->engine.destroy(this->reflections);
  this->engine.destroy(this->view);
  this->engine.destroyCameraComponent(this->cameraEntity);
  ::utils::EntityManager::get().destroy(this->cameraEntity);
  for (filament::RenderTarget *t : this->targets)
    this->engine.destroy(t);
  this->engine.destroy(this->depth);
  this->engine.destroy(this->cube);
}

//////////////////////////////////////////////////
bool ReflectionProbe::Render(const double3 &_pos, uint8_t _layers,
    float _near, float _far, const filament::math::float4 &_clear,
    unsigned int _faces, uint64_t _signature)
{
  if (_signature != this->signature)
  {
    this->signature = _signature;
    // a change during a refresh lets it finish first: restarting it would
    // never complete a cube in a scene that changes every frame. Before
    // its first face, the refresh simply starts over.
    if (this->staleFaces > 0u && this->staleFaces < 6u)
      this->changed = true;
    else
      this->staleFaces = 6u;
  }
  if (this->staleFaces == 0u && this->changed)
  {
    this->staleFaces = 6u;
    this->changed = false;
  }
  if (this->staleFaces == 0u)
    return false;
  auto *renderer = FilamentRenderEngine::Instance()->Renderer();
  filament::Renderer::ClearOptions clear;
  clear.clearColor = _clear;
  clear.clear = true;
  clear.discard = true;
  renderer->setClearOptions(clear);
  this->view->setVisibleLayers(0xff, _layers);
  this->camera->setProjection(90.0, 1.0, _near, _far,
      filament::Camera::Fov::HORIZONTAL);
  const unsigned int n = std::min(std::max(_faces, 1u), this->staleFaces);
  for (unsigned int i = 0; i < n; ++i)
  {
    const FaceBasis &b = kFaceBases[this->nextFace];
    this->camera->setModelMatrix(filament::math::mat4(
        double4{b.right, 0.0}, double4{-b.up, 0.0}, double4{-b.fwd, 0.0},
        double4{_pos, 1.0}));
    this->view->setRenderTarget(this->targets[this->nextFace]);
    renderer->renderStandaloneView(this->view);
    this->nextFace = (this->nextFace + 1u) % 6u;
  }
  this->staleFaces -= n;
  if (this->staleFaces > 0u)
    return false;

  IBLPrefilterContext::SpecularFilter::Options options;
  options.generateMipmap = true;
  this->reflections = (*this->filter)(options, this->cube,
      this->reflections);
  return true;
}

//////////////////////////////////////////////////
filament::IndirectLight *ReflectionProbe::Light(const float3 *_sh,
    uint8_t _bands)
{
  if (!this->reflections)
    return nullptr;
  const size_t n = static_cast<size_t>(_bands) * _bands;
  if (this->light && this->lightBands == _bands &&
      std::equal(_sh, _sh + n, this->lightSh.begin()))
  {
    return this->light;
  }
  if (this->light)
    this->engine.destroy(this->light);
  std::copy(_sh, _sh + n, this->lightSh.begin());
  this->lightBands = _bands;
  this->light = filament::IndirectLight::Builder()
      .irradiance(_bands, _sh)
      .reflections(this->reflections)
      .intensity(1.0f)
      .build(this->engine);
  return this->light;
}
