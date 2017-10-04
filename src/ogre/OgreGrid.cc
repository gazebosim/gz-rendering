/*
 * Copyright (C) 2012 Open Source Robotics Foundation
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

#include <sstream>

#include "ignition/rendering/Scene.hh"
#include "ignition/rendering/OgreGrid.hh"

using namespace ignition;
using namespace rendering;


//////////////////////////////////////////////////
OgreGrid::OgreGrid()
{
}

//////////////////////////////////////////////////
OgreGrid::~OgreGrid()
{
  this->scene->OgreSceneManager()->destroySceneNode(this->sceneNode->getName());
  this->scene->OgreSceneManager()->destroyManualObject(this->manualObject);
  this->material->unload();
}

//////////////////////////////////////////////////
void OgreGrid::UpdateColor()
{
  this->dataPtr->material->setDiffuse(_color.r, _color.g, _color.b, _color.a);
  this->dataPtr->material->setAmbient(_color.r, _color.g, _color.b);

  if ((this->color).a < 0.9998)
    this->dataPtr->material->setSceneBlending(Ogre::SBT_TRANSPARENT_ALPHA);
  else
    this->dataPtr->material->setSceneBlending(Ogre::SBT_REPLACE);

  this->dataPtr->material->setDepthWriteEnabled(false);
  this->dataPtr->material->setDepthCheckEnabled(true);
}

//////////////////////////////////////////////////
void OgreGrid::Init()
{
  this->dataPtr->anualObject =
    this->dataPtr->scene->OgreSceneManager()->createManualObject(this->name);

  this->dataPtr->manualObject->setDynamic(false);

  Ogre::SceneNode *parent_node =
      this->scene->OgreSceneManager()->getRootSceneNode();

  this->sceneNode = parent_node->createChildSceneNode(this->name);
  this->sceneNode->attachObject(this->manualObject);

  std::stringstream ss;
  ss << this->name << "Material";
  this->material =
    Ogre::MaterialManager::getSingleton().create(ss.str(),
        Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
  this->material->setReceiveShadows(false);
  this->material->getTechnique(0)->setLightingEnabled(false);

  this->SetColor(this->color);

  this->Create();
}

//////////////////////////////////////////////////
void OgreGrid::Create()
{
  this->manualObject->clear();

  float extent = (this->cellLength * static_cast<double>(this->cellCount))/2;

  this->manualObject->setCastShadows(false);
  this->manualObject->estimateVertexCount(
      this->cellCount * 4 * this->height +
      ((this->cellCount + 1) * (this->cellCount + 1)));

  this->manualObject->begin(this->material->getName(),
      Ogre::RenderOperation::OT_LINE_LIST);

  for (uint32_t h = 0; h <= this->height; ++h)
  {
    float h_real = this->heightOffset +
      (this->height / 2.0f - static_cast<float>(h)) * this->cellLength;
    for (uint32_t i = 0; i <= this->cellCount; i++)
    {
      float inc = extent - (i * this->cellLength);

      Ogre::Vector3 p1(inc, -extent, h_real);
      Ogre::Vector3 p2(inc, extent , h_real);
      Ogre::Vector3 p3(-extent, inc, h_real);
      Ogre::Vector3 p4(extent, inc, h_real);

      this->manualObject->position(p1);
      this->manualObject->colour(Conversions::Convert(this->color));
      this->manualObject->position(p2);
      this->manualObject->colour(Conversions::Convert(this->color));

      this->manualObject->position(p3);
      this->manualObject->colour(Conversions::Convert(this->color));
      this->manualObject->position(p4);
      this->manualObject->colour(Conversions::Convert(this->color));
    }
  }

  if (this->height > 0)
  {
    for (uint32_t x = 0; x <= this->cellCount; ++x)
    {
      for (uint32_t y = 0; y <= this->cellCount; ++y)
      {
        float x_real = extent - x * this->cellLength;
        float y_real = extent - y * this->cellLength;

        float z_top = (this->height / 2.0f) * this->cellLength;
        float z_bottom = -z_top;

        this->manualObject->position(x_real, y_real, z_bottom);
        this->manualObject->colour(Conversions::Convert(this->color));
        this->manualObject->position(x_real, y_real, z_bottom);
        this->manualObject->colour(Conversions::Convert(this->color));
      }
    }
  }

  this->manualObject->end();
}

//////////////////////////////////////////////////
void OgreGrid::SetUserData(const Ogre::Any &_data)
{
  this->manualObject->getUserObjectBindings().setUserAny(_data);
}

//////////////////////////////////////////////////
void OgreGrid::Enable(bool _enable)
{
  this->sceneNode->setVisible(_enable);
}
