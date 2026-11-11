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
#include <variant>

#include <backend/PixelBufferDescriptor.h>
#include <filament/Engine.h>
#include <filament/Renderer.h>
#include <filament/TextureSampler.h>
#include <filament/Material.h>
#include <filament/MaterialInstance.h>
#include <filament/Camera.h>
#include <filament/Texture.h>
#include <filament/View.h>

#include <gz/common/Console.hh>
#include <gz/common/Image.hh>
#include <gz/common/Util.hh>

#include "gz/rendering/Image.hh"
#include "gz/rendering/Material.hh"
#include "gz/rendering/PixelFormat.hh"
#include "gz/rendering/filament/FilamentConversions.hh"
#include "gz/rendering/filament/FilamentMaterial.hh"
#include "gz/rendering/filament/FilamentMesh.hh"
#include "gz/rendering/filament/FilamentRenderEngine.hh"
#include "gz/rendering/filament/FilamentRenderTarget.hh"
#include "gz/rendering/filament/FilamentScene.hh"
#include "gz/rendering/filament/FilamentThermalCamera.hh"
#include "FilamentUtils.hh"

using namespace gz;
using namespace rendering;

namespace
{
  /// \brief float to 8 bit unorm and back, like a RGBA8 render target
  float Unorm8(double _v)
  {
    return std::round(std::clamp(static_cast<float>(_v), 0.0f, 1.0f) *
        255.0f) / 255.0f;
  }

  /// \brief "temperature" user data _v as Kelvin, -1 when absent or a
  /// string (heat signature texture, see HeatSignatureInstance)
  float Temperature(const Variant &_v)
  {
    if (const float *f = std::get_if<float>(&_v))
      return std::max(*f, 0.0f);
    if (const double *d = std::get_if<double>(&_v))
      return std::max(static_cast<float>(*d), 0.0f);
    if (const int *i = std::get_if<int>(&_v))
      return std::max(static_cast<float>(*i), 0.0f);
    return -1.0f;
  }
}

//////////////////////////////////////////////////
FilamentThermalCamera::FilamentThermalCamera()
{
}

//////////////////////////////////////////////////
FilamentThermalCamera::~FilamentThermalCamera()
{
}

//////////////////////////////////////////////////
void FilamentThermalCamera::Init()
{
  FilamentCamera::Init();
  this->view->setShadowingEnabled(false);
}

//////////////////////////////////////////////////
void FilamentThermalCamera::Destroy()
{
  auto *engine = FilamentRenderEngine::Instance()->FilamentEngine();
  if (engine)
  {
    for (auto &it : this->texturedInstances)
      engine->destroy(it.second);
    for (auto &it : this->heatInstances)
      engine->destroy(it.second);
    for (auto &it : this->heatTextures)
    {
      if (it.second)
        engine->destroy(it.second);
    }
  }
  filament_utils::DestroyFlatInstances(this->idInstances);
  this->texturedInstances.clear();
  this->heatInstances.clear();
  this->heatTextures.clear();
  FilamentCamera::Destroy();
}

//////////////////////////////////////////////////
PixelFormat FilamentThermalCamera::ImageFormat() const
{
  return this->format;
}

//////////////////////////////////////////////////
void FilamentThermalCamera::SetImageFormat(PixelFormat _format, bool)
{
  // only 8 and 16 bit outputs, like ogre2
  this->format = _format == PF_L8 ? PF_L8 : PF_L16;
}

//////////////////////////////////////////////////
RenderTargetPtr FilamentThermalCamera::RenderTarget() const
{
  return this->renderTexture;
}

//////////////////////////////////////////////////
filament::MaterialInstance *FilamentThermalCamera::TexturedInstance(
    filament::Texture *_texture, const math::Color &_diffuse)
{
  const uint32_t key = _diffuse.AsRGBA();
  auto it = this->texturedInstances.find({_texture, key});
  if (it != this->texturedInstances.end())
    return it->second;
  filament::Material *mat =
      FilamentRenderEngine::Instance()->Material("gz_unlit");
  filament::MaterialInstance *mi = mat->createInstance();
  // alpha 0 marks "color, not an id" (ids and the background have alpha 1)
  mi->setParameter("baseColor", filament::math::float4{
      _diffuse.R(), _diffuse.G(), _diffuse.B(), 0.0f});
  mi->setParameter("hasTexture", 1);
  // same sampler as FilamentMaterial uses for the RGB camera
  mi->setParameter("baseColorMap", _texture, filament::TextureSampler(
      filament::TextureSampler::MinFilter::LINEAR_MIPMAP_LINEAR,
      filament::TextureSampler::MagFilter::LINEAR,
      filament::TextureSampler::WrapMode::REPEAT));
  this->texturedInstances[{_texture, key}] = mi;
  return mi;
}

//////////////////////////////////////////////////
filament::MaterialInstance *FilamentThermalCamera::HeatSignatureInstance(
    const VisualPtr &_visual, const std::string &_texture)
{
  // ogre2 adds the texture's directory as a resource path and loads it by
  // name: as a linear (not sRGB) texture without mipmaps. Cached by the
  // user data string, resolved once.
  auto texIt = this->heatTextures.find(_texture);
  if (texIt == this->heatTextures.end())
  {
    const std::string path = common::findFile(_texture);
    filament::Texture *tex = nullptr;
    common::Image img;
    if (!path.empty())
      img.Load(path);
    const unsigned int w = img.Width();
    const unsigned int h = img.Height();
    const std::vector<unsigned char> rgba =
        img.Valid() ? img.RGBAData() : std::vector<unsigned char>();
    if (rgba.size() == static_cast<size_t>(w) * h * 4u && !rgba.empty())
    {
      auto &engine = *FilamentRenderEngine::Instance()->FilamentEngine();
      // the shader reads red only. Rows flipped like
      // FilamentRenderEngine::Texture (bottom row first), so heat
      // signatures and diffuse textures share the mesh UVs (checked with
      // filament_parity PARITY_HEATSIG=1)
      auto *buf = new unsigned char[static_cast<size_t>(w) * h];
      for (size_t y = 0; y < h; ++y)
        for (size_t x = 0; x < w; ++x)
          buf[y * w + x] = rgba[((h - 1u - y) * w + x) * 4u];
      tex = filament::Texture::Builder()
          .width(w).height(h).levels(1)
          .sampler(filament::Texture::Sampler::SAMPLER_2D)
          .format(filament::Texture::InternalFormat::R8)
          .build(engine);
      filament::Texture::PixelBufferDescriptor desc(buf,
          static_cast<size_t>(w) * h, filament::Texture::Format::R,
          filament::Texture::Type::UBYTE,
          &filament_utils::FreeBuffer<unsigned char>);
      tex->setImage(engine, 0, std::move(desc));
    }
    else
    {
      gzerr << "Unable to load heat signature texture [" << _texture
            << "]" << std::endl;
    }
    texIt = this->heatTextures.emplace(_texture, tex).first;
  }
  if (!texIt->second)
    return nullptr;

  // like ogre2: the range applies only when both are float user data,
  // otherwise the material defaults (0 and 100 K) stay
  const int bitDepth = this->ImageFormat() == PF_L8 ? 8 : 16;
  float minT = 0.0f;
  float maxT = 100.0f;
  const Variant minV = _visual->UserData("minTemp");
  const Variant maxV = _visual->UserData("maxTemp");
  const float *minF = std::get_if<float>(&minV);
  const float *maxF = std::get_if<float>(&maxV);
  if (minF && maxF)
  {
    minT = std::max(*minF, 0.0f);
    maxT = std::min(*maxF, static_cast<float>(((1 << bitDepth) - 1.0) *
        static_cast<double>(this->resolution)));
  }
  const auto key = std::make_tuple(_texture, minT, maxT, bitDepth,
      this->resolution);
  auto it = this->heatInstances.find(key);
  if (it != this->heatInstances.end())
    return it->second;
  filament::Material *mat =
      FilamentRenderEngine::Instance()->Material("gz_heat_signature");
  if (!mat)
    return nullptr;
  filament::MaterialInstance *mi = mat->createInstance();
  mi->setParameter("minTemp", minT);
  mi->setParameter("maxTemp", maxT);
  mi->setParameter("bitMax", static_cast<float>((1 << bitDepth) - 1));
  mi->setParameter("resolution", this->resolution);
  // ogre2's texture unit: bilinear, wrap, a single mip level
  mi->setParameter("heatMap", texIt->second, filament::TextureSampler(
      filament::TextureSampler::MinFilter::LINEAR,
      filament::TextureSampler::MagFilter::LINEAR,
      filament::TextureSampler::WrapMode::REPEAT));
  this->heatInstances[key] = mi;
  return mi;
}

//////////////////////////////////////////////////
uint16_t FilamentThermalCamera::Encode(const float _rgba[4]) const
{
  // float math in the order of thermal_camera_fs.glsl (rgbToTemp is
  // always on in ogre2)
  const int bitDepth = this->ImageFormat() == PF_L8 ? 8 : 16;
  const float bitMax = static_cast<float>((1 << bitDepth) - 1);
  const float res = static_cast<float>(this->resolution);
  float temp = this->ambient;
  float heatRange = this->ambientRange;
  float dNorm = 0.5f;
  const bool isHeatSource =
      _rgba[1] == 0.0f && _rgba[2] == 0.0f && _rgba[3] == 0.0f;
  if (isHeatSource)
  {
    temp = _rgba[0] * bitMax * res;
    heatRange = this->heatSourceTempRange;
  }
  else
  {
    const float gray =
        _rgba[0] * 0.299f + _rgba[1] * 0.587f + _rgba[2] * 0.114f;
    dNorm = 1.0f - gray;
  }
  const float delta = (1.0f - dNorm) * heatRange;
  temp = temp - heatRange / 2.0f + delta;
  temp = std::clamp(temp, this->minTemp, this->maxTemp);
  temp /= res;
  temp /= bitMax;
  return static_cast<uint16_t>(
      std::round(std::clamp(temp, 0.0f, 1.0f) * bitMax));
}

//////////////////////////////////////////////////
void FilamentThermalCamera::Render()
{
  if (!this->textureCreated ||
      this->renderTexture->Width() != this->ImageWidth() ||
      this->renderTexture->Height() != this->ImageHeight())
  {
    this->renderTexture->SetFormat(PF_R8G8B8);
    this->renderTexture->SetSrgb(false);
    this->renderTexture->SetWidth(this->ImageWidth());
    this->renderTexture->SetHeight(this->ImageHeight());
    this->textureCreated = true;
  }
  this->renderTexture->PreRender();
  // ogre2 renders thermal with depth clamp, see SetClampedProjection
  this->SetClampedProjection(this->NearClipPlane() * 1e-3,
      this->FarClipPlane());
  // id 0: background, ogre2 clears its color pass to (0, 0, 0, 1)
  this->renderTexture->SetClearColor({0.0f, 0.0f, 0.0f, 1.0f});
  this->view->setVisibleLayers(0xff,
      FilamentConversions::ToSensorLayers(this->VisibilityMask()));

  const int bitDepth = this->ImageFormat() == PF_L8 ? 8 : 16;
  const double bitMax = (1 << bitDepth) - 1.0;
  this->idValue.clear();
  const float bg[4] = {0.0f, 0.0f, 0.0f, 1.0f};
  this->idValue.push_back(this->Encode(bg));

  this->overridden.clear();
  for (FilamentMesh *mesh : this->scene->Meshes())
  {
    VisualPtr visual = mesh->Parent();
    if (!visual)
      continue;
    const Variant tempV = visual->UserData("temperature");
    const float temp = Temperature(tempV);
    filament::MaterialInstance *heatSig = nullptr;
    if (const auto *tex = std::get_if<std::string>(&tempV))
    {
      // ogre2's Terra has no heat signature support
      if (mesh->terrain)
      {
        gzerr << "Heat Signature not yet supported by Heightmaps."
              << std::endl;
      }
      else
      {
        heatSig = this->HeatSignatureInstance(visual, *tex);
      }
    }
    for (unsigned int i = 0; i < mesh->SubMeshCount(); ++i)
    {
      if (heatSig)
      {
        // per pixel normalized temperature, alpha 0.5, see PostRender
        mesh->OverrideMaterial(i, heatSig);
        continue;
      }
      float rgba[4];
      if (temp >= 0.0f)
      {
        // heat source: normalized temperature through an 8 bit channel
        rgba[0] = Unorm8((temp / this->resolution) / bitMax);
        rgba[1] = rgba[2] = rgba[3] = 0.0f;
      }
      else
      {
        MaterialPtr mat = mesh->terrain ? nullptr :
            mesh->SubMeshByIndex(i)->Material();
        const math::Color c = mat ? mat->Diffuse() : math::Color::White;
        auto fmat = std::dynamic_pointer_cast<FilamentMaterial>(mat);
        if (fmat && fmat->FilamentTexture())
        {
          // ogre2 draws these unlit as diffuse x texture: the color varies
          // per pixel, so it is converted per pixel in PostRender
          mesh->OverrideMaterial(i,
              this->TexturedInstance(fmat->FilamentTexture(), c));
          continue;
        }
        // other objects: flat diffuse color, white without a material
        rgba[0] = Unorm8(c.R());
        rgba[1] = Unorm8(c.G());
        rgba[2] = Unorm8(c.B());
        rgba[3] = 1.0f;
      }
      const uint32_t id = static_cast<uint32_t>(this->idValue.size());
      this->idValue.push_back(this->Encode(rgba));
      mesh->OverrideMaterial(i,
          filament_utils::FlatInstance(this->idInstances, id));
    }
    this->overridden.push_back(mesh);
  }
  this->renderTexture->Render(this->view);
  for (FilamentMesh *m : this->overridden)
    m->RestoreMaterials();
}

//////////////////////////////////////////////////
void FilamentThermalCamera::PostRender()
{
  if (this->newThermalFrame.ConnectionCount() == 0u)
    return;
  const unsigned int width = this->ImageWidth();
  const unsigned int height = this->ImageHeight();
  const size_t len = static_cast<size_t>(width) * height;
  this->rgbaRaw.resize(len * 4u);
  FilamentRenderEngine::Instance()->Renderer()->readPixels(
      this->renderTexture->Native(), 0, 0, width, height,
      filament::backend::PixelBufferDescriptor(this->rgbaRaw.data(),
      this->rgbaRaw.size(), filament::backend::PixelDataFormat::RGBA,
      filament::backend::PixelDataType::UBYTE));
  FilamentRenderEngine::Instance()->FilamentEngine()->flushAndWait();
  const uint8_t *src = this->rgbaRaw.data();
  this->thermalImage.resize(len);
  for (size_t i = 0; i < len; ++i)
  {
    const uint8_t *p = src + i * 4u;
    if (p[3] == 0u)
    {
      // textured object: its color, converted like ogre2's shader does
      const float rgba[4] = {p[0] / 255.0f, p[1] / 255.0f, p[2] / 255.0f,
          1.0f};
      this->thermalImage[i] = this->Encode(rgba);
      continue;
    }
    if (p[3] != 255u)
    {
      // heat signature: red is the normalized temperature
      const float rgba[4] = {p[0] / 255.0f, 0.0f, 0.0f, 0.0f};
      this->thermalImage[i] = this->Encode(rgba);
      continue;
    }
    const uint32_t id = (static_cast<uint32_t>(p[0]) << 16) |
        (static_cast<uint32_t>(p[1]) << 8) | p[2];
    this->thermalImage[i] =
        id < this->idValue.size() ? this->idValue[id] : this->idValue[0];
  }
  this->newThermalFrame(this->thermalImage.data(), width, height, 1u,
      PixelUtil::Name(this->ImageFormat()));
}

//////////////////////////////////////////////////
common::ConnectionPtr FilamentThermalCamera::ConnectNewThermalFrame(
    std::function<void(const uint16_t *, unsigned int, unsigned int,
    unsigned int, const std::string &)> _subscriber)
{
  return this->newThermalFrame.Connect(_subscriber);
}
