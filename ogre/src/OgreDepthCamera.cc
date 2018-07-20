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
  : ogreCamera(new OgreCamera()), dataPtr(new OgreDepthCameraPrivate())
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

  this->ogreCamera->Init();

  this->CreateDepthTexture(this->Name() + "_RttTex_Depth");
  this->Reset();
}

/////////////////////////////////////////////////
void OgreDepthCamera::CreateDepthTexture(const std::string &_textureName)
{
  // Create the depth buffer
  std::string depthMaterialName = this->Name() + "_RttMat_Camera_Depth";

  this->depthTexture = Ogre::TextureManager::getSingleton().createManual(
      _textureName,
      Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
      Ogre::TEX_TYPE_2D,
      this->ImageWidth(), this->ImageHeight(), 0,
      Ogre::PF_FLOAT32_R,
      Ogre::TU_RENDERTARGET).getPointer();

  this->depthTarget.reset(dynamic_cast<OgreRenderTarget*>(
        this->depthTexture->getBuffer()->getRenderTarget()));
  this->depthTarget->SetAutoUpdated(false);

  this->SetDepthTarget(this->depthTarget);

  this->depthViewport->setOverlaysEnabled(false);
  this->depthViewport->setBackgroundColour(
      Ogre::ColourValue(Ogre::ColourValue(0, 0, 0)));

  // Create materials for all the render textures.
  Ogre::MaterialPtr matPtr = Ogre::MaterialManager::getSingleton().create(
      depthMaterialName,
      Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

  matPtr->getTechnique(0)->getPass(0)->setDepthCheckEnabled(false);
  matPtr->getTechnique(0)->getPass(0)->setDepthWriteEnabled(false);
  matPtr->getTechnique(0)->getPass(0)->setLightingEnabled(false);

  matPtr->getTechnique(0)->getPass(0)->createTextureUnitState(
      _textureName);

  this->dataPtr->depthMaterial = (Ogre::Material*)(
      Ogre::MaterialManager::getSingleton().getByName(
        "Ignition/DepthMap").get());

  this->dataPtr->depthMaterial->load();

  if (this->dataPtr->outputPoints)
  {
    this->dataPtr->pcdTexture =
        Ogre::TextureManager::getSingleton().createManual(
        _textureName + "_pcd",
        "General",
        Ogre::TEX_TYPE_2D,
        this->ImageWidth(), this->ImageHeight(), 0,
        Ogre::PF_FLOAT32_RGBA,
        Ogre::TU_RENDERTARGET).getPointer();

    this->dataPtr->pcdTarget.reset(dynamic_cast<OgreRenderTarget*>(
          this->dataPtr->pcdTexture->getBuffer()->getRenderTarget()));
    this->dataPtr->pcdTarget->SetAutoUpdated(false);

    this->dataPtr->pcdViewport =
        this->dataPtr->pcdTarget->AddViewport(this->ogreCamera);
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
        createTextureUnitState(this->depthTarget->Name());

    this->dataPtr->pcdMaterial->load();
  }
}

//////////////////////////////////////////////////
void OgreDepthCamera::PostRender()
{
  this->depthTarget->SwapBuffers();
  if (this->dataPtr->outputPoints)
    this->dataPtr->pcdTarget->SwapBuffers();

  if (this->newData && this->captureData)
  {
    unsigned int width = this->ImageWidth();
    unsigned int height = this->ImageHeight();

    if (!this->dataPtr->outputPoints)
    {
      Ogre::HardwarePixelBufferSharedPtr pixelBuffer;

      // Get access to the buffer and make an image and write it to file
      pixelBuffer = this->depthTexture->getBuffer();

      size_t size = Ogre::PixelUtil::getMemorySize(width, height, 1,
          Ogre::PF_FLOAT32_R);

      // Blit the depth buffer if needed
      if (!this->dataPtr->depthBuffer)
        this->dataPtr->depthBuffer = new float[size];

      Ogre::PixelBox dstBox(width, height,
          1, Ogre::PF_FLOAT32_R, this->dataPtr->depthBuffer);

      pixelBuffer->lock(Ogre::HardwarePixelBuffer::HBL_NORMAL);
      pixelBuffer->blitToMemory(dstBox);
      pixelBuffer->unlock();  // FIXME: do we need to lock/unlock still?

      this->dataPtr->newDepthFrame(
          this->dataPtr->depthBuffer, width, height, 1, "FLOAT32");
    }
    else
    {
      Ogre::HardwarePixelBufferSharedPtr pcdPixelBuffer;

      // Get access to the buffer and make an image and write it to file
      pcdPixelBuffer = this->dataPtr->pcdTexture->getBuffer();

      // Blit the depth buffer if needed
      if (!this->dataPtr->pcdBuffer)
        this->dataPtr->pcdBuffer = new float[width * height * 4];

      memset(this->dataPtr->pcdBuffer, 0, width * height * 4);

      Ogre::Box pcd_src_box(0, 0, width, height);
      Ogre::PixelBox pcd_dst_box(width, height,
          1, Ogre::PF_FLOAT32_RGBA, this->dataPtr->pcdBuffer);

      pcdPixelBuffer->lock(Ogre::HardwarePixelBuffer::HBL_NORMAL);
      pcdPixelBuffer->blitToMemory(pcd_src_box, pcd_dst_box);
      pcdPixelBuffer->unlock();

      this->dataPtr->newRGBPointCloud(
          this->dataPtr->pcdBuffer, width, height, 1, "RGBPOINTS");
    }
  }

  // also new image frame for camera texture
  this->ogreCamera->PostRender();

  this->newData = false;
}

//////////////////////////////////////////////////
void OgreDepthCamera::UpdateRenderTarget(OgreRenderTargetPtr &_target,
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
  autoParamDataSource.setCurrentRenderTarget(
      dynamic_cast<Ogre::RenderTarget*>(_target.get()));
  autoParamDataSource.setCurrentSceneManager(sceneMgr);
  autoParamDataSource.setCurrentCamera(this->ogreCamera->OgreCameraPtr(), true);

  renderSys->setLightingEnabled(false);
  renderSys->_setFog(Ogre::FOG_NONE);

  // These two lines don't seem to do anything useful
  renderSys->_setProjectionMatrix(
      this->ogreCamera->OgreCameraPtr()->getProjectionMatrixRS());
  renderSys->_setViewMatrix(this->ogreCamera->OgreCameraPtr()->getViewMatrix(true));

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

  this->UpdateRenderTarget(this->depthTarget,
                  this->dataPtr->depthMaterial, "Ignition/DepthMap");

  // Does actual rendering
  this->depthTarget->SetUpdate(false);

  sceneMgr->_suppressRenderStateChanges(false);
  sceneMgr->setShadowTechnique(shadowTech);

  // for camera image
  this->ogreCamera->Render();

  if (this->dataPtr->outputPoints)
  {
    sceneMgr->setShadowTechnique(Ogre::SHADOWTYPE_NONE);
    sceneMgr->_suppressRenderStateChanges(true);

    this->UpdateRenderTarget(this->dataPtr->pcdTarget,
                  this->dataPtr->pcdMaterial, "Ignition/XYZPoints");

    this->dataPtr->pcdTarget->SetUpdate(false);

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
void OgreDepthCamera::SetDepthTarget(OgreRenderTargetPtr &_target)
{
  this->depthTarget = _target;

  if (this->depthTarget)
  {
    // Setup the viewport to use the texture
    this->depthViewport = this->depthTarget->AddViewport(this->ogreCamera);
    this->depthViewport->setClearEveryFrame(true);
    auto const &ignBG = this->scene->BackgroundColor();
    this->depthViewport->setBackgroundColour(OgreConversions::Convert(ignBG));
    this->depthViewport->setVisibilityMask(
        IGN_VISIBILITY_ALL & ~(IGN_VISIBILITY_GUI | IGN_VISIBILITY_SELECTABLE));

    double ratio = static_cast<double>(this->depthViewport->getActualWidth()) /
                   static_cast<double>(this->depthViewport->getActualHeight());

    this->ogreCamera->SetAspectRatio(ratio);
    this->ogreCamera->SetHFOV(this->HFOV());
  }
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
  return this->depthTarget;
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
  this->ogreCamera->SetNearClipPlane(_near);
}

//////////////////////////////////////////////////
void OgreDepthCamera::SetFarClipPlane(const double _far)
{
  BaseDepthCamera::SetFarClipPlane(_far);
  this->ogreCamera->SetFarClipPlane(_far);
}
