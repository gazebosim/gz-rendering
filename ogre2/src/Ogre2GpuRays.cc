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

#include <ignition/common/Mesh.hh>
#include <ignition/common/MeshManager.hh>
#include <ignition/common/SubMesh.hh>

#include <ignition/math/Color.hh>
#include <ignition/math/Helpers.hh>
#include <ignition/math/Vector3.hh>

#include "ignition/rendering/RenderTypes.hh"
#include "ignition/rendering/ogre2/Ogre2Camera.hh"
#include "ignition/rendering/ogre2/Ogre2GpuRays.hh"
#include "ignition/rendering/ogre2/Ogre2RenderEngine.hh"

/// \internal
namespace ignition
{
namespace rendering
{
  class Ogre2GpuRays1stPassRTListener : public Ogre::RenderTargetListener
  {
    public: Ogre2GpuRays1stPassRTListener(Ogre::SceneNode *_node,
      unsigned int _camIndex, unsigned int _count, double _rotation)
    {
      this->node = _node;
      this->camIndex = _camIndex;
      this->count = _count;
      this->rotation = _rotation;
      this->origQuat = this->node->getOrientation();
      if (this->count == 2)
      {
        this->cameraYaws[0] = -this->rotation / 2.0;
        this->cameraYaws[1] = +this->rotation;
        this->cameraYaws[2] = 0;
        this->cameraYaws[3] = -this->rotation / 2.0;
      }
      else
      {
        this->cameraYaws[0] = -this->rotation;
        this->cameraYaws[1] = +this->rotation;
        this->cameraYaws[2] = +this->rotation;
        this->cameraYaws[3] = -this->rotation;
      }
    }

    public: void preRenderTargetUpdate(
      const Ogre::RenderTargetEvent &/*_evt*/)
    {
      std::cerr << "preRenderTargetUpdate " << rotation << std::endl;
      if (this->count < 1)
        return;

      for (unsigned int i = 0; i <= this->camIndex; ++i)
        this->node->roll(Ogre::Radian(this->cameraYaws[i]));
    };

    public: void postRenderTargetUpdate(
      const Ogre::RenderTargetEvent &/*_evt*/)
    {
      if (this->count < 1)
        return;

      this->node->setOrientation(this->origQuat);
    };

    private: Ogre::SceneNode *node = nullptr;
    private: unsigned int camIndex = 0u;
    private: unsigned int count = 0u;
    private: double rotation = 0.0;
    private: Ogre::Quaternion origQuat = Ogre::Quaternion::IDENTITY;
    private: double cameraYaws[4];
  };
}
}


/// \internal
/// \brief Private data for the Ogre2GpuRays class
class ignition::rendering::Ogre2GpuRaysPrivate
{
  /// \brief Event triggered when new gpu rays range data are available.
  /// \param[in] _frame New frame containing raw gpu rays data.
  /// \param[in] _width Width of frame.
  /// \param[in] _height Height of frame.
  /// \param[in] _depth Max depth of frame.
  /// \param[in] _format Format of frame.
  public: ignition::common::EventT<void(const float *,
               unsigned int, unsigned int, unsigned int,
               const std::string &)> newGpuRaysFrame;

  /// \brief Raw buffer of gpu rays data.
  public: float *gpuRaysBuffer = nullptr;

  /// \brief Outgoing gpu rays data, used by newGpuRaysFrame event.
  public: float *gpuRaysScan = nullptr;

  /// \brief Pointer to Ogre material for the first rendering pass.
  public: Ogre::Material *matFirstPass = nullptr;

  /// \brief Pointer to Ogre material for the sencod rendering pass.
  public: Ogre::Material *matSecondPass = nullptr;

  public: Ogre::Material *matCompose = nullptr;
  public: Ogre::TexturePtr composeTexture ;
  public: Ogre::TexturePtr cubemapTexture;
  public: Ogre::Camera *cubemapCam = nullptr;
  public: Ogre::CompositorWorkspace *ogreCompositorWorkspace = nullptr;
  public: Ogre::CompositorWorkspace *ogreCompositorWorkspaceCubemap = nullptr;

  public: Ogre::CompositorWorkspace *ogreCompositorWorkspace1st[3];
  public: Ogre::CompositorWorkspace *ogreCompositorWorkspace2nd;


  /// \brief Temporary pointer to the current material.
  public: Ogre::Material *currentMat = nullptr;

  /// \brief An array of first pass textures.
  public: Ogre::Texture *firstPassTextures[3];

  public: Ogre2GpuRays1stPassRTListener *rtListeners1st[3];

  /// \brief Second pass texture.
  public: Ogre::Texture *secondPassTexture = nullptr;

  /// \brief Temporary pointer to the current render target.
  public: Ogre::Texture *currentTexture = nullptr;

  /// \brief Ogre orthorgraphic camera used in the second pass for
  /// undistortion.
  public: Ogre::Camera *orthoCam = nullptr;

  /// \brief Pointer to the ogre camera
  public: Ogre::Camera *ogreCamera = nullptr;

  /// \brief Ogre scenenode where the orthorgraphic camera is attached to.
  public: Ogre::SceneNode *pitchNodeOrtho = nullptr;

  /// \brief Ogre mesh used to create a canvas for undistorting range values
  /// in the second rendering pass.
  public: common::Mesh *undistMesh = nullptr;

  /// \brief Pointer to visual that holds the canvas.
  public: VisualPtr visual;

  /// \brief Number of first pass textures.
  public: unsigned int textureCount = 0u;

  /// \brief A list of camera angles for first pass rendering.
//  public: double cameraYaws[4];

  /// \brief Image width of first pass.
  public: unsigned int w1st = 0u;

  /// \brief Image height of first pass.
  public: unsigned int h1st = 0u;

  /// \brief Image width of second pass.
  public: unsigned int w2nd = 0u;

  /// \brief Image height of second pass.
  public: unsigned int h2nd = 0u;

  /// \brief Amount of channels used to store the data
  // r = depth, g = retro, and b = n/a
  public: unsigned int channels = 3u;

  /// \brief List of texture unit indices used during the second
  /// rendering pass.
  public: std::vector<int> texIdx;

  /// Number of second pass texture units created.
  public: unsigned int texCount = 0;

  /// \brief Dummy render texture for the gpu rays
  public: RenderTexturePtr renderTexture;

  /// \brief Cos horizontal field-of-view.
  public: double chfov = 0;

  /// \brief Cos vertical field-of-view.
  public: double cvfov = 0;

  /// \brief Horizontal half angle.
  public: double horzHalfAngle = 0;

  /// \brief Vertical half angle.
  public: double vertHalfAngle = 0;

  /// \brief Number of cameras needed to generate the rays.
  public: unsigned int cameraCount = 1;
};

using namespace ignition;
using namespace rendering;

//////////////////////////////////////////////////
Ogre2GpuRays::Ogre2GpuRays()
  : dataPtr(new Ogre2GpuRaysPrivate)
{
}

//////////////////////////////////////////////////
Ogre2GpuRays::~Ogre2GpuRays()
{
  this->Destroy();
}

//////////////////////////////////////////////////
void Ogre2GpuRays::Init()
{
  BaseGpuRays::Init();

  // create internal camera for 1st pass
  this->CreateCamera();

  // create dummy render texture
  this->CreateRenderTexture();
}

//////////////////////////////////////////////////
void Ogre2GpuRays::Destroy()
{
  if (this->dataPtr->gpuRaysBuffer)
  {
    delete [] this->dataPtr->gpuRaysBuffer;
    this->dataPtr->gpuRaysBuffer = nullptr;
  }

  if (this->dataPtr->gpuRaysScan)
  {
    delete [] this->dataPtr->gpuRaysScan;
    this->dataPtr->gpuRaysScan = nullptr;
  }

  for (unsigned int i = 0; i < this->dataPtr->textureCount; ++i)
  {
    if (this->dataPtr->firstPassTextures[i])
    {
      Ogre::TextureManager::getSingleton().remove(
          this->dataPtr->firstPassTextures[i]->getName());
      this->dataPtr->firstPassTextures[i] = nullptr;
    }
  }

  if (this->dataPtr->secondPassTexture)
  {
    Ogre::TextureManager::getSingleton().remove(
        this->dataPtr->secondPassTexture->getName());
    this->dataPtr->secondPassTexture = nullptr;
  }

  if (this->dataPtr->matSecondPass)
  {
    Ogre::MaterialManager::getSingleton().remove(
        this->dataPtr->matSecondPass->getName());
    this->dataPtr->matSecondPass = nullptr;
  }

  if (this->scene && this->dataPtr->orthoCam)
  {
    this->scene->OgreSceneManager()->destroyCamera(this->dataPtr->orthoCam);
    this->dataPtr->orthoCam = nullptr;
  }

  this->dataPtr->visual.reset();
  this->dataPtr->texIdx.clear();
  this->dataPtr->texCount = 0u;
}

/////////////////////////////////////////////////
void Ogre2GpuRays::CreateRenderTexture()
{
  RenderTexturePtr base = this->scene->CreateRenderTexture();
  this->dataPtr->renderTexture =
      std::dynamic_pointer_cast<Ogre2RenderTexture>(base);
  this->dataPtr->renderTexture->SetWidth(1);
  this->dataPtr->renderTexture->SetHeight(1);
}

/////////////////////////////////////////////////
void Ogre2GpuRays::CreateCamera()
{
  // Create ogre camera object
  Ogre::SceneManager *ogreSceneManager  = this->scene->OgreSceneManager();
  if (ogreSceneManager == nullptr)
  {
    ignerr << "Scene manager cannot be obtained" << std::endl;
    return;
  }

  this->dataPtr->ogreCamera = ogreSceneManager->createCamera(
      this->Name() + "_Camera");
  if (this->dataPtr->ogreCamera == nullptr)
  {
    ignerr << "Ogre camera cannot be created" << std::endl;
    return;
  }

  // by default, ogre2 cameras are attached to root scene node
  this->dataPtr->ogreCamera->detachFromParent();
  this->ogreNode->attachObject(this->dataPtr->ogreCamera);
  this->dataPtr->ogreCamera->setFixedYawAxis(false);
  this->dataPtr->ogreCamera->yaw(Ogre::Degree(-90));
  this->dataPtr->ogreCamera->roll(Ogre::Degree(-90));
  this->dataPtr->ogreCamera->setAutoAspectRatio(true);
}

/////////////////////////////////////////////////
void Ogre2GpuRays::ConfigureCameras()
{
  // horizontal gpu rays setup
  this->SetHFOV(this->AngleMax() - this->AngleMin());

  if (this->HFOV().Radian() > 2.0 * IGN_PI)
  {
    this->SetHFOV(2.0 * IGN_PI);
    ignwarn << "Horizontal FOV for GPU rays is capped at 180 degrees.\n";
  }

  this->SetHorzHalfAngle((this->AngleMax() + this->AngleMin()).Radian() / 2.0);

  // determine number of cameras to use
  if (this->HFOV().Radian() > 2.8)
  {
    if (this->HFOV().Radian()  > 5.6)
    {
      this->dataPtr->cameraCount = 3;
    }
    else
    {
      this->dataPtr->cameraCount = 2;
    }
  }
  else
  {
    this->dataPtr->cameraCount = 1;
  }

  // horizontal fov of single frame
  this->SetHFOV(this->HFOV().Radian() / this->dataPtr->cameraCount);
  this->SetCosHorzFOV(this->HFOV().Radian());

  // Fixed minimum resolution of texture to reduce steps in ranges
  // when hitting surfaces where the angle between ray and surface is small.
  // Also have to keep in mind the GPU's max. texture size
  unsigned int horzRangeCountPerCamera =
      std::max(2048U, this->RangeCount() / this->dataPtr->cameraCount);
  unsigned int vertRangeCountPerCamera = this->VerticalRangeCount();

  // vertical laser setup
  double vfovAngle;

  if (this->VerticalRangeCount() > 1)
  {
    vfovAngle = (this->VerticalAngleMax() - this->VerticalAngleMin()).Radian();
  }
  else
  {
    vfovAngle = 0;

    if (this->VerticalAngleMax() != this->VerticalAngleMin())
    {
      ignwarn << "Only one vertical ray but vertical min. and max. angle "
          "are not equal. Min. angle is used.\n";
      this->SetVerticalAngleMax(this->VerticalAngleMin().Radian());
    }
  }

  if (vfovAngle > IGN_PI / 2.0)
  {
    vfovAngle = IGN_PI / 2.0;
    ignwarn << "Vertical FOV for GPU laser is capped at 90 degrees.\n";
  }

  this->SetVFOV(vfovAngle);
  this->SetVertHalfAngle((this->VerticalAngleMax()
                   + this->VerticalAngleMin()).Radian() / 2.0);

  this->SetVerticalAngleMin(this->VertHalfAngle() - (vfovAngle / 2));
  this->SetVerticalAngleMax(this->VertHalfAngle() + (vfovAngle / 2));

  // Assume camera always stays horizontally even if vert. half angle of
  // laser is not 0. Add padding to camera vfov.
  double vfovCamera = vfovAngle + 2.0 * std::abs(this->VertHalfAngle());

  // Add padding to vertical camera FOV to cover all possible rays
  // for given laser vert. and horiz. FOV
  vfovCamera = 2.0 * atan(tan(vfovCamera / 2.0) / cos(
        this->HFOV().Radian() / 2.0));

  if (vfovCamera > 2.8)
  {
    ignerr << "Vertical FOV of internal camera exceeds 2.8 radians.\n";
  }

  this->SetCosVertFOV(vfovCamera);

  // If vertical ray is not 1 adjust horizontal and vertical
  // ray count to maintain aspect ratio
  if (this->vSamples > 1)
  {
    double cameraAspectRatio =
      tan(this->HFOV().Radian() / 2.0) / tan(vfovCamera / 2.0);

    this->SetRayCountRatio(cameraAspectRatio);
    this->rangeCountRatio = cameraAspectRatio;

    if ((horzRangeCountPerCamera / this->rangeCountRatio) >
         vertRangeCountPerCamera)
    {
      vertRangeCountPerCamera =
          std::round(horzRangeCountPerCamera / this->rangeCountRatio);
    }
    else
    {
      horzRangeCountPerCamera =
          round(vertRangeCountPerCamera * this->rangeCountRatio);
    }
  }
  else
  {
    // In case of 1 vert. ray, set a very small vertical FOV for camera
    this->SetRayCountRatio(horzRangeCountPerCamera);
    double camVFOV = 2.0 * atan(tan(
          this->HFOV().Radian() / 2.0) / this->RayCountRatio());
    this->SetCosVertFOV(camVFOV);
  }

  // Configure first pass texture size
  this->Set1stTextureSize(horzRangeCountPerCamera, vertRangeCountPerCamera);
  // Configure second pass texture size
  this->SetRangeCount(this->RangeCount(), this->VerticalRangeCount());

  // Set ogre cam properties
  this->dataPtr->ogreCamera->setAspectRatio(this->RayCountRatio());
  this->dataPtr->ogreCamera->setFOVy(Ogre::Radian((this->CosVertFOV())));
  this->dataPtr->ogreCamera->setNearClipDistance(this->NearClipPlane());
  this->dataPtr->ogreCamera->setFarClipDistance(this->FarClipPlane());
  this->dataPtr->ogreCamera->setRenderingDistance(this->FarClipPlane());
  this->dataPtr->ogreCamera->yaw(Ogre::Radian(this->HorzHalfAngle()));
}

/////////////////////////////////////////////////////////
void Ogre2GpuRays::CreateCubemap()
{
  auto engine = Ogre2RenderEngine::Instance();
  auto ogreRoot = engine->OgreRoot();
  Ogre::CompositorManager2 *ogreCompMgr = ogreRoot->getCompositorManager2();


  std::string cubemapTexName = "cubemap_" + this->Name();
  this->dataPtr->cubemapTexture =
      Ogre::TextureManager::getSingleton().createManual(
      cubemapTexName, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
      Ogre::TEX_TYPE_CUBE_MAP, 1024, 1024,
      Ogre::PixelUtil::getMaxMipmapCount( 1024, 1024, 1 ),
      Ogre::PF_A8B8G8R8, Ogre::TU_RENDERTARGET | Ogre::TU_AUTOMIPMAP, 0, false);

//      1, Ogre::PF_FLOAT32_R, Ogre::TU_RENDERTARGET, 0, false);
//      1, Ogre::PF_B8G8R8, Ogre::TU_RENDERTARGET, 0, false);

  // Create the camera used to render to our cubemap
  Ogre::SceneManager *ogreSceneManager = this->scene->OgreSceneManager();
  this->dataPtr->cubemapCam =
      ogreSceneManager->createCamera("CubeMapCamera", true, true);
  this->dataPtr->cubemapCam->setFOVy(Ogre::Degree(90));
  this->dataPtr->cubemapCam->setAspectRatio(1);
  this->dataPtr->cubemapCam->setFixedYawAxis(false);
//  this->dataPtr->cubemapCam->setNearClipDistance(this->NearClipPlane());
//  this->dataPtr->cubemapCam->setFarClipDistance(this->FarClipPlane());
  this->dataPtr->cubemapCam->setFarClipDistance(10000);

//  this->dataPtr->cubemapCam->yaw(Ogre::Degree(-90));
//  this->dataPtr->cubemapCam->roll(Ogre::Degree(-90));
  // this->dataPtr->cubemapCam->setPosition( 0, 1.0, 0 );

/*  std::string cubemapWsDefName = "GpuRaysCubemapWorkspace";
  this->dataPtr->ogreCompositorWorkspaceCubemap =
      ogreCompMgr->addWorkspace(this->scene->OgreSceneManager(),
      this->dataPtr->cubemapTexture->getBuffer(0)->getRenderTarget(),
      this->dataPtr->cubemapCam, cubemapWsDefName, true);
*/

  // create cubemap compositor
  Ogre::CompositorChannelVec cubemapExternalChannels(1);
  // any of the cubemap's render targets will do
  cubemapExternalChannels[0].target =
      this->dataPtr->cubemapTexture->getBuffer(0)->getRenderTarget();
  cubemapExternalChannels[0].textures.push_back(this->dataPtr->cubemapTexture);

  std::string cubemapWsDefName = "GpuRaysCubemapWorkspace";
  if( !ogreCompMgr->hasWorkspaceDefinition(cubemapWsDefName))
  {
    Ogre::CompositorWorkspaceDef *workspaceDef =
        ogreCompMgr->addWorkspaceDefinition(cubemapWsDefName);
    // "CubemapRendererNode" has been defined in scripts.
    workspaceDef->connectExternal(0, "CubemapRendererNode", 0);
    std::cerr << "cureating GpuRaysCubemapWorkspace and CubemapRendererNode " << std::endl;
  }

  Ogre::ResourceLayoutMap initialCubemapLayouts;
  Ogre::ResourceAccessMap initialCubemapUavAccess;
  this->dataPtr->ogreCompositorWorkspaceCubemap =
      ogreCompMgr->addWorkspace(this->scene->OgreSceneManager(),
      cubemapExternalChannels, this->dataPtr->cubemapCam, cubemapWsDefName,
      true, -1, (Ogre::UavBufferPackedVec *)0, &initialCubemapLayouts,
      &initialCubemapUavAccess);

  // create output render texture
  std::string texName = this->Name() + "_compose";
  this->dataPtr->composeTexture =
    Ogre::TextureManager::getSingleton().createManual(
    texName, "General", Ogre::TEX_TYPE_2D,
    this->dataPtr->w2nd, this->dataPtr->h2nd, 0,
    Ogre::PF_FLOAT32_RGB, Ogre::TU_RENDERTARGET);

/*  this->dataPtr->matCompose = dynamic_cast<Ogre::Material *>(
      Ogre::MaterialManager::getSingleton().getByName("GpuRaysCompose").get());
  this->dataPtr->matCompose->load();
  Ogre::Pass *pass = this->dataPtr->matCompose->getTechnique(0)->getPass(0);
  auto texUnit = pass->getTextureUnitState(0);
  texUnit->setTexture(this->dataPtr->cubemapTexture);
  std::cerr << "tex unit state name " <<  texUnit->getName() << std::endl;
//  Ogre::GpuProgramParametersSharedPtr psParams =
//      pass->getFragmentProgramParameters();



  std::string wsDefName = "GpuRaysComposeWorkspace";
  this->dataPtr->ogreCompositorWorkspace =
      ogreCompMgr->addWorkspace(this->scene->OgreSceneManager(),
      this->dataPtr->composeTexture->getBuffer()->getRenderTarget(),
      this->dataPtr->ogreCamera, wsDefName, true);
*/


  // create final RT compositor
  auto rt = this->dataPtr->composeTexture->getBuffer()->getRenderTarget();
  Ogre::CompositorChannelVec externalChannels(2) ;
  externalChannels[0].target = rt;
  externalChannels[1].target =
      this->dataPtr->cubemapTexture->getBuffer(0)->getRenderTarget();
  externalChannels[1].textures.push_back(this->dataPtr->cubemapTexture);

  std::string wsDefName = "GpuRaysComposeWorkspace";
  this->dataPtr->ogreCompositorWorkspace =
      ogreCompMgr->addWorkspace(this->scene->OgreSceneManager(),
      externalChannels, this->dataPtr->ogreCamera, wsDefName, true,
      -1, (Ogre::UavBufferPackedVec *)0, &initialCubemapLayouts,
      &initialCubemapUavAccess);

//  // Setup the viewport to use the texture
//  Ogre::Viewport *vp = rt->getViewport(0);
//  vp->setOverlaysEnabled(false);
//  vp->_setVisibilityMask(IGN_VISIBILITY_ALL &
//      ~(IGN_VISIBILITY_GUI | IGN_VISIBILITY_SELECTABLE),
//      vp->getLightVisibilityMask());


/*  double projectionA = this->FarClipPlane() /
      (this->FarClipPlane() - this->NearClipPlane());
  double projectionB = (-this->FarClipPlane() * this->NearClipPlane()) /
      (this->FarClipPlane() - this->NearClipPlane());
  psParams->setNamedConstant("projectionParams",
      Ogre::Vector2(projectionA, projectionB));

  psParams->setNamedConstant("near",
      static_cast<float>(this->NearClipPlane()));
  psParams->setNamedConstant("far",
      static_cast<float>(this->FarClipPlane()));

  psParams->setNamedConstant("max",
      static_cast<float>(this->dataMaxVal));
  psParams->setNamedConstant("min",
      static_cast<float>(this->dataMinVal));
*/

/*
  std::string cubemapTexName = "cubemap_" + this->Name();
  this->dataPtr->cubemapTexture =
    Ogre::TextureManager::getSingleton().createManual(
    cubemapTexName, "General", Ogre::TEX_TYPE_2D,
    this->dataPtr->w2nd, this->dataPtr->h2nd, 0,
    Ogre::PF_FLOAT32_RGB, Ogre::TU_RENDERTARGET);

  auto engine = Ogre2RenderEngine::Instance();
  auto ogreRoot = engine->OgreRoot();
  Ogre::CompositorManager2 *ogreCompMgr = ogreRoot->getCompositorManager2();

  auto rt = this->dataPtr->cubemapTexture->getBuffer()->getRenderTarget();
  std::string wsDefName = "GpuRaysCubemapWorkspace";
  this->dataPtr->ogreCompositorWorkspaceCubemap =
      ogreCompMgr->addWorkspace(this->scene->OgreSceneManager(),
      rt, this->dataPtr->ogreCamera, wsDefName, false);
*/




}

/////////////////////////////////////////////////////////
void Ogre2GpuRays::CreateGpuRaysTextures()
{
  this->ConfigureCameras();

  this->CreateCubemap();

  this->CreateOrthoCam();

  this->dataPtr->textureCount = this->dataPtr->cameraCount;

  double camYaw = this->HFOV().Radian();
/*  if (this->dataPtr->textureCount == 2)
  {
    this->dataPtr->cameraYaws[0] = -this->HFOV().Radian() / 2.0;
    this->dataPtr->cameraYaws[1] = +this->HFOV().Radian();
    this->dataPtr->cameraYaws[2] = 0;
    this->dataPtr->cameraYaws[3] = -this->HFOV().Radian() / 2.0;
    rotation = this->HFOV().Radian() / 2.0;
  }
  else
  {
    this->dataPtr->cameraYaws[0] = -this->HFOV().Radian();
    this->dataPtr->cameraYaws[1] = +this->HFOV().Radian();
    this->dataPtr->cameraYaws[2] = +this->HFOV().Radian();
    this->dataPtr->cameraYaws[3] = -this->HFOV().Radian();
    rotation = this->HFOV().Radian();
  }
*/

  // create compositor workspace for rendering
  auto engine = Ogre2RenderEngine::Instance();
  auto ogreRoot = engine->OgreRoot();
  Ogre::CompositorManager2 *ogreCompMgr = ogreRoot->getCompositorManager2();

  // create 1st pass compositor
  // TODO create workspace programmatically
  //std::string wsDefName = "GpuRays1stPass_" + this->Name();
  std::string wsDefName = "GpuRays1stPassWorkspace";
/*  Ogre::CompositorWorkspaceDef *wsDef = nullptr;
  if (!ogreCompMgr->hasWorkspaceDefinition(wsDefName))
  {
    Ogre::CompositorNodeDef *nodeDef = ogreCompMgr->addNodeDefinition(
        "GpuRays_" + Ogre::IdString(wsDefName + "/Node").getReleaseText());
    // Input texture
    nodeDef->addTextureSourceName("rt_input", 0, Ogre::TextureDefinitionBase::TEXTURE_INPUT);
    nodeDef->setNumTargetPass(1);
    {
      Ogre::CompositorTargetDef *targetDef = nodeDef->addTargetPass("rt_input");
      targetDef->setNumPasses(2);
      {
        {
          Ogre::CompositorPassClearDef *passClear =
              static_cast<Ogre::CompositorPassClearDef *>(
              targetDef->addPass(Ogre::PASS_CLEAR));
          // passClear->mColourValue = Ogre::ColourValue(this->dataMaxVal, 0, 1.0);
          passClear->mColourValue = Ogre::ColourValue(1.32, 0, 1.0);
        }
        {
          Ogre::CompositorPassQuadDef *passQuad =
              static_cast<Ogre::CompositorPassQuadDef *>(
              targetDef->addPass(Ogre::PASS_QUAD));
          passQuad->mMaterialName = "GpuRaysScan1st";
          passQuad->addQuadTextureSource( 0, "rt_input", 0 );
       }
      }
    }

    Ogre::CompositorWorkspaceDef *workDef = ogreCompMgr->addWorkspaceDefinition(wsDefName);
    workDef->connectExternal(0, nodeDef->getName(), 0);
  }
  wsDef = ogreCompMgr->getWorkspaceDefinition(wsDefName);
*/

  std::string wsDefName2nd = "GpuRays2ndPassWorkspace";


  // Configure first pass textures that are not yet configured properly
  std::cerr << "configure first pass tex count " << this->dataPtr->textureCount << std::endl;
  for (unsigned int i = 0; i < this->dataPtr->textureCount; ++i)
  {
    // create render texture
    std::stringstream texName;
    texName << this->Name() << "_first_pass_" << i;
    this->dataPtr->firstPassTextures[i] =
      Ogre::TextureManager::getSingleton().createManual(
      texName.str(), "General", Ogre::TEX_TYPE_2D,
      this->dataPtr->w1st, this->dataPtr->h1st, 0,
      Ogre::PF_FLOAT32_RGB, Ogre::TU_RENDERTARGET).get();

    // add rt listener
    Ogre::RenderTarget *rt =
        this->dataPtr->firstPassTextures[i]->getBuffer()->getRenderTarget();
/*    this->dataPtr->rtListeners1st[i] = new Ogre2GpuRays1stPassRTListener(
        this->Node(), i, this->dataPtr->textureCount, camYaw);
    rt->addListener(this->dataPtr->rtListeners1st[i]);
*/
//    rt->setAutoUpdated(false);

    // create compositor
    const Ogre::String workspaceName = "GpuRays1stPass_" +
        this->dataPtr->ogreCamera->getName() + "_" + std::to_string(i);
    this->dataPtr->ogreCompositorWorkspace1st[i] =
        ogreCompMgr->addWorkspace(this->scene->OgreSceneManager(),
        rt, this->dataPtr->ogreCamera, wsDefName, false);
/*    auto nodeSeq = this->dataPtr->ogreCompositorWorkspace1st[i]->getNodeSequence();
    auto pass = nodeSeq[0]->_getPasses()[0]->getDefinition();
    auto clearPass = dynamic_cast<const Ogre::CompositorPassClearDef *>(pass);
    pass = nodeSeq[0]->_getPasses()[1]->getDefinition();
    // set out of range max value
    const_cast<Ogre::CompositorPassClearDef *>(clearPass)->mColourValue =
        Ogre::ColourValue(this->dataMaxVal, 0.0, 1.0);
    // set visibility mask
    auto quadPass = dynamic_cast<const Ogre::CompositorPassQuadDef *>(pass);
    const_cast<Ogre::CompositorPassQuadDef *>(quadPass)->mVisibilityMask =
        IGN_VISIBILITY_ALL & ~(IGN_VISIBILITY_GUI | IGN_VISIBILITY_SELECTABLE);
*/

    // Setup the viewport to use the texture
    Ogre::Viewport *vp = rt->getViewport(0);
    vp->setOverlaysEnabled(false);
    vp->_setVisibilityMask(IGN_VISIBILITY_ALL &
        ~(IGN_VISIBILITY_GUI | IGN_VISIBILITY_SELECTABLE),
        vp->getLightVisibilityMask());
  }

  this->dataPtr->matFirstPass = dynamic_cast<Ogre::Material *>(
      Ogre::MaterialManager::getSingleton().getByName("GpuRaysScan1st").get());
  this->dataPtr->matFirstPass->load();
  Ogre::Pass *pass = this->dataPtr->matFirstPass->getTechnique(0)->getPass(0);
  Ogre::GpuProgramParametersSharedPtr psParams =
      pass->getFragmentProgramParameters();

  double projectionA = this->FarClipPlane() /
      (this->FarClipPlane() - this->NearClipPlane());
  double projectionB = (-this->FarClipPlane() * this->NearClipPlane()) /
      (this->FarClipPlane() - this->NearClipPlane());
//  projectionB /= this->FarClipPlane();
  psParams->setNamedConstant("projectionParams",
      Ogre::Vector2(projectionA, projectionB));

  psParams->setNamedConstant("near",
      static_cast<float>(this->NearClipPlane()));
  psParams->setNamedConstant("far",
      static_cast<float>(this->FarClipPlane()));

  psParams->setNamedConstant("max",
      static_cast<float>(this->dataMaxVal));
  psParams->setNamedConstant("min",
      static_cast<float>(this->dataMinVal));



//  this->dataPtr->matFirstPass->setCullingMode(Ogre::CULL_NONE);


  // Configure second pass texture
  this->dataPtr->secondPassTexture =
      Ogre::TextureManager::getSingleton().createManual(
      this->Name() + "_second_pass",
      "General",
      Ogre::TEX_TYPE_2D,
      this->dataPtr->w2nd, this->dataPtr->h2nd, 0,
      Ogre::PF_FLOAT32_RGB,
      Ogre::TU_RENDERTARGET).get();

  Ogre::RenderTarget *rt =
      this->dataPtr->secondPassTexture->getBuffer()->getRenderTarget();

  // Set GpuRaysScan2nd material
  this->dataPtr->matSecondPass = dynamic_cast<Ogre::Material *>(
      Ogre::MaterialManager::getSingleton().getByName("GpuRaysScan2nd").get());
  // clone the material since we're modifying it's definitions
  // TODO cloning disabled for dev
//  this->dataPtr->matSecondPass = this->dataPtr->matSecondPass->clone(
//      this->Name() + "_" + this->dataPtr->matSecondPass->getName()).get();
  this->dataPtr->matSecondPass->load();

  Ogre::Technique *technique =
    this->dataPtr->matSecondPass->getTechnique(0);
  IGN_ASSERT(technique,
      "Ogre2GpuRays material script error: technique not found");

  Ogre::Pass *mat2ndPass = technique->getPass(0);
  IGN_ASSERT(mat2ndPass,
      "Ogre2GpuRays material script error: pass not found");
//  pass->removeAllTextureUnitStates();
  Ogre::TextureUnitState *texUnit = nullptr;
  for (unsigned int i = 0; i < this->dataPtr->textureCount; ++i)
  {
    unsigned int texIndex = 1 +  this->dataPtr->texCount++;
    {
      std::string texName = this->dataPtr->firstPassTextures[i]->getName();
      texUnit = mat2ndPass->createTextureUnitState(texName, texIndex);
      // texUnit = pass->getTextureUnitState(texIndex);
      auto tex = Ogre::TextureManager::getSingleton().getByName(texName);
      texUnit->setTexture(tex);

     std::cerr << "tex unit: " << texIndex << ": "
        << tex->getName() << " " << (tex) << std::endl;

      this->dataPtr->texIdx.push_back(texIndex);

//      texUnit->setTextureFiltering(Ogre::TFO_NONE);
//      texUnit->setTextureAddressingMode(Ogre::TextureUnitState::TAM_MIRROR);
    }
  }

  double orthoNear = this->dataPtr->orthoCam->getNearClipDistance();
  double orthoFar = this->dataPtr->orthoCam->getFarClipDistance();
  double orthoProjectionA =  orthoFar / (orthoFar - orthoNear);
  double orthoProjectionB = (-orthoFar * orthoNear) /
      (orthoFar - orthoNear);
//  projectionB /= this->FarClipPlane();
  Ogre::GpuProgramParametersSharedPtr mat2ndPsParams =
      mat2ndPass->getFragmentProgramParameters();
  mat2ndPsParams->setNamedConstant("projectionParams",
      Ogre::Vector2(orthoProjectionA, orthoProjectionB));




  // create 2nd pass compositor
  const Ogre::String workspaceName = "GpuRays2ndPass_" +
      this->dataPtr->orthoCam->getName();
  this->dataPtr->ogreCompositorWorkspace2nd =
      ogreCompMgr->addWorkspace(this->scene->OgreSceneManager(),
      rt, this->dataPtr->orthoCam, wsDefName2nd, false);

/*  // Setup the viewport
  Ogre::Viewport *vp = rt->getViewport(0);
  vp->setOverlaysEnabled(false);
  vp->_setVisibilityMask(IGN_VISIBILITY_ALL &
      ~(IGN_VISIBILITY_GUI | IGN_VISIBILITY_SELECTABLE),
      vp->getLightVisibilityMask());
*/


//  this->CreateCanvas();
}

/////////////////////////////////////////////////
void Ogre2GpuRays::UpdateRenderTargetCubemap()
{
//  this->dataPtr->ogreCompositorWorkspaceCubemap->setEnabled(true);
//  this->dataPtr->ogreCompositorWorkspace->setEnabled(true);
  auto engine = Ogre2RenderEngine::Instance();
  engine->OgreRoot()->renderOneFrame();
//  this->dataPtr->ogreCompositorWorkspace->setEnabled(false);
//  this->dataPtr->ogreCompositorWorkspaceCubemap->setEnabled(false);
//  return;

  // ----------------------- testing
  unsigned int width = this->dataPtr->w2nd;
  unsigned int height = this->dataPtr->h2nd;

  size_t size = Ogre::PixelUtil::getMemorySize(
    width, height, 1, Ogre::PF_FLOAT32_RGB);
  int len = width * height * this->dataPtr->channels;

  std::cerr << "wxh: " << width << " x " << height << std::endl;

  if (!this->dataPtr->gpuRaysBuffer)
  {
    this->dataPtr->gpuRaysBuffer = new float[len];
  }
  Ogre::PixelBox dstBox(width, height,
        1, Ogre::PF_FLOAT32_RGB, this->dataPtr->gpuRaysBuffer);

//  this->dataPtr->cubemapTexture->getBuffer()->getRenderTarget()->copyContentsToMemory(dstBox,
  this->dataPtr->composeTexture->getBuffer()->getRenderTarget()->copyContentsToMemory(dstBox,
      Ogre::RenderTarget::FB_FRONT);

  for (unsigned int i = 0; i < height; ++i)
  {
    for (unsigned int j = 0; j < width; ++j)
    {
      std::cerr << this->dataPtr->gpuRaysBuffer[i*width*3 + j*3] <<  " "
                << this->dataPtr->gpuRaysBuffer[i*width*3 + j*3 +1] <<  " "
                << this->dataPtr->gpuRaysBuffer[i*width*3 + j*3 +2] <<  " | ";

    }
    std::cerr << std::endl;
  }
  std::cerr << std::endl;
}

/////////////////////////////////////////////////
void Ogre2GpuRays::UpdateRenderTarget1stPass()
{
  // manual update
  for (unsigned int i = 0; i < this->dataPtr->textureCount; ++i)
  {
    this->dataPtr->ogreCompositorWorkspace1st[i]->setEnabled(true);
  }
  auto engine = Ogre2RenderEngine::Instance();
  engine->OgreRoot()->renderOneFrame();
  for (unsigned int i = 0; i < this->dataPtr->textureCount; ++i)
  {
    this->dataPtr->ogreCompositorWorkspace1st[i]->setEnabled(false);
  }

  unsigned int width = this->dataPtr->w1st;
  unsigned int height = this->dataPtr->h1st;

  size_t size = Ogre::PixelUtil::getMemorySize(
    width, height, 1, Ogre::PF_FLOAT32_RGB);
  int len = width * height * this->dataPtr->channels;

  std::cerr << "wxh: " << width << " x " << height << std::endl;

  if (!this->dataPtr->gpuRaysBuffer)
  {
    this->dataPtr->gpuRaysBuffer = new float[len];
  }
  Ogre::PixelBox dstBox(width, height,
        1, Ogre::PF_FLOAT32_RGB, this->dataPtr->gpuRaysBuffer);

//  auto pixelBuffer = this->dataPtr->firstPassTextures[0]->getBuffer();
//  pixelBuffer->blitToMemory(dstBox);

  this->dataPtr->firstPassTextures[0]->getBuffer()->getRenderTarget()->copyContentsToMemory(dstBox,
      Ogre::RenderTarget::FB_FRONT);


  for (unsigned int i = 0; i < height; ++i)
  {
    for (unsigned int j = 0; j < width; ++j)
    {
      std::cerr << this->dataPtr->gpuRaysBuffer[i*width*3 + j*3] <<  " ";
    }
    std::cerr << std::endl;
  }
  std::cerr << std::endl;


}

/////////////////////////////////////////////////
void Ogre2GpuRays::UpdateRenderTarget2ndPass()
{
  this->dataPtr->ogreCompositorWorkspace2nd->setEnabled(true);
//  this->dataPtr->visual->SetVisible(true);
  auto engine = Ogre2RenderEngine::Instance();
  engine->OgreRoot()->renderOneFrame();
//  this->dataPtr->visual->SetVisible(false);
  this->dataPtr->ogreCompositorWorkspace2nd->setEnabled(false);

  // ----------------------- testing
/*  unsigned int width = this->dataPtr->w2nd;
  unsigned int height = this->dataPtr->h2nd;

  size_t size = Ogre::PixelUtil::getMemorySize(
    width, height, 1, Ogre::PF_FLOAT32_RGB);
  int len = width * height * this->dataPtr->channels;

  std::cerr << " ortho derived dir " << this->dataPtr->orthoCam->getDerivedDirection() << std::endl;
  std::cerr << " ortho derived pos " << this->dataPtr->orthoCam->getDerivedPosition() << std::endl;

  Ogre::SceneNode *visualSceneNode =  std::dynamic_pointer_cast<
      ignition::rendering::Ogre2Node>(this->dataPtr->visual)->Node();

  std::cerr << " visual obj pos " << visualSceneNode->_getDerivedPosition() << std::endl;

  std::cerr << "wxh: " << width << " x " << height << std::endl;

  if (!this->dataPtr->gpuRaysBuffer)
  {
    this->dataPtr->gpuRaysBuffer = new float[len];
  }
  Ogre::PixelBox dstBox(width, height,
        1, Ogre::PF_FLOAT32_RGB, this->dataPtr->gpuRaysBuffer);

  this->dataPtr->secondPassTexture->getBuffer()->getRenderTarget()->copyContentsToMemory(dstBox,
      Ogre::RenderTarget::FB_FRONT);

  for (unsigned int i = 0; i < height; ++i)
  {
    for (unsigned int j = 0; j < width; ++j)
    {
      std::cerr << this->dataPtr->gpuRaysBuffer[i*width*3 + j*3] <<  " ";
    }
    std::cerr << std::endl;
  }
  std::cerr << std::endl;
*/

}


/////////////////////////////////////////////////
void Ogre2GpuRays::UpdateRenderTarget(Ogre::RenderTarget *_target,
                   Ogre::Material *_material, Ogre::Camera *_cam,
                   const bool _updateTex)
{
/*
  Ogre::SceneManager *sceneMgr = this->scene->OgreSceneManager();
  Ogre::RenderSystem *renderSys = sceneMgr->getDestinationRenderSystem();
  // Get pointer to the material pass
  Ogre::Pass *pass = _material->getBestTechnique()->getPass(0);

  // Render the depth texture
  // OgreSceneManager::_render function automatically sets farClip to 0.
  // Which normally equates to infinite distance. We don't want this. So
  // we have to set the distance every time.
  _cam->setFarClipDistance(this->FarClipPlane());

  Ogre::AutoParamDataSource autoParamDataSource;

  Ogre::Viewport *vp = _target->getViewport(0);

  // Need this line to render the ground plane. No idea why it's necessary.
  renderSys->_setViewport(vp);
  sceneMgr->_setPass(pass, true, false);
  autoParamDataSource.setCurrentPass(pass);
  autoParamDataSource.setCurrentViewport(vp);
  autoParamDataSource.setCurrentRenderTarget(_target);
  autoParamDataSource.setCurrentSceneManager(sceneMgr);
  autoParamDataSource.setCurrentCamera(_cam, true);

  renderSys->setLightingEnabled(false);
  renderSys->_setFog(Ogre::FOG_NONE);

  pass->_updateAutoParams(&autoParamDataSource, 1);

  if (_updateTex)
  {
    pass->getFragmentProgramParameters()->setNamedConstant("tex1",
      this->dataPtr->texIdx[0]);
    if (this->dataPtr->texIdx.size() > 1)
    {
      pass->getFragmentProgramParameters()->setNamedConstant("tex2",
        this->dataPtr->texIdx[1]);
      if (this->dataPtr->texIdx.size() > 2)
        pass->getFragmentProgramParameters()->setNamedConstant("tex3",
          this->dataPtr->texIdx[2]);
    }
  }

  // NOTE: We MUST bind parameters AFTER updating the autos
  if (pass->hasVertexProgram())
  {
    renderSys->bindGpuProgram(
        pass->getVertexProgram()->_getBindingDelegate());

    renderSys->bindGpuProgramParameters(Ogre::GPT_VERTEX_PROGRAM,
    pass->getVertexProgramParameters(), 1);
  }

  if (pass->hasFragmentProgram())
  {
    renderSys->bindGpuProgram(
    pass->getFragmentProgram()->_getBindingDelegate());

    renderSys->bindGpuProgramParameters(Ogre::GPT_FRAGMENT_PROGRAM,
    pass->getFragmentProgramParameters(), 1);
  }

  _target->update(false);

*/
}

//////////////////////////////////////////////////
void Ogre2GpuRays::Render()
{
/*  Ogre::SceneManager *sceneMgr = this->scene->OgreSceneManager();

  sceneMgr->_suppressRenderStateChanges(true);

  for (unsigned int i = 0; i < this->dataPtr->textureCount; ++i)
  {
    if (this->dataPtr->textureCount > 1)
    {
      // Cannot call Camera::RotateYaw because it rotates in world frame,
      // but we need rotation in camera local frame
      this->Node()->roll(Ogre::Radian(this->dataPtr->cameraYaws[i]));
    }

    this->dataPtr->currentMat = this->dataPtr->matFirstPass;
    this->dataPtr->currentTexture = this->dataPtr->firstPassTextures[i];

//    this->UpdateRenderTarget(
//        this->dataPtr->firstPassTextures[i]->getBuffer()->getRenderTarget(),
//        this->dataPtr->matFirstPass,
//        this->dataPtr->ogreCamera, false);
  }

  if (this->dataPtr->textureCount > 1)
      this->Node()->roll(Ogre::Radian(this->dataPtr->cameraYaws[3]));
*/

//  this->UpdateRenderTarget1stPass();
//  this->UpdateRenderTarget2ndPass();

/*  this->dataPtr->visual->SetVisible(true);

  this->UpdateRenderTarget(
      this->dataPtr->secondPassTexture->getBuffer()->getRenderTarget(),
      this->dataPtr->matSecondPass,
      this->dataPtr->orthoCam, true);

  this->dataPtr->visual->SetVisible(false);

  sceneMgr->_suppressRenderStateChanges(false);
*/


  this->UpdateRenderTargetCubemap();
}

//////////////////////////////////////////////////
void Ogre2GpuRays::PreRender()
{
  if (this->dataPtr->textureCount == 0)
    this->CreateGpuRaysTextures();
}

//////////////////////////////////////////////////
void Ogre2GpuRays::PostRender()
{
  // TODO
//  return;

/*  for (unsigned int i = 0; i < this->dataPtr->textureCount; ++i)
  {
    auto rt =
        this->dataPtr->firstPassTextures[i]->getBuffer()->getRenderTarget();
    rt->swapBuffers();
  }
*/


/*  auto rt = this->dataPtr->secondPassTexture->getBuffer()->getRenderTarget();
  rt->swapBuffers();

  const Ogre::Viewport *secondPassViewport = rt->getViewport(0);
  unsigned int width = secondPassViewport->getActualWidth();
  unsigned int height = secondPassViewport->getActualHeight();

  size_t size = Ogre::PixelUtil::getMemorySize(
    width, height, 1, Ogre::PF_FLOAT32_RGB);
  int len = width * height * this->dataPtr->channels;

  if (!this->dataPtr->gpuRaysBuffer)
  {
    this->dataPtr->gpuRaysBuffer = new float[len];
  }

  Ogre::PixelBox dstBox(width, height,
        1, Ogre::PF_FLOAT32_RGB, this->dataPtr->gpuRaysBuffer);

  auto pixelBuffer = this->dataPtr->secondPassTexture->getBuffer();
  pixelBuffer->blitToMemory(dstBox);

  if (!this->dataPtr->gpuRaysScan)
  {
    this->dataPtr->gpuRaysScan = new float[len];
  }

  memcpy(this->dataPtr->gpuRaysScan, this->dataPtr->gpuRaysBuffer, size);

  this->dataPtr->newGpuRaysFrame(this->dataPtr->gpuRaysScan,
      width, height, this->dataPtr->channels, "PF_FLOAT32_RGB");
*/
}

//////////////////////////////////////////////////
const float* Ogre2GpuRays::Data() const
{
  return this->dataPtr->gpuRaysScan;
}

//////////////////////////////////////////////////
void Ogre2GpuRays::Copy(float *_dataDest)
{
  auto rt = this->dataPtr->secondPassTexture->getBuffer()->getRenderTarget();
  const Ogre::Viewport *secondPassViewport = rt->getViewport(0);
  unsigned int width = secondPassViewport->getActualWidth();
  unsigned int height = secondPassViewport->getActualHeight();

  size_t size = Ogre::PixelUtil::getMemorySize(
    width, height, 1, Ogre::PF_FLOAT32_RGB);

  memcpy(_dataDest, this->dataPtr->gpuRaysScan, size);
}

/////////////////////////////////////////////////
void Ogre2GpuRays::CreateOrthoCam()
{
  // create ogre camera object
  Ogre::SceneManager *ogreSceneManager;
  ogreSceneManager = this->scene->OgreSceneManager();
  if (ogreSceneManager == nullptr)
  {
    ignerr << "Scene manager cannot be obtained" << std::endl;
    return;
  }

  this->dataPtr->orthoCam = ogreSceneManager->createCamera(
      this->Name() + "_Ortho_Camera");
  if (this->dataPtr->orthoCam == nullptr)
  {
    ignerr << "Ogre camera cannot be created" << std::endl;
    return;
  }

/*  Ogre::SceneNode *rootSceneNode = std::dynamic_pointer_cast<
      ignition::rendering::Ogre2Node>(this->scene->RootVisual())->Node();
  this->dataPtr->pitchNodeOrtho = rootSceneNode->createChildSceneNode();
  // by default, ogre2 cameras are attached to root scene node
  this->dataPtr->orthoCam->detachFromParent();
  this->dataPtr->pitchNodeOrtho->attachObject(this->dataPtr->orthoCam);
*/

  // Use X/Y as horizon, Z up
  this->dataPtr->orthoCam->pitch(Ogre::Degree(90));

  // Don't yaw along variable axis, causes leaning
  this->dataPtr->orthoCam->setFixedYawAxis(true, Ogre::Vector3::UNIT_Z);
  this->dataPtr->orthoCam->setDirection(1, 0, 0);
  this->dataPtr->orthoCam->setAutoAspectRatio(true);

//  this->dataPtr->orthoCam->setNearClipDistance(0.001);
  this->dataPtr->orthoCam->setNearClipDistance(0.008);
  this->dataPtr->orthoCam->setFarClipDistance(0.022);
//  this->dataPtr->orthoCam->setRenderingDistance(0.02);

  this->dataPtr->orthoCam->setProjectionType(Ogre::PT_ORTHOGRAPHIC);

  Ogre::Matrix4 p = this->BuildScaledOrthoMatrix(
      0, static_cast<float>(this->dataPtr->w2nd / 10.0),
//      -static_cast<float>(this->dataPtr->w2nd / 10.0), 0,
      0, static_cast<float>(this->dataPtr->h2nd / 10.0),
      0.01, 0.02);

//  this->dataPtr->orthoCam->setCustomProjectionMatrix(true, p);

  this->dataPtr->orthoCam->setFrustumExtents(0,
      static_cast<float>(this->dataPtr->w2nd / 10.0),
      static_cast<float>(this->dataPtr->h2nd / 10.0), 0);

  float l, r, t, b;
  this->dataPtr->orthoCam->getFrustumExtents(l, r, t, b);
  std::cerr << "l, r, t, b " << l << " " << r << " " << t << " " << b << std::endl;
}

/////////////////////////////////////////////////
Ogre::Matrix4 Ogre2GpuRays::BuildScaledOrthoMatrix(const float _left,
    const float _right, const float _bottom, const float _top,
    const float _near, const float _far)
{
  float invw = 1 / (_right - _left);
  float invh = 1 / (_top - _bottom);
  float invd = 1 / (_far - _near);

  Ogre::Matrix4 proj = Ogre::Matrix4::ZERO;
  proj[0][0] = 2 * invw;
  proj[0][3] = -(_right + _left) * invw;
  proj[1][1] = 2 * invh;
  proj[1][3] = -(_top + _bottom) * invh;
  proj[2][2] = -2 * invd;
  proj[2][3] = -(_far + _near) * invd;
  proj[3][3] = 1;

  return proj;
}

/////////////////////////////////////////////////
void Ogre2GpuRays::Set1stTextureSize(
    const unsigned int _w, const unsigned int _h)
{
  this->dataPtr->w1st = _w;
  this->dataPtr->h1st = _h;
}

/////////////////////////////////////////////////
void Ogre2GpuRays::SetRangeCount(
    const unsigned int _w, const unsigned int _h)
{
  this->dataPtr->w2nd = _w;
  this->dataPtr->h2nd = _h;
}

/////////////////////////////////////////////////
void Ogre2GpuRays::CreateMesh()
{
  std::string meshName = this->Name() + "_undistortion_mesh";

  common::Mesh *mesh = new common::Mesh();
  mesh->SetName(meshName);

  common::SubMesh *submesh = new common::SubMesh();

  double dx, dy;
  submesh->SetPrimitiveType(common::SubMesh::POINTS);

  if (this->dataPtr->h2nd == 1)
  {
    dy = 0;
  }
  else
  {
    dy = 0.1;
  }

  dx = 0.1;

  // startX ranges from 0 to -(w2nd/10) at dx=0.1 increments
  // startY ranges from h2nd/10 to 0 at dy=0.1 decrements
  // see Ogre2GpuRays::Set2ndPassTarget() on how the ortho cam is set up
  double startX = dx;
  double startY = this->dataPtr->h2nd/10.0;

  // half of actual camera vertical FOV without padding
  double phi = this->VFOV().Radian() / 2.0;
  double phiCamera = phi + std::abs(this->VertHalfAngle());
  double theta = this->CosHorzFOV() / 2.0;

  if (this->dataPtr->h2nd == 1)
  {
    phi = 0;
  }

  // index of ray
  unsigned int ptsOnLine = 0;

  // total gpu rays hfov
  double thfov = this->dataPtr->textureCount * this->CosHorzFOV();
  double hstep = thfov / (this->dataPtr->w2nd - 1);
  double vstep = 2.0 * phi / (this->dataPtr->h2nd - 1);

  if (this->dataPtr->h2nd == 1)
  {
    vstep = 0;
  }

  for (unsigned int j = 0; j < this->dataPtr->h2nd; ++j)
  {
    double gamma = 0;
    if (this->dataPtr->h2nd != 1)
    {
      // gamma: current vertical angle w.r.t. camera
      gamma = vstep * j - phi + this->VertHalfAngle();
    }

    for (unsigned int i = 0; i < this->dataPtr->w2nd; ++i)
    {
      // current horizontal angle from start of gpu rays scan
      double delta = hstep * i;

      // index of texture that contains the depth value
      unsigned int texture = delta / this->CosHorzFOV();

      // cap texture index and horizontal angle
      if (texture > this->dataPtr->textureCount-1)
      {
        texture -= 1;
        delta -= hstep;
      }

      startX -= dx;
      if (ptsOnLine == this->dataPtr->w2nd)
      {
        ptsOnLine = 0;
        startX = 0;
        startY -= dy;
      }
      ptsOnLine++;

      // the texture/1000.0 value is used in the gpu rays_2nd_pass.frag shader
      // as a trick to determine which camera texture to use when stitching
      // together the final depth image.
      submesh->AddVertex(texture/1000.0, startX, startY);
//      std::cerr << "add vert " << texture/1000.0 << " " << startX << " " << startY
//                << std::endl;

      // first compute angle from the start of current camera's horizontal
      // min angle, then set delta to be angle from center of current camera.
      delta = delta - (texture * this->CosHorzFOV());
      delta = delta - theta;

      // adjust uv coordinates of depth texture to match projection of current
      // gpu ray the depth image plane.
      double u = 0.5 - tan(delta) / (2.0 * tan(theta));
      double v = 0.5 - (tan(gamma) * cos(theta)) /
          (2.0 * tan(phiCamera) * cos(delta));

      submesh->AddTexCoord(u, v);
      submesh->AddIndex(this->dataPtr->w2nd * j + i);
    }
    std::cerr <<"-- " << std::endl;
  }

  mesh->AddSubMesh(*submesh);

  this->dataPtr->undistMesh = mesh;

  common::MeshManager::Instance()->AddMesh(this->dataPtr->undistMesh);
}

/////////////////////////////////////////////////
void Ogre2GpuRays::CreateCanvas()
{
  this->CreateMesh();

  this->dataPtr->visual = this->scene->CreateVisual(
      this->Name() + "second_pass_canvas");

/*  Ogre::SceneNode *visualSceneNode =  std::dynamic_pointer_cast<
      ignition::rendering::Ogre2Node>(this->dataPtr->visual)->Node();

  Ogre::Node *visualParent = visualSceneNode->getParent();
  if (visualParent != nullptr)
  {
    visualParent->removeChild(visualSceneNode);
  }
  this->dataPtr->pitchNodeOrtho->addChild(visualSceneNode);
*/

  // Convert mesh from common::Mesh to rendering::mesh and add it to
  // the canvas visual
//  MeshPtr renderingMesh = this->scene->CreateMesh(
//      this->dataPtr->undistMesh);

  GeometryPtr renderingMesh = this->scene->CreateBox();

  this->dataPtr->visual->AddGeometry(renderingMesh);

//  this->dataPtr->visual->SetLocalPosition(0.01, 0, 0);
  this->dataPtr->visual->SetLocalPosition(0.015, 0, 0);
  this->dataPtr->visual->SetLocalScale(0.01, 0.01, 0.01);

  this->dataPtr->visual->SetLocalRotation(0, 0, 0);

  VisualPtr root = this->scene->RootVisual();
  root->AddChild(this->dataPtr->visual);


/*  MaterialPtr canvasMaterial =
    this->scene->CreateMaterial(this->Name() + "_green");
  canvasMaterial->SetAmbient(ignition::math::Color(0, 1, 0, 1));
  this->dataPtr->visual->SetMaterial(canvasMaterial);
*/

//  this->dataPtr->visual->SetVisible(false);
}

//////////////////////////////////////////////////
ignition::common::ConnectionPtr Ogre2GpuRays::ConnectNewGpuRaysFrame(
    std::function<void(const float *_frame, unsigned int _width,
    unsigned int _height, unsigned int _channels,
    const std::string &/*_format*/)> _subscriber)
{
  return this->dataPtr->newGpuRaysFrame.Connect(_subscriber);
}

//////////////////////////////////////////////////
RenderTargetPtr Ogre2GpuRays::RenderTarget() const
{
  return this->dataPtr->renderTexture;
}

//////////////////////////////////////////////////
double Ogre2GpuRays::CosHorzFOV() const
{
  return this->dataPtr->chfov;
}

//////////////////////////////////////////////////
void Ogre2GpuRays::SetCosHorzFOV(const double _chfov)
{
  this->dataPtr->chfov = _chfov;
}

//////////////////////////////////////////////////
double Ogre2GpuRays::CosVertFOV() const
{
  return this->dataPtr->cvfov;
}

//////////////////////////////////////////////////
void Ogre2GpuRays::SetCosVertFOV(const double _cvfov)
{
  this->dataPtr->cvfov = _cvfov;
}

//////////////////////////////////////////////////
void Ogre2GpuRays::SetHorzHalfAngle(const double _angle)
{
  this->dataPtr->horzHalfAngle = _angle;
}

//////////////////////////////////////////////////
void Ogre2GpuRays::SetVertHalfAngle(const double _angle)
{
  this->dataPtr->vertHalfAngle = _angle;
}

//////////////////////////////////////////////////
double Ogre2GpuRays::HorzHalfAngle() const
{
  return this->dataPtr->horzHalfAngle;
}

//////////////////////////////////////////////////
double Ogre2GpuRays::VertHalfAngle() const
{
  return this->dataPtr->vertHalfAngle;
}
