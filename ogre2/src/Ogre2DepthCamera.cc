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

#if (_WIN32)
  /* Needed for std::min */
  #define NOMINMAX
  #include <windows.h>
#endif

#include <math.h>
#include <ignition/math/Helpers.hh>

#include "ignition/rendering/RenderTypes.hh"
#include "ignition/rendering/ogre2/Ogre2Conversions.hh"
#include "ignition/rendering/ogre2/Ogre2DepthCamera.hh"
#include "ignition/rendering/ogre2/Ogre2Includes.hh"
#include "ignition/rendering/ogre2/Ogre2RenderEngine.hh"
#include "ignition/rendering/ogre2/Ogre2RenderTarget.hh"
#include "ignition/rendering/ogre2/Ogre2RenderTypes.hh"
#include "ignition/rendering/ogre2/Ogre2Scene.hh"
#include "ignition/rendering/ogre2/Ogre2Sensor.hh"

/// \internal
/// \brief Private data for the Ogre2DepthCamera class
class ignition::rendering::Ogre2DepthCameraPrivate
{
  /// \brief The depth buffer
  public: float *depthBuffer = nullptr;

  /// \brief Outgoing depth data, used by newDepthFrame event.
  public: float *depthImage = nullptr;

  /// \brief Outgoing point cloud data, used by newRgbPointCloud event.
  public: float *pointCloudImage = nullptr;

  /// \brief maximum value used for data outside sensor range
  public: float dataMaxVal = ignition::math::INF_D;

  /// \brief minimum value used for data outside sensor range
  public: float dataMinVal = -ignition::math::INF_D;

  /// \brief 1st pass compositor workspace definition
  public: std::string ogreCompositorWorkspaceDef;

  /// \brief 1st pass compositor node definition
  public: std::string ogreCompositorNodeDef;

  /// \brief 1st pass compositor workspace. One for each cubemap camera
  public: Ogre::CompositorWorkspace *ogreCompositorWorkspace;

  /// \brief An array of first pass textures. One for each cubemap camera.
  public: Ogre::TexturePtr ogreDepthTexture;

  /// \brief Dummy render texture for the depth data
  public: RenderTexturePtr depthTexture;

  /// \brief The depth material
  public: Ogre::MaterialPtr depthMaterial;

  /// \brief Event used to signal rgb point cloud data
  public: ignition::common::EventT<void(const float *,
              unsigned int, unsigned int, unsigned int,
              const std::string &)> newRgbPointCloud;

  /// \brief Event used to signal depth data
  public: ignition::common::EventT<void(const float *,
              unsigned int, unsigned int, unsigned int,
              const std::string &)> newDepthFrame;
};

using namespace ignition;
using namespace rendering;

//////////////////////////////////////////////////
Ogre2DepthCamera::Ogre2DepthCamera()
  : dataPtr(new Ogre2DepthCameraPrivate())
{
  this->dataPtr->ogreCompositorWorkspace = nullptr;
}

//////////////////////////////////////////////////
Ogre2DepthCamera::~Ogre2DepthCamera()
{
  this->Destroy();
}

//////////////////////////////////////////////////
void Ogre2DepthCamera::Init()
{
  BaseDepthCamera::Init();

  // create internal camera
  this->CreateCamera();

  // create dummy render texture
  this->CreateRenderTexture();

  this->Reset();
}

//////////////////////////////////////////////////
void Ogre2DepthCamera::Destroy()
{
  if (this->dataPtr->depthBuffer)
  {
    delete [] this->dataPtr->depthBuffer;
    this->dataPtr->depthBuffer = nullptr;
  }

  if (this->dataPtr->depthImage)
  {
    delete [] this->dataPtr->depthImage;
    this->dataPtr->depthImage = nullptr;
  }

  if (this->dataPtr->pointCloudImage)
  {
    delete [] this->dataPtr->pointCloudImage;
    this->dataPtr->pointCloudImage = nullptr;
  }

  if (!this->ogreCamera)
    return;

  auto engine = Ogre2RenderEngine::Instance();
  auto ogreRoot = engine->OgreRoot();
  Ogre::CompositorManager2 *ogreCompMgr = ogreRoot->getCompositorManager2();

  // remove depth texture, material, compositor
  if (this->dataPtr->ogreDepthTexture)
  {
    Ogre::TextureManager::getSingleton().remove(
        this->dataPtr->ogreDepthTexture->getName());
  }
  if (this->dataPtr->ogreCompositorWorkspace)
  {
    ogreCompMgr->removeWorkspace(
        this->dataPtr->ogreCompositorWorkspace);
  }

  if (this->dataPtr->depthMaterial)
  {
    Ogre::MaterialManager::getSingleton().remove(
        this->dataPtr->depthMaterial->getName());
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
void Ogre2DepthCamera::CreateCamera()
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
  this->ogreCamera->setRenderingDistance(100);
  this->ogreCamera->setProjectionType(Ogre::PT_PERSPECTIVE);
  this->ogreCamera->setCustomProjectionMatrix(false);
}

/////////////////////////////////////////////////
void Ogre2DepthCamera::CreateRenderTexture()
{
  RenderTexturePtr base = this->scene->CreateRenderTexture();
  this->dataPtr->depthTexture =
      std::dynamic_pointer_cast<Ogre2RenderTexture>(base);
  this->dataPtr->depthTexture->SetWidth(1);
  this->dataPtr->depthTexture->SetHeight(1);
}

/////////////////////////////////////////////////////////
void Ogre2DepthCamera::CreateDepthTexture()
{
  // set aspect ratio and fov
  double vfov = 2.0 * atan(tan(this->HFOV().Radian() / 2.0) / this->aspect);
  this->ogreCamera->setAspectRatio(this->aspect);
  this->ogreCamera->setFOVy(Ogre::Radian(this->LimitFOV(vfov)));

  // Load depth material
  // The DepthCamera material is defined in script (depth_camera.material).
  // We need to clone it since we are going to modify its uniform variables
  std::string matDepthName = "DepthCamera";
  Ogre::MaterialPtr matDepth =
      Ogre::MaterialManager::getSingleton().getByName(matDepthName);
  this->dataPtr->depthMaterial = matDepth->clone(
      this->Name() + "_" + matDepthName);
  this->dataPtr->depthMaterial->load();
  Ogre::Pass *pass = this->dataPtr->depthMaterial->getTechnique(0)->getPass(0);
  Ogre::GpuProgramParametersSharedPtr psParams =
      pass->getFragmentProgramParameters();

  // Configure camera behaviour.
  // Make the clipping plane dist large and handle near clamping in shaders
  double nearPlane = this->NearClipPlane() * 0.9;
  double farPlane = this->FarClipPlane() * 1.1;
  this->ogreCamera->setNearClipDistance(nearPlane);
  this->ogreCamera->setFarClipDistance(farPlane);

  // Set the uniform variables (depth_camera_fs.glsl).
  // The projectParams is used to linearize depth buffer data
  // The other params are used to clamp the range output
  // Use the 'real' clip distance here so depth can be
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
      static_cast<float>(this->dataPtr->dataMaxVal));
  psParams->setNamedConstant("min",
      static_cast<float>(this->dataPtr->dataMinVal));
  Ogre::Vector3 bg(this->Scene()->BackgroundColor().R(),
    this->Scene()->BackgroundColor().G(),
    this->Scene()->BackgroundColor().B());
  psParams->setNamedConstant("backgroundColor", bg);
  // We need to include a tolerance for Clipping
  psParams->setNamedConstant("tolerance",
      static_cast<float>(1e-6));

  // Create depth camera compositor
  auto engine = Ogre2RenderEngine::Instance();
  auto ogreRoot = engine->OgreRoot();
  Ogre::CompositorManager2 *ogreCompMgr = ogreRoot->getCompositorManager2();

  // We need to programmatically create the compositor because we need to
  // configure it to use the cloned depth material created earlier.
  // The compositor workspace definition is equivalent to the following
  // ogre compositor script:
  // compositor_node DepthCamera
  // {
  //   in 0 rt_input
  //   // colorTexture shares same depth buffer as depthTexture
  //   texture colorTexture target_width target_height PF_R8G8B8
  //       depth_texture depth_format PF_D32_FLOAT
  //   texture depthTexture target_width target_height PF_D32_FLOAT
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
  //   target rt_input
  //   {
  //     pass clear
  //     {
  //       colour_value 0.0 0.0 0.0 1.0
  //     }
  //     pass render_quad
  //     {
  //       material DepthCamera // Use copy instead of original
  //       input 0 depthTexture
  //       quad_normals camera_far_corners_view_space
  //     }
  //   }
  //   out 0 rt_input
  // }
  std::string wsDefName = "DepthCameraWorkspace_" + this->Name();
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
    Ogre::TextureDefinitionBase::TextureDefinition *depthTexDef =
        nodeDef->addTextureDefinition("depthTexture");
    depthTexDef->textureType = Ogre::TEX_TYPE_2D;
    depthTexDef->width = 0;
    depthTexDef->height = 0;
    depthTexDef->depth = 1;
    depthTexDef->numMipmaps = 0;
    depthTexDef->widthFactor = 1;
    depthTexDef->heightFactor = 1;
    depthTexDef->formatList = {Ogre::PF_D32_FLOAT};
    depthTexDef->fsaa = 0;
    depthTexDef->uav = false;
    depthTexDef->automipmaps = false;
    depthTexDef->hwGammaWrite = Ogre::TextureDefinitionBase::BoolFalse;
    depthTexDef->depthBufferId = Ogre::DepthBuffer::POOL_DEFAULT;
    depthTexDef->depthBufferFormat = Ogre::PF_UNKNOWN;
    depthTexDef->fsaaExplicitResolve = false;

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
    // Enable gamma write to avoid discretization in the color values
    // Note we are using low level materials in quad pass so also had to perform
    // gamma correction in the fragment shaders (depth_camera_fs.glsl)
    colorTexDef->hwGammaWrite = Ogre::TextureDefinitionBase::BoolTrue;
    colorTexDef->depthBufferId = Ogre::DepthBuffer::POOL_DEFAULT;
    colorTexDef->depthBufferFormat = Ogre::PF_D32_FLOAT;
    colorTexDef->preferDepthTexture = true;
    colorTexDef->fsaaExplicitResolve = false;

    nodeDef->setNumTargetPass(2);
    Ogre::CompositorTargetDef *colorTargetDef =
        nodeDef->addTargetPass("colorTexture");
    colorTargetDef->setNumPasses(2);
    {
      // clear pass
      Ogre::CompositorPassClearDef *passClear =
          static_cast<Ogre::CompositorPassClearDef *>(
          colorTargetDef->addPass(Ogre::PASS_CLEAR));
      passClear->mColourValue = Ogre::ColourValue(
          Ogre2Conversions::Convert(this->Scene()->BackgroundColor()));
      // scene pass
      Ogre::CompositorPassSceneDef *passScene =
          static_cast<Ogre::CompositorPassSceneDef *>(
          colorTargetDef->addPass(Ogre::PASS_SCENE));
      passScene->mVisibilityMask = IGN_VISIBILITY_ALL
          & ~(IGN_VISIBILITY_GUI | IGN_VISIBILITY_SELECTABLE);
    }

    // rt_input target - converts depth to range
    Ogre::CompositorTargetDef *inputTargetDef =
        nodeDef->addTargetPass("rt_input");
    inputTargetDef->setNumPasses(2);
    {
      // clear pass
      Ogre::CompositorPassClearDef *passClear =
          static_cast<Ogre::CompositorPassClearDef *>(
          inputTargetDef->addPass(Ogre::PASS_CLEAR));
      passClear->mColourValue = Ogre::ColourValue(this->FarClipPlane(), 0, 1.0);
      // quad pass
      Ogre::CompositorPassQuadDef *passQuad =
          static_cast<Ogre::CompositorPassQuadDef *>(
          inputTargetDef->addPass(Ogre::PASS_QUAD));
      passQuad->mMaterialName = this->dataPtr->depthMaterial->getName();
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

  // create render texture - these textures pack the range data
  std::stringstream texName;
  this->dataPtr->ogreDepthTexture =
    Ogre::TextureManager::getSingleton().createManual(
    texName.str(), "General", Ogre::TEX_TYPE_2D,
    this->ImageWidth(), this->ImageHeight(), 1, 0,
    Ogre::PF_FLOAT32_RGBA, Ogre::TU_RENDERTARGET,
    0, false, 0, Ogre::BLANKSTRING, false, true);

  Ogre::RenderTarget *rt =
    this->dataPtr->ogreDepthTexture->getBuffer()->getRenderTarget();

  // create compositor worksspace
  this->dataPtr->ogreCompositorWorkspace =
      ogreCompMgr->addWorkspace(this->scene->OgreSceneManager(),
      rt, this->ogreCamera, wsDefName, false);
}

//////////////////////////////////////////////////
void Ogre2DepthCamera::Render()
{
  // update the compositors
  this->dataPtr->ogreCompositorWorkspace->setEnabled(true);
  auto engine = Ogre2RenderEngine::Instance();
  engine->OgreRoot()->renderOneFrame();
  this->dataPtr->ogreCompositorWorkspace->setEnabled(false);
}

//////////////////////////////////////////////////
void Ogre2DepthCamera::PreRender()
{
  if (!this->dataPtr->ogreDepthTexture)
    this->CreateDepthTexture();
}

//////////////////////////////////////////////////
void Ogre2DepthCamera::PostRender()
{
  unsigned int width = this->ImageWidth();
  unsigned int height = this->ImageHeight();

  PixelFormat format = PF_FLOAT32_RGBA;
  Ogre::PixelFormat imageFormat = Ogre2Conversions::Convert(format);

  size_t size = Ogre::PixelUtil::getMemorySize(width, height, 1, imageFormat);
  int len = width * height;
  unsigned int channelCount = PixelUtil::ChannelCount(format);

  if (!this->dataPtr->depthBuffer)
  {
    this->dataPtr->depthBuffer = new float[len * channelCount];
  }
  Ogre::PixelBox dstBox(width, height,
        1, imageFormat, this->dataPtr->depthBuffer);

  // blit data from gpu to cpu
  auto rt = this->dataPtr->ogreDepthTexture->getBuffer()->getRenderTarget();
  rt->copyContentsToMemory(dstBox, Ogre::RenderTarget::FB_FRONT);

  if (!this->dataPtr->depthImage)
  {
    this->dataPtr->depthImage = new float[len];
  }
  if (!this->dataPtr->pointCloudImage)
  {
    this->dataPtr->pointCloudImage = new float[len * channelCount];
  }

  // fill depth data
  for (unsigned int i = 0; i < height; ++i)
  {
    unsigned int step = i*width*channelCount;
    for (unsigned int j = 0; j < width; ++j)
    {
      float x = this->dataPtr->depthBuffer[step + j*channelCount];
      this->dataPtr->depthImage[i*width + j] = x;
    }
  }
  this->dataPtr->newDepthFrame(
        this->dataPtr->depthImage, width, height, 1, "FLOAT32");

  // point cloud data
  if (this->dataPtr->newRgbPointCloud.ConnectionCount() > 0u)
  {
    memcpy(this->dataPtr->pointCloudImage, this->dataPtr->depthBuffer, size);
    this->dataPtr->newRgbPointCloud(
        this->dataPtr->pointCloudImage, width, height, channelCount,
        "PF_FLOAT32_RGBA");

    // Uncomment to debug color output
    // for (unsigned int i = 0; i < height; ++i)
    // {
    //   unsigned int step = i*width*channelCount;
    //   for (unsigned int j = 0; j < width; ++j)
    //   {
    //     float color =
    //         this->dataPtr->pointCloudImage[step + j*channelCount + 3];
    //     // unpack rgb data
    //     uint32_t *rgba = reinterpret_cast<uint32_t *>(&color);
    //     unsigned int r = *rgba >> 24 & 0xFF;
    //     unsigned int g = *rgba >> 16 & 0xFF;
    //     unsigned int b = *rgba >> 8 & 0xFF;
    //     igndbg << "[" << r << "]" << "[" << g << "]" << "[" << b << "],";
    //   }
    //   igndbg << std::endl;
    // }

    // Uncomment to debug xyz output
    // igndbg << "wxh: " << width << " x " << height << std::endl;
    // for (unsigned int i = 0; i < height; ++i)
    // {
    //   for (unsigned int j = 0; j < width; ++j)
    //   {
    //     igndbg << "[" << this->dataPtr->pointCloudImage[i*width*4+j*4] << "]"
    //       << "[" << this->dataPtr->pointCloudImage[i*width*4+j*4+1] << "]"
    //       << "[" << this->dataPtr->pointCloudImage[i*width*4+j*4+2] << "],";
    //   }
    //   igndbg << std::endl;
    // }
  }

  // Uncomment to debug depth output
  // igndbg << "wxh: " << width << " x " << height << std::endl;
  // for (unsigned int i = 0; i < height; ++i)
  // {
  //   for (unsigned int j = 0; j < width; ++j)
  //   {
  //     igndbg << "[" << this->dataPtr->depthImage[i*width + j] << "]";
  //   }
  //   igndbg << std::endl;
  // }
}

//////////////////////////////////////////////////
const float *Ogre2DepthCamera::DepthData() const
{
  return this->dataPtr->depthBuffer;
}

//////////////////////////////////////////////////
ignition::common::ConnectionPtr Ogre2DepthCamera::ConnectNewDepthFrame(
    std::function<void(const float *, unsigned int, unsigned int,
      unsigned int, const std::string &)>  _subscriber)
{
  return this->dataPtr->newDepthFrame.Connect(_subscriber);
}

//////////////////////////////////////////////////
ignition::common::ConnectionPtr Ogre2DepthCamera::ConnectNewRgbPointCloud(
    std::function<void(const float *, unsigned int, unsigned int,
      unsigned int, const std::string &)>  _subscriber)
{
  return this->dataPtr->newRgbPointCloud.Connect(_subscriber);
}

//////////////////////////////////////////////////
RenderTargetPtr Ogre2DepthCamera::RenderTarget() const
{
  return this->dataPtr->depthTexture;
}

//////////////////////////////////////////////////
double Ogre2DepthCamera::LimitFOV(const double _fov)
{
  return std::min(std::max(0.001, _fov), IGN_PI * 0.999);
}

//////////////////////////////////////////////////
void Ogre2DepthCamera::SetNearClipPlane(const double _near)
{
  BaseDepthCamera::SetNearClipPlane(_near);
  // near plane clipping is handled in shaders
}

//////////////////////////////////////////////////
void Ogre2DepthCamera::SetFarClipPlane(const double _far)
{
  BaseDepthCamera::SetFarClipPlane(_far);
  // far plane clipping is handled in shaders
}

//////////////////////////////////////////////////
double Ogre2DepthCamera::NearClipPlane() const
{
  return BaseDepthCamera::NearClipPlane();
}

//////////////////////////////////////////////////
double Ogre2DepthCamera::FarClipPlane() const
{
  return BaseDepthCamera::FarClipPlane();
}
