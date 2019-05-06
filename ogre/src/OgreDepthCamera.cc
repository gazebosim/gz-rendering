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
#include <ignition/math/Helpers.hh>
#include "ignition/rendering/ogre/OgreDepthCamera.hh"

/// \internal
/// \brief Private data for the OgreDepthCamera class
class ignition::rendering::OgreDepthCameraPrivate
{
  /// \brief The depth buffer
  public: float *depthBuffer = nullptr;

  /// \brief The depth material
  public: Ogre::Material *depthMaterial = nullptr;

  /// \brief True to generate point clouds
  public: bool outputPoints = false;

  /// \brief Point cloud data buffer
  public: float *pcdBuffer = nullptr;

  /// \brief Point cloud view port
  public: Ogre::Viewport *pcdViewport = nullptr;

  /// \brief Point cloud material
  public: Ogre::Material *pcdMaterial = nullptr;

  /// \brief Point cloud texture
  public: OgreRenderTexturePtr pcdTexture;

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
OgreDepthCamera::OgreDepthCamera()
  : dataPtr(new OgreDepthCameraPrivate())
{
}

//////////////////////////////////////////////////
OgreDepthCamera::~OgreDepthCamera()
{
  if (this->dataPtr->depthBuffer)
    delete [] this->dataPtr->depthBuffer;

  if (this->dataPtr->pcdBuffer)
    delete [] this->dataPtr->pcdBuffer;
}

//////////////////////////////////////////////////
void OgreDepthCamera::Init()
{
  BaseDepthCamera::Init();
  this->CreateCamera();
  this->CreateDepthTexture();
  this->Reset();
}

//////////////////////////////////////////////////
void OgreDepthCamera::CreateCamera()
{
  // create ogre camera object
  Ogre::SceneManager *ogreSceneManager;
  ogreSceneManager = this->scene->OgreSceneManager();
  if (ogreSceneManager == nullptr)
  {
    ignerr << "Scene manager cannot be obtained" << std::endl;
    return;
  }

  this->ogreCamera = ogreSceneManager->createCamera(
      this->name + "_Depth_Camera");
  if (this->ogreCamera == nullptr)
  {
    ignerr << "Ogre camera cannot be created" << std::endl;
    return;
  }

  this->ogreNode->attachObject(this->ogreCamera);

  // rotate to Gazebo coordinate system
  this->ogreCamera->yaw(Ogre::Degree(-90.0));
  this->ogreCamera->roll(Ogre::Degree(-90.0));
  this->ogreCamera->setFixedYawAxis(false);

  // TODO(anyone): provide api access
  this->ogreCamera->setAutoAspectRatio(true);
  this->ogreCamera->setRenderingDistance(0);
  this->ogreCamera->setPolygonMode(Ogre::PM_SOLID);
  this->ogreCamera->setProjectionType(Ogre::PT_PERSPECTIVE);
  this->ogreCamera->setCustomProjectionMatrix(false);
}

/////////////////////////////////////////////////
void OgreDepthCamera::CreateDepthTexture()
{
  if (this->ogreCamera == nullptr)
  {
    ignerr << "Ogre camera cannot be created" << std::endl;
  }

  if (this->depthTexture == nullptr)
  {
    RenderTexturePtr depthTextureBase =
        this->scene->CreateRenderTexture();
    this->depthTexture = std::dynamic_pointer_cast<OgreRenderTexture>(
        depthTextureBase);
    this->depthTexture->SetFormat(PF_FLOAT32_R);
    this->depthTexture->SetCamera(this->ogreCamera);
    this->depthTexture->SetBackgroundColor(this->scene->BackgroundColor());

    MaterialPtr depthMat = this->scene->CreateMaterial();
    depthMat->SetDepthMaterial(this->FarClipPlane(), this->NearClipPlane());
    this->depthTexture->SetMaterial(depthMat);

    // Set default values for image size
    this->SetImageWidth(640);
    this->SetImageHeight(480);
  }

  double ratio = static_cast<double>(this->ImageWidth()) /
                 static_cast<double>(this->ImageHeight());

  double vfov = 2.0 * atan(tan(this->HFOV().Radian() / 2.0) / ratio);
  this->ogreCamera->setAspectRatio(ratio);
  this->ogreCamera->setFOVy(Ogre::Radian(this->LimitFOV(vfov)));

  // this->depthViewport->setVisibilityMask(
  // IGN_VISIBILITY_ALL & ~(IGN_VISIBILITY_GUI | IGN_VISIBILITY_SELECTABLE));
  if (this->dataPtr->outputPoints && this->dataPtr->pcdTexture == nullptr)
  {
    RenderTexturePtr pcdTextureBase = this->scene->CreateRenderTexture();
    this->dataPtr->pcdTexture = std::dynamic_pointer_cast<OgreRenderTexture>(
        pcdTextureBase);
    this->dataPtr->pcdTexture->SetCamera(this->ogreCamera);
    this->dataPtr->pcdTexture->SetFormat(PF_FLOAT32_RGBA);
    this->dataPtr->pcdTexture->SetWidth(this->ImageWidth());
    this->dataPtr->pcdTexture->SetHeight(this->ImageHeight());
    this->dataPtr->pcdTexture->SetBackgroundColor(
        this->scene->BackgroundColor());

    this->dataPtr->pcdViewport =
        this->dataPtr->pcdTexture->AddViewport(this->ogreCamera);
    this->dataPtr->pcdViewport->setClearEveryFrame(true);

    auto const &ignBG = this->scene->BackgroundColor();
    this->dataPtr->pcdViewport->setBackgroundColour(
        OgreConversions::Convert(ignBG));
    this->dataPtr->pcdViewport->setOverlaysEnabled(false);
    this->dataPtr->pcdViewport->setVisibilityMask(
        IGN_VISIBILITY_ALL & ~(IGN_VISIBILITY_GUI | IGN_VISIBILITY_SELECTABLE));

    this->dataPtr->pcdMaterial = (Ogre::Material*)(
    Ogre::MaterialManager::getSingleton().getByName(
      "Ignition/XYZPoints").get());

    this->dataPtr->pcdMaterial->getTechnique(0)->getPass(0)->
        createTextureUnitState("depth_target");

    this->dataPtr->pcdMaterial->load();
  }
}

//////////////////////////////////////////////////
void OgreDepthCamera::Render()
{
  Ogre::SceneManager *sceneMgr = this->scene->OgreSceneManager();

  Ogre::ShadowTechnique shadowTech = sceneMgr->getShadowTechnique();
  sceneMgr->setShadowTechnique(Ogre::SHADOWTYPE_NONE);

  // return farClip in case no renderable object is inside frustrum
  Ogre::Viewport *vp = this->depthTexture->Viewport(0);
  vp->setBackgroundColour(Ogre::ColourValue(this->FarClipPlane(),
      this->FarClipPlane(), this->FarClipPlane()));

  // Does actual rendering
  this->depthTexture->SetAutoUpdated(false);
  this->depthTexture->Render();

  sceneMgr->setShadowTechnique(shadowTech);

  if (this->dataPtr->outputPoints)
  {
    sceneMgr->setShadowTechnique(Ogre::SHADOWTYPE_NONE);
    sceneMgr->_suppressRenderStateChanges(true);

    // return farClip in case no renderable object is inside frustrum
    vp = this->dataPtr->pcdTexture->Viewport(0);
    vp->setBackgroundColour(Ogre::ColourValue(this->FarClipPlane(),
      this->FarClipPlane(), this->FarClipPlane()));

    this->dataPtr->pcdTexture->SetAutoUpdated(false);
    this->dataPtr->pcdTexture->Render();

    sceneMgr->_suppressRenderStateChanges(false);
    sceneMgr->setShadowTechnique(shadowTech);
  }
}

//////////////////////////////////////////////////
void OgreDepthCamera::PostRender()
{
  unsigned int width = this->ImageWidth();
  unsigned int height = this->ImageHeight();

  if (!this->dataPtr->outputPoints)
  {
    if (!this->dataPtr->depthBuffer)
    {
      Ogre::PixelFormat imageFormat = OgreConversions::Convert(
        this->depthTexture->Format());
      size_t size = Ogre::PixelUtil::getMemorySize(
        width, height, 1, imageFormat);
      this->dataPtr->depthBuffer = new float[size];
    }

    this->depthTexture->Buffer(this->dataPtr->depthBuffer);

    this->dataPtr->newDepthFrame(
        this->dataPtr->depthBuffer, width, height, 1, "FLOAT32");
  }
  else
  {
    if (!this->dataPtr->pcdBuffer)
    {
      Ogre::PixelFormat imageFormat = OgreConversions::Convert(
        this->depthTexture->Format());
      size_t size = Ogre::PixelUtil::getMemorySize(
        width, height, 1, imageFormat);
      this->dataPtr->pcdBuffer = new float[size];
    }

    this->dataPtr->pcdTexture->Buffer(this->dataPtr->pcdBuffer);

    this->dataPtr->newRgbPointCloud(
        this->dataPtr->pcdBuffer, width, height, 1, "RGBPOINTS");
  }
}

//////////////////////////////////////////////////
const float *OgreDepthCamera::DepthData() const
{
  return this->dataPtr->depthBuffer;
}

//////////////////////////////////////////////////
ignition::common::ConnectionPtr OgreDepthCamera::ConnectNewDepthFrame(
    std::function<void(const float *, unsigned int, unsigned int,
      unsigned int, const std::string &)>  _subscriber)
{
  return this->dataPtr->newDepthFrame.Connect(_subscriber);
}

//////////////////////////////////////////////////
ignition::common::ConnectionPtr OgreDepthCamera::ConnectNewRgbPointCloud(
    std::function<void(const float *, unsigned int, unsigned int,
      unsigned int, const std::string &)>  _subscriber)
{
  return this->dataPtr->newRgbPointCloud.Connect(_subscriber);
}

//////////////////////////////////////////////////
RenderTargetPtr OgreDepthCamera::RenderTarget() const
{
  return this->depthTexture;
}

//////////////////////////////////////////////////
double OgreDepthCamera::LimitFOV(const double _fov)
{
  return std::min(std::max(0.001, _fov), IGN_PI * 0.999);
}

//////////////////////////////////////////////////
void OgreDepthCamera::SetNearClipPlane(const double _near)
{
  BaseDepthCamera::SetNearClipPlane(_near);
  this->ogreCamera->setNearClipDistance(_near);
}

//////////////////////////////////////////////////
void OgreDepthCamera::SetFarClipPlane(const double _far)
{
  BaseDepthCamera::SetFarClipPlane(_far);
  this->ogreCamera->setFarClipDistance(_far);
}

//////////////////////////////////////////////////
double OgreDepthCamera::NearClipPlane() const
{
  if (this->ogreCamera)
    return this->ogreCamera->getNearClipDistance();
  else
    return 0;
}

//////////////////////////////////////////////////
double OgreDepthCamera::FarClipPlane() const
{
  if (this->ogreCamera)
    return this->ogreCamera->getFarClipDistance();
  else
    return 0;
}
