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
#include <filament/ColorGrading.h>
#include <filament/Engine.h>
#include <filament/TransformManager.h>
#include <filament/View.h>
#include <utils/EntityManager.h>

#include <gz/common/Console.hh>
#include <gz/math/Helpers.hh>
#include <gz/math/Matrix4.hh>

#include "gz/rendering/RayQuery.hh"
#include "gz/rendering/filament/FilamentCamera.hh"
#include "gz/rendering/filament/FilamentConversions.hh"
#include "gz/rendering/filament/FilamentMesh.hh"
#include "gz/rendering/filament/FilamentParticleEmitter.hh"
#include "gz/rendering/filament/FilamentRenderEngine.hh"
#include "gz/rendering/filament/FilamentScene.hh"

using namespace gz;
using namespace rendering;

/// \brief FilamentColorGrading.cc
filament::ColorGrading *CreateQualityColorGrading(filament::Engine &_engine);

//////////////////////////////////////////////////
void FilamentCamera::Destroy()
{
  auto *engine = FilamentRenderEngine::Instance()->FilamentEngine();
  if (engine)
  {
    if (this->view)
      engine->destroy(this->view);
    if (this->colorGrading)
      engine->destroy(this->colorGrading);
    if (!this->cameraEntity.isNull())
    {
      engine->destroyCameraComponent(this->cameraEntity);
      engine->getTransformManager().destroy(this->cameraEntity);
      ::utils::EntityManager::get().destroy(this->cameraEntity);
    }
  }
  this->view = nullptr;
  this->colorGrading = nullptr;
  this->camera = nullptr;
  this->cameraEntity = ::utils::Entity();
  if (this->renderTexture)
  {
    this->renderTexture->Destroy();
    this->renderTexture.reset();
  }
  BaseCamera::Destroy();
}

//////////////////////////////////////////////////
filament::View *FilamentCamera::View() const
{
  return this->view;
}

//////////////////////////////////////////////////
filament::Camera *FilamentCamera::Native() const
{
  return this->camera;
}

//////////////////////////////////////////////////
unsigned int FilamentCamera::RenderTextureGLId() const
{
  return this->renderTexture ? this->renderTexture->GLId() : 0u;
}

//////////////////////////////////////////////////
RenderTargetPtr FilamentCamera::RenderTarget() const
{
  return this->renderTexture;
}

//////////////////////////////////////////////////
void FilamentCamera::Init()
{
  BaseCamera::Init();
  auto *engine = FilamentRenderEngine::Instance()->FilamentEngine();
  auto &tm = engine->getTransformManager();

  this->cameraEntity = ::utils::EntityManager::get().create();
  tm.create(this->cameraEntity, tm.getInstance(this->entity),
      FilamentConversions::CameraFix());
  this->camera = engine->createCamera(this->cameraEntity);
  // exposure 1: pixel = radiance, no photographic scaling
  this->camera->setExposure(1.0f);

  this->view = engine->createView();
  this->view->setScene(this->scene->Native());
  this->view->setCamera(this->camera);
  this->view->setVisibleLayers(0xff, 0x1);
  // Sensors want plain radiance: no tone mapping, AA, dithering, bloom...
  // (the quality mode turns it on for the GUI camera, UpdateQualityMode)
  this->view->setPostProcessingEnabled(false);

  this->CreateRenderTexture();
  this->Reset();
}

//////////////////////////////////////////////////
void FilamentCamera::UpdateQualityMode()
{
  if (this->qualityChecked)
    return;
  this->qualityChecked = true;

  using namespace filament;
  // Sensors stop here: post-processing (tone mapping, GTAO, view MSAA)
  // cost +23..36% CPU on 6 cameras. gz-gui tags its camera before the
  // first render.
  const Variant user = this->UserData("user-camera");
  const bool *isUser = std::get_if<bool>(&user);
  if (!isUser || !*isUser)
    return;

  // the LUT is baked with filament's job system: on the render thread
  FilamentRenderEngine::Instance()->AdoptThread();
  auto *engine = FilamentRenderEngine::Instance()->FilamentEngine();
  // Tone mapping: identity up to 0.8, then PBR Neutral's highlight roll-off
  // (see FilamentColorGrading.cc), so base colours and mid-greys keep
  // ogre2's brightness (exposure stays 1) while sun + ambient on bright
  // surfaces no longer clip.
  this->colorGrading = CreateQualityColorGrading(*engine);
  this->view->setColorGrading(this->colorGrading);
  // also turns on temporal dithering, which assumes the LUT's sRGB output:
  // the target becomes plain RGBA8 (gz-gui reads it raw, like the sRGB one
  // it skips the decode of), rebuilt now so this frame lands in it
  this->view->setPostProcessingEnabled(true);
  this->renderTexture->SetSrgb(false);
  this->renderTexture->PreRender();

  // MSAA (set in Render) when the camera asks for it, else FXAA
  const bool msaa = this->AntiAliasing() > 1u;
  this->view->setAntiAliasing(msaa ?
      View::AntiAliasing::NONE : View::AntiAliasing::FXAA);

  // SSAO darkens the ambient (IndirectLight) term in creases
  // and contacts.
  // GTAO: SAO's default bias leaves self-occlusion noise on large flat
  // faces, a bias that removes it also removes most of the effect. Bent
  // normals and cone-traced shadows stay off: cost for little gain.
  View::AmbientOcclusionOptions ao;
  ao.enabled = true;
  ao.aoType = View::AmbientOcclusionOptions::AmbientOcclusionType::GTAO;
  ao.radius = 2.0f;
  ao.quality = View::QualityLevel::MEDIUM;
  ao.lowPassFilter = View::QualityLevel::HIGH;
  ao.upsampling = View::QualityLevel::MEDIUM;
  this->view->setAmbientOcclusionOptions(ao);

  // Shadows stay PCF. Filament 1.79's soft shadows (PCSS, VSM) filter
  // moments: Chebyshev's bound lets the sun bleed through thin walls into
  // interiors, with or without highPrecision / lightBleedReduction and a
  // smaller penumbra search. 1.79 has no PCF filter size (DPCF is gone).

  gzmsg << "Filament quality mode on for camera [" << this->Name()
        << "]: tone mapping, GTAO, physical lights, "
        << (msaa ? "MSAA " + std::to_string(std::min(this->AntiAliasing(),
            8u)) + "x" : std::string("FXAA")) << std::endl;
}

//////////////////////////////////////////////////
VisualPtr FilamentCamera::VisualAt(const math::Vector2i &_mousePos)
{
  if (!this->view || _mousePos.X() < 0 || _mousePos.Y() < 0 ||
      static_cast<unsigned int>(_mousePos.X()) >= this->ImageWidth() ||
      static_cast<unsigned int>(_mousePos.Y()) >= this->ImageHeight())
  {
    return VisualPtr();
  }

  // The query is answered by the next render of this view. Its callback can
  // fire after this function returns if the driver is slow, so the result
  // lives in a shared state rather than on the stack.
  struct Result { ::utils::Entity entity; bool done{false}; };
  auto result = std::make_shared<Result>();
  // filament picking coordinates have their origin at the bottom-left, as
  // the view renders (only the GUI's shared texture is turned top row
  // first, UpdateSharedTexture)
  this->view->pick(static_cast<uint32_t>(_mousePos.X()),
      this->ImageHeight() - 1u - static_cast<uint32_t>(_mousePos.Y()),
      [result](const filament::View::PickingQueryResult &_r)
      {
        result->entity = _r.renderable;
        result->done = true;
      });
  this->PreRender();
  this->Render();
  auto *engine = FilamentRenderEngine::Instance()->FilamentEngine();
  for (int i = 0; i < 10 && !result->done; ++i)
  {
    engine->flushAndWait();
    engine->pumpMessageQueues();
  }
  if (!result->done || result->entity.isNull())
    return VisualPtr();

  for (FilamentMesh *mesh : this->scene->Meshes())
  {
    if (mesh->Entity() == result->entity)
      return mesh->Parent();
  }
  return VisualPtr();
}

//////////////////////////////////////////////////
void FilamentCamera::CreateRenderTexture()
{
  RenderTexturePtr base = this->scene->CreateRenderTexture();
  this->renderTexture = std::dynamic_pointer_cast<FilamentRenderTexture>(base);
  this->renderTexture->SetFormat(PF_R8G8B8);
  this->renderTexture->SetWidth(this->ImageWidth());
  this->renderTexture->SetHeight(this->ImageHeight());
}

//////////////////////////////////////////////////
bool FilamentCamera::Orthographic() const
{
  // Sensor subclasses never get here with CPT_ORTHOGRAPHIC: their
  // SetProjectionType resolves to BaseCamera<FilamentCamera>, which only
  // stores the type, like ogre2 (only Ogre2Camera honours it)
  return this->projectionType == CPT_ORTHOGRAPHIC;
}

//////////////////////////////////////////////////
math::Matrix4d FilamentCamera::WithDepthRows(math::Matrix4d _matrix,
    double _near, double _far)
{
  const double invd = 1.0 / (_far - _near);
  _matrix(2, 0) = 0.0;
  _matrix(2, 1) = 0.0;
  if (!math::equal(_matrix(3, 3), 0.0))
  {
    // orthographic
    _matrix(2, 2) = -2.0 * invd;
    _matrix(2, 3) = -(_far + _near) * invd;
    return _matrix;
  }
  _matrix(2, 2) = -(_far + _near) * invd;
  _matrix(2, 3) = -2.0 * _far * _near * invd;
  _matrix(3, 0) = 0.0;
  _matrix(3, 1) = 0.0;
  _matrix(3, 2) = -1.0;
  return _matrix;
}

//////////////////////////////////////////////////
math::Matrix4d FilamentCamera::ProjectionMatrix() const
{
  if (this->customProjection)
    return this->projectionMatrix;
  if (this->Orthographic())
  {
    // ogre2 never sets an ortho window: ogre's default is 1000 units tall
    const double height = 1000.0;
    math::Matrix4d m = math::Matrix4d::Zero;
    m(0, 0) = 2.0 / (height * this->AspectRatio());
    m(1, 1) = 2.0 / height;
    m(3, 3) = 1.0;
    return WithDepthRows(m, this->NearClipPlane(), this->FarClipPlane());
  }
  return BaseCamera::ProjectionMatrix();
}

//////////////////////////////////////////////////
void FilamentCamera::SetProjectionMatrix(const math::Matrix4d &_matrix)
{
  BaseCamera::SetProjectionMatrix(_matrix);
  this->customProjection = true;
}

//////////////////////////////////////////////////
void FilamentCamera::SetProjectionType(CameraProjectionType _type)
{
  BaseCamera::SetProjectionType(_type);
  // like Ogre2Camera: a type change drops the custom matrix
  this->customProjection = false;
}

//////////////////////////////////////////////////
void FilamentCamera::SetClampedProjection(double _renderNear,
    double _renderFar)
{
  if (this->customProjection || this->Orthographic())
  {
    // image plane rows as given, depth rows from the clip planes: render
    // with the render ones, cull with the camera's own (ogre2 rebuilds the
    // depth row from its clip distances as well)
    const math::Matrix4d m = this->ProjectionMatrix();
    this->camera->setCustomProjection(
        FilamentConversions::Convert(WithDepthRows(m, _renderNear, _renderFar)),
        FilamentConversions::Convert(WithDepthRows(m, this->NearClipPlane(),
            this->FarClipPlane())),
        this->NearClipPlane(), this->FarClipPlane());
    return;
  }
  using filament::Camera;
  const double hfovDeg = this->HFOV().Degree();
  const double aspect = this->AspectRatio();
  this->camera->setCustomProjection(
      Camera::projection(Camera::Fov::HORIZONTAL, hfovDeg, aspect,
          _renderNear, _renderFar),
      Camera::projection(Camera::Fov::HORIZONTAL, hfovDeg, aspect,
          this->NearClipPlane(), this->FarClipPlane()),
      this->NearClipPlane(), this->FarClipPlane());
}

//////////////////////////////////////////////////
void FilamentCamera::UpdateProjection()
{
  if (!this->camera)
    return;
  if (this->customProjection || this->Orthographic())
  {
    this->SetClampedProjection(this->NearClipPlane(), this->FarClipPlane());
    return;
  }
  const double aspect = this->AspectRatio();
  const double hfovDeg = this->HFOV().Degree();
  this->camera->setProjection(hfovDeg, aspect, this->NearClipPlane(),
      this->FarClipPlane(), filament::Camera::Fov::HORIZONTAL);
}

//////////////////////////////////////////////////
void FilamentCamera::Render()
{
  this->UpdateQualityMode();
  // MSAA at the level the camera asks for, like ogre2's fsaa (gz-gui: 8,
  // gz-sensors: SDF <anti_aliasing>, default 4; unset: 0 = off).
  // Sensors (no post-processing): a multisampled render target, resolved
  // once into the camera's textures. View MSAA would add an R11G11B10F
  // intermediate, a resolve and a blit per frame (+16% CPU on 6 cameras).
  // The quality-mode GUI camera keeps view MSAA: its post-processing runs
  // on the intermediate buffer anyway.
  // Set every frame: gz-gui sets the level after the view exists.
  const auto aa = static_cast<uint8_t>(std::min(this->AntiAliasing(), 8u));
  const bool post = this->view->isPostProcessingEnabled();
  filament::View::MultiSampleAntiAliasingOptions msaa;
  msaa.enabled = post && aa > 1u;
  msaa.sampleCount = aa;
  this->view->setMultiSampleAntiAliasingOptions(msaa);
  this->renderTexture->SetSamples(post ? 1u : aa);
  this->renderTexture->PreRender();
  this->UpdateProjection();
  const math::Color bg = this->scene->BackgroundColor();
  this->renderTexture->SetClearColor(FilamentConversions::ToFloat4(bg));
  this->view->setVisibleLayers(0xff,
      FilamentConversions::MaskToLayers(this->VisibilityMask()));
  // particles are sorted back to front along the camera's forward vector
  const math::Vector3d forward =
      this->WorldPose().Rot() * math::Vector3d::UnitX;
  for (FilamentParticleEmitter *e : this->scene->ParticleEmitters())
    e->SortFor(forward);
  // the quality-mode GUI camera reflects the scene around it
  if (post)
  {
    this->scene->BeginReflections(this->ReflectionProbePosition(),
        this->VisibilityMask(), this->NearClipPlane(), this->FarClipPlane());
  }
  this->renderTexture->Render(this->view);
  if (post)
    this->scene->EndReflections();
  if (!this->scene->Projectors().empty())
  {
    // post-processing (the quality-mode GUI camera) renders the scene into
    // its own buffers and leaves the target's depth alone, and filament's
    // Vulkan backend resolves only the colour of a multisampled target:
    // fill the depth for the decals with a plain single-sample pass
    if (post)
    {
      this->view->setPostProcessingEnabled(false);
      this->renderTexture->RenderDepth(this->view);
      this->view->setPostProcessingEnabled(true);
    }
    else if (aa > 1u && FilamentRenderEngine::Instance()->FilamentEngine()
        ->getBackend() == filament::Engine::Backend::VULKAN)
    {
      this->renderTexture->RenderDepth(this->view);
    }
    // projected textures go on the scene, before the render passes
    const filament::math::mat4f invViewProj(FilamentConversions::Convert(
        (this->ProjectionMatrix() * this->ViewMatrix()).Inverse()));
    this->renderTexture->ApplyProjectors(this->scene->Projectors(),
        invViewProj, this->VisibilityMask());
  }
  this->renderTexture->ApplyRenderPasses(this->renderTexture->RenderPasses());
  // a GUI showing SharedGLId samples the image next, top row first
  this->renderTexture->UpdateSharedTexture();
}

//////////////////////////////////////////////////
math::Vector3d FilamentCamera::ReflectionProbePosition()
{
  const math::Pose3d pose = this->WorldPose();
  if (pose == this->probeCameraPose)
    return this->probePosition;
  this->probeCameraPose = pose;
  // 1 m short of what the camera looks at (at most half way), where the
  // reflective things the user looks at are; 5 m ahead if it looks at
  // nothing. Closer to the camera, the probe reflects too much of what is
  // behind the camera.
  constexpr double kBack = 1.0;
  constexpr double kNoHit = 5.0;
  const math::Vector3d dir = pose.Rot() * math::Vector3d::UnitX;
  RayQueryPtr query = this->scene->CreateRayQuery();
  query->SetOrigin(pose.Pos());
  query->SetDirection(dir);
  const RayQueryResult hit = query->ClosestPoint();
  const double d = hit ? hit.distance : 0.0;
  this->probePosition = pose.Pos() + dir *
      (hit ? std::max(0.5 * d, d - kBack) : kNoHit);
  return this->probePosition;
}
