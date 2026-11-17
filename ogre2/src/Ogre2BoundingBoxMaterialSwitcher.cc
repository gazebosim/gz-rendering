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

#include <array>
#include <cmath>
#include <variant>

#include "Ogre2BoundingBoxMaterialSwitcher.hh"

#include <gz/common/Profiler.hh>

#include "gz/rendering/ogre2/Ogre2Heightmap.hh"
#include "gz/rendering/ogre2/Ogre2RenderEngine.hh"
#include "gz/rendering/ogre2/Ogre2Scene.hh"
#include "gz/rendering/ogre2/Ogre2Visual.hh"

#include "Terra/Terra.h"

#ifdef _MSC_VER
  #pragma warning(push, 0)
#endif
#include <OgreCamera.h>
#include <OgreHlms.h>
#include <OgreHlmsManager.h>
#include <OgreRoot.h>
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

/////////////////////////////////////////////////
Ogre2BoundingBoxMaterialSwitcher::Ogre2BoundingBoxMaterialSwitcher(
    Ogre2ScenePtr _scene)
{
  this->scene = _scene;

  // plain material to switch item's material
  Ogre::ResourcePtr res =
    Ogre::MaterialManager::getSingleton().load("gz-rendering/plain_color",
        Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

  this->plainMaterial = res.staticCast<Ogre::Material>();
  this->plainMaterial->load();

  // plain overlay material
  this->plainOverlayMaterial =
      this->plainMaterial->clone("plain_color_overlay");
  if (!this->plainOverlayMaterial->getTechnique(0) ||
      !this->plainOverlayMaterial->getTechnique(0)->getPass(0))
  {
    gzerr << "Problem creating bounding box camera overlay material"
        << std::endl;
    return;
  }
  Ogre::Pass *overlayPass =
      this->plainOverlayMaterial->getTechnique(0)->getPass(0);
  Ogre::HlmsMacroblock macroblock(*overlayPass->getMacroblock());
  macroblock.mDepthCheck = false;
  macroblock.mDepthWrite = false;
  overlayPass->setMacroblock(macroblock);

  // index 0 is the background
  this->entries.resize(1);
}

/////////////////////////////////////////////////
Ogre2BoundingBoxMaterialSwitcher::~Ogre2BoundingBoxMaterialSwitcher()
{
}

/////////////////////////////////////////////////
const std::vector<Ogre2BoundingBoxMaterialSwitcher::Entry> &
    Ogre2BoundingBoxMaterialSwitcher::Entries() const
{
  return this->entries;
}

/////////////////////////////////////////////////
VisualPtr Ogre2BoundingBoxMaterialSwitcher::ItemVisual(Ogre::Item *_item)
{
  auto &cached = this->visualCache[_item->getId()];
  cached.lastSeen = this->frame;
  if (VisualPtr visual = cached.visual.lock())
    return visual;

  // first time this item is seen (or its visual is gone): slow lookup
  Ogre::Any userAny = _item->getUserObjectBindings().getUserAny();
  if (userAny.isEmpty() || userAny.getType() != typeid(unsigned int))
    return nullptr;

  VisualPtr visual;
  try
  {
    visual = this->scene->VisualById(Ogre::any_cast<unsigned int>(userAny));
  }
  catch(Ogre::Exception &e)
  {
    gzerr << "Ogre Error:" << e.getFullDescription() << "\n";
  }
  cached.visual = visual;
  return visual;
}

/////////////////////////////////////////////////
bool Ogre2BoundingBoxMaterialSwitcher::InView(const Ogre::Item *_item,
    const Ogre::Plane *_planes, size_t _planeCount)
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

  // An item Ogre renders but whose material wasn't switched would show
  // up in the image with a meaningless color, so only skip items that are
  // clearly out of view: grow the box before testing it.
  const Ogre::Vector3 grown = halfSize * 1.01f + Ogre::Vector3(1e-3f);
  // Same test as Ogre::Frustum::isVisible (planes point inwards), without
  // rebuilding the frustum and the box for every item.
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

////////////////////////////////////////////////
void Ogre2BoundingBoxMaterialSwitcher::cameraPreRenderScene(
    Ogre::Camera *_cam)
{
  GZ_PROFILE("Ogre2BoundingBoxMaterialSwitcher::cameraPreRenderScene");
  ++this->frame;
  this->datablocks.clear();
  this->coloredSubItems.clear();
  this->entries.resize(1);

  auto engine = Ogre2RenderEngine::Instance();
  engine->SetGzOgreRenderingMode(GORM_SOLID_COLOR);
  Ogre::HlmsManager *hlmsManager = engine->OgreRoot()->getHlmsManager();
  const Ogre::HlmsBlendblock *noBlend = nullptr;

  const unsigned int rootId = this->scene->RootVisual()->Id();

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

  size_t itemCount = 0;
  auto itor = this->scene->OgreSceneManager()->getMovableObjectIterator(
      Ogre::ItemFactory::FACTORY_TYPE_NAME);
  while (itor.hasMoreElements())
  {
    Ogre::Item *item = static_cast<Ogre::Item *>(itor.getNext());
    ++itemCount;

    // Items the camera won't render keep their material: switching and
    // restoring it is most of the cost in large scenes.
    if (!InView(item, planes.data(), planeCount))
      continue;

    // Index 0 (background) unless the item has a valid label
    uint32_t index = 0;
    VisualPtr visual = this->ItemVisual(item);
    if (visual)
    {
      Variant labelAny = visual->UserData(this->labelKey);
      const int *label = std::get_if<int>(&labelAny);
      if (label && *label >= 0 && *label < kLabelLimit)
      {
        // Multi-link models: find the top level visual. Walk up the
        // parents by id, cheaper than casting to Visual at every step.
        NodePtr model = visual;
        for (NodePtr parent = model->Parent();
             parent && parent->Id() != rootId; parent = parent->Parent())
        {
          model = parent;
        }

        index = static_cast<uint32_t>(this->entries.size());
        if (index > 0xFFFFFFu)
        {
          gzerr << "Too many labeled items for the bounding box camera; "
                << "ignoring the rest." << std::endl;
          index = 0;
        }
        else
        {
          this->entries.push_back(
              {item, static_cast<uint32_t>(*label), model->Id(),
               model.get()});
        }
      }
    }

    // Material color: the index in 24 bits
    const Ogre::Vector4 color(
        static_cast<float>(index & 0xFFu) / 255.0f,
        static_cast<float>((index >> 8) & 0xFFu) / 255.0f,
        static_cast<float>((index >> 16) & 0xFFu) / 255.0f,
        1.0f);

    // The flat color shaders (GORM_SOLID_COLOR) draw custom parameter 1.
    for (unsigned int i = 0; i < item->getNumSubItems(); i++)
    {
      Ogre::SubItem *subItem = item->getSubItem(i);
      subItem->setCustomParameter(1, color);
      this->coloredSubItems.push_back(subItem);

      Ogre::HlmsDatablock *datablock = subItem->getDatablock();
      if (!subItem->getMaterial().isNull())
      {
        // Low level material, with its own shaders: switch it to a flat
        // color material, and restore it after rendering.
        this->datablocks.emplace_back(subItem, datablock);
        if (!datablock->getMacroblock()->mDepthWrite &&
            !datablock->getMacroblock()->mDepthCheck)
          subItem->setMaterial(this->plainOverlayMaterial);
        else
          subItem->setMaterial(this->plainMaterial);
      }
      else if (!this->blendblocks.count(datablock))
      {
        // Hlms material: keeps its vertex shader (skinning, etc.) and
        // depth settings. The color is an index, so it can't be blended.
        const Ogre::HlmsBlendblock *blendblock = datablock->getBlendblock();
        if (blendblock->mSourceBlendFactor != Ogre::SBF_ONE ||
            blendblock->mDestBlendFactor != Ogre::SBF_ZERO ||
            blendblock->mBlendOperation != Ogre::SBO_ADD ||
            (blendblock->mSeparateBlend &&
             (blendblock->mSourceBlendFactorAlpha != Ogre::SBF_ONE ||
              blendblock->mDestBlendFactorAlpha != Ogre::SBF_ZERO ||
              blendblock->mBlendOperationAlpha != Ogre::SBO_ADD)))
        {
          if (!noBlend)
            noBlend = hlmsManager->getBlendblock(Ogre::HlmsBlendblock());
          hlmsManager->addReference(blendblock);
          this->blendblocks[datablock] = blendblock;
          datablock->setBlendblock(noBlend);
        }
      }
    }
  }

  // Heightmaps are drawn as background (index 0): they hide what's behind
  // them but have no box.
  for (const auto &weak : this->scene->Heightmaps())
  {
    if (auto heightmap = weak.lock())
      heightmap->Terra()->SetSolidColor(1u, Ogre::Vector4(0, 0, 0, 1));
  }

  if (noBlend)
    hlmsManager->destroyBlendblock(noBlend);

  // Drop cache entries of items that are gone, once there are many of
  // them. Entries of items out of view are kept for a while, so panning the
  // camera doesn't redo the slow lookups.
  if (this->visualCache.size() > 2 * itemCount + 1024 &&
      this->frame % 64 == 0)
  {
    for (auto it = this->visualCache.begin(); it != this->visualCache.end();)
    {
      if (it->second.visual.expired() ||
          this->frame - it->second.lastSeen > 1000)
        it = this->visualCache.erase(it);
      else
        ++it;
    }
  }
}

/////////////////////////////////////////////////
void Ogre2BoundingBoxMaterialSwitcher::cameraPostRenderScene(
    Ogre::Camera * /*_cam*/)
{
  GZ_PROFILE("Ogre2BoundingBoxMaterialSwitcher::cameraPostRenderScene");
  auto engine = Ogre2RenderEngine::Instance();
  Ogre::HlmsManager *hlmsManager = engine->OgreRoot()->getHlmsManager();

  // restore blending
  for (const auto &[datablock, blendblock] : this->blendblocks)
  {
    datablock->setBlendblock(blendblock);
    // remove the reference we added (this won't destroy it)
    hlmsManager->destroyBlendblock(blendblock);
  }
  this->blendblocks.clear();

  // Remove the custom parameter, so another camera using GORM_SOLID_COLOR
  // that forgets to set it fails loudly instead of drawing our colors.
  for (Ogre::SubItem *subItem : this->coloredSubItems)
    subItem->removeCustomParameter(1u);
  this->coloredSubItems.clear();

  // restore low level materials
  for (auto &[subItem, datablock] : this->datablocks)
    subItem->setDatablock(datablock);
  this->datablocks.clear();

  for (const auto &weak : this->scene->Heightmaps())
  {
    if (auto heightmap = weak.lock())
      heightmap->Terra()->UnsetSolidColors();
  }

  engine->SetGzOgreRenderingMode(GORM_NORMAL);
}
