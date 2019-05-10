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

#include <memory>
#include <ignition/math/Color.hh>

#include "ignition/common/Console.hh"
#include "ignition/rendering/RenderTypes.hh"
#include "ignition/rendering/ogre2/Ogre2Includes.hh"
#include "ignition/rendering/ogre2/Ogre2MaterialSwitcher.hh"
#include "ignition/rendering/ogre2/Ogre2RenderEngine.hh"
#include "ignition/rendering/ogre2/Ogre2RenderTarget.hh"
#include "ignition/rendering/ogre2/Ogre2Scene.hh"
#include "ignition/rendering/ogre2/Ogre2SelectionBuffer.hh"

using namespace ignition;
using namespace rendering;

class ignition::rendering::Ogre2SelectionBufferPrivate
{
  /// \brief This is a material listener and a RenderTargetListener.
  /// The material switcher is applied to only the selection camera
  /// and not applied globally to all targets. The class associates a
  /// color to an ogre entity
  public: std::unique_ptr<Ogre2MaterialSwitcher> materialSwitcher;

  /// \brief Ogre scene manager
  public: Ogre2ScenePtr scene;

  /// \brief Ogre scene manager
  public: Ogre::SceneManager *sceneMgr = nullptr;

  /// \brief Pointer to the camera that will be used as the reference
  /// for selection
  public: Ogre::Camera *camera = nullptr;

  /// \brief Selection buffer's render to texture camera
  public: Ogre::Camera *selectionCamera  = nullptr;

  /// \brief Ogre texture
  public: Ogre::TexturePtr texture;

  /// \brief Ogre render texture
  // public: Ogre2RenderTexturePtr renderTexture;
  public: Ogre::RenderTexture *renderTexture = nullptr;

  /// \brief Ogre's compositor workspace - the main interface to render
  /// into a render target or render texture.
  public: Ogre::CompositorWorkspace *ogreCompositorWorkspace = nullptr;

  /// \brief Render texture data buffer
  public: uint8_t *buffer = nullptr;

  /// \brief Ogre pixel box that contains description of the data buffer
  public: Ogre::PixelBox *pixelBox = nullptr;
};

/////////////////////////////////////////////////
Ogre2SelectionBuffer::Ogre2SelectionBuffer(const std::string &_cameraName,
    Ogre2ScenePtr _scene): dataPtr(new Ogre2SelectionBufferPrivate)
{
  this->dataPtr->scene = _scene;
  this->dataPtr->sceneMgr = _scene->OgreSceneManager();

  this->dataPtr->camera = this->dataPtr->sceneMgr->findCameraNoThrow(
      _cameraName);
  if (!this->dataPtr->camera)
  {
    ignerr << "No camera found. Unable to create Ogre 2 selection buffer "
           << std::endl;
    return;
  }

  this->dataPtr->selectionCamera =
      this->dataPtr->sceneMgr->createCamera(_cameraName + "_selection_buffer");

  this->dataPtr->materialSwitcher.reset(
      new Ogre2MaterialSwitcher(this->dataPtr->scene));
  this->CreateRTTBuffer();
}

/////////////////////////////////////////////////
Ogre2SelectionBuffer::~Ogre2SelectionBuffer()
{
  this->DeleteRTTBuffer();

  // remove selection buffer camera
  this->dataPtr->sceneMgr->destroyCamera(this->dataPtr->selectionCamera);
}

/////////////////////////////////////////////////
void Ogre2SelectionBuffer::Update()
{
  if (!this->dataPtr->renderTexture)
    return;

  this->dataPtr->materialSwitcher->Reset();

  // manual update
  this->dataPtr->ogreCompositorWorkspace->setEnabled(true);
  auto engine = Ogre2RenderEngine::Instance();
  engine->OgreRoot()->renderOneFrame();
  this->dataPtr->ogreCompositorWorkspace->setEnabled(false);

  this->dataPtr->renderTexture->copyContentsToMemory(*this->dataPtr->pixelBox,
      Ogre::RenderTarget::FB_FRONT);
}

/////////////////////////////////////////////////
void Ogre2SelectionBuffer::DeleteRTTBuffer()
{
  auto &manager = Ogre::TextureManager::getSingleton();
  manager.unload(this->dataPtr->texture->getName());
  manager.remove(this->dataPtr->texture->getName());

  if (this->dataPtr->buffer)
  {
    delete [] this->dataPtr->buffer;
    this->dataPtr->buffer = nullptr;
  }
  if (this->dataPtr->pixelBox)
    delete this->dataPtr->pixelBox;
}

/////////////////////////////////////////////////
void Ogre2SelectionBuffer::CreateRTTBuffer()
{
  // create a 1x1 pixel buffer
  unsigned int width = 1;
  unsigned int height = 1;
  Ogre::PixelFormat format = Ogre::PF_R8G8B8;
  this->dataPtr->texture = Ogre::TextureManager::getSingleton().createManual(
        "SelectionPassTex",
        Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
        Ogre::TEX_TYPE_2D, width, height, 0, Ogre::PF_R8G8B8,
        Ogre::TU_RENDERTARGET);

  this->dataPtr->renderTexture =
      this->dataPtr->texture->getBuffer()->getRenderTarget();
  this->dataPtr->renderTexture->addListener(
      this->dataPtr->materialSwitcher.get());

  // create compositor workspace for rendering
  auto engine = Ogre2RenderEngine::Instance();
  auto ogreRoot = engine->OgreRoot();
  Ogre::CompositorManager2 *ogreCompMgr = ogreRoot->getCompositorManager2();

  const Ogre::String workspaceName = "SelectionBufferWorkspace" +
      this->dataPtr->camera->getName();
  ogreCompMgr->createBasicWorkspaceDef(workspaceName,
      Ogre::ColourValue(0.0f, 0.0f, 0.0f, 1.0f));
  this->dataPtr->ogreCompositorWorkspace =
      ogreCompMgr->addWorkspace(this->dataPtr->scene->OgreSceneManager(),
      this->dataPtr->renderTexture,
      this->dataPtr->selectionCamera, workspaceName, false);

  // set visibility mask to see only items that are selectable
  auto nodeSeq = this->dataPtr->ogreCompositorWorkspace->getNodeSequence();
  auto pass = nodeSeq[0]->_getPasses()[1]->getDefinition();
  auto scenePass = dynamic_cast<const Ogre::CompositorPassSceneDef *>(pass);
  const_cast<Ogre::CompositorPassSceneDef *>(scenePass)->mVisibilityMask =
      IGN_VISIBILITY_SELECTABLE;

  // buffer to store render texture data
  size_t bufferSize = Ogre::PixelUtil::getMemorySize(width, height, 1, format);
  this->dataPtr->buffer = new uint8_t[bufferSize];
  this->dataPtr->pixelBox = new Ogre::PixelBox(width,
      height, 1, format, this->dataPtr->buffer);
}

/////////////////////////////////////////////////
Ogre::Item *Ogre2SelectionBuffer::OnSelectionClick(const int _x, const int _y)
{
  if (!this->dataPtr->renderTexture)
    return nullptr;

  Ogre::RenderTarget *rt =
      this->dataPtr->camera->getLastViewport()->getTarget();
  const unsigned int targetWidth = rt->getWidth();
  const unsigned int targetHeight = rt->getHeight();

  if (_x < 0 || _y < 0 || _x >= static_cast<int>(targetWidth)
      || _y >= static_cast<int>(targetHeight))
    return nullptr;

  // 1x1 selection buffer, adapted from rviz
  // http://docs.ros.org/indigo/api/rviz/html/c++/selection__manager_8cpp.html
  unsigned int width = 1;
  unsigned int height = 1;
  float x1 = static_cast<float>(_x) /
      static_cast<float>(targetWidth - 1) - 0.5f;
  float y1 = static_cast<float>(_y) /
      static_cast<float>(targetHeight - 1) - 0.5f;
  float x2 = static_cast<float>(_x+width) /
      static_cast<float>(targetWidth - 1) - 0.5f;
  float y2 = static_cast<float>(_y+height) /
      static_cast<float>(targetHeight - 1) - 0.5f;
  Ogre::Matrix4 scaleMatrix = Ogre::Matrix4::IDENTITY;
  Ogre::Matrix4 transMatrix = Ogre::Matrix4::IDENTITY;
  scaleMatrix[0][0] = 1.0 / (x2-x1);
  scaleMatrix[1][1] = 1.0 / (y2-y1);
  transMatrix[0][3] -= x1+x2;
  transMatrix[1][3] += y1+y2;
  this->dataPtr->selectionCamera->setCustomProjectionMatrix(true,
      scaleMatrix * transMatrix * this->dataPtr->camera->getProjectionMatrix());
  this->dataPtr->selectionCamera->setPosition(
      this->dataPtr->camera->getDerivedPosition());
  this->dataPtr->selectionCamera->setOrientation(
      this->dataPtr->camera->getDerivedOrientation());
  Ogre::Viewport* renderViewport =
      this->dataPtr->renderTexture->getViewport(0);
  renderViewport->setDimensions(0, 0, width, height);

  // update render texture
  this->Update();

  size_t posInStream = 0;
  ignition::math::Color::BGRA color(0);
  if (!this->dataPtr->buffer)
  {
    ignerr << "Selection buffer is null." << std::endl;
    return nullptr;
  }
  memcpy(static_cast<void *>(&color), this->dataPtr->buffer + posInStream, 4);
  ignition::math::Color cv;
  cv.SetFromARGB(color);
  cv.A(1.0);
  const std::string &entName =
    this->dataPtr->materialSwitcher->EntityName(cv);

  if (entName.empty())
  {
    return 0;
  }
  else
  {
    auto collection = this->dataPtr->sceneMgr->findMovableObjects(
        Ogre::ItemFactory::FACTORY_TYPE_NAME, entName);
    if (collection.empty())
      return nullptr;
    else
      return dynamic_cast<Ogre::Item *>(collection[0]);
  }
}
