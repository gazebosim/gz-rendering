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
#include <vector>

#include <backend/PixelBufferDescriptor.h>
#include <filament/Camera.h>
#include <filament/Engine.h>
#include <filament/Material.h>
#include <filament/MaterialInstance.h>
#include <filament/Renderer.h>
#include <filament/TextureSampler.h>
#include <filament/View.h>

#include <gz/common/Console.hh>
#include <gz/math/Rand.hh>

#include "gz/rendering/Camera.hh"
#include "gz/rendering/RenderPassSystem.hh"
#include "gz/rendering/Scene.hh"
#include "gz/rendering/filament/FilamentGaussianNoisePass.hh"
#include "gz/rendering/filament/FilamentRenderEngine.hh"
#include "gz/rendering/filament/FilamentRenderTarget.hh"
#include "FilamentFullscreenPass.hh"

using namespace gz;
using namespace rendering;

//////////////////////////////////////////////////
FilamentGaussianNoisePass::FilamentGaussianNoisePass()
{
}

//////////////////////////////////////////////////
FilamentGaussianNoisePass::~FilamentGaussianNoisePass()
{
  this->Destroy();
}

//////////////////////////////////////////////////
void FilamentGaussianNoisePass::Destroy()
{
  this->pass.reset();
  if (this->samples && FilamentRenderEngine::Instance()->FilamentEngine())
    this->samples->Destroy();
  this->samples.reset();
}

//////////////////////////////////////////////////
void FilamentGaussianNoisePass::SetParameters(filament::Texture *_colorIn,
    int _mode)
{
  auto *mi = this->pass->instance;
  mi->setParameter("colorIn", _colorIn, filament::TextureSampler());
  // per frame randomness, like the offsets ogre2 draws in PreRender
  mi->setParameter("seed", static_cast<uint32_t>(
      math::Rand::IntUniform(0, std::numeric_limits<int32_t>::max())));
  mi->setParameter("mean", static_cast<float>(this->mean));
  mi->setParameter("stddev", static_cast<float>(this->stdDev));
  mi->setParameter("mode", _mode);
}

//////////////////////////////////////////////////
filament::View *FilamentGaussianNoisePass::ColorView(
    filament::Texture *_colorIn)
{
  if (!FilamentFullscreenPass::Create(this->pass, "gz_noise"))
    return nullptr;
  this->SetParameters(_colorIn, 0);
  return this->pass->view;
}

//////////////////////////////////////////////////
void FilamentGaussianNoisePass::Samples(const ScenePtr &_scene,
    unsigned int _width, unsigned int _height, std::vector<float> &_out)
{
  if (!FilamentFullscreenPass::Create(this->pass, "gz_noise"))
    return;
  if (!this->samples)
  {
    this->samples = std::dynamic_pointer_cast<FilamentRenderTexture>(
        _scene->CreateRenderTexture());
    this->samples->SetFormat(PF_FLOAT32_R);
  }
  if (this->samples->Width() != _width || this->samples->Height() != _height)
  {
    this->samples->SetWidth(_width);
    this->samples->SetHeight(_height);
  }
  this->samples->PreRender();
  // the sampler is unused in mode 1 but has to be bound
  this->SetParameters(FilamentRenderEngine::Instance()->Texture(""), 1);
  this->samples->Render(this->pass->view);

  _out.resize(static_cast<size_t>(_width) * _height);
  FilamentRenderEngine::Instance()->Renderer()->readPixels(
      this->samples->Native(), 0, 0, _width, _height,
      filament::backend::PixelBufferDescriptor(_out.data(),
      _out.size() * sizeof(float), filament::backend::PixelDataFormat::R,
      filament::backend::PixelDataType::FLOAT));
  FilamentRenderEngine::Instance()->FilamentEngine()->flushAndWait();
}

//////////////////////////////////////////////////
void FilamentGaussianNoisePass::ApplyDepth(const Camera &_camera,
    float *_depth, float *_cloud)
{
  const unsigned int width = _camera.ImageWidth();
  const unsigned int height = _camera.ImageHeight();
  const size_t len = static_cast<size_t>(width) * height;
  for (unsigned int k = 0; k < _camera.RenderPassCount(); ++k)
  {
    auto *pass = dynamic_cast<FilamentGaussianNoisePass *>(
        _camera.RenderPassByIndex(k).get());
    if (!pass || !pass->IsEnabled())
      continue;
    std::vector<float> &z = pass->depthSamples;
    pass->Samples(_camera.Scene(), width, height, z);
    if (z.size() != len)
      continue;
    // No clamping afterwards: depth_camera_final_fs.glsl would reclamp to
    // near / far, but with a pass in its chain ogre2 reads back the noise
    // pass output (measured: noisy points beyond far or nearer than near
    // stay finite). +-inf stay what they are.
    for (size_t i = 0; i < len; ++i)
    {
      _depth[i] += z[i];
      if (!_cloud)
        continue;
      float *p = &_cloud[i * 4u];
      p[0] += z[i];
      p[1] += z[i];
      p[2] += z[i];
      // packed color: unpack to [0, 1], add, clamp, pack truncating
      uint32_t rgba;
      std::memcpy(&rgba, &p[3], sizeof(rgba));
      const float n = std::copysign(std::pow(std::abs(z[i]), 2.1f), z[i]);
      uint32_t out = rgba & 0xFFu;
      for (unsigned int s = 8u; s < 32u; s += 8u)
      {
        const float c = std::clamp(
            static_cast<float>((rgba >> s) & 0xFFu) / 255.0f + n, 0.0f, 1.0f);
        out |= static_cast<uint32_t>(c * 255.0f) << s;
      }
      std::memcpy(&p[3], &out, sizeof(out));
    }
  }
}

GZ_RENDERING_REGISTER_RENDER_PASS(FilamentGaussianNoisePass, GaussianNoisePass)
