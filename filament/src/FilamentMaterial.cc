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
#include <array>
#include <cmath>
#include <vector>

#include <filament/Engine.h>
#include <filament/Material.h>
#include <filament/MaterialInstance.h>
#include <filament/Texture.h>
#include <filament/TextureSampler.h>

#include <gz/common/Console.hh>
#include <gz/common/StringUtils.hh>

#include "gz/rendering/ShaderParams.hh"
#include "gz/rendering/filament/FilamentConversions.hh"
#include "gz/rendering/filament/FilamentMaterial.hh"
#include "gz/rendering/filament/FilamentRenderEngine.hh"
#include "gz/rendering/filament/FilamentScene.hh"
#include "gz/rendering/filament/FilamentMesh.hh"

using namespace gz;
using namespace rendering;

//////////////////////////////////////////////////
FilamentMaterial::FilamentMaterial()
  : vertexShaderParams(std::make_shared<ShaderParams>()),
    fragmentShaderParams(std::make_shared<ShaderParams>())
{
}

//////////////////////////////////////////////////
FilamentMaterial::~FilamentMaterial()
{
}

//////////////////////////////////////////////////
void FilamentMaterial::Destroy()
{
  BaseMaterial::Destroy();
  for (auto *mi : {this->lit, this->litTransparent, this->litMasked,
      this->unlit, this->unlitTransparent, this->custom})
  {
    this->DestroyInstance(mi);
  }
  this->lit = nullptr;
  this->unlit = nullptr;
  this->unlitTransparent = nullptr;
  this->litTransparent = nullptr;
  this->litMasked = nullptr;
  this->custom = nullptr;
  this->customSource.clear();
}

//////////////////////////////////////////////////
void FilamentMaterial::DestroyInstance(filament::MaterialInstance *_instance)
{
  auto *engine = FilamentRenderEngine::Instance()->FilamentEngine();
  if (!_instance || !engine)
    return;
  // filament refuses to destroy an instance a renderable still uses
  // ponytail: scans every mesh of the scene, a per-material user list if
  // destroying materials of big scenes ever shows up in profiles
  filament::Material *fallback =
      FilamentRenderEngine::Instance()->Material("gz_unlit");
  if (fallback && this->scene)
  {
    for (FilamentMesh *mesh : this->scene->Meshes())
      mesh->ReplaceInstance(_instance, fallback->getDefaultInstance());
  }
  engine->destroy(_instance);
}

//////////////////////////////////////////////////
bool FilamentMaterial::HasTexture() const
{
  return !this->textureName.empty();
}

//////////////////////////////////////////////////
std::string FilamentMaterial::Texture() const
{
  return this->textureName;
}

//////////////////////////////////////////////////
void FilamentMaterial::SetTexture(const std::string &_texture)
{
  this->SetTexture(_texture, nullptr);
}

//////////////////////////////////////////////////
void FilamentMaterial::SetTexture(const std::string &_texture,
    const std::shared_ptr<const common::Image> &_img)
{
  this->textureName = _texture;
  this->textureData = _texture.empty() ? nullptr : _img;
  // force a reload even when only the image data changed
  this->loadedTexture.clear();
}

//////////////////////////////////////////////////
std::shared_ptr<const common::Image> FilamentMaterial::TextureData() const
{
  return this->textureData;
}

//////////////////////////////////////////////////
void FilamentMaterial::ClearTexture()
{
  this->SetTexture("", nullptr);
}

//////////////////////////////////////////////////
double FilamentMaterial::Opacity() const
{
  return (1.0 - this->Transparency()) * this->Diffuse().A();
}

//////////////////////////////////////////////////
filament::MaterialInstance *FilamentMaterial::Instance() const
{
  if (this->custom)
    return this->custom;
  // like Ogre2Material::UpdateTransparency
  const bool blended = this->Opacity() < 1.0;
  if (this->LightingEnabled())
  {
    if (blended && this->litTransparent)
      return this->litTransparent;
    // alpha from texture: like ogre2, only when the texture has alpha
    // ponytail: unlit materials ignore it, and an opaque texture also drops
    // TwoSidedEnabled() (ogre2 keeps two-sided lighting); add an unlit /
    // double-sided-opaque variant if a scene needs them
    if (this->litMasked && this->TextureAlphaEnabled() && this->texture &&
        FilamentRenderEngine::Instance()->TextureHasAlpha(this->texture))
    {
      return this->litMasked;
    }
    return this->lit;
  }
  return blended && this->unlitTransparent ? this->unlitTransparent :
      this->unlit;
}

//////////////////////////////////////////////////
bool FilamentMaterial::AlphaTested() const
{
  return this->litMasked && this->Instance() == this->litMasked;
}

//////////////////////////////////////////////////
void FilamentMaterial::Init()
{
  BaseMaterial::Init();
  auto *engine = FilamentRenderEngine::Instance();
  filament::Material *litMat = engine->Material("gz_lit");
  filament::Material *unlitMat = engine->Material("gz_unlit");
  filament::Material *transparentMat = engine->Material("gz_unlit_transparent");
  if (!litMat || !unlitMat || !transparentMat)
  {
    gzerr << "Filament base materials not available" << std::endl;
    return;
  }
  this->lit = litMat->createInstance();
  this->unlit = unlitMat->createInstance();
  this->unlitTransparent = transparentMat->createInstance();
  this->Reset();
  this->PreRender();
}

//////////////////////////////////////////////////
void FilamentMaterial::PreRender()
{
  BaseMaterial::PreRender();
  if (!this->lit || !this->unlit || !this->unlitTransparent)
    return;
  this->UpdateCustomShader();

  if (this->HasTexture() && this->Texture() != this->loadedTexture)
  {
    this->texture = FilamentRenderEngine::Instance()->Texture(
        this->Texture(), this->TextureData());
    this->loadedTexture = this->Texture();
  }
  else if (!this->HasTexture())
  {
    this->texture = nullptr;
    this->loadedTexture.clear();
  }
  // the blended and alpha tested instances only exist once needed: filament
  // commits every material instance on every render (FEngine::prepare), so
  // an unused instance per material costs per frame in large scenes, and an
  // extra one would renumber every instance (filament's draw order)
  if (!this->litTransparent && this->LightingEnabled() &&
      this->Opacity() < 1.0)
  {
    if (filament::Material *mat =
        FilamentRenderEngine::Instance()->Material("gz_lit_transparent"))
    {
      this->litTransparent = mat->createInstance();
      this->pushed = false;
    }
  }
  if (!this->litMasked && this->TextureAlphaEnabled() && this->texture &&
      FilamentRenderEngine::Instance()->TextureHasAlpha(this->texture))
  {
    if (filament::Material *mat =
        FilamentRenderEngine::Instance()->Material("gz_lit_masked"))
    {
      this->litMasked = mat->createInstance();
      this->pushed = false;
    }
  }

  // PBR maps. Data maps are linear, color maps (emissive, light) sRGB like
  // ogre2; the environment map is a cube map.
  auto *engine = FilamentRenderEngine::Instance();
  std::array<filament::Texture *, MAP_COUNT> mapTextures{};
  for (int i = 0; i < MAP_COUNT; ++i)
  {
    Map &m = this->maps[i];
    if (!m.name.empty() && !m.loaded)
    {
      m.texture = i == ENVIRONMENT ? engine->CubeTexture(m.name, true) :
          engine->Texture(m.name, m.data, i == EMISSIVE || i == LIGHT);
      m.loaded = true;
    }
    mapTextures[i] = m.texture;
  }

  // Pushing parameters dirties the instance's uniform buffer, which filament
  // then re-uploads: only do it when something changed.
  const math::Color d = this->Diffuse();
  const math::Color s = this->Specular();
  const math::Color e = this->Emissive();
  const std::array<float, 18> key = {d.R(), d.G(), d.B(),
      static_cast<float>(this->Transparency()), s.R(), s.G(), s.B(),
      this->Roughness(), this->Metalness(),
      this->LightingEnabled() ? 1.0f : 0.0f, e.R(), e.G(), e.B(),
      static_cast<float>(this->lightMapUvSet), d.A(),
      (this->DepthCheckEnabled() ? 1.0f : 0.0f) +
      (this->DepthWriteEnabled() ? 2.0f : 0.0f),
      static_cast<float>(this->AlphaThreshold()),
      (this->TextureAlphaEnabled() ? 1.0f : 0.0f) +
      (this->TwoSidedEnabled() ? 2.0f : 0.0f)};
  if (this->pushed && key == this->pushedKey &&
      this->texture == this->pushedTexture && mapTextures == this->pushedMaps)
  {
    return;
  }
  this->Update(this->lit);
  this->Update(this->unlit);
  this->Update(this->unlitTransparent);
  if (this->litTransparent)
    this->Update(this->litTransparent);
  if (this->litMasked)
    this->Update(this->litMasked);
  this->pushed = true;
  this->pushedKey = key;
  this->pushedTexture = this->texture;
  this->pushedMaps = mapTextures;
}

//////////////////////////////////////////////////
void FilamentMaterial::SetRoughness(const float _roughness)
{
  this->roughness = _roughness;
}

//////////////////////////////////////////////////
float FilamentMaterial::Roughness() const
{
  return this->roughness;
}

//////////////////////////////////////////////////
void FilamentMaterial::SetMetalness(const float _metalness)
{
  this->metalness = _metalness;
}

//////////////////////////////////////////////////
float FilamentMaterial::Metalness() const
{
  return this->metalness;
}

//////////////////////////////////////////////////
void FilamentMaterial::SetMap(MapSlot _slot, const std::string &_name,
    const std::shared_ptr<const common::Image> &_img)
{
  Map &m = this->maps[_slot];
  m.name = _name;
  m.data = _name.empty() ? nullptr : _img;
  m.texture = nullptr;
  m.loaded = false;
}

//////////////////////////////////////////////////
bool FilamentMaterial::HasNormalMap() const
{
  return !this->maps[NORMAL].name.empty();
}

//////////////////////////////////////////////////
std::string FilamentMaterial::NormalMap() const
{
  return this->maps[NORMAL].name;
}

//////////////////////////////////////////////////
std::shared_ptr<const common::Image> FilamentMaterial::NormalMapData() const
{
  return this->maps[NORMAL].data;
}

//////////////////////////////////////////////////
void FilamentMaterial::SetNormalMap(const std::string &_name,
    const std::shared_ptr<const common::Image> &_img)
{
  this->SetMap(NORMAL, _name, _img);
}

//////////////////////////////////////////////////
void FilamentMaterial::ClearNormalMap()
{
  this->SetMap(NORMAL, "", nullptr);
}

//////////////////////////////////////////////////
bool FilamentMaterial::HasRoughnessMap() const
{
  return !this->maps[ROUGHNESS].name.empty();
}

//////////////////////////////////////////////////
std::string FilamentMaterial::RoughnessMap() const
{
  return this->maps[ROUGHNESS].name;
}

//////////////////////////////////////////////////
std::shared_ptr<const common::Image> FilamentMaterial::RoughnessMapData() const
{
  return this->maps[ROUGHNESS].data;
}

//////////////////////////////////////////////////
void FilamentMaterial::SetRoughnessMap(const std::string &_name,
    const std::shared_ptr<const common::Image> &_img)
{
  this->SetMap(ROUGHNESS, _name, _img);
}

//////////////////////////////////////////////////
void FilamentMaterial::ClearRoughnessMap()
{
  this->SetMap(ROUGHNESS, "", nullptr);
}

//////////////////////////////////////////////////
bool FilamentMaterial::HasMetalnessMap() const
{
  return !this->maps[METALNESS].name.empty();
}

//////////////////////////////////////////////////
std::string FilamentMaterial::MetalnessMap() const
{
  return this->maps[METALNESS].name;
}

//////////////////////////////////////////////////
std::shared_ptr<const common::Image> FilamentMaterial::MetalnessMapData() const
{
  return this->maps[METALNESS].data;
}

//////////////////////////////////////////////////
void FilamentMaterial::SetMetalnessMap(const std::string &_name,
    const std::shared_ptr<const common::Image> &_img)
{
  this->SetMap(METALNESS, _name, _img);
}

//////////////////////////////////////////////////
void FilamentMaterial::ClearMetalnessMap()
{
  this->SetMap(METALNESS, "", nullptr);
}

//////////////////////////////////////////////////
bool FilamentMaterial::HasEnvironmentMap() const
{
  return !this->maps[ENVIRONMENT].name.empty();
}

//////////////////////////////////////////////////
std::string FilamentMaterial::EnvironmentMap() const
{
  return this->maps[ENVIRONMENT].name;
}

//////////////////////////////////////////////////
std::shared_ptr<const common::Image> FilamentMaterial::EnvironmentMapData() const
{
  return this->maps[ENVIRONMENT].data;
}

//////////////////////////////////////////////////
void FilamentMaterial::SetEnvironmentMap(const std::string &_name,
    const std::shared_ptr<const common::Image> &_img)
{
  this->SetMap(ENVIRONMENT, _name, _img);
}

//////////////////////////////////////////////////
void FilamentMaterial::ClearEnvironmentMap()
{
  this->SetMap(ENVIRONMENT, "", nullptr);
}

//////////////////////////////////////////////////
bool FilamentMaterial::HasEmissiveMap() const
{
  return !this->maps[EMISSIVE].name.empty();
}

//////////////////////////////////////////////////
std::string FilamentMaterial::EmissiveMap() const
{
  return this->maps[EMISSIVE].name;
}

//////////////////////////////////////////////////
std::shared_ptr<const common::Image> FilamentMaterial::EmissiveMapData() const
{
  return this->maps[EMISSIVE].data;
}

//////////////////////////////////////////////////
void FilamentMaterial::SetEmissiveMap(const std::string &_name,
    const std::shared_ptr<const common::Image> &_img)
{
  this->SetMap(EMISSIVE, _name, _img);
}

//////////////////////////////////////////////////
void FilamentMaterial::ClearEmissiveMap()
{
  this->SetMap(EMISSIVE, "", nullptr);
}

//////////////////////////////////////////////////
bool FilamentMaterial::HasLightMap() const
{
  return !this->maps[LIGHT].name.empty();
}

//////////////////////////////////////////////////
std::string FilamentMaterial::LightMap() const
{
  return this->maps[LIGHT].name;
}

//////////////////////////////////////////////////
unsigned int FilamentMaterial::LightMapTexCoordSet() const
{
  return this->lightMapUvSet;
}

//////////////////////////////////////////////////
std::shared_ptr<const common::Image> FilamentMaterial::LightMapData() const
{
  return this->maps[LIGHT].data;
}

//////////////////////////////////////////////////
void FilamentMaterial::SetLightMap(const std::string &_name,
    const std::shared_ptr<const common::Image> &_img, unsigned int _uvSet)
{
  this->SetMap(LIGHT, _name, _img);
  this->lightMapUvSet = _name.empty() ? 0u : _uvSet;
}

//////////////////////////////////////////////////
void FilamentMaterial::ClearLightMap()
{
  this->SetLightMap("", nullptr, 0u);
}

//////////////////////////////////////////////////
MaterialType FilamentMaterial::Type() const
{
  return MaterialType::MT_PBS;
}

//////////////////////////////////////////////////
filament::Texture *FilamentMaterial::FilamentTexture() const
{
  return this->texture;
}

//////////////////////////////////////////////////
void FilamentMaterial::Update(filament::MaterialInstance *_instance)
{
  math::Color diffuse = this->Diffuse();
  diffuse.A(static_cast<float>(this->Opacity()));
  _instance->setParameter("baseColor", FilamentConversions::ToFloat4(diffuse));
  _instance->setParameter("hasTexture", this->texture ? 1 : 0);
  // overlays (gizmo, ...): ogre2 neither tests nor writes depth. The
  // transparent materials never write depth.
  _instance->setDepthCulling(this->DepthCheckEnabled());
  if (_instance == this->lit || _instance == this->unlit ||
      _instance == this->litMasked)
  {
    _instance->setDepthWrite(this->DepthWriteEnabled());
  }
  if (_instance == this->litMasked)
  {
    // ogre2's alpha test passes alpha >= threshold; two-sided lighting
    // also disables culling, of the shadow casters too
    _instance->setMaskThreshold(static_cast<float>(this->AlphaThreshold()));
    const bool twoSided = this->TwoSidedEnabled();
    _instance->setDoubleSided(twoSided);
    _instance->setCullingMode(twoSided ?
        filament::MaterialInstance::CullingMode::NONE :
        filament::MaterialInstance::CullingMode::BACK);
  }
  filament::TextureSampler sampler(
      filament::TextureSampler::MinFilter::LINEAR_MIPMAP_LINEAR,
      filament::TextureSampler::MagFilter::LINEAR,
      filament::TextureSampler::WrapMode::REPEAT);
  sampler.setAnisotropy(this->anisotropy);
  auto *engine = FilamentRenderEngine::Instance();
  _instance->setParameter("baseColorMap", this->texture ? this->texture :
      engine->Texture(""), sampler);
  if (_instance == this->lit || _instance == this->litTransparent ||
      _instance == this->litMasked)
  {
    _instance->setParameter("roughness",
        std::clamp(this->Roughness(), 0.05f, 1.0f));
    _instance->setParameter("metallic",
        std::clamp(this->Metalness(), 0.0f, 1.0f));
    // ogre2 scales the whole specular lobe by the gz specular color (Rs *
    // kS): F0 and the grazing Fresnel term alike. gz_lit's specularFactor
    // does the same (F0 = 0.03 * specular, F90 = specular), so a material
    // with a black specular has no grazing highlights, as in ogre2.
    // ponytail: grey (mean of rgb), a specular hue is lost.
    const math::Color s = this->Specular();
    _instance->setParameter("specular", static_cast<float>(
        std::clamp((s.R() + s.G() + s.B()) / 3.0, 0.0, 1.0)));

    // PBR maps, with ogre2's (Ogre PBS) semantics: roughness and metalness
    // maps (red channel) scale the scalars, the emissive map is tinted by the
    // emissive color when that is not black, the light map takes the
    // emissive slot and is modulated by the diffuse color, the environment
    // map is a reflection cube map. Unset maps get a 1x1 default and a
    // cleared flag bit.
    filament::Texture *white = engine->Texture("");
    int flags = 0;
    for (int i = 0; i < MAP_COUNT; ++i)
    {
      filament::Texture *t = this->maps[i].texture;
      if (t)
        flags |= 1 << i;
      static const char *kNames[MAP_COUNT] = {"normalMap", "roughnessMap",
          "metalnessMap", "emissiveMap", "lightMap", "environmentMap"};
      if (i == ENVIRONMENT)
      {
        filament::TextureSampler cube(
            filament::TextureSampler::MinFilter::LINEAR_MIPMAP_LINEAR,
            filament::TextureSampler::MagFilter::LINEAR,
            filament::TextureSampler::WrapMode::CLAMP_TO_EDGE);
        _instance->setParameter(kNames[i], t ? t : engine->CubeTexture(""),
            cube);
        _instance->setParameter("environmentMapLod",
            t ? static_cast<float>(t->getLevels() - 1) : 0.0f);
      }
      else
      {
        _instance->setParameter(kNames[i], t ? t : white, sampler);
      }
    }
    _instance->setParameter("mapFlags", flags);
    _instance->setParameter("lightMapUvSet",
        static_cast<int>(this->lightMapUvSet));
    const math::Color e = this->Emissive();
    filament::math::float3 emissive = {e.R(), e.G(), e.B()};
    // ogre2 only applies the emissive color when it is not black
    const bool emissiveConstant = e.R() > 0 || e.G() > 0 || e.B() > 0;
    filament::math::float3 mapTint = emissiveConstant ? emissive :
        filament::math::float3{1.0f, 1.0f, 1.0f};
    _instance->setParameter("emissive", emissive);
    _instance->setParameter("emissiveMapTint", mapTint);
  }
}

//////////////////////////////////////////////////
std::string FilamentMaterial::VertexShader() const
{
  return this->vertexShaderPath;
}

//////////////////////////////////////////////////
ShaderParamsPtr FilamentMaterial::VertexShaderParams()
{
  return this->vertexShaderParams;
}

//////////////////////////////////////////////////
void FilamentMaterial::SetVertexShader(const std::string &_path)
{
  this->vertexShaderPath = _path;
}

//////////////////////////////////////////////////
std::string FilamentMaterial::FragmentShader() const
{
  return this->fragmentShaderPath;
}

//////////////////////////////////////////////////
ShaderParamsPtr FilamentMaterial::FragmentShaderParams()
{
  return this->fragmentShaderParams;
}

//////////////////////////////////////////////////
void FilamentMaterial::SetFragmentShader(const std::string &_path)
{
  this->fragmentShaderPath = _path;
}

//////////////////////////////////////////////////
/// \brief Set a filament uniform from a gz float or int shader param buffer,
/// by the uniform's declared type. Returns false on a type mismatch.
template <typename S>
static bool SetUniform(filament::MaterialInstance *_mi,
    const filament::Material::ParameterInfo &_info, const S *_v, uint32_t _n)
{
  namespace fm = filament::math;
  using T = filament::backend::UniformType;
  const char *name = _info.name;
  // the array setter covers scalars too, count clamped to the declaration
  auto set = [&](auto *_typed, uint32_t _width)
  {
    const uint32_t count = std::min(_n / _width, _info.count);
    if (count > 0)
      _mi->setParameter(name, _typed, count);
    return count > 0;
  };
  if constexpr (std::is_same_v<S, float>)
  {
    switch (_info.type)
    {
      case T::FLOAT: return set(_v, 1);
      case T::FLOAT2: return set(reinterpret_cast<const fm::float2 *>(_v), 2);
      case T::FLOAT3: return set(reinterpret_cast<const fm::float3 *>(_v), 3);
      case T::FLOAT4: return set(reinterpret_cast<const fm::float4 *>(_v), 4);
      case T::MAT4: return set(reinterpret_cast<const fm::mat4f *>(_v), 16);
      default: return false;
    }
  }
  else
  {
    switch (_info.type)
    {
      case T::INT: return set(_v, 1);
      case T::INT2: return set(reinterpret_cast<const fm::int2 *>(_v), 2);
      case T::INT3: return set(reinterpret_cast<const fm::int3 *>(_v), 3);
      case T::INT4: return set(reinterpret_cast<const fm::int4 *>(_v), 4);
      default: return false;
    }
  }
}

//////////////////////////////////////////////////
void FilamentMaterial::UpdateCustomShader()
{
  // Filament takes material definitions, not GLSL: a .mat source given as
  // the fragment (or vertex) shader is compiled into a filament material.
  auto isMat = [](const std::string &_p)
  { return common::EndsWith(_p, ".mat"); };
  std::string source = isMat(this->fragmentShaderPath) ?
      this->fragmentShaderPath :
      (isMat(this->vertexShaderPath) ? this->vertexShaderPath : "");
  if (source.empty() &&
      (!this->fragmentShaderPath.empty() || !this->vertexShaderPath.empty()))
  {
    static bool warned = false;
    if (!warned)
    {
      gzwarn << "Filament does not take GLSL shaders ["
             << this->fragmentShaderPath << "]: give a filament material "
             << "source (.mat) instead. Drawing the regular material."
             << std::endl;
      warned = true;
    }
  }

  auto *engine = FilamentRenderEngine::Instance();
  bool fresh = false;
  if (source != this->customSource)
  {
    this->DestroyInstance(this->custom);
    this->custom = nullptr;
    this->customSource = source;
    filament::Material *mat = source.empty() ? nullptr :
        engine->Material(source);
    if (mat)
      this->custom = mat->createInstance();
    fresh = true;
  }
  if (!this->custom)
    return;

  const filament::Material *mat = this->custom->getMaterial();
  std::vector<filament::Material::ParameterInfo> infos(
      mat->getParameterCount());
  mat->getParameters(infos.data(), infos.size());
  filament::TextureSampler sampler(
      filament::TextureSampler::MinFilter::LINEAR_MIPMAP_LINEAR,
      filament::TextureSampler::MagFilter::LINEAR,
      filament::TextureSampler::WrapMode::REPEAT);
  if (fresh)
  {
    // filament wants every sampler bound: white 2D, black cube until set
    for (const auto &info : infos)
    {
      if (!info.isSampler)
        continue;
      filament::Texture *def =
          info.samplerType == filament::backend::SamplerType::SAMPLER_CUBEMAP ?
          engine->CubeTexture("") : engine->Texture("");
      this->custom->setParameter(info.name, def, sampler);
    }
  }
  for (const ShaderParamsPtr &params :
      {this->vertexShaderParams, this->fragmentShaderParams})
  {
    // like ogre2: only push what changed since the last frame
    if (!fresh && !params->IsDirty())
      continue;
    for (const auto &kv : *params)
    {
      const std::string &name = kv.first;
      const ShaderParam &param = kv.second;
      auto info = std::find_if(infos.begin(), infos.end(),
          [&](const filament::Material::ParameterInfo &_i)
          { return name == _i.name; });
      if (info == infos.end())
        continue;
      bool ok = false;
      if (info->isSampler)
      {
        // linear texels, wrapped, like ogre2's shader param textures
        // (ShaderParam::Value(string, uvSet) always returns false)
        std::string path;
        uint32_t uvSet = 0;
        param.Value(path, uvSet);
        filament::Texture *tex = nullptr;
        if (param.Type() == ShaderParam::PARAM_TEXTURE)
          tex = engine->Texture(path, nullptr, false);
        else if (param.Type() == ShaderParam::PARAM_TEXTURE_CUBE)
          tex = engine->CubeTexture(path, false);
        if (tex)
        {
          this->custom->setParameter(info->name, tex, sampler);
          ok = true;
        }
      }
      else
      {
        float f = 0.0f;
        int i = 0;
        std::shared_ptr<void> buf;
        switch (param.Type())
        {
          case ShaderParam::PARAM_FLOAT:
            ok = param.Value(&f) && SetUniform(this->custom, *info, &f, 1u);
            break;
          case ShaderParam::PARAM_INT:
            ok = param.Value(&i) && SetUniform(this->custom, *info, &i, 1u);
            break;
          case ShaderParam::PARAM_FLOAT_BUFFER:
            ok = param.Buffer(buf) && SetUniform(this->custom, *info,
                static_cast<const float *>(buf.get()), param.Count());
            break;
          case ShaderParam::PARAM_INT_BUFFER:
            ok = param.Buffer(buf) && SetUniform(this->custom, *info,
                static_cast<const int *>(buf.get()), param.Count());
            break;
          default:
            break;
        }
      }
      if (!ok)
      {
        gzwarn << "Shader param [" << name << "] could not be set: type "
               << "mismatch with [" << this->customSource << "] or texture "
               << "not loaded" << std::endl;
      }
    }
    params->ClearDirty();
  }
}
