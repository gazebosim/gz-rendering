/*
 * Copyright (C) 2019 Open Source Robotics Foundation
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

#if (_WIN32)
  /* Needed for std::min */
  #ifndef NOMINMAX
    #define NOMINMAX
  #endif
  #include <windows.h>
#endif

#include <math.h>
#include <limits>

#include <ignition/common/Console.hh>
#include <ignition/math/Helpers.hh>

#include "ignition/rendering/RenderTypes.hh"
#include "ignition/rendering/ogre2/Ogre2Conversions.hh"
#include "ignition/rendering/ogre2/Ogre2Includes.hh"
#include "ignition/rendering/ogre2/Ogre2RenderEngine.hh"
#include "ignition/rendering/ogre2/Ogre2RenderTarget.hh"
#include "ignition/rendering/ogre2/Ogre2RenderTypes.hh"
#include "ignition/rendering/ogre2/Ogre2Scene.hh"
#include "ignition/rendering/ogre2/Ogre2Sensor.hh"
#include "ignition/rendering/ogre2/Ogre2ThermalCamera.hh"
#include "ignition/rendering/ogre2/Ogre2Visual.hh"

namespace ignition
{
namespace rendering
{
inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {
//
/// \brief Helper class for switching the ogre item's material to heat source
/// material when a thermal camera is being rendered.
class Ogre2ThermalCameraMaterialSwitcher : public Ogre::RenderTargetListener
{
  /// \brief constructor
  /// \param[in] _scene the scene manager responsible for rendering
  public: explicit Ogre2ThermalCameraMaterialSwitcher(Ogre2ScenePtr _scene);

  /// \brief destructor
  public: ~Ogre2ThermalCameraMaterialSwitcher() = default;

  /// \brief Callback when a render target is about to be rendered
  /// \param[in] _evt Ogre render target event containing information about
  /// the source render target.
  private: virtual void preRenderTargetUpdate(
      const Ogre::RenderTargetEvent &_evt) override;

  /// \brief Callback when a render target is finisned being rendered
  /// \param[in] _evt Ogre render target event containing information about
  /// the source render target.
  private: virtual void postRenderTargetUpdate(
      const Ogre::RenderTargetEvent &_evt) override;

  /// \brief Scene manager
  private: Ogre2ScenePtr scene = nullptr;

  /// \brief Pointer to the heat source material
  private: Ogre::MaterialPtr heatSourceMaterial;

  /// \brief Custom parameter index of temperature data in an ogre subitem.
  /// This has to match the custom index specifed in ThermalHeatSource material
  /// script in media/materials/scripts/thermal_camera.material
  private: const unsigned int customParamIdx = 10u;

  /// \brief A map of ogre sub item pointer to their original hlms material
  private: std::map<Ogre::SubItem *, Ogre::HlmsDatablock *> datablockMap;
};
}
}
}


/// \internal
/// \brief Private data for the Ogre2ThermalCamera class
class ignition::rendering::Ogre2ThermalCameraPrivate
{
  /// \brief The thermal buffer
  public: uint16_t *thermalBuffer = nullptr;

  /// \brief Outgoing thermal data, used by newThermalFrame event.
  public: uint16_t *thermalImage = nullptr;

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
  public: Ogre::TexturePtr ogreThermalTexture;

  /// \brief Dummy render texture for the thermal data
  public: RenderTexturePtr thermalTexture = nullptr;

  /// \brief The thermal material
  public: Ogre::MaterialPtr thermalMaterial;

  /// \brief Event used to signal thermal image data
  public: ignition::common::EventT<void(const uint16_t *,
              unsigned int, unsigned int, unsigned int,
              const std::string &)> newThermalFrame;

  /// \brief Pointer to material switcher
  public: std::unique_ptr<Ogre2ThermalCameraMaterialSwitcher>
      thermalMaterialSwitcher = nullptr;
};

using namespace ignition;
using namespace rendering;

//////////////////////////////////////////////////
Ogre2ThermalCameraMaterialSwitcher::Ogre2ThermalCameraMaterialSwitcher(
    Ogre2ScenePtr _scene)
{
  this->scene = _scene;
  // plain opaque material
  Ogre::ResourcePtr res =
    Ogre::MaterialManager::getSingleton().load("ThermalHeatSource",
        Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

  this->heatSourceMaterial = res.staticCast<Ogre::Material>();
  this->heatSourceMaterial->load();
}

//////////////////////////////////////////////////
void Ogre2ThermalCameraMaterialSwitcher::preRenderTargetUpdate(
    const Ogre::RenderTargetEvent & /*_evt*/)
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

    std::string tempKey = "temperature";
    // get visual
    Ogre::Any userAny = item->getUserObjectBindings().getUserAny();
    if (!userAny.isEmpty() && userAny.getType() == typeid(unsigned int))
    {
      VisualPtr result;
      try
      {
        result = this->scene->VisualById(Ogre::any_cast<unsigned int>(userAny));
      }
      catch(Ogre::Exception &e)
      {
        ignerr << "Ogre Error:" << e.getFullDescription() << "\n";
      }
      Ogre2VisualPtr ogreVisual =
          std::dynamic_pointer_cast<Ogre2Visual>(result);

      // get temperature
      Variant tempAny = ogreVisual->UserData(tempKey);
      if (tempAny.index() != 0)
      {
        float temp = -1.0;
        try
        {
          temp = std::get<float>(tempAny);
        }
        catch(...)
        {
          try
          {
            temp = std::get<double>(tempAny);
          }
          catch(...)
          {
            try
            {
              temp = std::get<int>(tempAny);
            }
            catch(std::bad_variant_access &e)
            {
              ignerr << "Error casting user data: " << e.what() << "\n";
              temp = -1.0;
            }
          }
        }

        // only accept positive temperature (in kelvin)
        if (temp >= 0.0)
        {
          // set visibility flag so thermal camera can see it
          item->addVisibilityFlags(0x10000000);
          for (unsigned int i = 0; i < item->getNumSubItems(); ++i)
          {
            Ogre::SubItem *subItem = item->getSubItem(i);
            if (!subItem->hasCustomParameter(this->customParamIdx))
            {
              // normalize temperature value
              float color = temp * 100.0 /
                  static_cast<float>(std::numeric_limits<uint16_t>::max());
              subItem->setCustomParameter(this->customParamIdx,
                  Ogre::Vector4(color, color, color, 1.0));
            }
            Ogre::HlmsDatablock *datablock = subItem->getDatablock();
            this->datablockMap[subItem] = datablock;

            subItem->setMaterial(this->heatSourceMaterial);
          }
        }
      }
    }
    itor.moveNext();
  }
}

//////////////////////////////////////////////////
void Ogre2ThermalCameraMaterialSwitcher::postRenderTargetUpdate(
    const Ogre::RenderTargetEvent & /*_evt*/)
{
  // restore item to use hlms material
  for (auto it : this->datablockMap)
  {
    Ogre::SubItem *subItem = it.first;
    subItem->setDatablock(it.second);
  }
}

//////////////////////////////////////////////////
Ogre2ThermalCamera::Ogre2ThermalCamera()
  : dataPtr(new Ogre2ThermalCameraPrivate())
{
  this->dataPtr->ogreCompositorWorkspace = nullptr;
}

//////////////////////////////////////////////////
Ogre2ThermalCamera::~Ogre2ThermalCamera()
{
  this->Destroy();
}

//////////////////////////////////////////////////
void Ogre2ThermalCamera::Init()
{
  BaseThermalCamera::Init();

  // create internal camera
  this->CreateCamera();

  // create dummy render texture
  this->CreateRenderTexture();

  this->Reset();
}

//////////////////////////////////////////////////
void Ogre2ThermalCamera::Destroy()
{
  if (this->dataPtr->thermalBuffer)
  {
    delete [] this->dataPtr->thermalBuffer;
    this->dataPtr->thermalBuffer = nullptr;
  }

  if (this->dataPtr->thermalImage)
  {
    delete [] this->dataPtr->thermalImage;
    this->dataPtr->thermalImage = nullptr;
  }

  if (!this->ogreCamera)
    return;

  auto engine = Ogre2RenderEngine::Instance();
  auto ogreRoot = engine->OgreRoot();
  Ogre::CompositorManager2 *ogreCompMgr = ogreRoot->getCompositorManager2();

  // remove thermal texture, material, compositor
  if (this->dataPtr->ogreThermalTexture)
  {
    Ogre::TextureManager::getSingleton().remove(
        this->dataPtr->ogreThermalTexture->getName());
  }
  if (this->dataPtr->ogreCompositorWorkspace)
  {
    ogreCompMgr->removeWorkspace(
        this->dataPtr->ogreCompositorWorkspace);
  }

  if (this->dataPtr->thermalMaterial)
  {
    Ogre::MaterialManager::getSingleton().remove(
        this->dataPtr->thermalMaterial->getName());
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
}

//////////////////////////////////////////////////
void Ogre2ThermalCamera::CreateCamera()
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

/////////////////////////////////////////////////
void Ogre2ThermalCamera::CreateRenderTexture()
{
  RenderTexturePtr base = this->scene->CreateRenderTexture();
  this->dataPtr->thermalTexture =
      std::dynamic_pointer_cast<Ogre2RenderTexture>(base);
  this->dataPtr->thermalTexture->SetWidth(1);
  this->dataPtr->thermalTexture->SetHeight(1);
}

/////////////////////////////////////////////////////////
void Ogre2ThermalCamera::CreateThermalTexture()
{
  // set aspect ratio and fov
  double vfov = 2.0 * atan(tan(this->HFOV().Radian() / 2.0) / this->aspect);
  this->ogreCamera->setAspectRatio(this->aspect);
  this->ogreCamera->setFOVy(Ogre::Radian(vfov));

  // Load thermal material
  // The ThermalCamera material is defined in script (thermal_camera.material).
  // We need to clone it since we are going to modify its uniform variables
  std::string matThermalName = "ThermalCamera";
  Ogre::MaterialPtr matThermal =
      Ogre::MaterialManager::getSingleton().getByName(matThermalName);
  this->dataPtr->thermalMaterial = matThermal->clone(
      this->Name() + "_" + matThermalName);
  this->dataPtr->thermalMaterial->load();
  Ogre::Pass *pass =
      this->dataPtr->thermalMaterial->getTechnique(0)->getPass(0);
  Ogre::GpuProgramParametersSharedPtr psParams =
      pass->getFragmentProgramParameters();

  // Configure camera behaviour.
  double nearPlane = this->NearClipPlane();
  double farPlane = this->FarClipPlane();
  this->ogreCamera->setNearClipDistance(nearPlane);
  this->ogreCamera->setFarClipDistance(farPlane);

  // Set the uniform variables (thermal_camera_fs.glsl).
  // The projectParams is used to linearize thermal buffer data
  // The other params are used to clamp the range output
  // Use the 'real' clip distance here so thermal can be
  // linearized correctly
  double projectionA = farPlane /
      (farPlane - nearPlane);
  double projectionB = (-farPlane * nearPlane) /
      (farPlane - nearPlane);
  projectionB /= farPlane;
  psParams->setNamedConstant("projectionParams",
      Ogre::Vector2(projectionA, projectionB));
  psParams->setNamedConstant("near",
      static_cast<float>(this->NearClipPlane()));
  psParams->setNamedConstant("far",
      static_cast<float>(this->FarClipPlane()));
  psParams->setNamedConstant("max",
      static_cast<float>(this->maxTemp));
  psParams->setNamedConstant("min",
      static_cast<float>(this->minTemp));
  psParams->setNamedConstant("resolution",
      static_cast<float>(this->resolution));
  psParams->setNamedConstant("ambient",
      static_cast<float>(this->ambient));
  psParams->setNamedConstant("range",
      static_cast<float>(this->ambientRange));
  psParams->setNamedConstant("heatSourceTempRange",
      static_cast<float>(this->heatSourceTempRange));

  // Create thermal camera compositor
  auto engine = Ogre2RenderEngine::Instance();
  auto ogreRoot = engine->OgreRoot();
  Ogre::CompositorManager2 *ogreCompMgr = ogreRoot->getCompositorManager2();

  // We need to programmatically create the compositor because we need to
  // configure it to use the cloned thermal material created earlier.
  // The compositor workspace definition is equivalent to the following
  // ogre compositor script:
  // compositor_node ThermalCamera
  // {
  //   in 0 rt_input
  //   texture depthTexture target_width target_height PF_D32_FLOAT
  //   texture colorTexture target_width target_height PF_R8G8B8
  //   target depthTexture
  //   {
  //     pass clear
  //     {
  //       colour_value 0.0 0.0 0.0 1.0
  //     }
  //     pass render_scene
  //     {
  //     }
  //   }
  //   target colorTexture
  //   {
  //     pass clear
  //     {
  //       colour_value 0.0 0.0 0.0 1.0
  //     }
  //     pass render_scene
  //     {
  //     }
  //   }
  //   target rt_input
  //   {
  //     pass clear
  //     {
  //       colour_value 0.0 0.0 0.0 1.0
  //     }
  //     pass render_quad
  //     {
  //       material ThermalCamera // Use copy instead of original
  //       input 0 depthTexture
  //       input 1 colorTexture
  //       quad_normals camera_far_corners_view_space
  //     }
  //   }
  //   out 0 rt_input
  // }
  std::string wsDefName = "ThermalCameraWorkspace_" + this->Name();
  this->dataPtr->ogreCompositorWorkspaceDef = wsDefName;
  if (!ogreCompMgr->hasWorkspaceDefinition(wsDefName))
  {
    std::string nodeDefName = wsDefName + "/Node";
    this->dataPtr->ogreCompositorNodeDef = nodeDefName;
    Ogre::CompositorNodeDef *nodeDef =
        ogreCompMgr->addNodeDefinition(nodeDefName);
    // Input texture
    nodeDef->addTextureSourceName("rt_input", 0,
        Ogre::TextureDefinitionBase::TEXTURE_INPUT);
    Ogre::TextureDefinitionBase::TextureDefinition *thermalTexDef =
        nodeDef->addTextureDefinition("depthTexture");
    thermalTexDef->textureType = Ogre::TEX_TYPE_2D;
    thermalTexDef->width = 0;
    thermalTexDef->height = 0;
    thermalTexDef->depth = 1;
    thermalTexDef->numMipmaps = 0;
    thermalTexDef->widthFactor = 1;
    thermalTexDef->heightFactor = 1;
    thermalTexDef->formatList = {Ogre::PF_D32_FLOAT};
    thermalTexDef->fsaa = 0;
    thermalTexDef->uav = false;
    thermalTexDef->automipmaps = false;
    thermalTexDef->hwGammaWrite = Ogre::TextureDefinitionBase::BoolFalse;
    thermalTexDef->depthBufferId = Ogre::DepthBuffer::POOL_NON_SHAREABLE;
    thermalTexDef->depthBufferFormat = Ogre::PF_UNKNOWN;
    thermalTexDef->fsaaExplicitResolve = false;

    Ogre::TextureDefinitionBase::TextureDefinition *colorTexDef =
        nodeDef->addTextureDefinition("colorTexture");
    colorTexDef->textureType = Ogre::TEX_TYPE_2D;
    colorTexDef->width = 0;
    colorTexDef->height = 0;
    colorTexDef->depth = 1;
    colorTexDef->numMipmaps = 0;
    colorTexDef->widthFactor = 1;
    colorTexDef->heightFactor = 1;
    colorTexDef->formatList = {Ogre::PF_R8G8B8};
    colorTexDef->fsaa = 0;
    colorTexDef->uav = false;
    colorTexDef->automipmaps = false;
    colorTexDef->hwGammaWrite = Ogre::TextureDefinitionBase::BoolFalse;
    colorTexDef->depthBufferId = Ogre::DepthBuffer::POOL_DEFAULT;
    colorTexDef->depthBufferFormat = Ogre::PF_D32_FLOAT;
    colorTexDef->preferDepthTexture = true;
    colorTexDef->fsaaExplicitResolve = false;

    nodeDef->setNumTargetPass(3);
    Ogre::CompositorTargetDef *depthTargetDef =
        nodeDef->addTargetPass("depthTexture");
    depthTargetDef->setNumPasses(2);
    {
      // clear pass
      Ogre::CompositorPassClearDef *passClear =
          static_cast<Ogre::CompositorPassClearDef *>(
          depthTargetDef->addPass(Ogre::PASS_CLEAR));
      passClear->mColourValue = Ogre::ColourValue(this->FarClipPlane(), 0, 0);
      // scene pass
      Ogre::CompositorPassSceneDef *passScene =
          static_cast<Ogre::CompositorPassSceneDef *>(
          depthTargetDef->addPass(Ogre::PASS_SCENE));
      passScene->mVisibilityMask = IGN_VISIBILITY_ALL
          & ~(IGN_VISIBILITY_GUI | IGN_VISIBILITY_SELECTABLE);
    }

    Ogre::CompositorTargetDef *colorTargetDef =
        nodeDef->addTargetPass("colorTexture");
    colorTargetDef->setNumPasses(2);
    {
      // clear pass
      Ogre::CompositorPassClearDef *passClear =
          static_cast<Ogre::CompositorPassClearDef *>(
          colorTargetDef->addPass(Ogre::PASS_CLEAR));
      passClear->mColourValue = Ogre::ColourValue(0, 0, 0);
      // scene pass
      Ogre::CompositorPassSceneDef *passScene =
          static_cast<Ogre::CompositorPassSceneDef *>(
          colorTargetDef->addPass(Ogre::PASS_SCENE));
      // set thermal camera custom visibility mask when rendering heat sources
      passScene->mVisibilityMask = 0x10000000;
    }

    // rt_input target - converts depth to thermal
    Ogre::CompositorTargetDef *inputTargetDef =
        nodeDef->addTargetPass("rt_input");
    inputTargetDef->setNumPasses(2);
    {
      // clear pass
      Ogre::CompositorPassClearDef *passClear =
          static_cast<Ogre::CompositorPassClearDef *>(
          inputTargetDef->addPass(Ogre::PASS_CLEAR));
      passClear->mColourValue = Ogre::ColourValue(this->ambient, 0, 1.0);
      // quad pass
      Ogre::CompositorPassQuadDef *passQuad =
          static_cast<Ogre::CompositorPassQuadDef *>(
          inputTargetDef->addPass(Ogre::PASS_QUAD));
      passQuad->mMaterialName = this->dataPtr->thermalMaterial->getName();
      passQuad->addQuadTextureSource(0, "depthTexture", 0);
      passQuad->addQuadTextureSource(1, "colorTexture", 0);
      passQuad->mFrustumCorners =
          Ogre::CompositorPassQuadDef::VIEW_SPACE_CORNERS;
    }
    nodeDef->mapOutputChannel(0, "rt_input");
    Ogre::CompositorWorkspaceDef *workDef =
        ogreCompMgr->addWorkspaceDefinition(wsDefName);
    workDef->connectExternal(0, nodeDef->getName(), 0);
  }
  Ogre::CompositorWorkspaceDef *wsDef =
      ogreCompMgr->getWorkspaceDefinition(wsDefName);

  if (!wsDef)
  {
    ignerr << "Unable to add workspace definition [" << wsDefName << "] "
           << " for " << this->Name();
  }

  // create render texture - these textures pack the thermal data
  this->dataPtr->ogreThermalTexture =
    Ogre::TextureManager::getSingleton().createManual(
    this->Name() + "_thermal", "General", Ogre::TEX_TYPE_2D,
    this->ImageWidth(), this->ImageHeight(), 1, 0,
    Ogre::PF_L16, Ogre::TU_RENDERTARGET,
    0, false, 0, Ogre::BLANKSTRING, false, true);

  Ogre::RenderTarget *rt =
    this->dataPtr->ogreThermalTexture->getBuffer()->getRenderTarget();

  // create compositor worksspace
  this->dataPtr->ogreCompositorWorkspace =
      ogreCompMgr->addWorkspace(this->scene->OgreSceneManager(),
      rt, this->ogreCamera, wsDefName, false);

  // add thermal material swticher to render target listener
  // so we can switch to use heat material when the camera is being udpated
  Ogre::CompositorNode *node =
      this->dataPtr->ogreCompositorWorkspace->getNodeSequence()[0];
  auto channels = node->getLocalTextures();
  for (auto c : channels)
  {
    if (c.textures[0]->getSrcFormat() == Ogre::PF_R8G8B8)
    {
      this->dataPtr->thermalMaterialSwitcher.reset(
          new Ogre2ThermalCameraMaterialSwitcher(this->scene));
      c.target->addListener(this->dataPtr->thermalMaterialSwitcher.get());
      break;
    }
  }
}

//////////////////////////////////////////////////
void Ogre2ThermalCamera::Render()
{
  // update the compositors
  this->dataPtr->ogreCompositorWorkspace->setEnabled(true);
  auto engine = Ogre2RenderEngine::Instance();
  engine->OgreRoot()->renderOneFrame();
  this->dataPtr->ogreCompositorWorkspace->setEnabled(false);
}

//////////////////////////////////////////////////
void Ogre2ThermalCamera::PreRender()
{
  if (!this->dataPtr->ogreThermalTexture)
    this->CreateThermalTexture();
}

//////////////////////////////////////////////////
void Ogre2ThermalCamera::PostRender()
{
  if (this->dataPtr->newThermalFrame.ConnectionCount() <= 0u)
    return;

  unsigned int width = this->ImageWidth();
  unsigned int height = this->ImageHeight();

  PixelFormat format = PF_L16;
  Ogre::PixelFormat imageFormat = Ogre2Conversions::Convert(format);

  int len = width * height;
  unsigned int channelCount = PixelUtil::ChannelCount(format);
  unsigned int bytesPerChannel = PixelUtil::BytesPerChannel(format);

  if (!this->dataPtr->thermalBuffer)
  {
    this->dataPtr->thermalBuffer = new uint16_t[len * channelCount];
  }
  Ogre::PixelBox dstBox(width, height,
        1, imageFormat, this->dataPtr->thermalBuffer);

  // blit data from gpu to cpu
  auto rt = this->dataPtr->ogreThermalTexture->getBuffer()->getRenderTarget();
  rt->copyContentsToMemory(dstBox, Ogre::RenderTarget::FB_FRONT);

  if (!this->dataPtr->thermalImage)
  {
    this->dataPtr->thermalImage = new uint16_t[len];
  }

  // fill thermal data
  memcpy(this->dataPtr->thermalImage, this->dataPtr->thermalBuffer,
      height*width*channelCount*bytesPerChannel);

  this->dataPtr->newThermalFrame(
        this->dataPtr->thermalImage, width, height, 1, "L16");

  // Uncomment to debug thermal output
  // igndbg << "wxh: " << width << " x " << height << std::endl;
  // for (unsigned int i = 0; i < height; ++i)
  // {
  //   for (unsigned int j = 0; j < width; ++j)
  //   {
  //     igndbg << "[" << this->dataPtr->thermalImage[i*width + j] << "]";
  //   }
  //   igndbg << std::endl;
  // }
}

//////////////////////////////////////////////////
common::ConnectionPtr Ogre2ThermalCamera::ConnectNewThermalFrame(
    std::function<void(const uint16_t *, unsigned int, unsigned int,
      unsigned int, const std::string &)>  _subscriber)
{
  return this->dataPtr->newThermalFrame.Connect(_subscriber);
}

//////////////////////////////////////////////////
RenderTargetPtr Ogre2ThermalCamera::RenderTarget() const
{
  return this->dataPtr->thermalTexture;
}
