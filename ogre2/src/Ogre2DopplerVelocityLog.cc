/*
 * Copyright (C) 2022 Open Source Robotics Foundation
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
#include "gz/rendering/ogre2/Ogre2DopplerVelocityLog.hh"

namespace ignition
{
namespace rendering
{
inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {

class Ogre2DopplerVelocityLogMaterialSwitcher : public Ogre::Camera::Listener
{
  /// \brief constructor
  /// \param[in] _scene the scene manager responsible for rendering
  /// \param[in] _name the name of the thermal camera
  public: explicit Ogre2ThermalCameraMaterialSwitcher(Ogre2ScenePtr _scene,
              const std::string & _name)
  {
    this->scene = _scene;
    // plain opaque material
    Ogre::ResourcePtr res =
      Ogre::MaterialManager::getSingleton().load("ThermalHeatSource",
          Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

    this->baseHeatSigMaterial = Ogre::MaterialManager::getSingleton().
      getByName("ThermalHeatSignature");

    this->ogreCamera = this->scene->OgreSceneManager()->findCamera(this->name);
  }

  /// \brief destructor
  public: ~Ogre2ThermalCameraMaterialSwitcher() = default;

  /// \brief Set image format
  /// \param[in] _format Image format
  public: void SetFormat(PixelFormat _format)
  {
    this->format = _format;
    this->bitDepth = 8u * PixelUtil::BytesPerChannel(format);
  }

  /// \brief Set temperature linear resolution
  /// \param[in] _resolution Temperature linear resolution
  public: void SetLinearResolution(double _resolution)
  {
    this->resolution = _resolution;
  }

  /// \brief Callback when a camera is about to be rendered
  /// \param[in] _cam Ogre camera pointer which is about to render
  private: virtual void cameraPreRenderScene(
    Ogre::Camera * _cam) override
  {
    auto engine = Ogre2RenderEngine::Instance();
    engine->SetIgnOgreRenderingMode(IORM_SOLID_COLOR);

    auto itor = this->scene->OgreSceneManager()->getMovableObjectIterator(
      Ogre::ItemFactory::FACTORY_TYPE_NAME);
    while (itor.hasMoreElements)
    {
      itor.moveNext();

      //Extract entity 

      Ogre::Item *item = static_cast<Ogre::Item *>(object);
    }

    auto heightmaps = this->scene->Heightmaps();
    for (auto h : heightmaps)
    {
    }
  }

  /// \brief Callback when a camera is finisned being rendered
  /// \param[in] _cam Ogre camera pointer which has already render
  private: virtual void cameraPostRenderScene(
    Ogre::Camera * _cam) override;

  /// \brief Scene manager
  private: Ogre2ScenePtr scene = nullptr;

  /// \brief Pointer to the "base" heat signature material.
  /// All renderable items with a heat signature texture use their own
  /// copy of this base material, with the item's specific heat
  /// signature texture applied to it
  private: Ogre::MaterialPtr baseHeatSigMaterial;

  /// \brief A map of all items that have a heat signature material.
  /// The key is the item's ID, and the value is the heat signature
  /// for that item.
  private: std::unordered_map<Ogre::IdType, Ogre::MaterialPtr>
            heatSignatureMaterials;

  /// \brief The name of the thermal camera sensor
  private: const std::string name;

  /// \brief The thermal camera
  private: const Ogre::Camera* ogreCamera{nullptr};

  /// \brief A map of ogre sub item pointer to their original hlms material
  private: std::vector<std::pair<Ogre::SubItem *, Ogre::HlmsDatablock *>>
      itemDatablockMap;

  /// \brief A map of ogre sub item pointer to their original low level
  /// material.
  /// Most objects don't use one so it should be almost always empty.
  private:
    std::vector<std::pair<Ogre::SubItem *, Ogre::MaterialPtr>> materialMap;

  /// \brief A map of ogre datablock pointer to their original blendblocks
  private: std::unordered_map<Ogre::HlmsDatablock *,
      const Ogre::HlmsBlendblock *> datablockMap;

  /// \brief linear temperature resolution. Defaults to 10mK
  private: double resolution = 0.01;

  /// \brief thermal camera image format
  private: PixelFormat format = PF_BYTE_RGBA;

  /// \brief thermal camera image bit depth
  private: unsigned int bitDepth = 32u;
}


class Ogre2DopplerVelocityLogPrivate
{
  /// \brief Outgoing thermal data, used by newThermalFrame event.
  public: uint16_t *depthVisibilityImage = nullptr;

  /// \brief maximum value used for data outside sensor range
  public: uint16_t dataMaxVal = std::numeric_limits<uint16_t>::max();

  /// \brief minimum value used for data outside sensor range
  public: uint16_t dataMinVal = 0u;

  /// \brief 1st pass compositor workspace definition
  public: std::string ogreCompositorWorkspaceDef;

  /// \brief 1st pass compositor node definition
  public: std::string ogreCompositorNodeDef;

  /// \brief 1st pass compositor workspace. One for each cubemap camera
  public: Ogre::CompositorWorkspace *ogreCompositorWorkspace;

  /// \brief Thermal textures.
  public: Ogre::TextureGpu *ogreDoppler;

  /// \brief Dummy render texture for the thermal data
  public: RenderTexturePtr depthVisibilityTexture = nullptr;

  /// \brief The thermal material
  public: Ogre::MaterialPtr depthVisibilityMaterial;

  /// \brief Event used to signal thermal image data
  public: ignition::common::EventT<void(const uint16_t *,
              unsigned int, unsigned int, unsigned int,
              const std::string &)> newDepthVisibilityFrame;

  /// \brief Pointer to material switcher
  public: std::unique_ptr<Ogre2DopplerVelocityLogMaterialSwitcher>
      thermalMaterialSwitcher = nullptr;

  /// \brief Add variation to temperature values based on object rgb values
  /// This only affects objects that are not heat sources
  /// TODO(anyone) add API for setting this value?
  public: bool rgbToTemp = true;

  /// \brief bit depth of each pixel
  public: unsigned int bitDepth = 16u;
};

//////////////////////////////////////////////////
Ogre2DopplerVelocityLog::Ogre2DopplerVelocityLog()
{
}

//////////////////////////////////////////////////
Ogre2DopplerVelocityLog::~Ogre2DopplerVelocityLog()
{
}

//////////////////////////////////////////////////
void Ogre2DopplerVelocityLog::Init()
{

}

//////////////////////////////////////////////////
void Ogre2DopplerVelocityLog::Destroy()
{

}

//////////////////////////////////////////////////
void Ogre2DopplerVelocityLog::PreRender()
{
  if (!this->dataPtr->ogreDoppler)
    this->CreateDopplerVelocityLogTexture();
}

//////////////////////////////////////////////////
void Ogre2DopplerVelocityLog::PostRender()
{

}

//////////////////////////////////////////////////
ignition::common::ConnectionPtr
        Ogre2DopplerVelocityLog::ConnectNewDepthVisibilitymapFrame(
        std::function<void(const uint8_t *, unsigned int, unsigned int,
        unsigned int, const std::string &)>  _subscriber)
{
  ConnectNewDepthVisibilitymapFrame
}

//////////////////////////////////////////////////
void Ogre2DopplerVelocityLog::Render()
{
  // GL_DEPTH_CLAMP is disabled in later version of ogre2.2
  // however our shaders rely on clamped values so enable it for this sensor
  auto engine = Ogre2RenderEngine::Instance();
  std::string renderSystemName =
      engine->OgreRoot()->getRenderSystem()->getFriendlyName();
  bool useGL = renderSystemName.find("OpenGL") != std::string::npos;
#ifndef _WIN32
  if (useGL)
    glEnable(GL_DEPTH_CLAMP);
#endif

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

#ifndef _WIN32
  if (useGL)
    glDisable(GL_DEPTH_CLAMP);
#endif
}

//////////////////////////////////////////////////
RenderTargetPtr Ogre2DopplerVelocityLog::RenderTarget() const
{
  return this->dataPtr->depthVisibilityTexture;
}

//////////////////////////////////////////////////
Ogre::Camera *Ogre2DopplerVelocityLog::OgreCamera() const
{
}

//////////////////////////////////////////////////
void Ogre2DopplerVelocityLog::CreateCamera()
{
  // create ogre camera object
  Ogre::SceneManager *ogreSceneManager;
  ogreSceneManager = this->scene->OgreSceneManager();
  if (ogreSceneManager == nullptr)
  {
    ignerr << "Scene manager cannot be obtained" << std::endl;
    return;
  }

  this->ogreCamera = ogreSceneManager->createCamera(this->name);
  if (this->ogreCamera == nullptr)

  {
    ignerr << "Ogre camera cannot be created" << std::endl;
    return;
  }

  // by default, ogre2 cameras are attached to root scene node
  this->ogreCamera->detachFromParent();
  this->ogreNode->attachObject(this->ogreCamera);

  // rotate to Gazebo coordinate system
  this->ogreCamera->yaw(Ogre::Degree(-90.0));
  this->ogreCamera->roll(Ogre::Degree(-90.0));
  this->ogreCamera->setFixedYawAxis(false);

  // TODO(anyone): provide api access
  this->ogreCamera->setAutoAspectRatio(true);
  this->ogreCamera->setProjectionType(Ogre::PT_PERSPECTIVE);
  this->ogreCamera->setCustomProjectionMatrix(false);
}

//////////////////////////////////////////////////
void Ogre2DopplerVelocityLog::CreateRenderTexture()
{
  RenderTexturePtr base = this->scene->CreateRenderTexture();
  this->dataPtr->depthVisibilityTexture =
      std::dynamic_pointer_cast<Ogre2RenderTexture>(base);
  this->dataPtr->depthVisibilityTexture->SetWidth(1);
  this->dataPtr->depthVisibilityTexture->SetHeight(1);
}

//////////////////////////////////////////////////
void Ogre2DopplerVelocityLog::CreateDopplerVelocityLogTexture()
{
}

}
}
}
