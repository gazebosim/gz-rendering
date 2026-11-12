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
#include <vector>

#include <filament-iblprefilter/IBLPrefilterContext.h>
#include <filament/Engine.h>
#include <filament/IndirectLight.h>
#include <filament/LightManager.h>
#include <filament/RenderableManager.h>
#include <filament/Scene.h>
#include <filament/Skybox.h>
#include <filament/Texture.h>
#include <filament/TransformManager.h>

#include <gz/common/Console.hh>
#include <gz/math/Helpers.hh>

#include "gz/rendering/filament/FilamentArrowVisual.hh"
#include "gz/rendering/filament/FilamentAxisVisual.hh"
#include "gz/rendering/filament/FilamentBoundingBoxCamera.hh"
#include "gz/rendering/filament/FilamentCOMVisual.hh"
#include "gz/rendering/filament/FilamentCamera.hh"
#include "gz/rendering/filament/FilamentCapsule.hh"
#include "gz/rendering/filament/FilamentConversions.hh"
#include "gz/rendering/filament/FilamentDepthCamera.hh"
#include "gz/rendering/filament/FilamentDynamicMesh.hh"
#include "gz/rendering/filament/FilamentFrustumVisual.hh"
#include "gz/rendering/filament/FilamentGizmoVisual.hh"
#include "gz/rendering/filament/FilamentGpuRays.hh"
#include "gz/rendering/filament/FilamentGrid.hh"
#include "gz/rendering/filament/FilamentHeightmap.hh"
#include "gz/rendering/filament/FilamentInertiaVisual.hh"
#include "gz/rendering/filament/FilamentJointVisual.hh"
#include "gz/rendering/filament/FilamentLidarVisual.hh"
#include "gz/rendering/filament/FilamentLight.hh"
#include "gz/rendering/filament/FilamentLightVisual.hh"
#include "gz/rendering/filament/FilamentMarker.hh"
#include "gz/rendering/filament/FilamentMaterial.hh"
#include "gz/rendering/filament/FilamentMesh.hh"
#include "gz/rendering/filament/FilamentMeshFactory.hh"
#include "gz/rendering/filament/FilamentParticleEmitter.hh"
#include "gz/rendering/filament/FilamentProjector.hh"
#include "gz/rendering/filament/FilamentRayQuery.hh"
#include "gz/rendering/filament/FilamentRenderEngine.hh"
#include "gz/rendering/filament/FilamentRenderTarget.hh"
#include "gz/rendering/filament/FilamentScene.hh"
#include "gz/rendering/filament/FilamentSegmentationCamera.hh"
#include "gz/rendering/filament/FilamentThermalCamera.hh"
#include "gz/rendering/filament/FilamentVisual.hh"
#include "gz/rendering/filament/FilamentWideAngleCamera.hh"
#include "gz/rendering/filament/FilamentWireBox.hh"
#include "FilamentReflectionProbe.hh"
#include "FilamentUtils.hh"

using namespace gz;
using namespace rendering;

namespace
{
/// \brief Irradiance of a cube map (RGBA8 faces as CubeFaces gives them,
/// read as linear values) as 3-band SH in filament's shader form, i.e.
/// irradiance / pi = sum_i _sh[i] * poly_i(n), poly_i as in filament's
/// Irradiance_SphericalHarmonics (what cmgen --sh-shader writes).
void SkyIrradianceSh(const std::vector<uint8_t> &_faces, unsigned int _size,
    float _sh[9][3])
{
  // real SH basis Y_i = k_i * poly_i(n)
  const double k[9] = {0.282095, 0.488603, 0.488603, 0.488603, 1.092548,
      1.092548, 0.315392, 1.092548, 0.546274};
  // clamped cosine convolution over pi, per band (A_l / pi)
  const double a[9] = {1.0, 2.0 / 3, 2.0 / 3, 2.0 / 3, 0.25, 0.25, 0.25,
      0.25, 0.25};
  double acc[9][3] = {};
  double wsum = 0.0;
  // 3 bands are smooth: about 128 x 128 samples per face are plenty
  const unsigned int step = std::max(1u, _size / 128u);
  for (int f = 0; f < 6; ++f)
  {
    for (unsigned int y = 0; y < _size; y += step)
    {
      for (unsigned int x = 0; x < _size; x += step)
      {
        const uint8_t *t = _faces.data() +
            ((static_cast<size_t>(f) * _size + y) * _size + x) * 4u;
        // texel centre to direction, filament's (GL) cube map faces with
        // row 0 at the top, as libibl's Cubemap::getDirectionFor
        const double cx = (x + 0.5) * 2.0 / _size - 1.0;
        const double cy = 1.0 - (y + 0.5) * 2.0 / _size;
        const double d[6][3] = {{1, cy, -cx}, {-1, cy, cx}, {cx, 1, -cy},
            {cx, -1, cy}, {cx, cy, 1}, {-cx, cy, -1}};
        const double l2 = cx * cx + cy * cy + 1.0;
        const double l = std::sqrt(l2);
        const double nx = d[f][0] / l, ny = d[f][1] / l, nz = d[f][2] / l;
        // texel solid angle, up to a constant (normalised below)
        const double w = 1.0 / (l2 * l);
        const double p[9] = {1.0, ny, nz, nx, ny * nx, ny * nz,
            3.0 * nz * nz - 1.0, nz * nx, nx * nx - ny * ny};
        for (int i = 0; i < 9; ++i)
        {
          for (int c = 0; c < 3; ++c)
            acc[i][c] += w * k[i] * p[i] * t[c];
        }
        wsum += w;
      }
    }
  }
  const double s = 4.0 * GZ_PI / wsum / 255.0;
  for (int i = 0; i < 9; ++i)
  {
    for (int c = 0; c < 3; ++c)
      _sh[i][c] = static_cast<float>(acc[i][c] * s * a[i] * k[i]);
  }
}

/// \brief Luminance of a linear colour
double Luminance(double _r, double _g, double _b)
{
  return 0.2126 * _r + 0.7152 * _g + 0.0722 * _b;
}
}

//////////////////////////////////////////////////
FilamentScene::~FilamentScene()
{
}

//////////////////////////////////////////////////
FilamentScene::FilamentScene(unsigned int _id, const std::string &_name)
  : BaseScene(_id, _name)
{
}

//////////////////////////////////////////////////
void FilamentScene::Destroy()
{
  // meshes detached from every visual but still referenced by the user
  // keep their renderables (and thus material instances) alive; ogre2 drops
  // them with its scene manager, do the same. Attached ones go with their
  // visual in Clear(). (copy: Destroy unregisters)
  const std::set<FilamentMesh *> leftover = this->meshes;
  for (FilamentMesh *mesh : leftover)
  {
    if (mesh->HasParent())
      continue;
    auto keepAlive = mesh->shared_from_this();
    mesh->Destroy();
  }
  BaseScene::Destroy();
}

//////////////////////////////////////////////////
void FilamentScene::Fini()
{
  this->probe.reset();
  this->DestroySkyLight();
  auto *engine = FilamentRenderEngine::Instance()->FilamentEngine();
  if (engine && this->ambientLight)
  {
    engine->destroy(this->ambientLight);
    this->ambientLight = nullptr;
  }
  if (engine && this->skybox)
  {
    engine->destroy(this->skybox);
    this->skybox = nullptr;
  }
  if (engine && this->scene)
  {
    engine->destroy(this->scene);
    this->scene = nullptr;
  }
}

//////////////////////////////////////////////////
RenderEngine *FilamentScene::Engine() const
{
  return FilamentRenderEngine::Instance();
}

//////////////////////////////////////////////////
VisualPtr FilamentScene::RootVisual() const
{
  return this->rootVisual;
}

//////////////////////////////////////////////////
math::Color FilamentScene::AmbientLight() const
{
  return this->ambient;
}

//////////////////////////////////////////////////
VisualPtr FilamentScene::VisualAt(const CameraPtr &_camera,
    const math::Vector2i &_mousePos)
{
  return _camera ? _camera->VisualAt(_mousePos) : VisualPtr();
}

//////////////////////////////////////////////////
void FilamentScene::SetAmbientLight(const math::Color &_color)
{
  this->ambient = _color;
  auto *engine = FilamentRenderEngine::Instance()->FilamentEngine();
  if (!engine || !this->scene)
    return;
  if (this->ambientLight)
    engine->destroy(this->ambientLight);
  // A single-band SH is a uniform irradiance: classic constant ambient term.
  // Intensity calibrated against ogre2, whose ambient term measures as
  // ~0.32 * ambient * albedo (linear) on a white plane. The 1/3 goes in the
  // SH rather than the intensity, which also scales the skybox.
  // ogre2's fixed ambient (same upper and lower hemisphere) only lights the
  // diffuse term: black reflections leave metals dark like there (gz-sim's
  // default "gz-grey" primitive material is a metal).
  filament::math::float3 sh[1] = {
      FilamentConversions::ToFloat3(_color) * (1.0f / 3.0f)};
  this->ambientLight = filament::IndirectLight::Builder()
      .irradiance(1, sh)
      .reflections(FilamentRenderEngine::Instance()->CubeTexture(""))
      .intensity(1.0f)
      .build(*engine);
  this->scene->setIndirectLight(this->ambientLight);
  // the sky light and its reflections follow the ambient level: rebuilt
  // by UpdateSkyLight
  this->DestroySkyLight();
}

//////////////////////////////////////////////////
void FilamentScene::PreRender()
{
  BaseScene::PreRender();
  this->UpdateSkyLight();
}

//////////////////////////////////////////////////
void FilamentScene::UpdateSkyLight()
{
  if (!this->scene)
    return;

  if (!this->SkyEnabled())
  {
    // back to the uniform ambient (also when it is unset: nullptr)
    if (this->scene->getIndirectLight() == this->skyLight)
      this->scene->setIndirectLight(this->ambientLight);
    return;
  }

  auto *gzEngine = FilamentRenderEngine::Instance();
  auto *engine = gzEngine->FilamentEngine();
  if (!this->skyReflections)
  {
    // the prefilter runs on filament's job system: on the render thread,
    // which may not have rendered a camera yet
    gzEngine->AdoptThread();
    // once per sky cube map: the skybox's (SetSkyEnabled)
    unsigned int size = 0u;
    std::vector<uint8_t> faces;
    if (!gzEngine->CubeFaces(this->skyCubemap, size, faces))
      return;
    SkyIrradianceSh(faces, size, this->skySh);
    // Reflections: the sky at 0.3x the diffuse SH's scale (ambient over
    // sky luminance, see below). Nothing occludes them, so at the sky's own
    // brightness every glossy dielectric indoors mirrors a bright sky at
    // grazing angles. 0.3 keeps interiors within a few grey levels of the
    // default mode and a sky tint on water and rocks. The IndirectLight
    // intensity can't scale them: it also scales the skybox.
    // TODO: tuned on Harmonic; reflection probes or SSR for real
    // indoor occlusion
    constexpr double kReflectionScale = 0.3;
    const double skyLum = Luminance(this->skySh[0][0], this->skySh[0][1],
        this->skySh[0][2]);
    const double scale = skyLum > 0.0 ? std::min(1.0, kReflectionScale *
        Luminance(this->ambient.R(), this->ambient.G(), this->ambient.B()) /
        3.0 / skyLum) : 0.0;
    for (uint8_t &b : faces)
      b = static_cast<uint8_t>(std::lround(b * scale));
    uint8_t levels = 1u;
    while ((size >> levels) > 0u)
      ++levels;
    filament::Texture *scaled = filament::Texture::Builder()
        .width(size).height(size).levels(levels)
        .sampler(filament::Texture::Sampler::SAMPLER_CUBEMAP)
        .format(filament::Texture::InternalFormat::RGBA8)
        .usage(filament::Texture::Usage::DEFAULT |
               filament::Texture::Usage::GEN_MIPMAPPABLE)
        .build(*engine);
    auto *buf = new uint8_t[faces.size()];
    std::memcpy(buf, faces.data(), faces.size());
    scaled->setImage(*engine, 0, 0, 0, 0, size, size, 6,
        filament::Texture::PixelBufferDescriptor(buf, faces.size(),
        filament::Texture::Format::RGBA, filament::Texture::Type::UBYTE,
        &filament_utils::FreeBuffer<uint8_t>));
    {
      // GGX prefiltered mip chain on the GPU (5 roughness levels, 256 px,
      // filament's iblprefilter), from the scaled copy's mipmaps
      IBLPrefilterContext context(*engine);
      IBLPrefilterContext::SpecularFilter::Config config;
      // 1024 by default; the low dynamic range sky needs fewer
      config.sampleCount = 256u;
      IBLPrefilterContext::SpecularFilter filter(context, config);
      IBLPrefilterContext::SpecularFilter::Options options;
      options.generateMipmap = true;
      this->skyReflections = filter(options, scaled);
    }
    engine->flushAndWait();
    engine->destroy(scaled);
    gzdbg << "Filament quality mode: sky lighting ready" << std::endl;
  }
  if (!this->skyLight)
  {
    // Diffuse: the sky's directional irradiance as luminance, in the
    // uniform ambient's colour and level (band 0 = SetAmbientLight's SH, so
    // the view keeps ogre2's brightness and tint; a coloured sky SH turned
    // shaded sides and foliage blue). Reflections: the dimmed sky (above).
    // Intensity 1 also keeps the skybox as drawn without sky lighting.
    filament::math::float3 sh[9];
    this->SkyLightSh(sh);
    this->skyLight = filament::IndirectLight::Builder()
        .irradiance(3, sh)
        .reflections(this->skyReflections)
        .intensity(1.0f)
        .build(*engine);
  }
  if (this->scene->getIndirectLight() != this->skyLight)
    this->scene->setIndirectLight(this->skyLight);
}

//////////////////////////////////////////////////
void FilamentScene::SkyLightSh(filament::math::float3 _sh[9]) const
{
  const double skyLum = Luminance(this->skySh[0][0], this->skySh[0][1],
      this->skySh[0][2]);
  const filament::math::float3 ambient3 =
      FilamentConversions::ToFloat3(this->ambient) * (1.0f / 3.0f);
  for (int i = 0; i < 9; ++i)
  {
    const double lum = Luminance(this->skySh[i][0], this->skySh[i][1],
        this->skySh[i][2]);
    _sh[i] = ambient3 * (skyLum > 0.0 ? static_cast<float>(lum / skyLum) :
        0.0f);
  }
}

//////////////////////////////////////////////////
void FilamentScene::SetSkyEnabled(bool _enabled)
{
  auto *engine = FilamentRenderEngine::Instance();
  if (!this->scene)
    return;
  if (!_enabled)
  {
    this->scene->setSkybox(nullptr);
    return;
  }
  // ogre2's skybox: the cube map of the material "Default/skybox" if the
  // user made one (environment map, e.g. SDF <sky><cubemap_uri>), else
  // skybox.dds; drawn without lighting
  std::string path = "skybox.dds";
  MaterialPtr skyMat = this->Material("Default/skybox");
  if (skyMat && !skyMat->EnvironmentMap().empty())
    path = skyMat->EnvironmentMap();
  if (this->skybox && path != this->skyCubemap)
  {
    // another cube map: drop the sky and its lighting (rebuilt lazily)
    auto *fe = engine->FilamentEngine();
    this->scene->setSkybox(nullptr);
    fe->destroy(this->skybox);
    this->skybox = nullptr;
    this->DestroySkyLight();
  }
  if (!this->skybox)
  {
    filament::Texture *cube = engine->CubeTexture(path);
    if (!cube && path != "skybox.dds")
      cube = engine->CubeTexture(path = "skybox.dds");
    if (!cube)
      return;
    this->skyCubemap = path;
    // intensity: used while no ambient light (IndirectLight) is set, else
    // the IndirectLight's, which is 1 (see SetAmbientLight)
    this->skybox = filament::Skybox::Builder().environment(cube)
        .intensity(1.0f).build(*engine->FilamentEngine());
    // drawn by color views only: the sensor views (depth, thermal,
    // segmentation, lidar) leave the particle layer out
    // TODO: shares the particle layer, a layer of its own if a camera
    // ever hides particles but wants the sky
    this->skybox->setLayerMask(0xff, FilamentConversions::kParticleLayer);
  }
  this->scene->setSkybox(this->skybox);
}

//////////////////////////////////////////////////
bool FilamentScene::SkyEnabled() const
{
  return this->scene && this->scene->getSkybox();
}

//////////////////////////////////////////////////
bool FilamentScene::SetShadowTextureSize(LightType _lightType,
    unsigned int _textureSize)
{
  // same validation as Ogre2Scene
  if (_lightType != LightType::DIRECTIONAL)
  {
    gzerr << "Light type [" << static_cast<int>(_lightType)
          << "] is not supported." << std::endl;
    return false;
  }
  constexpr unsigned int kMaxTexSize = 16384u;
  const unsigned int maxTexSize = std::min<size_t>(kMaxTexSize,
      filament::Texture::getMaxTextureSize(
          *FilamentRenderEngine::Instance()->FilamentEngine(),
          filament::Texture::Sampler::SAMPLER_2D));
  if (_textureSize > maxTexSize)
  {
    gzerr << "<texture_size> of '" << _textureSize
          << "' exceeds maximum possible texture size of " << maxTexSize
          << ", using default texture size" << std::endl;
    return false;
  }
  if (_textureSize < 512u || !math::isPowerOfTwo(_textureSize))
  {
    gzerr << "<texture_size> of '" << _textureSize
          << "' is not a valid texture size,"
          << " using default texture size" << std::endl;
    return false;
  }
  this->dirTexSize = _textureSize;
  for (unsigned int i = 0; i < this->lights->Size(); ++i)
  {
    if (auto light = std::dynamic_pointer_cast<FilamentLight>(
        this->lights->GetByIndex(i)))
      light->SetShadowMapSize(_textureSize);
  }
  return true;
}

//////////////////////////////////////////////////
unsigned int FilamentScene::ShadowTextureSize(LightType _lightType) const
{
  switch (_lightType)
  {
    case LightType::DIRECTIONAL:
      return this->dirTexSize;
    case LightType::SPOT:
    case LightType::POINT:
      // not configurable, like ogre2
      return 2048u;
    default:
      gzerr << "Invalid light type [" << static_cast<int>(_lightType) << "]"
            << std::endl;
      return 0u;
  }
}

//////////////////////////////////////////////////
filament::Scene *FilamentScene::Native() const
{
  return this->scene;
}

//////////////////////////////////////////////////
const std::set<FilamentMesh *> &FilamentScene::Meshes() const
{
  return this->meshes;
}

//////////////////////////////////////////////////
void FilamentScene::RegisterMesh(FilamentMesh *_mesh)
{
  this->meshes.insert(_mesh);
}

//////////////////////////////////////////////////
void FilamentScene::UnregisterMesh(FilamentMesh *_mesh)
{
  this->meshes.erase(_mesh);
}

//////////////////////////////////////////////////
const std::vector<FilamentParticleEmitter *> &
    FilamentScene::ParticleEmitters() const
{
  return this->particleEmitters;
}

//////////////////////////////////////////////////
void FilamentScene::RegisterParticleEmitter(FilamentParticleEmitter *_emitter)
{
  this->particleEmitters.push_back(_emitter);
}

//////////////////////////////////////////////////
void FilamentScene::UnregisterParticleEmitter(
    FilamentParticleEmitter *_emitter)
{
  auto &v = this->particleEmitters;
  v.erase(std::remove(v.begin(), v.end(), _emitter), v.end());
}

//////////////////////////////////////////////////
const std::set<FilamentProjector *> &FilamentScene::Projectors() const
{
  return this->projectors;
}

//////////////////////////////////////////////////
void FilamentScene::RegisterProjector(FilamentProjector *_projector)
{
  this->projectors.insert(_projector);
}

//////////////////////////////////////////////////
void FilamentScene::UnregisterProjector(FilamentProjector *_projector)
{
  this->projectors.erase(_projector);
}

//////////////////////////////////////////////////
ProjectorPtr FilamentScene::CreateProjectorImpl(unsigned int _id,
    const std::string &_name)
{
  auto object = std::shared_ptr<FilamentProjector>(new FilamentProjector);
  bool result = this->InitObject(object, _id, _name);
  return (result) ? object : nullptr;
}

//////////////////////////////////////////////////
ParticleEmitterPtr FilamentScene::CreateParticleEmitterImpl(unsigned int _id,
    const std::string &_name)
{
  FilamentParticleEmitterPtr visual(new FilamentParticleEmitter);
  bool result = this->InitObject(visual, _id, _name);
  return (result) ? visual : nullptr;
}

//////////////////////////////////////////////////
DirectionalLightPtr FilamentScene::CreateDirectionalLightImpl(
    unsigned int _id, const std::string &_name)
{
  FilamentDirectionalLightPtr light(new FilamentDirectionalLight);
  bool result = this->InitObject(light, _id, _name);
  return (result) ? light : nullptr;
}

//////////////////////////////////////////////////
PointLightPtr FilamentScene::CreatePointLightImpl(unsigned int _id,
    const std::string &_name)
{
  FilamentPointLightPtr light(new FilamentPointLight);
  bool result = this->InitObject(light, _id, _name);
  return (result) ? light : nullptr;
}

//////////////////////////////////////////////////
SpotLightPtr FilamentScene::CreateSpotLightImpl(unsigned int _id,
    const std::string &_name)
{
  FilamentSpotLightPtr light(new FilamentSpotLight);
  bool result = this->InitObject(light, _id, _name);
  return (result) ? light : nullptr;
}

//////////////////////////////////////////////////
CameraPtr FilamentScene::CreateCameraImpl(unsigned int _id,
    const std::string &_name)
{
  FilamentCameraPtr camera(new FilamentCamera);
  bool result = this->InitObject(camera, _id, _name);
  return (result) ? camera : nullptr;
}

//////////////////////////////////////////////////
DepthCameraPtr FilamentScene::CreateDepthCameraImpl(unsigned int _id,
    const std::string &_name)
{
  FilamentDepthCameraPtr camera(new FilamentDepthCamera);
  bool result = this->InitObject(camera, _id, _name);
  return (result) ? camera : nullptr;
}

//////////////////////////////////////////////////
ThermalCameraPtr FilamentScene::CreateThermalCameraImpl(
    unsigned int _id, const std::string &_name)
{
  FilamentThermalCameraPtr camera(new FilamentThermalCamera);
  bool result = this->InitObject(camera, _id, _name);
  return (result) ? camera : nullptr;
}

//////////////////////////////////////////////////
SegmentationCameraPtr FilamentScene::CreateSegmentationCameraImpl(
    unsigned int _id, const std::string &_name)
{
  FilamentSegmentationCameraPtr camera(new FilamentSegmentationCamera);
  bool result = this->InitObject(camera, _id, _name);
  return (result) ? camera : nullptr;
}

//////////////////////////////////////////////////
BoundingBoxCameraPtr FilamentScene::CreateBoundingBoxCameraImpl(
    unsigned int _id, const std::string &_name)
{
  FilamentBoundingBoxCameraPtr camera(new FilamentBoundingBoxCamera);
  bool result = this->InitObject(camera, _id, _name);
  return (result) ? camera : nullptr;
}

//////////////////////////////////////////////////
GpuRaysPtr FilamentScene::CreateGpuRaysImpl(unsigned int _id,
    const std::string &_name)
{
  FilamentGpuRaysPtr rays(new FilamentGpuRays);
  bool result = this->InitObject(rays, _id, _name);
  return (result) ? rays : nullptr;
}

//////////////////////////////////////////////////
WideAngleCameraPtr FilamentScene::CreateWideAngleCameraImpl(
    unsigned int _id, const std::string &_name)
{
  FilamentWideAngleCameraPtr camera(new FilamentWideAngleCamera);
  bool result = this->InitObject(camera, _id, _name);
  return (result) ? camera : nullptr;
}

//////////////////////////////////////////////////
VisualPtr FilamentScene::CreateVisualImpl(unsigned int _id,
    const std::string &_name)
{
  FilamentVisualPtr visual(new FilamentVisual);
  bool result = this->InitObject(visual, _id, _name);
  return (result) ? visual : nullptr;
}

//////////////////////////////////////////////////
ArrowVisualPtr FilamentScene::CreateArrowVisualImpl(unsigned int _id,
    const std::string &_name)
{
  auto object = std::shared_ptr<FilamentArrowVisual>(new FilamentArrowVisual);
  bool result = this->InitObject(object, _id, _name);
  return (result) ? object : nullptr;
}

//////////////////////////////////////////////////
AxisVisualPtr FilamentScene::CreateAxisVisualImpl(unsigned int _id,
    const std::string &_name)
{
  auto object = std::shared_ptr<FilamentAxisVisual>(new FilamentAxisVisual);
  bool result = this->InitObject(object, _id, _name);
  return (result) ? object : nullptr;
}

//////////////////////////////////////////////////
GeometryPtr FilamentScene::CreateBoxImpl(unsigned int _id,
    const std::string &_name)
{
  return this->CreateMeshImpl(_id, _name, "unit_box");
}

//////////////////////////////////////////////////
GeometryPtr FilamentScene::CreateConeImpl(unsigned int _id,
    const std::string &_name)
{
  return this->CreateMeshImpl(_id, _name, "unit_cone");
}

//////////////////////////////////////////////////
GeometryPtr FilamentScene::CreateCylinderImpl(unsigned int _id,
    const std::string &_name)
{
  return this->CreateMeshImpl(_id, _name, "unit_cylinder");
}

//////////////////////////////////////////////////
GeometryPtr FilamentScene::CreatePlaneImpl(unsigned int _id,
    const std::string &_name)
{
  return this->CreateMeshImpl(_id, _name, "unit_plane");
}

//////////////////////////////////////////////////
GeometryPtr FilamentScene::CreateSphereImpl(unsigned int _id,
    const std::string &_name)
{
  return this->CreateMeshImpl(_id, _name, "unit_sphere");
}

//////////////////////////////////////////////////
MeshPtr FilamentScene::CreateMeshImpl(unsigned int _id,
    const std::string &_name, const std::string &_meshName)
{
  MeshDescriptor descriptor(_meshName);
  return this->CreateMeshImpl(_id, _name, descriptor);
}

//////////////////////////////////////////////////
MeshPtr FilamentScene::CreateMeshImpl(unsigned int _id,
    const std::string &_name, const MeshDescriptor &_desc)
{
  FilamentMeshPtr mesh(new FilamentMesh);
  if (!this->InitObject(mesh, _id, _name))
    return nullptr;
  if (!this->meshFactory->Load(mesh, _desc))
  {
    mesh->Destroy();
    return nullptr;
  }
  return mesh;
}

//////////////////////////////////////////////////
CapsulePtr FilamentScene::CreateCapsuleImpl(unsigned int _id,
    const std::string &_name)
{
  auto capsule = std::shared_ptr<FilamentCapsule>(new FilamentCapsule);
  bool result = this->InitObject(capsule, _id, _name);
  return (result) ? capsule : nullptr;
}

//////////////////////////////////////////////////
GridPtr FilamentScene::CreateGridImpl(unsigned int _id,
    const std::string &_name)
{
  auto object = std::shared_ptr<FilamentGrid>(new FilamentGrid);
  bool result = this->InitObject(object, _id, _name);
  return (result) ? object : nullptr;
}

//////////////////////////////////////////////////
MarkerPtr FilamentScene::CreateMarkerImpl(unsigned int _id,
    const std::string &_name)
{
  auto object = std::shared_ptr<FilamentMarker>(new FilamentMarker);
  bool result = this->InitObject(object, _id, _name);
  return (result) ? object : nullptr;
}

//////////////////////////////////////////////////
LidarVisualPtr FilamentScene::CreateLidarVisualImpl(unsigned int _id,
    const std::string &_name)
{
  auto object = std::shared_ptr<FilamentLidarVisual>(new FilamentLidarVisual);
  bool result = this->InitObject(object, _id, _name);
  return (result) ? object : nullptr;
}

//////////////////////////////////////////////////
FrustumVisualPtr FilamentScene::CreateFrustumVisualImpl(unsigned int _id,
    const std::string &_name)
{
  auto object = std::shared_ptr<FilamentFrustumVisual>(
      new FilamentFrustumVisual);
  bool result = this->InitObject(object, _id, _name);
  return (result) ? object : nullptr;
}

//////////////////////////////////////////////////
HeightmapPtr FilamentScene::CreateHeightmapImpl(unsigned int _id,
    const std::string &_name, const HeightmapDescriptor &_desc)
{
  FilamentHeightmapPtr heightmap(new FilamentHeightmap(_desc));
  this->InitObject(heightmap, _id, _name);
  if (heightmap->Entity().isNull())
  {
    heightmap->Destroy();
    return nullptr;
  }
  return heightmap;
}

//////////////////////////////////////////////////
WireBoxPtr FilamentScene::CreateWireBoxImpl(unsigned int _id,
    const std::string &_name)
{
  auto object = std::shared_ptr<FilamentWireBox>(new FilamentWireBox);
  bool result = this->InitObject(object, _id, _name);
  return (result) ? object : nullptr;
}

//////////////////////////////////////////////////
std::shared_ptr<FilamentDynamicMesh> FilamentScene::CreateDynamicMesh()
{
  auto mesh = std::make_shared<FilamentDynamicMesh>();
  const unsigned int objId = this->CreateObjectId();
  this->InitObject(mesh, objId, this->CreateObjectName(objId, "DynamicMesh"));
  return mesh;
}

//////////////////////////////////////////////////
MaterialPtr FilamentScene::CreateMaterialImpl(unsigned int _id,
    const std::string &_name)
{
  FilamentMaterialPtr material(new FilamentMaterial);
  bool result = this->InitObject(material, _id, _name);
  return (result) ? material : nullptr;
}

//////////////////////////////////////////////////
RenderTexturePtr FilamentScene::CreateRenderTextureImpl(unsigned int _id,
    const std::string &_name)
{
  FilamentRenderTexturePtr renderTexture(new FilamentRenderTexture);
  bool result = this->InitObject(renderTexture, _id, _name);
  return (result) ? renderTexture : nullptr;
}

//////////////////////////////////////////////////
RenderWindowPtr FilamentScene::CreateRenderWindowImpl(unsigned int,
    const std::string &)
{
  return nullptr;
}

//////////////////////////////////////////////////
RayQueryPtr FilamentScene::CreateRayQueryImpl(unsigned int _id,
    const std::string &_name)
{
  auto rayQuery = std::shared_ptr<FilamentRayQuery>(new FilamentRayQuery);
  bool result = this->InitObject(rayQuery, _id, _name);
  return (result) ? rayQuery : nullptr;
}

//////////////////////////////////////////////////
COMVisualPtr FilamentScene::CreateCOMVisualImpl(unsigned int _id,
    const std::string &_name)
{
  auto object = std::shared_ptr<FilamentCOMVisual>(new FilamentCOMVisual);
  bool result = this->InitObject(object, _id, _name);
  return (result) ? object : nullptr;
}

//////////////////////////////////////////////////
InertiaVisualPtr FilamentScene::CreateInertiaVisualImpl(unsigned int _id,
    const std::string &_name)
{
  auto object = std::shared_ptr<FilamentInertiaVisual>(
      new FilamentInertiaVisual);
  bool result = this->InitObject(object, _id, _name);
  return (result) ? object : nullptr;
}

//////////////////////////////////////////////////
JointVisualPtr FilamentScene::CreateJointVisualImpl(unsigned int _id,
    const std::string &_name)
{
  auto object = std::shared_ptr<FilamentJointVisual>(new FilamentJointVisual);
  bool result = this->InitObject(object, _id, _name);
  return (result) ? object : nullptr;
}

//////////////////////////////////////////////////
LightVisualPtr FilamentScene::CreateLightVisualImpl(unsigned int _id,
    const std::string &_name)
{
  auto object = std::shared_ptr<FilamentLightVisual>(new FilamentLightVisual);
  bool result = this->InitObject(object, _id, _name);
  return (result) ? object : nullptr;
}

//////////////////////////////////////////////////
GizmoVisualPtr FilamentScene::CreateGizmoVisualImpl(unsigned int _id,
    const std::string &_name)
{
  auto object = std::shared_ptr<FilamentGizmoVisual>(
      new FilamentGizmoVisual);
  bool result = this->InitObject(object, _id, _name);
  return (result) ? object : nullptr;
}

//////////////////////////////////////////////////
LightStorePtr FilamentScene::Lights() const
{
  return this->lights;
}

//////////////////////////////////////////////////
SensorStorePtr FilamentScene::Sensors() const
{
  return this->sensors;
}

//////////////////////////////////////////////////
VisualStorePtr FilamentScene::Visuals() const
{
  return this->visuals;
}

//////////////////////////////////////////////////
MaterialMapPtr FilamentScene::Materials() const
{
  return this->materials;
}

//////////////////////////////////////////////////
bool FilamentScene::LoadImpl()
{
  return true;
}

//////////////////////////////////////////////////
bool FilamentScene::InitImpl()
{
  auto *engine = FilamentRenderEngine::Instance()->FilamentEngine();
  if (!engine)
  {
    gzerr << "Filament engine not initialized" << std::endl;
    return false;
  }
  this->scene = engine->createScene();
#ifndef GZ_FILAMENT_POLYNOMIAL_ATTENUATION
  gzwarn << "Filament is built without gz-rendering's patch 0004: point "
         << "and spot lights use Filament's inverse-square falloff instead "
         << "of their attenuation, and light specular colours are ignored "
         << "(see filament/README.md)" << std::endl;
#endif
  this->CreateStores();
  this->CreateRootVisual();
  this->meshFactory = FilamentMeshFactoryPtr(
      new FilamentMeshFactory(this->SharedThis()));
  return true;
}

//////////////////////////////////////////////////
bool FilamentScene::InitObject(FilamentObjectPtr _object, unsigned int _id,
    const std::string &_name)
{
  _object->id = _id;
  _object->name = _name;
  _object->scene = this->SharedThis();
  _object->Load();
  _object->Init();
  return true;
}

//////////////////////////////////////////////////
void FilamentScene::CreateRootVisual()
{
  if (this->rootVisual)
    return;
  this->rootVisual = FilamentVisualPtr(new FilamentVisual);
  unsigned int rootId = this->CreateObjectId();
  std::string rootName = this->CreateObjectName(rootId, "_ROOT_");
  if (!this->InitObject(this->rootVisual, rootId, rootName))
  {
    gzerr << "Unable to create root visual" << std::endl;
    this->rootVisual = nullptr;
  }
}

//////////////////////////////////////////////////
void FilamentScene::CreateStores()
{
  this->lights = FilamentLightStorePtr(new FilamentLightStore);
  this->sensors = FilamentSensorStorePtr(new FilamentSensorStore);
  this->visuals = FilamentVisualStorePtr(new FilamentVisualStore);
  this->materials = FilamentMaterialMapPtr(new FilamentMaterialMap);
}

//////////////////////////////////////////////////
FilamentScenePtr FilamentScene::SharedThis()
{
  ScenePtr sharedBase = this->shared_from_this();
  return std::dynamic_pointer_cast<FilamentScene>(sharedBase);
}

//////////////////////////////////////////////////
void FilamentScene::BeginReflections(const math::Vector3d &_pos,
    uint32_t _visibilityMask, double _near, double _far)
{
  if (!this->scene || FilamentRenderEngine::Instance()->InFrame())
    return;
  // 128 px faces, 64 prefilter samples, one face per frame while stale
  // (a change is in the reflections 5 frames later, up to 10 if a refresh
  // was under way), sun shadows on: the
  // cheaper settings measured worse (no shadows) or no faster
  constexpr uint32_t kSize = 128u;
  constexpr uint16_t kSamples = 64u;
  constexpr unsigned int kFacesPerFrame = 1u;
  auto *engine = FilamentRenderEngine::Instance()->FilamentEngine();
  if (!this->probe)
  {
    this->probe = std::make_unique<filament_probe::ReflectionProbe>(
        *engine, this->scene, kSize, kSamples);
  }
  // the cube sees the scene IBL: no reflections of reflections
  filament::IndirectLight *base = this->scene->getIndirectLight();
  // and the sky at the ambient light's level (the radiance a uniform sky
  // of the ambient colour would have); without sky lighting, black like the
  // uniform ambient's reflections
  filament::Skybox *sky = this->scene->getSkybox();
  const bool skyLit = base && base == this->skyLight && this->skyReflections;
  if (skyLit && !this->probeSkybox)
  {
    // UpdateSkyLight's reflections without its kReflectionScale: that
    // dims an unoccluded sky for interiors, here the cube's walls occlude
    // it.
    unsigned int cubeSize = 0u;
    std::vector<uint8_t> faces;
    const double skyLum = Luminance(this->skySh[0][0], this->skySh[0][1],
        this->skySh[0][2]);
    if (skyLum > 0.0 && FilamentRenderEngine::Instance()->CubeFaces(
        this->skyCubemap, cubeSize, faces))
    {
      const double scale = std::min(1.0, Luminance(this->ambient.R(),
          this->ambient.G(), this->ambient.B()) / 3.0 / skyLum);
      auto *buf = new uint8_t[faces.size()];
      for (size_t i = 0; i < faces.size(); ++i)
        buf[i] = static_cast<uint8_t>(std::lround(faces[i] * scale));
      this->probeSkyTexture = filament::Texture::Builder()
          .width(cubeSize).height(cubeSize).levels(1)
          .sampler(filament::Texture::Sampler::SAMPLER_CUBEMAP)
          .format(filament::Texture::InternalFormat::RGBA8)
          .build(*engine);
      this->probeSkyTexture->setImage(*engine, 0, 0, 0, 0, cubeSize,
          cubeSize, 6, filament::Texture::PixelBufferDescriptor(buf,
          faces.size(), filament::Texture::Format::RGBA,
          filament::Texture::Type::UBYTE,
          &filament_utils::FreeBuffer<uint8_t>));
      this->probeSkybox = filament::Skybox::Builder()
          .environment(this->probeSkyTexture).build(*engine);
      this->probeSkybox->setLayerMask(0xff,
          FilamentConversions::kParticleLayer);
    }
  }
  this->scene->setSkybox(skyLit ? this->probeSkybox : nullptr);
  this->probe->Render({_pos.X(), _pos.Y(), _pos.Z()},
      FilamentConversions::MaskToLayers(_visibilityMask),
      static_cast<float>(_near), static_cast<float>(_far),
      {0.0f, 0.0f, 0.0f, 0.0f}, kFacesPerFrame,
      this->ReflectionsSignature(_pos) ^
      (reinterpret_cast<uintptr_t>(base) * 0x9e3779b97f4a7c15ull));
  this->scene->setSkybox(sky);
  filament::math::float3 sh[9];
  uint8_t bands = 1u;
  if (base && base == this->skyLight)
  {
    this->SkyLightSh(sh);
    bands = 3u;
  }
  else
  {
    sh[0] = FilamentConversions::ToFloat3(this->ambient) * (1.0f / 3.0f);
  }
  filament::IndirectLight *light = this->probe->Light(sh, bands);
  if (!light)
    return;
  this->probeSavedLight = base;
  this->probeActive = true;
  this->scene->setIndirectLight(light);
}

//////////////////////////////////////////////////
uint64_t FilamentScene::ReflectionsSignature(const math::Vector3d &_pos) const
{
  // FNV-1a over what the probe's cube shows: mesh world transforms and
  // layers, lights, the probe position.
  // Material and sky changes are not hashed: they show up with the next
  // move of anything.
  uint64_t h = 1469598103934665603ull;
  auto add = [&h](const void *_p, size_t _n)
  {
    const auto *b = static_cast<const uint8_t *>(_p);
    for (size_t i = 0; i < _n; ++i)
      h = (h ^ b[i]) * 1099511628211ull;
  };
  const double pos[3] = {_pos.X(), _pos.Y(), _pos.Z()};
  add(pos, sizeof(pos));
  auto *engine = FilamentRenderEngine::Instance()->FilamentEngine();
  auto &tm = engine->getTransformManager();
  auto &rm = engine->getRenderableManager();
  for (FilamentMesh *mesh : this->meshes)
  {
    const ::utils::Entity e = mesh->Entity();
    add(&e, sizeof(e));
    if (tm.hasComponent(e))
    {
      const filament::math::mat4f m =
          tm.getWorldTransform(tm.getInstance(e));
      add(&m, sizeof(m));
    }
    if (rm.hasComponent(e))
    {
      const uint8_t layers = rm.getLayerMask(rm.getInstance(e));
      add(&layers, 1);
    }
  }
  auto &lm = engine->getLightManager();
  const ::utils::Entity *lights = lm.getEntities();
  for (size_t i = 0; i < lm.getComponentCount(); ++i)
  {
    auto inst = lm.getInstance(lights[i]);
    const filament::math::float3 v[3] = {lm.getPosition(inst),
        lm.getDirection(inst), lm.getColor(inst)};
    const float intensity = lm.getIntensity(inst);
    add(v, sizeof(v));
    add(&intensity, sizeof(intensity));
  }
  return h;
}

//////////////////////////////////////////////////
void FilamentScene::DestroySkyLight()
{
  auto *engine = FilamentRenderEngine::Instance()->FilamentEngine();
  if (!engine)
    return;
  if (this->skyLight)
  {
    if (this->scene && this->scene->getIndirectLight() == this->skyLight)
      this->scene->setIndirectLight(this->ambientLight);
    engine->destroy(this->skyLight);
  }
  if (this->probeSkybox)
    engine->destroy(this->probeSkybox);
  if (this->probeSkyTexture)
    engine->destroy(this->probeSkyTexture);
  if (this->skyReflections)
    engine->destroy(this->skyReflections);
  this->skyLight = nullptr;
  this->probeSkybox = nullptr;
  this->probeSkyTexture = nullptr;
  this->skyReflections = nullptr;
}

//////////////////////////////////////////////////
void FilamentScene::EndReflections()
{
  if (!this->probeActive)
    return;
  this->scene->setIndirectLight(this->probeSavedLight);
  this->probeActive = false;
}
