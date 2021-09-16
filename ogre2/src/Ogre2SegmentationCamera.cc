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


#include <string>

#include "ignition/common/Console.hh"
#include "ignition/math/Color.hh"
#include "ignition/rendering/ogre2/Ogre2Camera.hh"
#include "ignition/rendering/ogre2/Ogre2Conversions.hh"
#include "ignition/rendering/ogre2/Ogre2Includes.hh"
#include "ignition/rendering/ogre2/Ogre2RenderTarget.hh"
#include "ignition/rendering/ogre2/Ogre2RenderTypes.hh"
#include "ignition/rendering/ogre2/Ogre2RenderEngine.hh"
#include "ignition/rendering/ogre2/Ogre2Scene.hh"
#include "ignition/rendering/ogre2/Ogre2SegmentationCamera.hh"
#include "ignition/rendering/RenderTypes.hh"
#include "ignition/rendering/Utils.hh"

#include "Ogre2SegmentationMaterialSwitcher.hh"


/// \brief Private data for the Ogre2SegmentationCamera class
class ignition::rendering::Ogre2SegmentationCameraPrivate
{
  /// \brief Material Switcher to switch item's material
  /// with colored version for segmentation
  public: Ogre2SegmentationMaterialSwitcher *materialSwitcher {nullptr};

  /// \brief Compositor Manager to create workspace
  public: Ogre::CompositorManager2 *ogreCompositorManager {nullptr};

  /// \brief Workspace to interface with render texture
  public: Ogre::CompositorWorkspace *ogreCompositorWorkspace {nullptr};

  /// \brief Workspace Definition
  public: std::string workspaceDefinition;

  /// \brief Render Texture to store the final segmentation data
  public: Ogre::RenderTexture *ogreRenderTexture {nullptr};

  /// \brief Texture to create the render texture from.
  public: Ogre::TexturePtr ogreTexture;

  /// \brief Pixel Box to copy render texture data to a buffer
  public: Ogre::PixelBox *pixelBox {nullptr};

  /// \brief buffer to store render texture data & to be sent to listeners
  public: uint8_t *buffer = nullptr;

  /// \brief dummy render texture to set image dims
  public: Ogre2RenderTexturePtr dummyTexture {nullptr};

  /// \brief New Segmentation Frame Event to notify listeners with new data
  /// \param[in] _data Segmentation buffer data
  /// \param[in] _width Width of the image
  /// \param[in] _height Height of the image
  /// \param[in] _channels Number of channels
  /// \param[in] _format Image Format
  public: ignition::common::EventT<void(const uint8_t *_data,
    unsigned int _width, unsigned int _height, unsigned int _channels,
    const std::string &_format)> newSegmentationFrame;

  /// \brief Image / Render Texture Format
  public: const Ogre::PixelFormat format = Ogre::PF_R8G8B8;
};

using namespace ignition;
using namespace rendering;

/////////////////////////////////////////////////
Ogre2SegmentationCamera::Ogre2SegmentationCamera() :
  dataPtr(new Ogre2SegmentationCameraPrivate())
{
}

/////////////////////////////////////////////////
Ogre2SegmentationCamera::~Ogre2SegmentationCamera()
{
  this->Destroy();
}

/////////////////////////////////////////////////
void Ogre2SegmentationCamera::Init()
{
  BaseCamera::Init();
  this->CreateCamera();
  this->CreateRenderTexture();

  this->dataPtr->materialSwitcher =
    new Ogre2SegmentationMaterialSwitcher(this->scene);
}

/////////////////////////////////////////////////
void Ogre2SegmentationCamera::CreateCamera()
{
  auto ogreScene = this->scene->OgreSceneManager();
  if (ogreScene == nullptr)
  {
    ignerr << "Scene manager cannot be obtained" << std::endl;
    return;
  }

  this->ogreCamera = ogreScene->createCamera(this->Name());
  if (this->ogreCamera == nullptr)
  {
    ignerr << "Ogre camera cannot be created" << std::endl;
    return;
  }

  this->ogreCamera->detachFromParent();
  this->ogreNode->attachObject(this->ogreCamera);

  // rotate to ignition gazebo coord.
  this->ogreCamera->yaw(Ogre::Degree(-90));
  this->ogreCamera->roll(Ogre::Degree(-90));
  this->ogreCamera->setFixedYawAxis(false);

  this->ogreCamera->setAutoAspectRatio(true);
  this->ogreCamera->setRenderingDistance(100);
  this->ogreCamera->setProjectionType(Ogre::ProjectionType::PT_PERSPECTIVE);
  this->ogreCamera->setCustomProjectionMatrix(false);
}

/////////////////////////////////////////////////
void Ogre2SegmentationCamera::Destroy()
{
  if (this->dataPtr->buffer)
  {
    delete [] this->dataPtr->buffer;
    this->dataPtr->buffer = nullptr;
  }

  if (!this->ogreCamera)
    return;

  auto engine = Ogre2RenderEngine::Instance();
  auto ogreRoot = engine->OgreRoot();
  Ogre::CompositorManager2 *ogreCompMgr =
    ogreRoot->getCompositorManager2();

  // remove thermal texture, material, compositor
  if (this->dataPtr->ogreRenderTexture)
  {
    Ogre::TextureManager::getSingleton().remove(
        this->dataPtr->ogreRenderTexture->getName());
  }
  if (this->dataPtr->ogreCompositorWorkspace)
  {
    ogreCompMgr->removeWorkspace(
        this->dataPtr->ogreCompositorWorkspace);
  }

  if (!this->dataPtr->workspaceDefinition.empty())
  {
    ogreCompMgr->removeWorkspaceDefinition(
        this->dataPtr->workspaceDefinition);
  }

  Ogre::SceneManager *ogreSceneManager;
  ogreSceneManager = this->scene->OgreSceneManager();
  if (ogreSceneManager == nullptr)
  {
    ignerr << "Scene manager cannot be obtained" << std::endl;
  }
  else
  {
    if (ogreSceneManager->findCameraNoThrow(this->name) != nullptr)
    {
      ogreSceneManager->destroyCamera(this->ogreCamera);
      this->ogreCamera = nullptr;
    }
  }

  if (this->dataPtr->pixelBox)
    delete this->dataPtr->pixelBox;
  this->dataPtr->pixelBox = nullptr;

  if (this->dataPtr->materialSwitcher)
    delete this->dataPtr->materialSwitcher;
  this->dataPtr->materialSwitcher = nullptr;
}

/////////////////////////////////////////////////
void Ogre2SegmentationCamera::PreRender()
{
  if (!this->dataPtr->ogreRenderTexture)
    this->CreateSegmentationTexture();
}

/////////////////////////////////////////////////
void Ogre2SegmentationCamera::CreateSegmentationTexture()
{
  // Camera Parameters
  this->ogreCamera->setNearClipDistance(this->NearClipPlane());
  this->ogreCamera->setFarClipDistance(this->FarClipPlane());
  this->ogreCamera->setAspectRatio(this->AspectRatio());
  double vfov = 2.0 * atan(tan(this->HFOV().Radian() / 2.0) /
    this->AspectRatio());
  this->ogreCamera->setFOVy(Ogre::Radian(vfov));

  auto width = this->ImageWidth();
  auto height = this->ImageHeight();

  // texture
  this->dataPtr->ogreTexture =
    Ogre::TextureManager::getSingleton().createManual(
    "SegmentationCameraTexture",
    Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
    Ogre::TEX_TYPE_2D, width, height, 0, this->dataPtr->format,
    Ogre::TU_RENDERTARGET
  );

  // render texture
  auto hardwareBuffer = this->dataPtr->ogreTexture->getBuffer();
  this->dataPtr->ogreRenderTexture = hardwareBuffer->getRenderTarget();

  // switch the material to a unique color for each object
  // in the pre render & get the original material again in the post render
  this->dataPtr->ogreRenderTexture->addListener(
    this->dataPtr->materialSwitcher);

  // workspace
  auto engine = Ogre2RenderEngine::Instance();
  auto ogreRoot = engine->OgreRoot();
  this->dataPtr->ogreCompositorManager = ogreRoot->getCompositorManager2();

  this->dataPtr->workspaceDefinition = "SegmentationCameraWorkspace_" +
    this->Name();
  auto backgroundColor = Ogre2Conversions::Convert(
    this->dataPtr->materialSwitcher->backgroundColor);

  // basic workspace consist of clear pass with the given color &
  // a render scene pass to the givin render texture
  this->dataPtr->ogreCompositorManager->createBasicWorkspaceDef(
    this->dataPtr->workspaceDefinition, backgroundColor);

  // connect the compositor with the render texture to render the final output
  this->dataPtr->ogreCompositorWorkspace =
    this->dataPtr->ogreCompositorManager->addWorkspace(
      this->scene->OgreSceneManager(),
      this->dataPtr->ogreRenderTexture,
      this->ogreCamera,
      this->dataPtr->workspaceDefinition,
      false
    );

  // set visibility mask
  auto node = this->dataPtr->ogreCompositorWorkspace->getNodeSequence()[0];
  auto pass = node->_getPasses()[1]->getDefinition();
  auto renderScenePass =
    dynamic_cast<const Ogre::CompositorPassSceneDef *>(pass);
  const_cast<Ogre::CompositorPassSceneDef *>(
    renderScenePass)->setVisibilityMask(IGN_VISIBILITY_ALL);

  // buffer to store render texture data
  auto bufferSize = Ogre::PixelUtil::getMemorySize(
    width, height, 1, this->dataPtr->format);
  if (this->dataPtr->buffer)
    delete [] this->dataPtr->buffer;
  this->dataPtr->buffer = new uint8_t[bufferSize];
  if (this->dataPtr->pixelBox)
    delete this->dataPtr->pixelBox;
  this->dataPtr->pixelBox = new Ogre::PixelBox(width, height, 1,
    this->dataPtr->format, this->dataPtr->buffer);
}

/////////////////////////////////////////////////
void Ogre2SegmentationCamera::Render()
{
  this->dataPtr->ogreCompositorWorkspace->setEnabled(true);
  auto engine = Ogre2RenderEngine::Instance();
  auto ogreRoot = engine->OgreRoot();
  ogreRoot->renderOneFrame();
  this->dataPtr->ogreCompositorWorkspace->setEnabled(false);
}

/////////////////////////////////////////////////
void Ogre2SegmentationCamera::PostRender()
{
  // copy render texture data to the pixel box & its buffer
  this->dataPtr->ogreRenderTexture->copyContentsToMemory(
    *this->dataPtr->pixelBox,
    Ogre::RenderTarget::FB_FRONT
  );

  // return if no one is listening to the new frame
  if (this->dataPtr->newSegmentationFrame.ConnectionCount() == 0)
    return;

  uint width = this->ImageWidth();
  uint height = this->ImageHeight();
  uint channelCount = 3;

  this->dataPtr->newSegmentationFrame(
    this->dataPtr->buffer,
    width, height, channelCount,
    Ogre::PixelUtil::getFormatName(this->dataPtr->format)
  );
}

/////////////////////////////////////////////////
void Ogre2SegmentationCamera::Capture(Image &_image)
{
  unsigned int width = this->ImageWidth();
  unsigned int height = this->ImageHeight();

  if (_image.Width() != width || _image.Height() != height)
  {
    ignerr << "Invalid image dimensions" << std::endl;
    return;
  }

  // image buffer
  void *data = _image.Data();

  // pixel box to copy data from the render texture to the image buffer
  Ogre::PixelBox ogrePixelBox(width, height, 1, this->dataPtr->format, data);
  this->dataPtr->ogreRenderTexture->copyContentsToMemory(
    ogrePixelBox, Ogre::RenderTarget::FB_FRONT);
}

/////////////////////////////////////////////////
uint8_t *Ogre2SegmentationCamera::SegmentationData() const
{
  return this->dataPtr->buffer;
}

/////////////////////////////////////////////////
ignition::common::ConnectionPtr
  Ogre2SegmentationCamera::ConnectNewSegmentationFrame(
  std::function<void(const uint8_t *, unsigned int, unsigned int,
  unsigned int, const std::string &)>  _subscriber)
{
  return this->dataPtr->newSegmentationFrame.Connect(_subscriber);
}

/////////////////////////////////////////////////
void Ogre2SegmentationCamera::CreateRenderTexture()
{
  RenderTexturePtr base = this->scene->CreateRenderTexture();
  this->dataPtr->dummyTexture =
    std::dynamic_pointer_cast<Ogre2RenderTexture>(base);
  this->dataPtr->dummyTexture->SetWidth(1);
  this->dataPtr->dummyTexture->SetHeight(1);
}

/////////////////////////////////////////////////
RenderTargetPtr Ogre2SegmentationCamera::RenderTarget() const
{
  return this->dataPtr->dummyTexture;
}

/////////////////////////////////////////////////
void Ogre2SegmentationCamera::SetBackgroundColor(
  const math::Color &_color)
{
  this->dataPtr->materialSwitcher->backgroundColor.Set(
    _color.R(), _color.G(), _color.B()
  );
}

/////////////////////////////////////////////////
void Ogre2SegmentationCamera::SetBackgroundLabel(int _label)
{
  this->dataPtr->materialSwitcher->backgroundLabel = _label;
  this->SetBackgroundColor(
    math::Color(_label / 255.0, _label / 255.0, _label / 255.0));
}

/////////////////////////////////////////////////
math::Color Ogre2SegmentationCamera::BackgroundColor() const
{
  return this->dataPtr->materialSwitcher->backgroundColor;
}

/////////////////////////////////////////////////
int Ogre2SegmentationCamera::BackgroundLabel() const
{
  return this->dataPtr->materialSwitcher->backgroundLabel;
}

/////////////////////////////////////////////////
void Ogre2SegmentationCamera::SetSegmentationType(SegmentationType _type)
{
  this->dataPtr->materialSwitcher->type = _type;
}

/////////////////////////////////////////////////
void Ogre2SegmentationCamera::EnableColoredMap(bool _enable)
{
  this->dataPtr->materialSwitcher->isColoredMap = _enable;
}

/////////////////////////////////////////////////
SegmentationType Ogre2SegmentationCamera::Type() const
{
  return this->dataPtr->materialSwitcher->type;
}

/////////////////////////////////////////////////
bool Ogre2SegmentationCamera::IsColoredMap() const
{
  return this->dataPtr->materialSwitcher->isColoredMap;
}

/////////////////////////////////////////////////
void Ogre2SegmentationCamera::LabelMapFromColoredBuffer(
  uint8_t * _labelBuffer) const
{
  if (!this->dataPtr->materialSwitcher->isColoredMap)
    return;

  if (!this->dataPtr->buffer)
    return;

  const auto &colorToLabel = this->dataPtr->materialSwitcher->colorToLabel;

  auto width = this->ImageWidth();
  auto height = this->ImageHeight();

  for (uint32_t i = 0; i < height; i++)
  {
    for (uint32_t j = 0; j < width; j++)
    {
      auto index = (i * width + j) * 3;
      auto r = this->dataPtr->buffer[index + 2];
      auto g = this->dataPtr->buffer[index + 1];
      auto b = this->dataPtr->buffer[index];

      // get color 24 bit unique id, we don't multiply it by 255 like before
      // as they are not normalized we read it from the buffer in
      // range [0-255] already
      int64_t colorId = r * 256 * 256 + g * 256 + b;

      // initialize the pixel with the background label value
      {
        uint8_t label8bit = this->dataPtr->materialSwitcher->backgroundLabel;

        _labelBuffer[index] =     label8bit;
        _labelBuffer[index + 1] = label8bit;
        _labelBuffer[index + 2] = label8bit;
      }

      // skip if not exist
      auto it = colorToLabel.find(colorId);
      if (it == colorToLabel.end())
        continue;

      int64_t label = it->second;

      if (this->dataPtr->materialSwitcher->type ==
          SegmentationType::ST_SEMANTIC)
      {
        uint8_t label8bit = label % 256;

        _labelBuffer[index] =     label8bit;
        _labelBuffer[index + 1] = label8bit;
        _labelBuffer[index + 2] = label8bit;
      }
      else if (this->dataPtr->materialSwitcher->type ==
        SegmentationType::ST_PANOPTIC)
      {
        // get the label and instance counts from the composite label id
        uint8_t label8bit = label / (256 * 256);
        // get the rest 16 bit
        uint16_t instanceCount = label % (256 * 256);
        // composite that 16 bit count to two 8 bit channels
        uint8_t instanceCount1 = instanceCount / 256;
        uint8_t instanceCount2 = instanceCount % 256;

        _labelBuffer[index + 2] = label8bit;
        _labelBuffer[index + 1] = instanceCount1;
        _labelBuffer[index] = instanceCount2;
      }
    }
  }
}
