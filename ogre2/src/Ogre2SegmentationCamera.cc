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

#include <algorithm>
#include <array>
#include <cstring>
#include <string>

#include <gz/common/Console.hh>
#include <gz/common/Profiler.hh>

#include <gz/math/Color.hh>
#include <gz/math/Matrix4.hh>

#include "gz/rendering/ogre2/Ogre2Camera.hh"
#include "gz/rendering/ogre2/Ogre2Conversions.hh"
#include "gz/rendering/ogre2/Ogre2Includes.hh"
#include "gz/rendering/ogre2/Ogre2ParticleEmitter.hh"
#include "gz/rendering/ogre2/Ogre2RenderTarget.hh"
#include "gz/rendering/ogre2/Ogre2RenderTypes.hh"
#include "gz/rendering/ogre2/Ogre2RenderEngine.hh"
#include "gz/rendering/ogre2/Ogre2Scene.hh"
#include "gz/rendering/ogre2/Ogre2SegmentationCamera.hh"
#include "gz/rendering/RenderTypes.hh"
#include "gz/rendering/Utils.hh"

#include "Ogre2GpuReadbackTicket.hh"
#include "Ogre2SegmentationMaterialSwitcher.hh"

/// \brief Private data for the Ogre2SegmentationCamera class
class gz::rendering::Ogre2SegmentationCameraPrivate
{
  /// \brief buffer to store render texture data & to be sent to listeners
  public: uint8_t *buffer {nullptr};

  /// \brief Workspace Definition
  public: std::string ogreCompositorWorkspaceDef;

  /// \brief Final pass compositor node definition
  public: std::string ogreCompositorNodeDef;

  /// \brief 1st pass compositor workspace
  public: Ogre::CompositorWorkspace *ogreCompositorWorkspace {nullptr};

  /// \brief Output texture
  public: Ogre::TextureGpu *ogreSegmentationTexture {nullptr};

  /// \brief Dummy render texture for the depth data
  public: RenderTexturePtr segmentationTexture {nullptr};

  /// \brief New Segmentation Frame Event to notify listeners with new data
  /// \param[in] _data Segmentation buffer data
  /// \param[in] _width Width of the image
  /// \param[in] _height Height of the image
  /// \param[in] _channels Number of channels
  /// \param[in] _format Image Format
  public: gz::common::EventT<void(const uint8_t *_data,
    unsigned int _width, unsigned int _height, unsigned int _channels,
    const std::string &_format)> newSegmentationFrame;

  /// \brief Material Switcher to switch item's material
  /// with colored version for segmentation
  public: std::unique_ptr<Ogre2SegmentationMaterialSwitcher>
          materialSwitcher {nullptr};

  /// \brief Persistent GPU->CPU readback of the segmentation texture.
  public: Ogre2GpuReadbackTicket readback;
};

using namespace gz;
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

  this->dataPtr->materialSwitcher.reset(
      new Ogre2SegmentationMaterialSwitcher(this->scene, this));
}

/////////////////////////////////////////////////
void Ogre2SegmentationCamera::Destroy()
{
  this->RemoveAllRenderPasses();

  if (this->dataPtr->buffer)
  {
    delete [] this->dataPtr->buffer;
    this->dataPtr->buffer = nullptr;
  }

  if (!this->ogreCamera)
    return;

  auto engine = Ogre2RenderEngine::Instance();
  auto ogreRoot = engine->OgreRoot();
  auto ogreCompMgr = ogreRoot->getCompositorManager2();

  this->dataPtr->readback.Destroy();

  if (this->dataPtr->ogreSegmentationTexture)
  {
    ogreRoot->getRenderSystem()->getTextureGpuManager()->destroyTexture(
      this->dataPtr->ogreSegmentationTexture);
    this->dataPtr->ogreSegmentationTexture = nullptr;

  }
  if (this->dataPtr->ogreCompositorWorkspace)
  {
    ogreCompMgr->removeWorkspace(
        this->dataPtr->ogreCompositorWorkspace);
    this->dataPtr->ogreCompositorWorkspace = nullptr;
  }

  if (!this->dataPtr->ogreCompositorWorkspaceDef.empty())
  {
    ogreCompMgr->removeWorkspaceDefinition(
        this->dataPtr->ogreCompositorWorkspaceDef);
    ogreCompMgr->removeNodeDefinition(
        this->dataPtr->ogreCompositorNodeDef);
  }

  Ogre::SceneManager *ogreSceneManager;
  ogreSceneManager = this->scene->OgreSceneManager();
  if (ogreSceneManager == nullptr)
  {
    gzerr << "Scene manager cannot be obtained" << std::endl;
  }
  else
  {
    if (ogreSceneManager->findCameraNoThrow(this->name) != nullptr)
    {
      ogreSceneManager->destroyCamera(this->ogreCamera);
      this->ogreCamera = nullptr;
    }
  }

  this->dataPtr->materialSwitcher.reset();

  BaseCamera::Destroy();
}

/////////////////////////////////////////////////
void Ogre2SegmentationCamera::PreRender()
{
  GZ_PROFILE("Ogre2SegmentationCamera::PreRender");
  if (!this->dataPtr->ogreSegmentationTexture)
    this->CreateSegmentationTexture();
}

/////////////////////////////////////////////////
void Ogre2SegmentationCamera::CreateCamera()
{
  auto ogreSceneManager = this->scene->OgreSceneManager();
  if (ogreSceneManager == nullptr)
  {
    gzerr << "Scene manager cannot be obtained" << std::endl;
    return;
  }

  this->ogreCamera = ogreSceneManager->createCamera(this->Name());
  if (this->ogreCamera == nullptr)
  {
    gzerr << "Ogre camera cannot be created" << std::endl;
    return;
  }

  this->ogreCamera->detachFromParent();
  this->ogreNode->attachObject(this->ogreCamera);

  // rotate to Gazebo Sim coord.
  this->ogreCamera->yaw(Ogre::Degree(-90));
  this->ogreCamera->roll(Ogre::Degree(-90));
  this->ogreCamera->setFixedYawAxis(false);

  this->ogreCamera->setRenderingDistance(100);
  this->ogreCamera->setProjectionType(Ogre::ProjectionType::PT_PERSPECTIVE);
  this->ogreCamera->setCustomProjectionMatrix(false);
}

/////////////////////////////////////////////////
void Ogre2SegmentationCamera::CreateSegmentationTexture()
{
  GZ_PROFILE("Ogre2SegmentationCamera::CreateSegmentationTexture");
  // Camera Parameters
  this->ogreCamera->setNearClipDistance(this->NearClipPlane());
  this->ogreCamera->setFarClipDistance(this->FarClipPlane());
  const double aspectRatio = this->AspectRatio();
  const double angle = this->HFOV().Radian();
  const double vfov = 2.0 * atan(tan(angle / 2.0) / aspectRatio);
  this->ogreCamera->setFOVy(Ogre::Radian((Ogre::Real)vfov));
  this->ogreCamera->setAspectRatio((Ogre::Real)aspectRatio);

  auto engine = Ogre2RenderEngine::Instance();
  auto ogreRoot = engine->OgreRoot();
  Ogre::CompositorManager2 *ogreCompMgr = ogreRoot->getCompositorManager2();

  this->SetImageFormat(PixelFormat::PF_R8G8B8);
  Ogre::PixelFormatGpu ogrePF = Ogre::PFG_RGBA8_UNORM;

  auto backgroundColor_ = Ogre2Conversions::Convert(
      this->backgroundColor);

  std::string wsDefName = "SegmentationCameraWorkspace_" + this->Name();
  ogreCompMgr->createBasicWorkspaceDef(wsDefName, backgroundColor_);

  // Objects are drawn in flat colors: no lights needed. Setting up lights
  // (Forward+ light culling) is most of the cost of the pass in small
  // scenes.
  if (Ogre::CompositorWorkspaceDef *workspaceDef =
      ogreCompMgr->getWorkspaceDefinitionNoThrow(wsDefName))
  {
    for (const auto &[alias, nodeName] : workspaceDef->getNodeAliasMap())
    {
      Ogre::CompositorNodeDef *nodeDef =
          ogreCompMgr->getNodeDefinitionNonConst(nodeName);
      for (size_t t = 0; t < nodeDef->getNumTargetPasses(); ++t)
      {
        for (Ogre::CompositorPassDef *pass :
             nodeDef->getTargetPass(t)->getCompositorPassesNonConst())
        {
          if (pass->getType() == Ogre::PASS_SCENE)
          {
            auto *scenePass = static_cast<Ogre::CompositorPassSceneDef *>(
                pass);
            scenePass->mEnableForwardPlus = false;
            scenePass->setLightVisibilityMask(0u);
          }
        }
      }
    }
  }

  Ogre::TextureGpuManager *textureMgr =
    ogreRoot->getRenderSystem()->getTextureGpuManager();
  // create render texture
  this->dataPtr->ogreSegmentationTexture =
    textureMgr->createOrRetrieveTexture(this->Name() + "_segmentation",
      Ogre::GpuPageOutStrategy::SaveToSystemRam,
      Ogre::TextureFlags::RenderToTexture,
      Ogre::TextureTypes::Type2D);

  this->dataPtr->ogreSegmentationTexture->setResolution(
      this->ImageWidth(), this->ImageHeight());
  this->dataPtr->ogreSegmentationTexture->setNumMipmaps(1u);
  this->dataPtr->ogreSegmentationTexture->setPixelFormat(ogrePF);
  this->dataPtr->ogreSegmentationTexture->scheduleTransitionTo(
    Ogre::GpuResidency::Resident);

  // create compositor worksspace
  this->dataPtr->ogreCompositorWorkspace =
      ogreCompMgr->addWorkspace(
        this->scene->OgreSceneManager(),
        this->dataPtr->ogreSegmentationTexture,
        this->ogreCamera,
        wsDefName,
        false);

  this->ogreCamera->addListener(
    this->dataPtr->materialSwitcher.get());
}

/////////////////////////////////////////////////
void Ogre2SegmentationCamera::PostRender()
{
  GZ_PROFILE("Ogre2SegmentationCamera::PostRender");
  // return if no one is listening to the new frame
  if (this->dataPtr->newSegmentationFrame.ConnectionCount() == 0)
    return;

  const auto width = this->ImageWidth();
  const auto height = this->ImageHeight();
  PixelFormat format = this->ImageFormat();

  const auto len = width * height;
  const auto channelCount = PixelUtil::ChannelCount(format);
  const auto bytesPerChannel = PixelUtil::BytesPerChannel(format);
  const auto bufferSize = len * channelCount * bytesPerChannel;

  // Read the RGBA texture back, reusing the same staging buffer every
  // frame, unless the legacy readback is requested.
  Ogre::Image2 legacyImage;
  Ogre::TextureBox box;
  const bool legacy = Ogre2UseLegacyReadback();
  if (legacy)
  {
    legacyImage.convertFromTexture(this->dataPtr->ogreSegmentationTexture,
        0u, 0u);
    box = legacyImage.getData(0);
  }
  else
  {
    box = this->dataPtr->readback.DownloadAndMap(
        this->dataPtr->ogreSegmentationTexture);
  }
  if (!box.data)
  {
    gzerr << "Segmentation camera: GPU readback failed; dropping frame"
          << std::endl;
    return;
  }

  if (!this->dataPtr->buffer)
  {
    this->dataPtr->buffer = new uint8_t[bufferSize];
  }

  // RGBA -> RGB
  for (unsigned int row = 0; row < height; ++row)
  {
    const uint8_t *src = static_cast<const uint8_t *>(box.data) +
        row * box.bytesPerRow;
    uint8_t *dst = this->dataPtr->buffer + row * width * channelCount;
    for (unsigned int column = 0; column < width; ++column)
    {
      dst[0] = src[0];
      dst[1] = src[1];
      dst[2] = src[2];
      src += 4;
      dst += 3;
    }
  }

  if (!legacy)
    this->dataPtr->readback.Unmap();

  this->dataPtr->newSegmentationFrame(
    this->dataPtr->buffer,
    width, height, channelCount,
    PixelUtil::Name(format));
}

/////////////////////////////////////////////////
gz::common::ConnectionPtr
  Ogre2SegmentationCamera::ConnectNewSegmentationFrame(
  std::function<void(const uint8_t *, unsigned int, unsigned int,
  unsigned int, const std::string &)>  _subscriber)
{
  return this->dataPtr->newSegmentationFrame.Connect(_subscriber);
}

/////////////////////////////////////////////////
void Ogre2SegmentationCamera::Render()
{
  GZ_PROFILE("Ogre2ThermalCamera::Render");
  // update the compositors
  this->scene->StartRendering(this->ogreCamera);

  this->dataPtr->ogreCompositorWorkspace->_validateFinalTarget();
  this->dataPtr->ogreCompositorWorkspace->_beginUpdate(false);
  this->dataPtr->ogreCompositorWorkspace->_update();
  this->dataPtr->ogreCompositorWorkspace->_endUpdate(false);

  Ogre::vector<Ogre::TextureGpu*>::type swappedTargets;
  swappedTargets.reserve(2u);
  this->dataPtr->ogreCompositorWorkspace->_swapFinalTarget(swappedTargets);

  this->scene->FlushGpuCommandsAndStartNewFrame(1u, false);
}

/////////////////////////////////////////////////
RenderTargetPtr Ogre2SegmentationCamera::RenderTarget() const
{
  return this->dataPtr->segmentationTexture;
}

/////////////////////////////////////////////////
void Ogre2SegmentationCamera::CreateRenderTexture()
{
  RenderTexturePtr base = this->scene->CreateRenderTexture();
  this->dataPtr->segmentationTexture =
    std::dynamic_pointer_cast<Ogre2RenderTexture>(base);
  this->dataPtr->segmentationTexture->SetWidth(1);
  this->dataPtr->segmentationTexture->SetHeight(1);
}

/////////////////////////////////////////////////
void Ogre2SegmentationCamera::SetBackgroundLabel(int _label)
{
  this->backgroundLabel = _label;
  this->SetBackgroundColor(
    math::Color(_label / 255.0, _label / 255.0, _label / 255.0));
}

/////////////////////////////////////////////////
void Ogre2SegmentationCamera::LabelMapFromColoredBuffer(
  uint8_t * _labelBuffer) const
{
  GZ_PROFILE("Ogre2SegmentationCamera::LabelMapFromColoredBuffer");
  if (!this->isColoredMap)
    return;

  if (!this->dataPtr->buffer)
    return;

  const auto &colorToLabel = this->dataPtr->materialSwitcher->ColorToLabel();

  const auto width = this->ImageWidth();
  const auto height = this->ImageHeight();
  const uint8_t background = static_cast<uint8_t>(this->backgroundLabel);

  // Label map pixel for a color. Neighboring pixels almost always have the
  // same color, so remember the last one instead of looking it up again.
  int64_t lastColorId = -1;
  std::array<uint8_t, 3> lastPixel{};
  auto pixelForColor = [&](int64_t _colorId)
  {
    std::array<uint8_t, 3> pixel{background, background, background};
    auto it = colorToLabel.find(_colorId);
    if (it == colorToLabel.end())
      return pixel;

    // Labels outside 0-255 are clamped, as in the label maps rendered
    // directly.
    auto label8bit = [](int64_t _label)
    {
      return static_cast<uint8_t>(std::clamp<int64_t>(_label, 0, 255));
    };
    const int64_t key = it->second;
    if (this->type == SegmentationType::ST_SEMANTIC)
    {
      const uint8_t label = label8bit(key);
      pixel = {label, label, label};
    }
    else if (this->type == SegmentationType::ST_PANOPTIC)
    {
      // the composite id is label * 65536 + instance (16 bit)
      int64_t label = key / (256 * 256);
      if (key < 0 && key % (256 * 256) != 0)
        --label;
      const auto instanceCount =
          static_cast<uint16_t>(key - label * 256 * 256);
      // composite that 16 bit count to two 8 bit channels
      pixel = {static_cast<uint8_t>(instanceCount % 256),
          static_cast<uint8_t>(instanceCount / 256), label8bit(label)};
    }
    return pixel;
  };

  const size_t count = static_cast<size_t>(width) * height * 3;
  const uint8_t *src = this->dataPtr->buffer;
  for (size_t index = 0; index < count; index += 3)
  {
    // color 24 bit unique id; the buffer holds [0-255] values
    const int64_t colorId = src[index] * 256 * 256 + src[index + 1] * 256 +
        src[index + 2];
    if (colorId != lastColorId)
    {
      lastPixel = pixelForColor(colorId);
      lastColorId = colorId;
    }
    std::memcpy(_labelBuffer + index, lastPixel.data(), 3);
  }
}

//////////////////////////////////////////////////
Ogre::Camera *Ogre2SegmentationCamera::OgreCamera() const
{
  return this->ogreCamera;
}

/////////////////////////////////////////////////
math::Matrix4d Ogre2SegmentationCamera::ProjectionMatrix() const
{
  return Ogre2Conversions::Convert(this->ogreCamera->getProjectionMatrix());
}

/////////////////////////////////////////////////
void Ogre2SegmentationCamera::SetProjectionMatrix(const math::Matrix4d &_matrix)
{
  BaseSegmentationCamera::SetProjectionMatrix(_matrix);
  this->ogreCamera->setCustomProjectionMatrix(
      true, Ogre2Conversions::Convert(this->projectionMatrix));
}
