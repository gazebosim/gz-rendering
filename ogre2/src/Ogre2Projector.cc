/*
 * Copyright (C) 2023 Open Source Robotics Foundation
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

#include <string>

#include <OgreDecal.h>
#include <OgreRoot.h>
#include <OgreSceneManager.h>
#include <OgreSceneNode.h>
#include <OgreTextureGpuManager.h>

// TODO remove
#include <OgreWireAabb.h>

#include "gz/rendering/ogre2/Ogre2Projector.hh"
#include "gz/rendering/ogre2/Ogre2RenderEngine.hh"
#include "gz/rendering/ogre2/Ogre2Scene.hh"
#include "gz/rendering/Utils.hh"

using namespace gz;
using namespace rendering;

/// \brief Private data for the Ogre2Projector class
class gz::rendering::Ogre2Projector::Implementation
{
  /// \brief The decal ogre scene node
  public: Ogre::SceneNode *decalNode{nullptr};

  /// \brief Decal diffuse texture
  public: Ogre::TextureGpu *textureDiff{nullptr};

  /// \brief Decal - Texture projected onto a surface
  public: Ogre::Decal *decal{nullptr};

  /// \brief Indicate whether the projector is intialized or not
  public: bool initialized{false};
};

/////////////////////////////////////////////////
Ogre2Projector::Ogre2Projector()
    : dataPtr(utils::MakeUniqueImpl<Implementation>())
{
}

/////////////////////////////////////////////////
Ogre2Projector::~Ogre2Projector()
{
  this->SetEnabled(false);

  if (!this->scene->IsInitialized())
    return;

  if (this->dataPtr->textureDiff)
  {
    auto engine = Ogre2RenderEngine::Instance();
    auto ogreRoot = engine->OgreRoot();
    auto textureGpuManager = ogreRoot->getRenderSystem()->getTextureGpuManager();
    textureGpuManager->destroyTexture(this->dataPtr->textureDiff);
    this->dataPtr->textureDiff = nullptr;
  }
  if (this->dataPtr->decal)
  {
    this->scene->OgreSceneManager()->destroyDecal(this->dataPtr->decal);
    this->dataPtr->decal = nullptr;
  }
}

/////////////////////////////////////////////////
void Ogre2Projector::PreRender()
{
  if (!this->dataPtr->initialized)
  {
    this->CreateProjector();
    this->dataPtr->initialized = true;
    this->SetEnabled(true);
  }
  // return;

  if (this->dataPtr->textureDiff)
  {
    this->scene->OgreSceneManager()->setDecalsDiffuse(
        this->dataPtr->textureDiff);
  }
}

/////////////////////////////////////////////////
void Ogre2Projector::CreateProjector()
{
  this->dataPtr->decalNode = this->ogreNode->createChildSceneNode();
  this->dataPtr->decalNode->roll(Ogre::Degree(-90));

  this->dataPtr->decal = this->scene->OgreSceneManager()->createDecal();
  this->dataPtr->decalNode->attachObject(this->dataPtr->decal);

  if (common::isFile(this->textureName))
  {
    std::string baseName = common::basename(this->textureName);
    size_t idx = this->textureName.rfind(baseName);
    if (idx != std::string::npos)
    {
      std::string dirPath = this->textureName.substr(0, idx);
      if (!dirPath.empty() &&
          !Ogre::ResourceGroupManager::getSingleton().resourceLocationExists(
          dirPath))
      {
        Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
            dirPath, "FileSystem", "General");
      }
    }
  }
  else
  {
    gzerr << "Unable to create projector. Projector texture not found: "
          << this->textureName << std::endl;
    return;
  }

///////////
//  return;



  auto engine = Ogre2RenderEngine::Instance();
  auto ogreRoot = engine->OgreRoot();
  Ogre::TextureGpuManager *textureManager =
    ogreRoot->getRenderSystem()->getTextureGpuManager();
  this->dataPtr->textureDiff = textureManager->createOrRetrieveTexture(
      this->textureName, this->textureName + "_alias",
       Ogre::GpuPageOutStrategy::Discard,
      Ogre::CommonTextureTypes::Diffuse,
      Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME, 1 /*,
      decalDiffuseId */ );
  this->dataPtr->textureDiff->scheduleTransitionTo(Ogre::GpuResidency::Resident);

  this->dataPtr->decal->setDiffuseTexture(this->dataPtr->textureDiff);

  common::Image image(this->textureName);
  const double aspectRatio = image.Width() / image.Height();
  const double vfov = 2.0 * atan(tan(this->hfov.Radian() / 2.0)
      / aspectRatio);

  const double depth = this->farClip - this->nearClip;
  const double width = 2 * (tan(this->hfov.Radian() / 2) * this->farClip);
  const double height = 2 * (tan(vfov / 2) * this->farClip);

  // this->dataPtr->decalNode->setScale(width, depth, height);
  this->dataPtr->decalNode->setScale(Ogre::Vector3(11.0f));

  // TODO remove
   // Ogre::WireAabb *wireAabb = this->scene->OgreSceneManager()->createWireAabb();
   // wireAabb->track(this->dataPtr->decal);

  //For the SceneManager, any of the textures belonging to the same pool will do!
  // sceneManager->setDecalsDiffuse(textureDiff);
}

/////////////////////////////////////////////////
void Ogre2Projector::SetEnabled(bool _enabled)
{
  BaseProjector::SetEnabled(_enabled);
//  this->dataPtr->projector.SetEnabled(_enabled);
}
