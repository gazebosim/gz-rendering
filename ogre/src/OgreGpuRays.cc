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

#include <ignition/common/Time.hh>
#include <ignition/common/Timer.hh>
#include <ignition/common/Mesh.hh>
#include <ignition/common/SubMesh.hh>
#include <ignition/common/MeshManager.hh>
#include <ignition/common/PluginMacros.hh>

#include <ignition/math/Color.hh>
#include <ignition/math/Helpers.hh>
#include <ignition/math/Vector3.hh>
#include <ignition/math/Angle.hh>

#include "ignition/rendering/ogre/OgreGpuRays.hh"
#include "ignition/rendering/ShaderParams.hh"

using namespace ignition;
using namespace rendering;

//////////////////////////////////////////////////
OgreGpuRays::OgreGpuRays()
  : dataPtr(new OgreGpuRaysPrivate())
{
}

//////////////////////////////////////////////////
OgreGpuRays::~OgreGpuRays()
{
  if (this->dataPtr->laserBuffer)
    delete [] this->dataPtr->laserBuffer;

  if (this->dataPtr->laserScan)
    delete [] this->dataPtr->laserScan;

  this->dataPtr->visual.reset();
  this->dataPtr->texIdx.clear();
  this->dataPtr->texCount = 0;
}

//////////////////////////////////////////////////
void OgreGpuRays::Init()
{
  BaseGpuRays::Init();
  this->CreateMaterials();
  this->CreateCamera();
//  this->Reset();
}

/////////////////////////////////////////////////
void OgreGpuRays::CreateMaterials()
{
  // Base material used to create each pass OgreMaterial
  MaterialPtr baseMaterial;

  // Get shader parameters path
  const char *env = std::getenv("IGN_RENDERING_RESOURCE_PATH");
  std::string resourcePath = (env) ? std::string(env) :
      IGN_RENDERING_RESOURCE_PATH;

  // Path to look for vertex and fragment shader parameters
  std::string laser_scan1st_vertex_shader_path = common::joinPaths(
      resourcePath, "ogre", "media", "materials", "programs",
      this->dataPtr->laser_scan1st_vertex_shader_file);
  std::string laser_scan1st_fragment_shader_path = common::joinPaths(
      resourcePath, "ogre", "media", "materials", "programs",
      this->dataPtr->laser_scan1st_fragment_shader_file);

  // Path to look for vertex and fragment shader parameters
  std::string laser_scan2nd_vertex_shader_path = common::joinPaths(
      resourcePath, "ogre", "media", "materials", "programs",
      this->dataPtr->laser_scan2nd_vertex_shader_file);
  std::string laser_scan2nd_fragment_shader_path = common::joinPaths(
      resourcePath, "ogre", "media", "materials", "programs",
      this->dataPtr->laser_scan2nd_fragment_shader_file);

  // Set LaserScan1st material
  baseMaterial = this->scene->CreateMaterial();
  this->dataPtr->matFirstPass =
    std::dynamic_pointer_cast<OgreMaterial>(baseMaterial);
  this->dataPtr->matFirstPass->SetVertexShader(
      laser_scan1st_vertex_shader_path);
  this->dataPtr->matFirstPass->SetFragmentShader(
      laser_scan1st_fragment_shader_path);

  Ogre::Pass *pass =
      this->dataPtr->matFirstPass->Material()->getBestTechnique()->getPass(0);
  pass->setCullingMode(Ogre::CULL_NONE);

  // Set LaserScan2nd material
  baseMaterial = this->scene->CreateMaterial();
  this->dataPtr->matSecondPass =
    std::dynamic_pointer_cast<OgreMaterial>(baseMaterial);
  this->dataPtr->matSecondPass->SetVertexShader(
      laser_scan2nd_vertex_shader_path);
  this->dataPtr->matSecondPass->SetFragmentShader(
      laser_scan2nd_fragment_shader_path);
}

/////////////////////////////////////////////////
void OgreGpuRays::SetFragmentShadersParams()
{
  ShaderParamsPtr matFirstParams =
    this->dataPtr->matFirstPass->FragmentShaderParams();

  // Configure fragment shader variables
  (*matFirstParams)["far"] = static_cast<float>(this->FarClipPlane());
  (*matFirstParams)["near"] = static_cast<float>(this->NearClipPlane());
  (*matFirstParams)["retro"] = static_cast<float>(0.0);

  ShaderParamsPtr matSecondParams =
    this->dataPtr->matSecondPass->FragmentShaderParams();

  // Configure fragment shader variables
  (*matSecondParams)["tex1"] = static_cast<int>(0);
  (*matSecondParams)["tex2"] = static_cast<int>(1);
  (*matSecondParams)["tex3"] = static_cast<int>(2);
  (*matSecondParams)["texSize"] = static_cast<int>(this->dataPtr->w2nd * this->dataPtr->h2nd);
}

/////////////////////////////////////////////////
void OgreGpuRays::CreateCamera()
{
  // Create ogre camera object
  Ogre::SceneManager *ogreSceneManager;
  ogreSceneManager = this->scene->OgreSceneManager();
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

  // Create first pass texture which is going to be used as initial
  // render texture. If more render texture are used will be initiated
  // later with the same parameters than the first one.
  if (this->dataPtr->firstPassTextures[0] == nullptr)
  {
    RenderTexturePtr firstTextureBase =
        this->scene->CreateRenderTexture();
    this->dataPtr->firstPassTextures[0] =
      std::dynamic_pointer_cast<OgreRenderTexture>(firstTextureBase);

    this->SetRenderTexture(this->dataPtr->firstPassTextures[0],
                          this->dataPtr->matFirstPass,
                          this->dataPtr->ogreCamera,
                          Ogre::ColourValue(this->farClip, 0.0, 1.0),
                          PF_FLOAT32_RGB, 320, 1);
  }
}

/////////////////////////////////////////////////
void OgreGpuRays::CreateLaserTexture()
{
  // horizontal laser setup
  this->hfov = (this->AngleMax() - this->AngleMin()).Radian();

  if (this->hfov > 2 * M_PI)
  {
    this->hfov = 2 * M_PI;
    ignwarn << "Horizontal FOV for GPU laser is capped at 180 degrees.\n";
  }

  this->SetHorzHalfAngle((this->AngleMax() + this->AngleMin()).Radian() / 2.0);

  // determine number of cameras to use
  if (this->hfov > 2.8)
  {
    if (this->hfov > 5.6)
    {
      this->cameraCount = 3;
    }
    else
    {
      this->cameraCount = 2;
    }
  }
  else
  {
    this->cameraCount = 1;
  }
  this->SetCameraCount(cameraCount);

  // horizontal fov of single frame
  this->hfov = this->hfov / cameraCount;
  this->SetCosHorzFOV(hfov);

  // Fixed minimum resolution of texture to reduce steps in ranges
  // when hitting surfaces where the angle between ray and surface is small.
  // Also have to keep in mind the GPU's max. texture size
  unsigned int horzRayCountPerCamera =
      std::max(2048U, this->hSamples / cameraCount);
  unsigned int vertRayCountPerCamera = this->vSamples;

  // vertical laser setup
  this->vfov = (this->VerticalAngleMax() - this->VerticalAngleMin()).Radian();

  if (this->vfov > M_PI / 2)
  {
    this->vfov = M_PI / 2;
    ignwarn << "Vertical FOV for GPU laser is capped at 90 degrees.\n";
  }

  this->SetVertHalfAngle((this->VerticalAngleMax() +
        this->VerticalAngleMin()).Radian() / 2.0);
  this->SetVerticalAngleMin(this->VertHalfAngle() - (this->vfov / 2));
  this->SetVerticalAngleMax(this->VertHalfAngle() + (this->vfov / 2));

  // Assume camera always stays horizontally even if vert. half angle of
  // laser is not 0. Add padding to camera vfov.
  double vfovCamera = this->vfov + 2 * std::abs(this->VertHalfAngle());

  // Add padding to vertical camera FOV to cover all possible rays
  // for given laser vert. and horiz. FOV
  vfovCamera = 2 * atan(tan(vfovCamera / 2) / cos(this->hfov / 2));

  if (vfovCamera > 2.8)
  {
    ignerr << "Vertical FOV of internal camera exceeds 2.8 radians.\n";
  }

  this->SetCosVertFOV(vfovCamera);

  // If vertical ray is not 1 adjust horizontal and vertical
  // ray count to maintain aspect ratio
  if (this->vSamples > 1)
  {
    double cameraAspectRatio = tan(hfov / 2.0) / tan(vfovCamera / 2.0);

    this->SetRayCountRatio(cameraAspectRatio);
    this->rayCountRatio = cameraAspectRatio;

    if ((horzRayCountPerCamera / this->RayCountRatio()) >
         vertRayCountPerCamera)
    {
      vertRayCountPerCamera =
          round(horzRayCountPerCamera / this->RayCountRatio());
    }
    else
    {
      horzRayCountPerCamera =
          round(vertRayCountPerCamera * this->RayCountRatio());
    }
  }
  else
  {
    // In case of 1 vert. ray, set a very small vertical FOV for camera
    this->SetRayCountRatio(horzRayCountPerCamera);
  }

  // Configure second pass texture size
  this->SetSecondPassTextureSize(this->RayCount(), this->VerticalRayCount());

  this->CreateOrthoCam();

  this->dataPtr->textureCount = this->cameraCount;
  if (this->dataPtr->textureCount == 2)
  {
    this->dataPtr->cameraYaws[0] = -this->hfov/2;
    this->dataPtr->cameraYaws[1] = +this->hfov;
    this->dataPtr->cameraYaws[2] = 0;
    this->dataPtr->cameraYaws[3] = -this->hfov/2;
  }
  else
  {
    this->dataPtr->cameraYaws[0] = -this->hfov;
    this->dataPtr->cameraYaws[1] = +this->hfov;
    this->dataPtr->cameraYaws[2] = +this->hfov;
    this->dataPtr->cameraYaws[3] = -this->hfov;
  }

  this->dataPtr->ogreCamera->yaw(Ogre::Radian(this->horzHalfAngle));
  this->dataPtr->ogreCamera->setAspectRatio(this->RayCountRatio());
  this->dataPtr->ogreCamera->setFOVy(Ogre::Radian(this->CosVertFOV()));

  // Configure first pass textures that are not yet configured properly
  for (unsigned int i = 0; i < this->dataPtr->textureCount; ++i)
  {
    RenderTexturePtr firstTextureBase =
        this->scene->CreateRenderTexture();
    this->dataPtr->firstPassTextures[i] =
      std::dynamic_pointer_cast<OgreRenderTexture>(firstTextureBase);

    this->SetRenderTexture(this->dataPtr->firstPassTextures[i],
                          this->dataPtr->matFirstPass,
                          this->dataPtr->ogreCamera,
                          Ogre::ColourValue(this->farClip, 0.0, 1.0),
                          PF_FLOAT32_RGB,
                          horzRayCountPerCamera,
                          vertRayCountPerCamera);
  }

  // Configure second pass texture
  RenderTexturePtr secondTextureBase =
      this->scene->CreateRenderTexture();
  this->dataPtr->secondPassTexture =
    std::dynamic_pointer_cast<OgreRenderTexture>(secondTextureBase);

  this->SetRenderTexture(this->dataPtr->secondPassTexture,
                        this->dataPtr->matSecondPass,
                        this->dataPtr->orthoCam,
                        Ogre::ColourValue(0.0, 0.0, 1.0),
                        PF_FLOAT32_RGB,
                        this->dataPtr->w2nd,
                        this->dataPtr->h2nd);
  this->dataPtr->secondPassTexture->SetBackgroundColor(
      this->scene->BackgroundColor());

  Ogre::Matrix4 p = this->BuildScaledOrthoMatrix(
      0, static_cast<float>(this->dataPtr->w2nd / 10.0),
      0, static_cast<float>(this->dataPtr->h2nd / 10.0),
      0.01, 0.02);

  this->dataPtr->orthoCam->setCustomProjectionMatrix(true, p);

  Ogre::TextureUnitState *texUnit;
  for (unsigned int i = 0; i < this->dataPtr->textureCount; ++i)
  {
    unsigned int texIndex = this->dataPtr->texCount++;
    Ogre::Technique *technique =
      this->dataPtr->matSecondPass->Material()->getTechnique(0);
    IGN_ASSERT(technique,
        "OgreGpuRays material script error: technique not found");

    Ogre::Pass *pass = technique->getPass(0);
    IGN_ASSERT(pass,
        "OgreGpuRays material script error: pass not found");

    if (!pass->getTextureUnitState(
        this->dataPtr->firstPassTextures[i]->Name()))
    {
      texUnit = pass->createTextureUnitState(
            this->dataPtr->firstPassTextures[i]->Name(), texIndex);

      this->dataPtr->texIdx.push_back(texIndex);

      texUnit->setTextureFiltering(Ogre::TFO_NONE);
      texUnit->setTextureAddressingMode(Ogre::TextureUnitState::TAM_MIRROR);
    }
  }

  this->SetFragmentShadersParams();
  this->CreateCanvas();
}

/////////////////////////////////////////////////
void OgreGpuRays::UpdateRenderTarget(Ogre::RenderTarget *_target,
                   Ogre::MaterialPtr _material, Ogre::Camera *_cam,
                   bool _updateTex)
{
  std::cerr << " ogregpuray update render target " << std::endl;
  Ogre::RenderSystem *renderSys;
  Ogre::Viewport *vp = NULL;
  Ogre::SceneManager *sceneMgr = this->scene->OgreSceneManager();
  Ogre::Pass *pass;

  renderSys = this->scene->OgreSceneManager()->getDestinationRenderSystem();
  // Get pointer to the material pass
  pass = _material->getBestTechnique()->getPass(0);

  std::cerr << "_material name " << _material->getName() << " " << _material->isLoaded() << std::endl;

  // Render the depth texture
  // OgreSceneManager::_render function automatically sets farClip to 0.
  // Which normally equates to infinite distance. We don't want this. So
  // we have to set the distance every time.
  _cam->setFarClipDistance(this->FarClipPlane());
  std::cerr << "this->far " << this->FarClipPlane() << std::endl;

  Ogre::AutoParamDataSource autoParamDataSource;

  vp = _target->getViewport(0);

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

  std::cerr << " ogregpuray update render target done" << std::endl;
}

//////////////////////////////////////////////////
void OgreGpuRays::Render()
{
  std::cerr << " ogre gpu rays render " << this->dataPtr->textureCount << std::endl;
  ignition::common::Timer firstPassTimer, secondPassTimer;

  firstPassTimer.Start();

  Ogre::SceneManager *sceneMgr = this->scene->OgreSceneManager();

  sceneMgr->_suppressRenderStateChanges(true);
  sceneMgr->addRenderObjectListener(this);

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

    //this->dataPtr->currentTexture->Render();
    this->UpdateRenderTarget(this->dataPtr->firstPassTextures[i]->RenderTarget(),
                             this->dataPtr->matFirstPass->Material(),
                             this->dataPtr->ogreCamera, false);
  }

  if (this->dataPtr->textureCount > 1)
      this->Node()->roll(Ogre::Radian(this->dataPtr->cameraYaws[3]));

  sceneMgr->removeRenderObjectListener(this);

  double firstPassDur = firstPassTimer.Elapsed().Double();
  secondPassTimer.Start();

  this->dataPtr->visual->SetVisible(true);

  this->UpdateRenderTarget(this->dataPtr->secondPassTexture->RenderTarget(),
                           this->dataPtr->matSecondPass->Material(),
                           this->dataPtr->orthoCam, true);

  this->dataPtr->visual->SetVisible(false);

  sceneMgr->_suppressRenderStateChanges(false);

  double secondPassDur = secondPassTimer.Elapsed().Double();
  this->dataPtr->lastRenderDuration = firstPassDur + secondPassDur;

  std::cerr << " ogre gpu rays render done" << std::endl;
}

//////////////////////////////////////////////////
void OgreGpuRays::PostRender()
{
  for (unsigned int i = 0; i < this->dataPtr->textureCount; ++i)
  {
    this->dataPtr->firstPassTextures[i]->SwapBuffers();
  }
  this->dataPtr->secondPassTexture->SwapBuffers();

  const Ogre::Viewport *secondPassViewport =
    this->dataPtr->secondPassTexture->Viewport(0);
  unsigned int width = secondPassViewport->getActualWidth();
  unsigned int height = secondPassViewport->getActualHeight();

  Ogre::PixelFormat imageFormat = OgreConversions::Convert(
    this->dataPtr->secondPassTexture->Format());
  size_t size = Ogre::PixelUtil::getMemorySize(
    width, height, 1, imageFormat);

  if (!this->dataPtr->laserBuffer)
    this->dataPtr->laserBuffer = new float[size];

  this->dataPtr->secondPassTexture->Buffer(
      this->dataPtr->laserBuffer);

  if (!this->dataPtr->laserScan)
  {
    int len = this->dataPtr->w2nd * this->dataPtr->h2nd * 3;
    this->dataPtr->laserScan = new float[len];
  }

  memcpy(this->dataPtr->laserScan, this->dataPtr->laserBuffer,
         this->dataPtr->w2nd * this->dataPtr->h2nd * 3 *
         sizeof(this->dataPtr->laserScan[0]));

  this->dataPtr->newLaserFrame(this->dataPtr->laserScan,
      this->dataPtr->w2nd, this->dataPtr->h2nd, 3, "BLABLA");
}

//////////////////////////////////////////////////
const float* OgreGpuRays::RaysData() const
{
  return this->dataPtr->laserScan;
}

//////////////////////////////////////////////////
void OgreGpuRays::SetRenderTexture(OgreRenderTexturePtr _texture,
                                   OgreMaterialPtr _material,
                                   Ogre::Camera *_camera,
                                   Ogre::ColourValue _color,
                                   PixelFormat _format,
                                   const unsigned int _width,
                                   const unsigned int _height)
{
  // Configure material pass
  Ogre::Technique *technique = _material->Material()->getTechnique(0);
  Ogre::Pass *pass = technique->getPass(0);
//  pass->setDepthCheckEnabled(false);
//  pass->setDepthWriteEnabled(false);
//  pass->setLightingEnabled(false);
//  pass->setFog(true, Ogre::FOG_NONE);
  // pass->setCullingMode(Ogre::CULL_NONE);

  // Configure the texture
  _texture->SetCamera(_camera);
  _texture->SetFormat(_format);
  _texture->SetWidth(_width);
  _texture->SetHeight(_height);
  _texture->PreRender();
  _texture->SetMaterial(_material);
  _texture->SetAutoUpdated(false);

  // Setup the viewport to use the texture
  Ogre::Viewport *vp = _texture->RenderTarget()->getViewport(0);
  vp->setClearEveryFrame(true);
  vp->setOverlaysEnabled(false);
  vp->setShadowsEnabled(false);
  vp->setSkiesEnabled(false);
  vp->setBackgroundColour(_color);
//  vp->setVisibilityMask(
//      IGN_VISIBILITY_ALL & ~(IGN_VISIBILITY_GUI | IGN_VISIBILITY_SELECTABLE));
}

/////////////////////////////////////////////////
void OgreGpuRays::CreateOrthoCam()
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

  Ogre::SceneNode *rootSceneNode = std::dynamic_pointer_cast<
    ignition::rendering::OgreNode>(this->scene->RootVisual())->Node();
  this->dataPtr->pitchNodeOrtho = rootSceneNode->createChildSceneNode();
  this->dataPtr->pitchNodeOrtho->attachObject(this->dataPtr->orthoCam);

  // Use X/Y as horizon, Z up
  this->dataPtr->orthoCam->pitch(Ogre::Degree(90));

  // Don't yaw along variable axis, causes leaning
  this->dataPtr->orthoCam->setFixedYawAxis(true, Ogre::Vector3::UNIT_Z);
  this->dataPtr->orthoCam->setDirection(1, 0, 0);
  this->dataPtr->orthoCam->setAutoAspectRatio(true);

  if (this->dataPtr->orthoCam)
  {
    this->dataPtr->orthoCam->setNearClipDistance(0.01);
    this->dataPtr->orthoCam->setFarClipDistance(0.02);
    this->dataPtr->orthoCam->setRenderingDistance(0.02);

    this->dataPtr->orthoCam->setProjectionType(Ogre::PT_ORTHOGRAPHIC);
  }
}

/////////////////////////////////////////////////
Ogre::Matrix4 OgreGpuRays::BuildScaledOrthoMatrix(const float _left,
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
void OgreGpuRays::SetSecondPassTextureSize(
    const unsigned int _w, const unsigned int _h)
{
  this->dataPtr->w2nd = _w;
  this->dataPtr->h2nd = _h;
}

/////////////////////////////////////////////////
void OgreGpuRays::CreateMesh()
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
  // see OgreGpuRays::Set2ndPassTarget() on how the ortho cam is set up
  double startX = dx;
  double startY = this->dataPtr->h2nd/10.0;

  // half of actual camera vertical FOV without padding
  double phi = this->VertFOV() / 2;
  double phiCamera = phi + std::abs(this->VertHalfAngle());
  double theta = this->CosHorzFOV() / 2;

  if (this->ImageHeight() == 1)
  {
    phi = 0;
  }

  // index of ray
  unsigned int ptsOnLine = 0;

  // total laser hfov
  double thfov = this->dataPtr->textureCount * this->CosHorzFOV();
  double hstep = thfov / (this->dataPtr->w2nd - 1);
  double vstep = 2 * phi / (this->dataPtr->h2nd - 1);

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
      // current horizontal angle from start of laser scan
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

      // the texture/1000.0 value is used in the laser_2nd_pass.frag shader
      // as a trick to determine which camera texture to use when stitching
      // together the final depth image.
      submesh->AddVertex(texture/1000.0, startX, startY);

      // first compute angle from the start of current camera's horizontal
      // min angle, then set delta to be angle from center of current camera.
      delta = delta - (texture * this->CosHorzFOV());
      delta = delta - theta;

      // adjust uv coordinates of depth texture to match projection of current
      // laser ray the depth image plane.
      double u = 0.5 - tan(delta) / (2.0 * tan(theta));
      double v = 0.5 - (tan(gamma) * cos(theta)) /
          (2.0 * tan(phiCamera) * cos(delta));

      submesh->AddTexCoord(u, v);
      submesh->AddIndex(this->dataPtr->w2nd * j + i);
    }
  }

  mesh->AddSubMesh(*submesh);

  this->dataPtr->undistMesh = mesh;

  common::MeshManager::Instance()->AddMesh(this->dataPtr->undistMesh);
}

/////////////////////////////////////////////////
void OgreGpuRays::CreateCanvas()
{
  this->CreateMesh();

  this->dataPtr->visual = this->scene->CreateVisual(
      this->Name() + "second_pass_canvas");

  Ogre::SceneNode *visualSceneNode =  std::dynamic_pointer_cast<
    ignition::rendering::OgreNode>(this->dataPtr->visual)->Node();

  Ogre::Node *visualParent = visualSceneNode->getParent();
  if (visualParent != nullptr)
  {
    visualParent->removeChild(visualSceneNode);

  }
  this->dataPtr->pitchNodeOrtho->addChild(visualSceneNode);

  // Convert mesh from common::Mesh to rendering::mesh and add it to
  // the canvas visual
  MeshPtr renderingMesh = this->scene->CreateMesh(
      this->dataPtr->undistMesh);
  this->dataPtr->visual->AddGeometry(renderingMesh);

  this->dataPtr->visual->SetLocalPosition(0.01, 0, 0);
  this->dataPtr->visual->SetLocalRotation(0, 0, 0);

  MaterialPtr canvasMaterial =
    this->scene->CreateMaterial(this->Name() + "_green");
  canvasMaterial->SetAmbient(ignition::math::Color(0, 1, 0, 1));
  this->dataPtr->visual->SetMaterial(canvasMaterial);

  this->dataPtr->visual->SetVisible(true);
}

/////////////////////////////////////////////////
void OgreGpuRays::notifyRenderSingleObject(Ogre::Renderable *_rend,
      const Ogre::Pass* /*pass*/, const Ogre::AutoParamDataSource* /*source*/,
      const Ogre::LightList* /*lights*/, bool /*supp*/)
{
  std::cerr << " notify render single object " << std::endl;

  Ogre::Vector4 retro = Ogre::Vector4(0, 0, 0, 0);
  try
  {
    retro = _rend->getCustomParameter(1);
  }
  catch(Ogre::ItemIdentityException& e)
  {
    _rend->setCustomParameter(1, Ogre::Vector4(0, 0, 0, 0));
  }

  std::cout << "Material notify " << this->dataPtr->currentMat->Material()->getName() << std::endl;
  Ogre::Pass *pass =
    this->dataPtr->currentMat->Material()->getBestTechnique()->getPass(0);
  Ogre::RenderSystem *renderSys =
    this->scene->OgreSceneManager()->getDestinationRenderSystem();

  Ogre::AutoParamDataSource autoParamDataSource;

  Ogre::Viewport *vp =
    this->dataPtr->currentTexture->RenderTarget()->getViewport(0);

  renderSys->_setViewport(vp);
  autoParamDataSource.setCurrentRenderable(_rend);
  autoParamDataSource.setCurrentPass(pass);
  autoParamDataSource.setCurrentViewport(vp);
  autoParamDataSource.setCurrentRenderTarget(
      this->dataPtr->currentTexture->RenderTarget());
  autoParamDataSource.setCurrentSceneManager(this->scene->OgreSceneManager());
  autoParamDataSource.setCurrentCamera(this->dataPtr->ogreCamera, true);

  pass->_updateAutoParams(&autoParamDataSource,
      Ogre::GPV_GLOBAL || Ogre::GPV_PER_OBJECT);
  pass->getFragmentProgramParameters()->setNamedConstant("retro", retro[0]);
  renderSys->bindGpuProgram(
      pass->getVertexProgram()->_getBindingDelegate());

  renderSys->bindGpuProgramParameters(Ogre::GPT_VERTEX_PROGRAM,
      pass->getVertexProgramParameters(),
      Ogre::GPV_GLOBAL || Ogre::GPV_PER_OBJECT);

  renderSys->bindGpuProgram(
      pass->getFragmentProgram()->_getBindingDelegate());

  renderSys->bindGpuProgramParameters(Ogre::GPT_FRAGMENT_PROGRAM,
      pass->getFragmentProgramParameters(),
      (Ogre::GPV_GLOBAL || Ogre::GPV_PER_OBJECT));
}

//////////////////////////////////////////////////
ignition::common::ConnectionPtr OgreGpuRays::ConnectNewLaserFrame(
    std::function<void(const float *_frame, unsigned int _width,
    unsigned int _height, unsigned int _depth,
    const std::string &_format)> _subscriber)
{
  return this->dataPtr->newLaserFrame.Connect(_subscriber);
}

//////////////////////////////////////////////////
RenderTargetPtr OgreGpuRays::RenderTarget() const
{
  return this->dataPtr->firstPassTextures[0];
}

