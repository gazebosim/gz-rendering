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

#include "ignition/rendering/ogre/OgreDepthCamera.hh"

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
  this->CreateDepthTexture("_RttTex_Depth");
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
  }

  this->ogreCamera = ogreSceneManager->createCamera(this->name);
  if (ogreCamera == nullptr)
  {
    ignerr << "Ogre camera cannot be created" << std::endl;
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
void OgreDepthCamera::CreateDepthTexture(const std::string &_textureName)
{
  // create ogre camera object
  Ogre::SceneManager *ogreSceneManager;
  ogreSceneManager = this->scene->OgreSceneManager();
  if (ogreSceneManager == nullptr)
  {
    ignerr << "Scene manager cannot be obtained" << std::endl;
  }

  this->ogreCamera = ogreSceneManager->createCamera(
      this->name + "_Depth_Camera");
  if (ogreCamera == nullptr)
  {
    ignerr << "Ogre camera cannot be created" << std::endl;
  }

  RenderTexturePtr depthTextureBase =
    this->scene->CreateRenderTexture();
  this->depthTexture = std::dynamic_pointer_cast<OgreRenderTexture>(
      depthTextureBase);
  this->depthTexture->SetCamera(this->ogreCamera);
  this->depthTexture->SetFormat(PF_FLOAT32_R);
  this->depthTexture->SetWidth(this->ImageWidth());
  this->depthTexture->SetHeight(this->ImageHeight());
  this->depthTexture->SetBackgroundColor(this->scene->BackgroundColor());
  // this->depthTexture->SetAutoUpdated(false);

  double ratio = static_cast<double>(this->ImageWidth()) /
                 static_cast<double>(this->ImageHeight());

  double vfov = 2.0 * atan(tan(this->HFOV().Radian() / 2.0) / ratio);
  this->ogreCamera->setAspectRatio(ratio);
  this->ogreCamera->setFOVy(Ogre::Radian(this->LimitFOV(vfov)));

  // this->depthViewport->setVisibilityMask(
  // IGN_VISIBILITY_ALL & ~(IGN_VISIBILITY_GUI | IGN_VISIBILITY_SELECTABLE));

  MaterialPtr depthMat = this->scene->CreateMaterial();
  depthMat->SetDepthMaterial();
  // (*(depthMat->FragmentShaderParams()))["maxRange"] = 10.0f;
  this->depthTexture->SetMaterial(depthMat);

  if (this->dataPtr->outputPoints)
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

    this->dataPtr->pcdTexture->SetAutoUpdated(false);

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
void OgreDepthCamera::PostRender()
{
  this->depthTexture->SwapBuffers();
  if (this->dataPtr->outputPoints)
    this->dataPtr->pcdTexture->SwapBuffers();

  if (this->newData && this->captureData)
  {
    unsigned int width = this->ImageWidth();
    unsigned int height = this->ImageHeight();

    if (!this->dataPtr->outputPoints)
    {
      if (!this->dataPtr->depthBuffer)
        this->dataPtr->depthBuffer = this->depthTexture->Buffer();

      this->dataPtr->newDepthFrame(
          this->dataPtr->depthBuffer, width, height, 1, "FLOAT32");
    }
    else
    {
      if (!this->dataPtr->pcdBuffer)
        this->dataPtr->pcdBuffer = this->dataPtr->pcdTexture->Buffer();

      this->dataPtr->newRGBPointCloud(
          this->dataPtr->pcdBuffer, width, height, 1, "RGBPOINTS");
    }
  }

  // also new image frame for camera texture
  // DepthCamera::PostRender();

  this->newData = false;
}

//////////////////////////////////////////////////
void OgreDepthCamera::UpdateRenderTarget(OgreRenderTexturePtr _target,
          Ogre::Material *_material, const std::string &_matName)
{
  Ogre::RenderSystem *renderSys;
  Ogre::Viewport *vp = NULL;
  Ogre::SceneManager *sceneMgr = this->scene->OgreSceneManager();
  Ogre::Pass *pass;

  renderSys = this->scene->OgreSceneManager()->getDestinationRenderSystem();
  // Get pointer to the material pass
  pass = _material->getBestTechnique()->getPass(0);

  // Render the depth texture
  // OgreSceneManager::_render function automatically sets farClip to 0.
  // Which normally equates to infinite distance. We don't want this. So
  // we have to set the distance every time.
  double farClipDistance = this->FarClipPlane();

  Ogre::AutoParamDataSource autoParamDataSource;

  vp = _target->GetViewport(0);

  // return farClip in case no renderable object is inside frustrum
  vp->setBackgroundColour(Ogre::ColourValue(farClipDistance,
      farClipDistance, farClipDistance));

  Ogre::CompositorManager::getSingleton().setCompositorEnabled(vp,
      _matName, true);

  // Need this line to render the ground plane. No idea why it's necessary.
  renderSys->_setViewport(vp);
  sceneMgr->_setPass(pass, true, false);
  autoParamDataSource.setCurrentPass(pass);
  autoParamDataSource.setCurrentViewport(vp);
  autoParamDataSource.setCurrentRenderTarget(_target->RenderTarget());
  autoParamDataSource.setCurrentSceneManager(sceneMgr);
  autoParamDataSource.setCurrentCamera(this->ogreCamera, true);

  renderSys->setLightingEnabled(false);
  renderSys->_setFog(Ogre::FOG_NONE);

  // These two lines don't seem to do anything useful
  renderSys->_setProjectionMatrix(
      this->ogreCamera->getProjectionMatrixRS());
  renderSys->_setViewMatrix(this->ogreCamera->getViewMatrix(true));

#if OGRE_VERSION_MAJOR == 1 && OGRE_VERSION_MINOR == 6
  pass->_updateAutoParamsNoLights(&autoParamDataSource);
#else
  pass->_updateAutoParams(&autoParamDataSource, 1);
#endif

  // NOTE: We MUST bind parameters AFTER updating the autos
  if (pass->hasVertexProgram())
  {
    renderSys->bindGpuProgram(
    pass->getVertexProgram()->_getBindingDelegate());

#if OGRE_VERSION_MAJOR == 1 && OGRE_VERSION_MINOR == 6
    renderSys->bindGpuProgramParameters(Ogre::GPT_VERTEX_PROGRAM,
    pass->getVertexProgramParameters());
#else
    renderSys->bindGpuProgramParameters(Ogre::GPT_VERTEX_PROGRAM,
      pass->getVertexProgramParameters(), 1);
#endif
  }

  if (pass->hasFragmentProgram())
  {
    renderSys->bindGpuProgram(
    pass->getFragmentProgram()->_getBindingDelegate());

#if OGRE_VERSION_MAJOR == 1 && OGRE_VERSION_MINOR == 6
    renderSys->bindGpuProgramParameters(Ogre::GPT_FRAGMENT_PROGRAM,
    pass->getFragmentProgramParameters());
#else
      renderSys->bindGpuProgramParameters(Ogre::GPT_FRAGMENT_PROGRAM,
      pass->getFragmentProgramParameters(), 1);
#endif
  }
}

//////////////////////////////////////////////////
void OgreDepthCamera::Render()
{
  Ogre::SceneManager *sceneMgr = this->scene->OgreSceneManager();

  Ogre::ShadowTechnique shadowTech = sceneMgr->getShadowTechnique();

  sceneMgr->setShadowTechnique(Ogre::SHADOWTYPE_NONE);
  sceneMgr->_suppressRenderStateChanges(true);

  this->UpdateRenderTarget(this->depthTexture,
                  this->dataPtr->depthMaterial, "Ignition/DepthMap");

  // Does actual rendering
  this->depthTexture->SetAutoUpdated(false);

  sceneMgr->_suppressRenderStateChanges(false);
  sceneMgr->setShadowTechnique(shadowTech);

  // for camera image
  // DepthCamera::Render();

  if (this->dataPtr->outputPoints)
  {
    sceneMgr->setShadowTechnique(Ogre::SHADOWTYPE_NONE);
    sceneMgr->_suppressRenderStateChanges(true);

    this->UpdateRenderTarget(this->dataPtr->pcdTexture,
                  this->dataPtr->pcdMaterial, "Ignition/XYZPoints");

    this->dataPtr->pcdTexture->SetAutoUpdated(false);

    sceneMgr->_suppressRenderStateChanges(false);
    sceneMgr->setShadowTechnique(shadowTech);
  }
}

//////////////////////////////////////////////////
const float* OgreDepthCamera::DepthData() const
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
ignition::common::ConnectionPtr OgreDepthCamera::ConnectNewRGBPointCloud(
    std::function<void(const float *, unsigned int, unsigned int,
      unsigned int, const std::string &)>  _subscriber)
{
  return this->dataPtr->newRGBPointCloud.Connect(_subscriber);
}

//////////////////////////////////////////////////
RenderTargetPtr OgreDepthCamera::RenderTarget() const
{
  return this->depthTexture;
}

//////////////////////////////////////////////////
double OgreDepthCamera::LimitFOV(const double _fov)
{
  return std::min(std::max(0.001, _fov), M_PI * 0.999);
}

//////////////////////////////////////////////////
void OgreDepthCamera::SetNearClipPlane(const double _near)
{
  // this->nearClip = _near;
  BaseDepthCamera::SetNearClipPlane(_near);
  this->ogreCamera->setNearClipDistance(_near);
}

//////////////////////////////////////////////////
void OgreDepthCamera::SetFarClipPlane(const double _far)
{
  BaseDepthCamera::SetFarClipPlane(_far);
  this->ogreCamera->setFarClipDistance(_far);
}
