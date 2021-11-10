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

#ifdef __APPLE__
  #define GL_SILENCE_DEPRECATION
  #include <OpenGL/gl.h>
  #include <OpenGL/glext.h>
#else
#ifndef _WIN32
  #include <GL/gl.h>
  #include <GL/glext.h>
#endif
#endif

#include "ignition/rendering/CameraLens.hh"

#include "ignition/rendering/ogre/OgreConversions.hh"
#include "ignition/rendering/ogre/OgreRenderEngine.hh"
#include "ignition/rendering/ogre/OgreRTShaderSystem.hh"
#include "ignition/rendering/ogre/OgreWideAngleCamera.hh"

/// \brief Private data for the WideAngleCamera class
class ignition::rendering::OgreWideAngleCameraPrivate
{
  /// \brief Environment texture size
  public: int envTextureSize = 512u;

  /// \brief Compositor used to render rectangle with attached cube map
  public: Ogre::CompositorInstance *cubeMapCompInstance = nullptr;

  /// \brief A Set of 6 cameras,
  ///   each pointing in different direction with FOV of 90deg
  public: Ogre::Camera *envCameras[6];

  /// \brief Render targets for envCameras
  public: Ogre::RenderTarget *envRenderTargets[6];

  /// \brief Viewports for the render targets
  public: Ogre::Viewport *envViewports[6];

  /// \brief Pixel format for cube map texture
  public: Ogre::PixelFormat envCubeMapTextureFormat;

  /// \brief A single cube map texture
  public: Ogre::Texture *envCubeMapTexture = nullptr;

  /// \brief Output texture
  public: Ogre::Texture *ogreRenderTexture = nullptr;

  /// \brief Pointer to material, used for second rendering pass
  public: Ogre::MaterialPtr compMat;

  /// \brief Camera lens description
  public: CameraLens lens;

  /// \brief Pointer to the ogre camera
  public: Ogre::Camera *ogreCamera = nullptr;

  /// \brief Dummy texture
  public: OgreRenderTexturePtr wideAngleTexture;

  /// \brief The image buffer
  public: unsigned char *imageBuffer = nullptr;

  /// \brief Outgoing image data, used by newImageFrame event.
  public: unsigned char *wideAngleImage = nullptr;

  /// \brief Mutex to lock while rendering the world
//  public: std::mutex renderMutex;

  /// \brief Mutex to lock while setting or reading camera properties
  /// \brief Event used to signal camera data
  public: ignition::common::EventT<void(const unsigned char *,
              unsigned int, unsigned int, unsigned int,
              const std::string &)> newImageFrame;
};

using namespace ignition;
using namespace rendering;

//////////////////////////////////////////////////
OgreWideAngleCamera::OgreWideAngleCamera()
    : dataPtr(std::make_unique<OgreWideAngleCameraPrivate>())
{
  // this->dataPtr->lens = new CameraLens();
  for (int i = 0; i < 6; ++i)
  {
    this->dataPtr->envCameras[i] = nullptr;
    this->dataPtr->envRenderTargets[i] = nullptr;
  }
}
//////////////////////////////////////////////////
OgreWideAngleCamera::~OgreWideAngleCamera()
{
//  delete this->Lens();
  this->Destroy();
}

//////////////////////////////////////////////////
void OgreWideAngleCamera::Init()
{
  BaseWideAngleCamera::Init();
  this->CreateCamera();
  this->CreateRenderTexture();
  this->Reset();
  // this->CreateEnvRenderTexture(this->scopedUniqueName + "_envRttTex");
}

/////////////////////////////////////////////////
void OgreWideAngleCamera::CreateRenderTexture()
{
  RenderTexturePtr base = this->scene->CreateRenderTexture();
  this->dataPtr->wideAngleTexture =
      std::dynamic_pointer_cast<OgreRenderTexture>(base);
  this->dataPtr->wideAngleTexture->SetWidth(1);
  this->dataPtr->wideAngleTexture->SetHeight(1);
}

//////////////////////////////////////////////////
void OgreWideAngleCamera::PreRender()
{
  BaseCamera::PreRender();
  if (!this->dataPtr->ogreRenderTexture)
    this->CreateWideAngleTexture();
}

//////////////////////////////////////////////////
//void OgreWideAngleCamera::Load()
//{
//  Camera::Load();
//
//  // Cube map texture format defaults to matching image pixel format
//  this->dataPtr->envCubeMapTextureFormat =
//    static_cast<Ogre::PixelFormat>(this->imageFormat);
//
//  this->CreateEnvCameras();
//
//  if (this->sdf->HasElement("lens"))
//  {
//    sdf::ElementPtr sdfLens = this->sdf->GetElement("lens");
//
//    this->dataPtr->lens->Load(sdfLens);
//
//    if (sdfLens->HasElement("env_texture_size"))
//      this->dataPtr->envTextureSize = sdfLens->Get<int>("env_texture_size");
//
//    const std::string envTextureFormat = "ignition:env_texture_format";
//    if (sdfLens->HasElement(envTextureFormat))
//    {
//      this->dataPtr->envCubeMapTextureFormat = static_cast<Ogre::PixelFormat>(
//        this->OgrePixelFormat(sdfLens->Get<std::string>(envTextureFormat)));
//    }
//  }
//  else
//    this->dataPtr->lens->Load();
//
//  std::string lensType = this->dataPtr->lens->Type();
//  if (lensType == "gnomonical" && this->HFOV() > (IGN_PI/2.0))
//  {
//    ignerr << "The recommended camera horizontal FOV should be <= PI/2"
//        << " for lens of type 'gnomonical'." << std::endl;
//  }
//}

//////////////////////////////////////////////////
void OgreWideAngleCamera::Destroy()
{
  if (this->dataPtr->imageBuffer)
  {
    delete [] this->dataPtr->imageBuffer;
    this->dataPtr->imageBuffer = nullptr;
  }

  if (this->dataPtr->wideAngleImage)
  {
    delete [] this->dataPtr->wideAngleImage;
    this->dataPtr->wideAngleImage = nullptr;
  }

  for (int i = 0; i < 6; ++i)
  {
    OgreRTShaderSystem::DetachViewport(this->dataPtr->envViewports[i],
        this->scene);

    this->dataPtr->envRenderTargets[i]->removeAllViewports();
    this->dataPtr->envRenderTargets[i] = nullptr;

    this->scene->OgreSceneManager()->destroyCamera(
        this->dataPtr->envCameras[i]->getName());
    this->dataPtr->envCameras[i] = nullptr;
  }

  if (this->dataPtr->envCubeMapTexture)
  {
    Ogre::TextureManager::getSingleton().remove(
      this->dataPtr->envCubeMapTexture->getName());
    this->dataPtr->envCubeMapTexture = nullptr;
  }

  if (this->dataPtr->ogreRenderTexture)
  {
    Ogre::TextureManager::getSingleton().remove(
      this->dataPtr->ogreRenderTexture->getName());
    this->dataPtr->ogreRenderTexture = nullptr;
  }
}

//////////////////////////////////////////////////
unsigned int OgreWideAngleCamera::EnvTextureSize() const
{
//  std::lock_guard<std::mutex> lock(this->dataPtr->dataMutex);

  return this->dataPtr->envTextureSize;
}

// //////////////////////////////////////////////////
// OgreCameraLens *OgreWideAngleCamera::Lens() const
// {
//   return this->dataPtr->lens;
// }

//////////////////////////////////////////////////
// void OgreWideAngleCamera::SetRenderTarget(Ogre::RenderTarget *_target)
// {
// //  Camera::SetRenderTarget(_target);
//
//   if (this->renderTarget)
//   {
//     this->dataPtr->cubeMapCompInstance =
//       Ogre::CompositorManager::getSingleton().addCompositor(this->viewport,
//           "WideOgreCameraLensMap/ParametrisedMap");
//
//     if (this->dataPtr->envCubeMapTexture)
//     {
//       this->dataPtr->compMat =
//           Ogre::MaterialManager::getSingleton().getByName("WideLensMap");
//
//       auto pass = this->dataPtr->compMat->getTechnique(0)->getPass(0);
//
//       if (!pass->getNumTextureUnitStates())
//         pass->createTextureUnitState();
//
//       this->dataPtr->cubeMapCompInstance->addListener(this);
//     }
//     else
//       ignerr << "WideAngleCamera compositor texture missng" << std::endl;
//
//
//     this->dataPtr->cubeMapCompInstance->setEnabled(true);
//   }
// }

//////////////////////////////////////////////////
void OgreWideAngleCamera::SetEnvTextureSize(int _size)
{
//  std::lock_guard<std::mutex> lock(this->dataPtr->dataMutex);

//  if (this->sdf->HasElement("env_texture_size"))
//    this->sdf->AddElement("env_texture_size")->Set(_size);
//
//  this->sdf->GetElement("env_texture_size")->Set(_size);
  this->dataPtr->envTextureSize = _size;
}

/////////////////////////////////////////////////
void OgreWideAngleCamera::CreateCamera()
{
  // Create dummy ogre camera object
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

  this->ogreNode->attachObject(this->dataPtr->ogreCamera);
  this->dataPtr->ogreCamera->setFixedYawAxis(false);
  this->dataPtr->ogreCamera->yaw(Ogre::Degree(-90));
  this->dataPtr->ogreCamera->roll(Ogre::Degree(-90));
  this->dataPtr->ogreCamera->setAutoAspectRatio(true);
}


//////////////////////////////////////////////////
void OgreWideAngleCamera::CreateEnvCameras()
{
  double nearPlane = this->NearClipPlane();
  double farPlane = this->FarClipPlane();
  for (int i = 0; i < 6; ++i)
  {
    std::stringstream name_str;

    name_str << this->name << "_env_" << i;

    this->dataPtr->envCameras[i] =
        this->scene->OgreSceneManager()->createCamera(name_str.str());

    this->dataPtr->envCameras[i]->setFixedYawAxis(false);
    this->dataPtr->envCameras[i]->setFOVy(Ogre::Degree(90));
    this->dataPtr->envCameras[i]->setAspectRatio(1);

    this->dataPtr->envCameras[i]->yaw(Ogre::Degree(-90.0));
    this->dataPtr->envCameras[i]->roll(Ogre::Degree(-90.0));

    this->dataPtr->envCameras[i]->setNearClipDistance(nearPlane);
    this->dataPtr->envCameras[i]->setFarClipDistance(farPlane);
  }

  for (int i = 0; i < 6; ++i)
    this->ogreNode->attachObject(this->dataPtr->envCameras[i]);

  // set environment cameras orientation
  this->dataPtr->envCameras[0]->yaw(Ogre::Degree(-90));
  this->dataPtr->envCameras[1]->yaw(Ogre::Degree(90));
  this->dataPtr->envCameras[2]->pitch(Ogre::Degree(90));
  this->dataPtr->envCameras[3]->pitch(Ogre::Degree(-90));
  this->dataPtr->envCameras[5]->yaw(Ogre::Degree(180));
}


//////////////////////////////////////////////////
bool OgreWideAngleCamera::SetBackgroundColor(const math::Color &_color)
{
  bool retVal = true;
  Ogre::ColourValue clr = OgreConversions::Convert(_color);
  if (this->dataPtr->ogreCamera->getViewport())
  {
    this->dataPtr->ogreCamera->getViewport()->setBackgroundColour(clr);
    for (int i = 0; i < 6; ++i)
    {
      if (this->dataPtr->envViewports[i])
      {
        this->dataPtr->envViewports[i]->setBackgroundColour(clr);
      }
      else
      {
        retVal = false;
      }
    }
  }
  else
  {
    retVal = false;
  }
  return retVal;
}

//////////////////////////////////////////////////
void OgreWideAngleCamera::CreateWideAngleTexture()
{
  if (this->dataPtr->ogreCamera == nullptr)
  {
    ignerr << "Ogre camera cannot be created" << std::endl;
    return;
  }

  if (!this->dataPtr->ogreRenderTexture)
  {
    this->dataPtr->ogreRenderTexture =
      Ogre::TextureManager::getSingleton().createManual(
      this->Name() + "_wideAngleCamera", "General", Ogre::TEX_TYPE_2D,
      this->ImageWidth(), this->ImageHeight(), 0,
      Ogre::PF_R8G8B8, Ogre::TU_RENDERTARGET,
      0, false, 0).get();
    Ogre::RenderTarget *rt =
        this->dataPtr->ogreRenderTexture->getBuffer()->getRenderTarget();
    rt->setAutoUpdated(false);
    Ogre::Viewport *vp = rt->addViewport(this->dataPtr->ogreCamera);
    vp->setClearEveryFrame(true);
    vp->setShadowsEnabled(false);
    vp->setOverlaysEnabled(false);
  }

  double ratio = static_cast<double>(this->ImageWidth()) /
                 static_cast<double>(this->ImageHeight());

  double vfov = 2.0 * atan(tan(this->HFOV().Radian() / 2.0) / ratio);
  this->dataPtr->ogreCamera->setAspectRatio(ratio);
  this->dataPtr->ogreCamera->setFOVy(Ogre::Radian(vfov));

  // create the env cameras and textures
  this->CreateEnvCameras();

  unsigned int fsaa = 0;
  std::vector<unsigned int> fsaaLevels =
      OgreRenderEngine::Instance()->FSAALevels();
  // check if target fsaa is supported
  unsigned int targetFSAA = this->antiAliasing;
  auto const it = std::find(fsaaLevels.begin(), fsaaLevels.end(), targetFSAA);
  if (it != fsaaLevels.end())
    fsaa = targetFSAA;
  else
  {
    // output warning but only do it once
    static bool ogreFSAAWarn = false;
    if (ogreFSAAWarn)
    {
      ignwarn << "Anti-aliasing level of '" << this->antiAliasing << "' "
              << "is not supported. Setting to 0" << std::endl;
      ogreFSAAWarn = true;
    }
  }

  this->dataPtr->envCubeMapTexture =
      Ogre::TextureManager::getSingleton().createManual(
          this->name + "_env_tex",
          "General",
          Ogre::TEX_TYPE_CUBE_MAP,
          this->dataPtr->envTextureSize,
          this->dataPtr->envTextureSize,
          0,
          this->dataPtr->envCubeMapTextureFormat,
          Ogre::TU_RENDERTARGET,
          0,
          false,
          fsaa).getPointer();

  for (int i = 0; i < 6; ++i)
  {
    Ogre::RenderTarget *rtt;
    rtt = this->dataPtr->envCubeMapTexture->getBuffer(i)->getRenderTarget();
    rtt->setAutoUpdated(false);

    Ogre::Viewport *vp = rtt->addViewport(this->dataPtr->envCameras[i]);
    vp->setClearEveryFrame(true);
    vp->setShadowsEnabled(true);
    vp->setOverlaysEnabled(false);

    OgreRTShaderSystem::AttachViewport(vp, this->scene);

    auto const &bgColor = this->scene->BackgroundColor();
    vp->setBackgroundColour(OgreConversions::Convert(bgColor));
    // vp->setVisibilityMask(GZ_VISIBILITY_ALL &
        // ~(GZ_VISIBILITY_GUI | GZ_VISIBILITY_SELECTABLE));

    this->dataPtr->envViewports[i] = vp;

//    if (this->scene->GetSkyX())
//      rtt->addListener(this->scene->GetSkyX());

    this->dataPtr->envRenderTargets[i] = rtt;
  }

  // create the compositor
  this->dataPtr->cubeMapCompInstance =
    Ogre::CompositorManager::getSingleton().addCompositor(
    this->dataPtr->ogreCamera->getViewport(),
    "WideCameraLensMap/ParametrisedMap");

  this->dataPtr->compMat =
      Ogre::MaterialManager::getSingleton().getByName("WideLensMap");

  auto pass = this->dataPtr->compMat->getTechnique(0)->getPass(0);

  if (!pass->getNumTextureUnitStates())
    pass->createTextureUnitState();
  this->dataPtr->cubeMapCompInstance->addListener(this);
  this->dataPtr->cubeMapCompInstance->setEnabled(true);
}

//////////////////////////////////////////////////
void OgreWideAngleCamera::Render()
{
  for (int i = 0; i < 6; ++i)
  {
    this->dataPtr->envRenderTargets[i]->update();
  }

  this->dataPtr->compMat->getTechnique(0)->getPass(0)->getTextureUnitState(0)->
      setTextureName(this->dataPtr->envCubeMapTexture->getName());

  Ogre::RenderTarget *rt =
      this->dataPtr->ogreRenderTexture->getBuffer()->getRenderTarget();
  rt->setAutoUpdated(false);
  rt->update(false);
}

//////////////////////////////////////////////////
void OgreWideAngleCamera::notifyMaterialRender(Ogre::uint32 /*_pass_id*/,
                                           Ogre::MaterialPtr &_material)
{
  if (_material.isNull())
    return;

  Ogre::Technique *pTechnique = _material->getBestTechnique();
  if (!pTechnique)
    return;

  Ogre::Pass *pPass = pTechnique->getPass(0);
  if (!pPass || !pPass->hasFragmentProgram())
    return;

  this->SetUniformVariables(pPass,
    this->AspectRatio(),
    this->HFOV().Radian());

#ifndef _WIN32
  // XXX: OGRE doesn't allow to enable cubemap filtering extention thru its API
  // suppose that this function was invoked in a thread that has OpenGL context
  // glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
#endif
}


//////////////////////////////////////////////////
void OgreWideAngleCamera::SetUniformVariables(Ogre::Pass *_pass,
    float _ratio, float _hfov)
{
  Ogre::GpuProgramParametersSharedPtr uniforms =
    _pass->getFragmentProgramParameters();

  uniforms->setNamedConstant("c1", static_cast<Ogre::Real>(
    this->Lens().C1()));
  uniforms->setNamedConstant("c2", static_cast<Ogre::Real>(
    this->Lens().C2()));
  uniforms->setNamedConstant("c3", static_cast<Ogre::Real>(
    this->Lens().C3()));

  if (this->Lens().ScaleToHFOV())
  {
    float param = (_hfov/2)/this->Lens().C2() +
        this->Lens().C3();
    float funRes = this->Lens().ApplyMappingFunction(
        static_cast<float>(param));

    float newF = 1.0f/(this->Lens().C1() * funRes);

    uniforms->setNamedConstant("f", static_cast<Ogre::Real>(newF));
  }
  else
  {
    uniforms->setNamedConstant("f",
        static_cast<Ogre::Real>(this->Lens().F()));
  }

  auto vecFun = this->Lens().MappingFunctionAsVector3d();

  uniforms->setNamedConstant("fun", Ogre::Vector3(
      vecFun.X(), vecFun.Y(), vecFun.Z()));

  uniforms->setNamedConstant("cutOffAngle",
    static_cast<Ogre::Real>(this->Lens().CutOffAngle()));

  Ogre::GpuProgramParametersSharedPtr uniformsVs =
    _pass->getVertexProgramParameters();

  uniformsVs->setNamedConstant("ratio", static_cast<Ogre::Real>(_ratio));

//  std::cerr << "set uniform var "  << _hfov << std::endl;
//  std::cerr << "  c " << this->Lens().C1() << " " << this->Lens().C2() << " " << this->Lens().C3() << std::endl;
//  std::cerr << "  scaleToFov " << this->Lens().ScaleToHFOV() << ", f " << this->Lens().F() << std::endl;
//  std::cerr << "  cutoffangle " << this->Lens().CutOffAngle() << ", ratio " << _ratio << std::endl;

}

//////////////////////////////////////////////////
math::Vector3d OgreWideAngleCamera::Project3d(
    const math::Vector3d &_pt) const
{
  // project onto cubemap face then onto
  ignition::math::Vector3d screenPos;
  // loop through all env cameras can find the one that sees the 3d world point
  for (int i = 0; i < 6; ++i)
  {
    // project world point to camera clip space.
    auto viewProj = this->dataPtr->envCameras[i]->getProjectionMatrix() *
        this->dataPtr->envCameras[i]->getViewMatrix();
    auto pos = viewProj * Ogre::Vector4(OgreConversions::Convert(_pt));
    pos.x /= pos.w;
    pos.y /= pos.w;
    // check if point is visible
    if (std::fabs(pos.x) <= 1 && std::fabs(pos.y) <= 1 && pos.z > 0)
    {
      // determine dir vector to projected point from env camera
      // work in y up, z forward, x right clip space
      ignition::math::Vector3d dir(pos.x, pos.y, 1);
      ignition::math::Quaterniond rot = ignition::math::Quaterniond::Identity;

      // rotate dir vector into wide angle camera frame based on the
      // face of the cube. Note: operate in clip space so
      // left handed coordinate system rotation
      if (i == 0)
        rot = ignition::math::Quaterniond(0.0, M_PI*0.5, 0.0);
      else if (i == 1)
        rot = ignition::math::Quaterniond(0.0, -M_PI*0.5, 0.0);
      else if (i == 2)
        rot = ignition::math::Quaterniond(-M_PI*0.5, 0.0, 0.0);
      else if (i == 3)
        rot = ignition::math::Quaterniond(M_PI*0.5, 0.0, 0.0);
      else if (i == 5)
        rot = ignition::math::Quaterniond(0.0, M_PI, 0.0);
      dir = rot * dir;
      dir.Normalize();

      // compute theta and phi from the dir vector
      // theta is angle to dir vector from z (forward)
      // phi is angle from x in x-y plane
      // direction vector (x, y, z)
      // x = sin(theta)cos(phi)
      // y = sin(theta)sin(phi)
      // z = cos(theta)
      double theta =  std::atan2(
          std::sqrt(dir.X() * dir.X() + dir.Y() * dir.Y()), dir.Z());
      double phi = std::atan2(dir.Y(), dir.X());
      // this also works:
      // double theta = std::acos(dir.Z());
      // double phi = std::asin(dir.Y() / std::sin(theta));

      double f = this->Lens().F();
      double fov = this->HFOV().Radian();
      // recompute f if scale to HFOV is true
      if (this->Lens().ScaleToHFOV())
      {
        double param = (fov/2.0) / this->Lens().C2() + this->Lens().C3();
        double funRes = this->Lens().ApplyMappingFunction(
            static_cast<float>(param));
        f = 1.0/(this->Lens().C1()*funRes);
      }

      // Apply fisheye lens mapping function
      // r is distance of point from image center
      double r = this->Lens().C1() * f *
          this->Lens().ApplyMappingFunction(
          theta/this->Lens().C2() + this->Lens().C3());

      // compute projected x and y in clip space
      double x = cos(phi) * r;
      double y = sin(phi) * r;

      unsigned int vpWidth =
          this->dataPtr->ogreCamera->getViewport()->getActualWidth();
      unsigned int vpHeight =
          this->dataPtr->ogreCamera->getViewport()->getActualHeight();
      // env cam cube map texture is square and likely to be different size from
      // viewport. We need to adjust projected pos based on aspect ratio
      double asp = static_cast<double>(vpWidth) /
        static_cast<double>(vpHeight);
      y *= asp;

      // convert to screen space
      screenPos.X() = ((x / 2.0) + 0.5) * vpWidth;
      screenPos.Y() = (1 - ((y / 2.0) + 0.5)) * vpHeight;

      // r will be > 1.0 if point is not visible (outside of image)
      screenPos.Z() = r;
      return screenPos;
    }
  }

  return screenPos;
}

//////////////////////////////////////////////////
std::vector<Ogre::Camera *> OgreWideAngleCamera::OgreEnvCameras() const
{
  return std::vector<Ogre::Camera *>(
    std::begin(this->dataPtr->envCameras), std::end(this->dataPtr->envCameras));
}

//////////////////////////////////////////////////
void OgreWideAngleCamera::PostRender()
{
  if (this->dataPtr->newImageFrame.ConnectionCount() <= 0u)
    return;

  unsigned int width = this->ImageWidth();
  unsigned int height = this->ImageHeight();
  unsigned int len = width * height;

  PixelFormat format = PF_R8G8B8;
  unsigned int channelCount = PixelUtil::ChannelCount(format);
  unsigned int bytesPerChannel = PixelUtil::BytesPerChannel(format);

  if (!this->dataPtr->wideAngleImage)
    this->dataPtr->wideAngleImage = new unsigned char[len * channelCount];
  if (!this->dataPtr->imageBuffer)
    this->dataPtr->imageBuffer = new unsigned char[len * channelCount];

  // get image data
  Ogre::RenderTarget *rt =
      this->dataPtr->ogreRenderTexture->getBuffer()->getRenderTarget();
  Ogre::PixelBox ogrePixelBox(width, height, 1,
      OgreConversions::Convert(format), this->dataPtr->imageBuffer);
  rt->copyContentsToMemory(ogrePixelBox);

  // fill image data
  memcpy(this->dataPtr->wideAngleImage, this->dataPtr->imageBuffer,
      height*width*channelCount*bytesPerChannel);

  this->dataPtr->newImageFrame(
      this->dataPtr->wideAngleImage, width, height, channelCount, "PF_R8G8B8");

  // Uncomment to debug wide angle cameraoutput
  // igndbg << "wxh: " << width << " x " << height << std::endl;
  // for (unsigned int i = 0; i < height; ++i)
  // {
  //   for (unsigned int j = 0; j < width * channelCount; j += channelCount)
  //   {
  //     unsigned int idx = i * width * channelCount + j;
  //     unsigned int r = this->dataPtr->wideAngleImage[idx];
  //     unsigned int g = this->dataPtr->wideAngleImage[idx + 1];
  //     unsigned int b = this->dataPtr->wideAngleImage[idx + 2];
  //     std::cout << "[" << r << "," << g << "," << b << "]";
  //   }
  //   std::cout << std::endl;
  // }
}

//////////////////////////////////////////////////
common::ConnectionPtr OgreWideAngleCamera::ConnectNewWideAngleFrame(
    std::function<void(const unsigned char *, unsigned int, unsigned int,
      unsigned int, const std::string &)>  _subscriber)
{
  return this->dataPtr->newImageFrame.Connect(_subscriber);
}

//////////////////////////////////////////////////
RenderTargetPtr OgreWideAngleCamera::RenderTarget() const
{
  return this->dataPtr->wideAngleTexture;
}
