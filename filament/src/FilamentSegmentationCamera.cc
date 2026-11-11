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

#include <filament/MaterialInstance.h>
#include <filament/View.h>

#include "gz/rendering/Image.hh"
#include "gz/rendering/PixelFormat.hh"
#include "gz/rendering/filament/FilamentConversions.hh"
#include "gz/rendering/filament/FilamentMesh.hh"
#include "gz/rendering/filament/FilamentRenderEngine.hh"
#include "gz/rendering/filament/FilamentRenderTarget.hh"
#include "gz/rendering/filament/FilamentScene.hh"
#include "gz/rendering/filament/FilamentSegmentationCamera.hh"
#include "FilamentUtils.hh"

using namespace gz;
using namespace rendering;

//////////////////////////////////////////////////
FilamentSegmentationCamera::FilamentSegmentationCamera()
{
}

//////////////////////////////////////////////////
FilamentSegmentationCamera::~FilamentSegmentationCamera()
{
}

//////////////////////////////////////////////////
void FilamentSegmentationCamera::Init()
{
  FilamentCamera::Init();
  this->view->setShadowingEnabled(false);
}

//////////////////////////////////////////////////
void FilamentSegmentationCamera::Destroy()
{
  filament_utils::DestroyFlatInstances(this->flatInstances);
  FilamentCamera::Destroy();
}

//////////////////////////////////////////////////
RenderTargetPtr FilamentSegmentationCamera::RenderTarget() const
{
  return this->renderTexture;
}

//////////////////////////////////////////////////
void FilamentSegmentationCamera::CreateSegmentationTexture()
{
  this->SetImageFormat(PF_R8G8B8);
  this->renderTexture->SetFormat(PF_R8G8B8);
  // labels must be written as is, not sRGB encoded
  this->renderTexture->SetSrgb(false);
  this->renderTexture->SetWidth(this->ImageWidth());
  this->renderTexture->SetHeight(this->ImageHeight());
  this->textureCreated = true;
}

//////////////////////////////////////////////////
void FilamentSegmentationCamera::SetBackgroundLabel(int _label)
{
  this->backgroundLabel = _label;
  this->SetBackgroundColor(
      math::Color(_label / 255.0, _label / 255.0, _label / 255.0));
}

//////////////////////////////////////////////////
void FilamentSegmentationCamera::Render()
{
  if (!this->textureCreated ||
      this->renderTexture->Width() != this->ImageWidth() ||
      this->renderTexture->Height() != this->ImageHeight())
    this->CreateSegmentationTexture();
  this->renderTexture->PreRender();

  this->UpdateProjection();
  const math::Color bg = this->BackgroundColor();
  this->renderTexture->SetClearColor(
      {static_cast<float>(bg.R()), static_cast<float>(bg.G()),
       static_cast<float>(bg.B()), 1.0f});
  this->view->setVisibleLayers(0xff,
      FilamentConversions::ToSensorLayers(this->VisibilityMask()));

  // Meshes with their visual, sorted like ogre2 sorts its items (name
  // "<visual>_<geometry>", descending): that order decides panoptic
  // instance numbers.
  struct Entry { std::string key; FilamentMesh *mesh; VisualPtr visual; };
  std::vector<Entry> meshes;
  for (FilamentMesh *m : this->scene->Meshes())
  {
    VisualPtr v = m->Parent();
    if (v)
      meshes.push_back({v->Name() + "_" + m->Name(), m, v});
  }
  std::stable_sort(meshes.begin(), meshes.end(),
      [](const Entry &_a, const Entry &_b) { return _a.key > _b.key; });

  this->ClearLabelColors();
  std::string prevParentName;
  for (Entry &e : meshes)
  {
    filament::MaterialInstance *mi = filament_utils::FlatInstance(
        this->flatInstances, this->ColorForVisual(e.visual, prevParentName));
    if (mi)
      e.mesh->OverrideMaterial(mi);
  }

  this->renderTexture->Render(this->view);
  for (Entry &e : meshes)
    e.mesh->RestoreMaterials();
}

//////////////////////////////////////////////////
void FilamentSegmentationCamera::PostRender()
{
  if (this->newSegmentationFrame.ConnectionCount() == 0u)
    return;
  const unsigned int width = this->ImageWidth();
  const unsigned int height = this->ImageHeight();
  if (this->image.Width() != width || this->image.Height() != height)
    this->image = Image(width, height, PF_R8G8B8);
  this->renderTexture->Copy(this->image);
  this->newSegmentationFrame(this->image.Data<uint8_t>(), width, height, 3u,
      PixelUtil::Name(PF_R8G8B8));
}

//////////////////////////////////////////////////
uint8_t *FilamentSegmentationCamera::SegmentationData() const
{
  return const_cast<uint8_t *>(this->image.Data<uint8_t>());
}

//////////////////////////////////////////////////
common::ConnectionPtr FilamentSegmentationCamera::ConnectNewSegmentationFrame(
    std::function<void(const uint8_t *, unsigned int, unsigned int,
    unsigned int, const std::string &)> _subscriber)
{
  return this->newSegmentationFrame.Connect(_subscriber);
}

//////////////////////////////////////////////////
void FilamentSegmentationCamera::LabelMapFromColoredBuffer(
    uint8_t *_labelBuffer) const
{
  BaseSegmentationCamera::LabelMapFromColoredBuffer(
      this->image.Data<uint8_t>(), _labelBuffer);
}
