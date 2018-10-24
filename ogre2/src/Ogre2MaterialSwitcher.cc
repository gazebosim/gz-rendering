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
#include "ignition/rendering/ogre2/Ogre2Includes.hh"
#include "ignition/rendering/ogre2/Ogre2MaterialSwitcher.hh"
#include "ignition/rendering/ogre2/Ogre2Scene.hh"
#include "ignition/rendering/RenderTypes.hh"

using namespace ignition;
using namespace rendering;


/////////////////////////////////////////////////
Ogre2MaterialSwitcher::Ogre2MaterialSwitcher(Ogre2ScenePtr _scene)
{
  this->currentColor = ignition::math::Color(0.0, 0.0, 0.1);
  this->scene = _scene;

  // plain opaque material
  Ogre::ResourcePtr res =
    Ogre::MaterialManager::getSingleton().load("ign-rendering/plain_color",
        Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

  this->plainMaterial = res.staticCast<Ogre::Material>();
  this->plainMaterial->load();
}

/////////////////////////////////////////////////
Ogre2MaterialSwitcher::~Ogre2MaterialSwitcher()
{
}

////////////////////////////////////////////////
void Ogre2MaterialSwitcher::preRenderTargetUpdate(
    const Ogre::RenderTargetEvent &/*_evt*/)
{
  // swap item to use v1 shader material
  // Note: keep an eye out for performance impact on switching materials
  // on the fly. We are not doing this often so should be ok.
  this->datablockMap.clear();
  auto itor = this->scene->OgreSceneManager()->getMovableObjectIterator(
      Ogre::ItemFactory::FACTORY_TYPE_NAME);
  while (itor.hasMoreElements())
  {
      Ogre::MovableObject *object = itor.peekNext();
      Ogre::Item *item = static_cast<Ogre::Item *>(object);

      this->colorDict[this->currentColor.AsRGBA()] = item->getName();

      for (unsigned int i = 0; i < item->getNumSubItems(); ++i)
      {
        Ogre::SubItem *subItem = item->getSubItem(i);
        this->datablockMap[subItem] = subItem->getDatablock();

        subItem->setCustomParameter(1,
            Ogre::Vector4(this->currentColor.R(), this->currentColor.G(),
                          this->currentColor.B(), 1.0));

        subItem->setMaterial(this->plainMaterial);
      }

      this->NextColor();
      itor.moveNext();
  }
}

/////////////////////////////////////////////////
void Ogre2MaterialSwitcher::postRenderTargetUpdate(
    const Ogre::RenderTargetEvent &/*_evt*/)
{
  // restore item to use hlms material
  auto itor = this->scene->OgreSceneManager()->getMovableObjectIterator(
      Ogre::ItemFactory::FACTORY_TYPE_NAME);
  while (itor.hasMoreElements())
  {
      Ogre::MovableObject *object = itor.peekNext();
      Ogre::Item *item = static_cast<Ogre::Item *>(object);
      for (unsigned int i = 0; i < item->getNumSubItems(); ++i)
      {
        Ogre::SubItem *subItem = item->getSubItem(i);
        subItem->setDatablock(this->datablockMap[subItem]);
      }
      itor.moveNext();
  }
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
  this->currentColor = ignition::math::Color(0.0, 0.0, 0.1);
  this->colorDict.clear();
}
