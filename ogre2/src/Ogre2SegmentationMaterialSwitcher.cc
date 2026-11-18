/*
 * Copyright (C) 2021 Open Source Robotics Foundation
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

#include "Ogre2SegmentationMaterialSwitcher.hh"

#include <algorithm>
#include <array>
#include <cmath>
#include <utility>
#include <vector>
#include <variant>

#include <gz/common/Console.hh>
#include <gz/common/Profiler.hh>

#include "gz/rendering/ogre2/Ogre2Heightmap.hh"
#include "gz/rendering/ogre2/Ogre2RenderEngine.hh"
#include "gz/rendering/ogre2/Ogre2Scene.hh"
#include "gz/rendering/ogre2/Ogre2Visual.hh"
#include "gz/rendering/RenderTypes.hh"

#include "Terra/Terra.h"

#ifdef _MSC_VER
  #pragma warning(push, 0)
#endif
#include <OgreItem.h>
#include <OgreMaterialManager.h>
#include <OgrePass.h>
#include <OgreSceneManager.h>
#include <OgreTechnique.h>
#ifdef _MSC_VER
  #pragma warning(pop)
#endif

using namespace gz;
using namespace rendering;

namespace
{
/////////////////////////////////////////////////
/// \brief Whether the camera may render an item: visible, and its bounding
/// box not clearly outside the view frustum.
/// \param[in] _item The item
/// \param[in] _planes The frustum planes, pointing inwards
/// \param[in] _planeCount Number of planes
bool InView(const Ogre::Item *_item, const Ogre::Plane *_planes,
    size_t _planeCount)
{
  if (!_item->isVisible())
    return false;

  const Ogre::Aabb aabb = _item->getWorldAabb();
  const Ogre::Vector3 &halfSize = aabb.mHalfSize;
  // Infinite, empty or invalid boxes: let Ogre decide.
  if (!std::isfinite(halfSize.x) || !std::isfinite(halfSize.y) ||
      !std::isfinite(halfSize.z) || !std::isfinite(aabb.mCenter.x) ||
      !std::isfinite(aabb.mCenter.y) || !std::isfinite(aabb.mCenter.z) ||
      halfSize.x < 0 || halfSize.y < 0 || halfSize.z < 0)
  {
    return true;
  }

  // An item Ogre renders without its color set would make the shaders
  // throw, so only skip items that are clearly out of view: grow the box
  // before testing it.
  const Ogre::Vector3 grown = halfSize * 1.01f + Ogre::Vector3(1e-3f);
  // Same test as Ogre::Frustum::isVisible, without rebuilding the frustum
  // and the box for every item.
  for (size_t i = 0; i < _planeCount; ++i)
  {
    const Ogre::Plane &plane = _planes[i];
    if (plane.normal.dotProduct(aabb.mCenter) + plane.d <
        -plane.normal.absDotProduct(grown))
    {
      return false;
    }
  }
  return true;
}
}  // namespace

/////////////////////////////////////////////////
Ogre2SegmentationMaterialSwitcher::Ogre2SegmentationMaterialSwitcher(
  Ogre2ScenePtr _scene, SegmentationCamera *_camera)
{
  this->scene = _scene;
  this->segmentationCamera = _camera;
}

/////////////////////////////////////////////////
Ogre2SegmentationMaterialSwitcher::~Ogre2SegmentationMaterialSwitcher()
{
  this->segmentationCamera = nullptr;
}

/////////////////////////////////////////////////
int Ogre2SegmentationMaterialSwitcher::VisualLabel(const Visual &_visual)
{
  const Variant labelAny = _visual.UserData("label");
  const int *label = std::get_if<int>(&labelAny);
  // items with no class are considered background
  if (!label)
    return this->segmentationCamera->BackgroundLabel();

  if ((*label < 0 || *label > 255) && this->warnedLabels.insert(*label).second)
  {
    gzwarn << "Segmentation camera: label " << *label << " is outside the "
           << "range of label maps (0-255); it is stored as "
           << (*label < 0 ? 0 : 255) << " in them." << std::endl;
  }
  return *label;
}

/////////////////////////////////////////////////
const Node *Ogre2SegmentationMaterialSwitcher::TopLevelModel(
    const Node *_node) const
{
  // Walk up the parents by id: cheaper than comparing with RootVisual() and
  // casting to Visual at every step.
  NodePtr parent = _node->Parent();
  while (parent && parent->Id() != this->rootId)
  {
    _node = parent.get();
    parent = parent->Parent();
  }
  return _node;
}

/////////////////////////////////////////////////
Ogre::Vector4 Ogre2SegmentationMaterialSwitcher::KeyColor(int64_t _key)
{
  auto it = this->keyColors.find(_key);
  if (it != this->keyColors.end())
    return it->second;

  // Use the key as seed, so a key gets the same color every time. If the
  // color is taken by another key, or is the background color, try the next
  // one from the same seed.
  this->generator.seed(_key);
  std::uniform_int_distribution<int> distribution(0, 255);
  int r, g, b;
  int64_t colorId;
  do
  {
    r = distribution(this->generator);
    g = distribution(this->generator);
    b = distribution(this->generator);
    colorId = r * 256 * 256 + g * 256 + b;
  }
  while (colorId == this->backgroundColorId ||
         this->colorToLabel.count(colorId));

  this->colorToLabel[colorId] = _key;
  const Ogre::Vector4 color(r / 255.0f, g / 255.0f, b / 255.0f, 1.0f);
  this->keyColors.emplace(_key, color);
  return color;
}

/////////////////////////////////////////////////
Ogre::Vector4 Ogre2SegmentationMaterialSwitcher::SemanticColor(int _label)
{
  if (this->segmentationCamera->IsColoredMap())
  {
    if (_label == this->segmentationCamera->BackgroundLabel())
      return this->backgroundColor;
    return this->KeyColor(_label);
  }

  // label map: each pixel has the item's label
  const float labelColor = _label / 255.0f;
  return Ogre::Vector4(labelColor, labelColor, labelColor, 1.0);
}

/////////////////////////////////////////////////
Ogre::Vector4 Ogre2SegmentationMaterialSwitcher::PanopticColor(int _label,
    unsigned int _instance)
{
  // 16 bits for the instance
  if (_instance > 0xFFFFu)
  {
    if (!this->warnedInstances)
    {
      gzwarn << "Segmentation camera: more than 65535 instances of label "
             << _label << "; the rest share instance 65535." << std::endl;
      this->warnedInstances = true;
    }
    _instance = 0xFFFFu;
  }

  if (this->segmentationCamera->IsColoredMap())
  {
    if (_label == this->segmentationCamera->BackgroundLabel())
      return this->backgroundColor;
    return this->KeyColor(static_cast<int64_t>(_label) * 256 * 256 +
        _instance);
  }

  // label map: label in the 3rd channel, instance in the first two
  const float labelColor = _label / 255.0f;
  const float instanceColor1 = (_instance / 256) / 255.0f;
  const float instanceColor2 = (_instance % 256) / 255.0f;
  return Ogre::Vector4(instanceColor2, instanceColor1, labelColor, 1.0);
}

/////////////////////////////////////////////////
void Ogre2SegmentationMaterialSwitcher::UpdateColorState()
{
  const math::Color &bg = this->segmentationCamera->BackgroundColor();
  this->backgroundColor = Ogre::Vector4(bg.R(), bg.G(), bg.B(), 1.0);
  const int64_t bgId =
      static_cast<int64_t>(std::lround(bg.R() * 255)) * 256 * 256 +
      static_cast<int64_t>(std::lround(bg.G() * 255)) * 256 +
      static_cast<int64_t>(std::lround(bg.B() * 255));
  const SegmentationType type = this->segmentationCamera->Type();
  const bool colored = this->segmentationCamera->IsColoredMap();

  // Colors are kept between frames. They depend on the mode and on the
  // background color, and keys of removed objects pile up: start over when
  // any of these change, or when there are many more keys than in use.
  if (type != this->colorType || colored != this->colorColored ||
      bgId != this->backgroundColorId ||
      this->keyColors.size() > 2 * this->keysInUse + 1024)
  {
    this->keyColors.clear();
    this->colorToLabel.clear();
    this->colorType = type;
    this->colorColored = colored;
    this->backgroundColorId = bgId;
  }
}

/////////////////////////////////////////////////
uint32_t Ogre2SegmentationMaterialSwitcher::Group(const Node *_model,
    int _label)
{
  const uint64_t key = (static_cast<uint64_t>(_model->Id()) << 32) |
      static_cast<uint32_t>(_label);
  auto [it, inserted] = this->groupIndex.try_emplace(key,
      static_cast<uint32_t>(this->groups.size()));
  if (inserted)
    this->groups.push_back({key, _model, _label, 0u, Ogre::Vector4()});
  return it->second;
}

/////////////////////////////////////////////////
void Ogre2SegmentationMaterialSwitcher::NumberInstances()
{
  GZ_PROFILE("Ogre2SegmentationMaterialSwitcher::NumberInstances");
  // Instances of a label are numbered 1, 2, ... in descending order of
  // their model's name, as they have always been; then the heightmaps that
  // are not part of those models. Sorting is only needed when the models
  // change.
  this->groupKeys.clear();
  for (const auto &group : this->groups)
    this->groupKeys.push_back(group.key);
  if (this->groupKeys != this->sortedGroupKeys ||
      this->itemGroupCount != this->sortedItemGroupCount)
  {
    std::vector<std::string> names(this->itemGroupCount);
    for (size_t g = 0; g < this->itemGroupCount; ++g)
      names[g] = this->groups[g].model->Name();
    this->groupOrder.resize(this->groups.size());
    for (size_t g = 0; g < this->groupOrder.size(); ++g)
      this->groupOrder[g] = static_cast<uint32_t>(g);
    std::sort(this->groupOrder.begin(),
        this->groupOrder.begin() + this->itemGroupCount,
        [&](uint32_t _a, uint32_t _b)
        {
          if (names[_a] != names[_b])
            return names[_a] > names[_b];
          return this->groups[_a].key < this->groups[_b].key;
        });
    this->sortedGroupKeys = this->groupKeys;
    this->sortedItemGroupCount = this->itemGroupCount;
  }

  this->instanceCounts.clear();
  for (uint32_t g : this->groupOrder)
  {
    InstanceGroup &group = this->groups[g];
    group.instance = ++this->instanceCounts[group.label];
    group.color = this->PanopticColor(group.label, group.instance);
  }
}

////////////////////////////////////////////////
VisualPtr Ogre2SegmentationMaterialSwitcher::ItemVisual(
    const Ogre::Item *_item, unsigned int _visualId)
{
  auto &cached = this->visualCache[_item->getId()];
  cached.lastSeen = this->frame;
  if (cached.visualId == _visualId)
  {
    if (VisualPtr visual = cached.visual.lock())
      return visual;
  }

  // first time this item is seen (or its visual changed): slow lookup
  VisualPtr visual;
  try
  {
    visual = this->scene->VisualById(_visualId);
  }
  catch(Ogre::Exception &e)
  {
    gzerr << "Ogre Error:" << e.getFullDescription() << "\n";
  }
  cached.visualId = _visualId;
  cached.visual = visual;
  return visual;
}

////////////////////////////////////////////////
void Ogre2SegmentationMaterialSwitcher::cameraPreRenderScene(
    Ogre::Camera *_cam)
{
  GZ_PROFILE("Ogre2SegmentationMaterialSwitcher::cameraPreRenderScene");
  auto engine = Ogre2RenderEngine::Instance();
  engine->SetGzOgreRenderingMode(GORM_SOLID_COLOR);

  this->rootId = this->scene->RootVisual()->Id();
  this->UpdateColorState();
  const bool panoptic =
      this->segmentationCamera->Type() == SegmentationType::ST_PANOPTIC;

  // Find the visual, label and, for panoptic segmentation, the model of
  // every item. Items of the same model and label are one instance.
  this->items.clear();
  this->groups.clear();
  this->groupIndex.clear();
  auto itor = this->scene->OgreSceneManager()->getMovableObjectIterator(
      Ogre::ItemFactory::FACTORY_TYPE_NAME);
  while (itor.hasMoreElements())
  {
    Ogre::Item *item = static_cast<Ogre::Item *>(itor.getNext());

    // get visual from ogre item
    const Ogre::Any &userAny = item->getUserObjectBindings().getUserAny();
    if (userAny.isEmpty() || userAny.getType() != typeid(unsigned int))
      continue;
    VisualPtr visual =
        this->ItemVisual(item, Ogre::any_cast<unsigned int>(userAny));

    // Without a visual there is no label: draw it as the background.
    if (!visual)
    {
      this->items.push_back({item, 0, kBackgroundGroup});
      continue;
    }

    const int label = this->VisualLabel(*visual);
    uint32_t group = 0;
    if (panoptic)
      group = this->Group(this->TopLevelModel(visual.get()), label);
    this->items.push_back({item, label, group});
  }

  this->itemGroupCount = this->groups.size();

  // Heightmaps / terrain
  this->heightmapGroups.clear();
  for (const auto &h : this->scene->Heightmaps())
  {
    auto heightmap = h.lock();
    if (!heightmap || !heightmap->Terra())
      continue;
    VisualPtr visual = heightmap->Parent();
    if (!visual)
    {
      this->heightmapGroups.push_back({heightmap.get(), 0,
          kBackgroundGroup});
      continue;
    }
    const int label = this->VisualLabel(*visual);
    uint32_t group = 0;
    if (panoptic)
      group = this->Group(this->TopLevelModel(visual.get()), label);
    this->heightmapGroups.push_back({heightmap.get(), label, group});
  }

  if (panoptic)
    this->NumberInstances();

  this->materialMap.clear();
  this->datablockMap.clear();
  this->coloredSubItems.clear();

  // Frustum planes, to leave out-of-view items alone. The far plane is
  // ignored when the far distance is infinite, as Ogre does.
  std::array<Ogre::Plane, 6> planes;
  size_t planeCount = 0;
  if (_cam)
  {
    const Ogre::Plane *frustum = _cam->getFrustumPlanes();
    for (int i = 0; i < 6; ++i)
    {
      if (i == Ogre::FRUSTUM_PLANE_FAR && !(_cam->getFarClipDistance() > 0))
        continue;
      planes[planeCount++] = frustum[i];
    }
  }

  Ogre::HlmsManager *hlmsManager = engine->OgreRoot()->getHlmsManager();

  Ogre::HlmsDatablock *defaultPbs =
    hlmsManager->getHlms(Ogre::HLMS_PBS)->getDefaultDatablock();

  // Construct one now so that datablock->setBlendblock
  // each is as fast as possible
  const Ogre::HlmsBlendblock *noBlend =
    hlmsManager->getBlendblock(Ogre::HlmsBlendblock());

  for (const auto &entry : this->items)
  {
    Ogre::Item *item = entry.item;

    // Items the camera won't render keep their materials: setting and
    // restoring them is most of the cost in large scenes.
    if (!InView(item, planes.data(), planeCount))
      continue;

    Ogre::Vector4 customParameter;
    if (entry.group == kBackgroundGroup)
      customParameter = this->backgroundColor;
    else if (panoptic)
      customParameter = this->groups[entry.group].color;
    else
      customParameter = this->SemanticColor(entry.label);
    const size_t numSubItems = item->getNumSubItems();
    for (size_t i = 0; i < numSubItems; ++i)
    {
      // Set the custom value to the sub item to render
      Ogre::SubItem *subItem = item->getSubItem(i);
      subItem->setCustomParameter(1, customParameter);
      this->coloredSubItems.push_back(subItem);

      if (!subItem->getMaterial().isNull())
      {
        this->materialMap.push_back({ subItem, subItem->getMaterial() });

        // We need to keep the material's vertex shader
        // to keep vertex deformation consistent; so we use
        // a cloned material with a different pixel shader
        // https://github.com/gazebosim/gz-rendering/issues/544
        //
        // material may be a nullptr if we called setMaterial directly
        // (i.e. it's not using Ogre2Material interface).
        // In those cases we fallback to PBS in the current GORM mode.
        auto material = Ogre::MaterialManager::getSingleton().getByName(
          subItem->getMaterial()->getName() + "_solid",
          Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
        if (material)
        {
          if (material->getLoadingState() ==
              Ogre::Resource::LOADSTATE_UNLOADED)
          {
            // Manually defined materials like PointCloudPoint_solid need this
            material->load();
          }

          if (material->getNumSupportedTechniques() > 0u)
          {
            subItem->setMaterial(material);
          }
        }
        else
        {
          // The supplied vertex shader could not pair with the
          // pixel shader we provide. Try to salvage the situation
          // using PBS shader. Custom deformation won't work but
          // if we're lucky that won't matter
          subItem->setDatablock(defaultPbs);
        }
      }
      else
      {
        Ogre::HlmsDatablock *datablock = subItem->getDatablock();
        const Ogre::HlmsBlendblock *blendblock = datablock->getBlendblock();

        // We can't do any sort of blending. This isn't colour what we're
        // storing, but rather an ID.
        if (blendblock->mSourceBlendFactor != Ogre::SBF_ONE ||
            blendblock->mDestBlendFactor != Ogre::SBF_ZERO ||
            blendblock->mBlendOperation != Ogre::SBO_ADD ||
            (blendblock->mSeparateBlend &&
             (blendblock->mSourceBlendFactorAlpha != Ogre::SBF_ONE ||
              blendblock->mDestBlendFactorAlpha != Ogre::SBF_ZERO ||
              blendblock->mBlendOperationAlpha != Ogre::SBO_ADD)))
        {
          hlmsManager->addReference(blendblock);
          this->datablockMap[datablock] = blendblock;
          datablock->setBlendblock(noBlend);
        }
      }
    }
  }

  // Do the same with heightmaps / terrain
  for (const auto &entry : this->heightmapGroups)
  {
    // TODO(anyone): Retrieve datablock and make sure it's not blending
    // like we do with Items (it should be impossible?)
    Ogre::Vector4 customParameter;
    if (entry.group == kBackgroundGroup)
      customParameter = this->backgroundColor;
    else if (panoptic)
      customParameter = this->groups[entry.group].color;
    else
      customParameter = this->SemanticColor(entry.label);
    entry.heightmap->Terra()->SetSolidColor(1u, customParameter);
  }
  // at most one color key per item
  this->keysInUse = this->items.size() + this->heightmapGroups.size();

  // Remove the reference count on noBlend we created
  hlmsManager->destroyBlendblock(noBlend);

  // Drop cache entries of items that are gone, once there are many of them
  ++this->frame;
  if (this->visualCache.size() > 2 * this->items.size() + 1024 &&
      this->frame % 64 == 0)
  {
    for (auto it = this->visualCache.begin(); it != this->visualCache.end();)
    {
      if (it->second.visual.expired() ||
          this->frame - it->second.lastSeen > 1000)
      {
        it = this->visualCache.erase(it);
      }
      else
      {
        ++it;
      }
    }
  }

}

////////////////////////////////////////////////
void Ogre2SegmentationMaterialSwitcher::cameraPostRenderScene(
    Ogre::Camera * /*_cam*/)
{
  GZ_PROFILE("Ogre2SegmentationMaterialSwitcher::cameraPostRenderScene");
  auto engine = Ogre2RenderEngine::Instance();
  Ogre::HlmsManager *hlmsManager = engine->OgreRoot()->getHlmsManager();

  // Restore original blending to modified materials
  for (const auto &[datablock, blendblock] : this->datablockMap)
  {
    datablock->setBlendblock(blendblock);
    // Remove the reference we added (this won't actually destroy it)
    hlmsManager->destroyBlendblock(blendblock);
  }
  this->datablockMap.clear();

  // Remove the custom parameter. Why? If there are multiple cameras that
  // use GORM_SOLID_COLOR (or any other mode), we want them to throw if
  // that code forgot to call setCustomParameter. We may miss those errors
  // if that code forgets to call but it was already carrying the value
  // we set here.
  //
  // Only the sub items colored this frame have it.
  for (Ogre::SubItem *subItem : this->coloredSubItems)
    subItem->removeCustomParameter(1u);
  this->coloredSubItems.clear();

  // Restore Items with low level materials
  for (auto &subItemMat : this->materialMap)
  {
    subItemMat.first->setMaterial(subItemMat.second);
  }
  this->materialMap.clear();

  // Remove the custom parameter (same reason as with Items)
  auto heightmaps = this->scene->Heightmaps();
  for (auto h : heightmaps)
  {
    auto heightmap = h.lock();
    if (heightmap)
      heightmap->Terra()->UnsetSolidColors();
  }

  engine->SetGzOgreRenderingMode(GORM_NORMAL);
}

////////////////////////////////////////////////
const std::unordered_map<int64_t, int64_t> &
Ogre2SegmentationMaterialSwitcher::ColorToLabel() const
{
  return this->colorToLabel;
}
