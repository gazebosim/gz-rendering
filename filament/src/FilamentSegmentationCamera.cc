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
#include <variant>

#include <filament/Engine.h>
#include <filament/Material.h>
#include <filament/MaterialInstance.h>
#include <filament/View.h>

#include <gz/common/Console.hh>

#include "gz/rendering/Image.hh"
#include "gz/rendering/PixelFormat.hh"
#include "gz/rendering/filament/FilamentConversions.hh"
#include "gz/rendering/filament/FilamentMesh.hh"
#include "gz/rendering/filament/FilamentRenderEngine.hh"
#include "gz/rendering/filament/FilamentRenderTarget.hh"
#include "gz/rendering/filament/FilamentScene.hh"
#include "gz/rendering/filament/FilamentSegmentationCamera.hh"

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
  auto *engine = FilamentRenderEngine::Instance()->FilamentEngine();
  if (engine)
  {
    for (auto &kv : this->flatInstances)
      engine->destroy(kv.second);
  }
  this->flatInstances.clear();
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
filament::MaterialInstance *FilamentSegmentationCamera::FlatInstance(
    const math::Color &_color)
{
  const uint32_t key =
      (static_cast<uint32_t>(std::lround(_color.R() * 255)) << 16) |
      (static_cast<uint32_t>(std::lround(_color.G() * 255)) << 8) |
      static_cast<uint32_t>(std::lround(_color.B() * 255));
  auto it = this->flatInstances.find(key);
  if (it != this->flatInstances.end())
    return it->second;
  filament::Material *mat =
      FilamentRenderEngine::Instance()->Material("gz_flat");
  if (!mat)
    return nullptr;
  filament::MaterialInstance *mi = mat->createInstance();
  mi->setParameter("color", filament::math::float3{
      _color.R(), _color.G(), _color.B()});
  this->flatInstances[key] = mi;
  return mi;
}

//////////////////////////////////////////////////
math::Color FilamentSegmentationCamera::LabelToColor(int64_t _label,
    bool _isMultiLink)
{
  if (_label == this->BackgroundLabel())
    return this->BackgroundColor();

  // same generator and sequence as ogre2: the label seeds the color
  this->generator.seed(_label);
  std::uniform_int_distribution<int> distribution(0, 255);
  const int r = distribution(this->generator);
  const int g = distribution(this->generator);
  const int b = distribution(this->generator);
  const math::Color color(r / 255.0f, g / 255.0f, b / 255.0f);

  if (this->Type() == SegmentationType::ST_SEMANTIC &&
      this->coloredLabel.count(_label))
    return color;
  if (_isMultiLink)
    return color;

  const int64_t colorId = static_cast<int64_t>(r) * 256 * 256 + g * 256 + b;
  if (!this->takenColors.insert(colorId).second)
    return this->LabelToColor(_label);

  this->coloredLabel.insert(_label);
  this->colorToLabel[colorId] = _label;
  return color;
}

//////////////////////////////////////////////////
math::Color FilamentSegmentationCamera::ColorForVisual(
    const VisualPtr &_visual, std::string &_prevParentName)
{
  int label = this->BackgroundLabel();
  Variant labelAny = _visual->UserData("label");
  if (const int *labelPtr = std::get_if<int>(&labelAny))
    label = *labelPtr;

  if (this->Type() == SegmentationType::ST_SEMANTIC)
  {
    if (this->IsColoredMap())
      return this->LabelToColor(label);
    return math::Color(label / 255.0f, label / 255.0f, label / 255.0f);
  }

  // panoptic: instances are counted per top level model
  VisualPtr top = _visual;
  while (top->Parent() && top->Parent() != this->scene->RootVisual())
    top = std::dynamic_pointer_cast<Visual>(top->Parent());
  const std::string parentName = top->Name();

  int &count = this->instancesCount[label];
  bool isMultiLink = false;
  if (parentName == _prevParentName)
  {
    isMultiLink = true;
  }
  else
  {
    ++count;
    _prevParentName = parentName;
  }

  if (this->IsColoredMap())
  {
    if (label == this->BackgroundLabel())
      return this->LabelToColor(label, isMultiLink);
    return this->LabelToColor(
        static_cast<int64_t>(label) * 256 * 256 + count, isMultiLink);
  }
  return math::Color((count % 256) / 255.0f, (count / 256) / 255.0f,
      label / 255.0f);
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

  this->colorToLabel.clear();
  std::string prevParentName;
  for (Entry &e : meshes)
  {
    filament::MaterialInstance *mi =
        this->FlatInstance(this->ColorForVisual(e.visual, prevParentName));
    if (mi)
      e.mesh->OverrideMaterial(mi);
  }
  this->instancesCount.clear();
  this->takenColors.clear();
  this->coloredLabel.clear();

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
  const uint8_t *src = this->image.Data<uint8_t>();
  this->buffer.assign(src, src + static_cast<size_t>(width) * height * 3u);
  this->newSegmentationFrame(this->buffer.data(), width, height, 3u,
      PixelUtil::Name(PF_R8G8B8));
}

//////////////////////////////////////////////////
uint8_t *FilamentSegmentationCamera::SegmentationData() const
{
  return this->buffer.empty() ? nullptr :
      const_cast<uint8_t *>(this->buffer.data());
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
  if (!this->IsColoredMap() || this->buffer.empty())
    return;
  const size_t n = static_cast<size_t>(this->ImageWidth()) *
      this->ImageHeight();
  for (size_t i = 0; i < n; ++i)
  {
    const size_t index = i * 3;
    const int64_t colorId = this->buffer[index] * 256 * 256 +
        this->buffer[index + 1] * 256 + this->buffer[index + 2];
    _labelBuffer[index] = this->backgroundLabel;
    _labelBuffer[index + 1] = this->backgroundLabel;
    _labelBuffer[index + 2] = this->backgroundLabel;
    auto it = this->colorToLabel.find(colorId);
    if (it == this->colorToLabel.end())
      continue;
    const int64_t label = it->second;
    if (this->Type() == SegmentationType::ST_SEMANTIC)
    {
      const uint8_t label8 = label % 256;
      _labelBuffer[index] = label8;
      _labelBuffer[index + 1] = label8;
      _labelBuffer[index + 2] = label8;
    }
    else
    {
      const uint16_t instance = label % (256 * 256);
      _labelBuffer[index + 2] = static_cast<uint8_t>(label / (256 * 256));
      _labelBuffer[index + 1] = static_cast<uint8_t>(instance / 256);
      _labelBuffer[index] = static_cast<uint8_t>(instance % 256);
    }
  }
}
