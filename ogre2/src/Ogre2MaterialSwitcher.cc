/*
 * Copyright (C) 2018 Open Source Robotics Foundation
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

#include "ignition/common/Console.hh"

#include "ignition/rendering/ogre2/Ogre2MaterialSwitcher.hh"
#include "ignition/rendering/ogre2/Ogre2RenderEngine.hh"
#include "ignition/rendering/ogre2/Ogre2Scene.hh"
#include "ignition/rendering/RenderTypes.hh"

#ifdef _MSC_VER
  #pragma warning(push, 0)
#endif
#include <OgreItem.h>
#include <OgreMaterialManager.h>
#include <OgrePass.h>
#include <OgreRoot.h>
#include <OgreSceneManager.h>
#include <OgreTechnique.h>
#ifdef _MSC_VER
  #pragma warning(pop)
#endif

using namespace ignition;
using namespace rendering;


/// \brief A map of ogre sub item pointer to their original low level material
/// \todo(anyone) Added here for ABI compatibity This can be removed once
/// ign-rendering7 switches to Hlms customization for "switching" materials
std::map<Ogre2MaterialSwitcher *,
         std::map<Ogre::SubItem *, Ogre::MaterialPtr>> materialMap;

/////////////////////////////////////////////////
Ogre2MaterialSwitcher::Ogre2MaterialSwitcher(Ogre2ScenePtr _scene)
{
  this->currentColor = ignition::math::Color(0.0, 0.0, 0.1);
  this->scene = _scene;
}

/////////////////////////////////////////////////
Ogre2MaterialSwitcher::~Ogre2MaterialSwitcher()
{
}

////////////////////////////////////////////////
void Ogre2MaterialSwitcher::cameraPreRenderScene(
    Ogre::Camera * /*_evt*/)
{
  auto engine = Ogre2RenderEngine::Instance();
  engine->SetIgnOgreRenderingMode(IORM_SOLID_COLOR);

  this->datablockMap.clear();
  Ogre::HlmsManager *hlmsManager = engine->OgreRoot()->getHlmsManager();

  // Construct one now so that datablock->setBlendblock
  // each is as fast as possible
  const Ogre::HlmsBlendblock *noBlend =
    hlmsManager->getBlendblock(Ogre::HlmsBlendblock());

  auto itor = this->scene->OgreSceneManager()->getMovableObjectIterator(
      Ogre::ItemFactory::FACTORY_TYPE_NAME);
  while (itor.hasMoreElements())
  {
    this->NextColor();

    Ogre::MovableObject *object = itor.peekNext();
    Ogre::Item *item = static_cast<Ogre::Item *>(object);

    this->colorDict[this->currentColor.AsRGBA()] = item->getName();

    for (unsigned int i = 0; i < item->getNumSubItems(); ++i)
    {
      Ogre::SubItem *subItem = item->getSubItem(i);

      subItem->setCustomParameter(1,
          Ogre::Vector4(this->currentColor.R(), this->currentColor.G(),
                        this->currentColor.B(), 1.0));

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
    itor.moveNext();
  }

  // Remove the reference count on noBlend we created
  hlmsManager->destroyBlendblock(noBlend);
}

/////////////////////////////////////////////////
void Ogre2MaterialSwitcher::cameraPostRenderScene(
    Ogre::Camera * /*_evt*/)
{
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

  engine->SetIgnOgreRenderingMode(IORM_NORMAL);
}

/////////////////////////////////////////////////
std::string Ogre2MaterialSwitcher::EntityName(
    const ignition::math::Color &_color) const
{
  auto iter = this->colorDict.find(_color.AsRGBA());

  if (iter != this->colorDict.end())
    return (*iter).second;
  else
    return std::string();
}

/////////////////////////////////////////////////
void Ogre2MaterialSwitcher::NextColor()
{
  auto color = this->currentColor.AsARGB();
  color++;
  this->currentColor.SetFromARGB(color);
}

/////////////////////////////////////////////////
void Ogre2MaterialSwitcher::Reset()
{
  this->currentColor = ignition::math::Color(
      0.0, 0.0, 0.0);
  this->colorDict.clear();
}
