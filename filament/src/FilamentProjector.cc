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
#include <filament/Material.h>
#include <filament/MaterialInstance.h>
#include <filament/Texture.h>
#include <filament/TextureSampler.h>
#include <filament/View.h>

#include <gz/common/Console.hh>
#include <gz/common/Filesystem.hh>
#include <gz/common/Image.hh>
#include <gz/math/Matrix4.hh>

#include "gz/rendering/Utils.hh"
#include "gz/rendering/filament/FilamentConversions.hh"
#include "gz/rendering/filament/FilamentProjector.hh"
#include "gz/rendering/filament/FilamentRenderEngine.hh"
#include "gz/rendering/filament/FilamentScene.hh"
#include "FilamentFullscreenPass.hh"

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
  this->pass.reset();
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
  this->texture = FilamentRenderEngine::Instance()->Texture(this->textureName);
  if (this->texture)
  {
    this->aspect = static_cast<double>(this->texture->getWidth()) /
        static_cast<double>(this->texture->getHeight());
  }
}

//////////////////////////////////////////////////
bool FilamentProjector::CreateResources()
{
  if (this->pass)
    return true;
  filament::Material *mat =
      FilamentRenderEngine::Instance()->Material("gz_projector");
  if (!mat)
    return false;
  this->pass = std::make_unique<FilamentFullscreenPass>(*mat);
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
  const filament::math::mat4f unit(
      FilamentConversions::Convert(unitFromWorld));

  auto *mi = this->pass->instance;
  mi->setParameter("depthIn", _depth, filament::TextureSampler());
  mi->setParameter("decalMap", this->texture, filament::TextureSampler(
      filament::TextureSampler::MinFilter::LINEAR,
      filament::TextureSampler::MagFilter::LINEAR,
      filament::TextureSampler::WrapMode::CLAMP_TO_EDGE));
  mi->setParameter("invViewProj", _invViewProj);
  mi->setParameter("unitFromWorld", unit);
  return this->pass->view;
}
