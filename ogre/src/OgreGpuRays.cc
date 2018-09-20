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
  this->CreateLaserTexture();
  this->Reset();
  this->dataPtr->visual.reset();
  this->Reset();
}

/////////////////////////////////////////////////
void OgreGpuRays::CreateCamera()
{
  // create ogre camera object
  Ogre::SceneManager *ogreSceneManager;
  ogreSceneManager = this->scene->OgreSceneManager();
  if (ogreSceneManager == nullptr)
  {
    ignerr << "Scene manager cannot be obtained" << std::endl;
    return;
  }

  this->dataPtr->ogreCamera = ogreSceneManager->createCamera(
      this->name + "_Camera");
  if (this->dataPtr->ogreCamera == nullptr)
  {
    ignerr << "Ogre camera cannot be created" << std::endl;
    return;
  }

  this->ogreNode->attachObject(this->dataPtr->ogreCamera);

  this->dataPtr->ogreCamera->yaw(Ogre::Radian(this->horzHalfAngle));
}

/////////////////////////////////////////////////
void OgreGpuRays::CreateLaserTexture()
{
  this->CreateCamera();
  this->CreateOrthoCam();

  this->dataPtr->textureCount = this->cameraCount;
  this->SetRangeCount(640, 480);
  this->SetHorzFOV(2.4);
  this->SetVertFOV(0);

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

  // Set Gazebo/LaserScan1st material
  this->dataPtr->matFirstPass = this->scene->CreateMaterial();
  this->dataPtr->matFirstPass->SetDepthMaterial(
      this->FarClipPlane(), this->NearClipPlane());

  for (unsigned int i = 0; i < this->dataPtr->textureCount; ++i)
  {
    std::stringstream texName;
    texName << "gpu_rays_texture_first_pass_" << i;
    RenderTexturePtr firstTextureBase =
        this->scene->CreateRenderTexture();
    this->dataPtr->firstPassTextures[i] =
      std::dynamic_pointer_cast<OgreRenderTexture>(firstTextureBase);
    this->dataPtr->firstPassTextures[i]->SetFormat(PF_FLOAT32_RGB);
    // this->Set1stPassTarget(this->dataPtr->firstPassTextures[i], i);
    // this->dataPtr->firstPassTextures[i]->SetAutoUpdated(false);
    this->dataPtr->firstPassTextures[i]->SetMaterial(
        this->dataPtr->matFirstPass);
  }

  RenderTexturePtr secondTextureBase =
      this->scene->CreateRenderTexture();
  this->dataPtr->secondPassTexture =
    std::dynamic_pointer_cast<OgreRenderTexture>(secondTextureBase);
  this->dataPtr->secondPassTexture->SetFormat(PF_FLOAT32_RGB);
  // this->Set2ndPassTarget(this->dataPtr->secondPassTexture, i);
  // this->dataPtr->secondPassTexture->SetAutoUpdated(false);

  // Set Gazebo/LaserScan2nd material
  this->dataPtr->matSecondPass = this->scene->CreateMaterial();
  this->dataPtr->matSecondPass->SetDepthMaterial(
      this->FarClipPlane(), this->NearClipPlane());
  this->dataPtr->secondPassTexture->SetMaterial(
      this->dataPtr->matSecondPass);

  Ogre::TextureUnitState *texUnit;
  for (unsigned int i = 0; i < this->dataPtr->textureCount; ++i)
  {
    unsigned int texIndex = this->dataPtr->texCount++;
    OgreMaterialPtr ogreMaterial = std::dynamic_pointer_cast<OgreMaterial>(
        this->dataPtr->matSecondPass);
    Ogre::Technique *technique = ogreMaterial->Material()->getTechnique(0);
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

  this->CreateCanvas();
}

//////////////////////////////////////////////////
void OgreGpuRays::Render()
{
  ignition::common::Timer firstPassTimer, secondPassTimer;

  firstPassTimer.Start();

  Ogre::SceneManager *sceneMgr = this->scene->OgreSceneManager();

  sceneMgr->_suppressRenderStateChanges(true);
  sceneMgr->addRenderObjectListener(this);

  Ogre::ShadowTechnique shadowTech = sceneMgr->getShadowTechnique();
  sceneMgr->setShadowTechnique(Ogre::SHADOWTYPE_NONE);

  for (unsigned int i = 0; i < this->dataPtr->textureCount; ++i)
  {
    // return farClip in case no renderable object is inside frustrum
    Ogre::Viewport *vp = this->dataPtr->firstPassTextures[i]->Viewport(0);
    vp->setBackgroundColour(Ogre::ColourValue(this->FarClipPlane(),
        this->FarClipPlane(), this->FarClipPlane()));

    if (this->dataPtr->textureCount > 1)
    {
      // Cannot call Camera::RotateYaw because it rotates in world frame,
      // but we need rotation in camera local frame
      this->Node()->roll(Ogre::Radian(this->dataPtr->cameraYaws[i]));
    }

    this->dataPtr->currentMat = this->dataPtr->matFirstPass;
    this->dataPtr->currentTexture = this->dataPtr->firstPassTextures[i];

    this->dataPtr->firstPassTextures[i]->SetAutoUpdated(false);
    this->dataPtr->firstPassTextures[i]->Render();
  }

  if (this->dataPtr->textureCount > 1)
      this->Node()->roll(Ogre::Radian(this->dataPtr->cameraYaws[3]));

  sceneMgr->removeRenderObjectListener(this);

  double firstPassDur = firstPassTimer.Elapsed().Double();
  secondPassTimer.Start();

  // this->dataPtr->visual->SetVisible(true);

  // return farClip in case no renderable object is inside frustrum
  Ogre::Viewport *vp = this->dataPtr->secondPassTexture->Viewport(0);
  vp->setBackgroundColour(Ogre::ColourValue(this->FarClipPlane(),
      this->FarClipPlane(), this->FarClipPlane()));

  this->dataPtr->secondPassTexture->SetAutoUpdated(false);
  this->dataPtr->secondPassTexture->Render();

  // this->dataPtr->visual->SetVisible(false);

  sceneMgr->_suppressRenderStateChanges(false);
  sceneMgr->setShadowTechnique(shadowTech);

  double secondPassDur = secondPassTimer.Elapsed().Double();
  this->dataPtr->lastRenderDuration = firstPassDur + secondPassDur;
}

//////////////////////////////////////////////////
void OgreGpuRays::PostRender()
{
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
const float* OgreGpuRays::LaserData() const
{
  return this->dataPtr->laserBuffer;
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
      this->name + "_Ortho_Camera");
  if (this->dataPtr->orthoCam == nullptr)
  {
    ignerr << "Ogre camera cannot be created" << std::endl;
    return;
  }

  this->dataPtr->pitchNodeOrtho = this->Node()->createChildSceneNode();

  // Use X/Y as horizon, Z up
  this->dataPtr->orthoCam->pitch(Ogre::Degree(90));

  // Don't yaw along variable axis, causes leaning
  this->dataPtr->orthoCam->setFixedYawAxis(true, Ogre::Vector3::UNIT_Z);

  this->dataPtr->orthoCam->setDirection(1, 0, 0);

  this->dataPtr->pitchNodeOrtho->attachObject(this->dataPtr->orthoCam);
  this->dataPtr->orthoCam->setAutoAspectRatio(true);

  if (this->dataPtr->orthoCam)
  {
    this->dataPtr->orthoCam->setNearClipDistance(0.01);
    this->dataPtr->orthoCam->setFarClipDistance(0.02);
    this->dataPtr->orthoCam->setRenderingDistance(0.02);

    Ogre::Matrix4 p = this->BuildScaledOrthoMatrix(
        0, static_cast<float>(this->dataPtr->w2nd / 10.0),
        0, static_cast<float>(this->dataPtr->h2nd / 10.0),
        0.01, 0.02);

    this->dataPtr->orthoCam->setCustomProjectionMatrix(true, p);
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
void OgreGpuRays::SetRangeCount(const unsigned int _w, const unsigned int _h)
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

  this->dataPtr->visual = this->scene->CreateVisual();

  // NodePtr node(this->Node());
  // this->Parent()->RemoveChild(node);
  // this->dataPtr->pitchNodeOrtho->addChild(this->Node());

  // Convert mesh from common::Mesh to rendering::mesh and add it to
  // the canvas visual
  MeshPtr renderingMesh = this->scene->CreateMesh(
      this->dataPtr->undistMesh);
  this->dataPtr->visual->AddGeometry(renderingMesh);

  this->dataPtr->visual->SetLocalPosition(0.01, 0, 0);
  this->dataPtr->visual->SetLocalRotation(0, 0, 0);

  MaterialPtr canvasMaterial = this->scene->CreateMaterial(this->Name() + "_Green");
  canvasMaterial->SetAmbient(ignition::math::Color(0, 1, 0, 1));
  this->dataPtr->visual->SetMaterial(canvasMaterial);

  this->dataPtr->visual->SetVisible(true);
  VisualPtr root = this->scene->RootVisual();
  root->AddChild(this->dataPtr->visual);
}

/////////////////////////////////////////////////
void OgreGpuRays::notifyRenderSingleObject(Ogre::Renderable *_rend,
      const Ogre::Pass* /*pass*/, const Ogre::AutoParamDataSource* /*source*/,
      const Ogre::LightList* /*lights*/, bool /*supp*/)
{
  Ogre::Vector4 retro = Ogre::Vector4(0, 0, 0, 0);
  try
  {
    retro = _rend->getCustomParameter(1);
  }
  catch(Ogre::ItemIdentityException& e)
  {
    _rend->setCustomParameter(1, Ogre::Vector4(0, 0, 0, 0));
  }

  OgreMaterialPtr ogreMaterial = std::dynamic_pointer_cast<OgreMaterial>(
      this->dataPtr->currentMat);

  Ogre::Pass *pass = ogreMaterial->Material()->getBestTechnique()->getPass(0);
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
  return this->dataPtr->secondPassTexture;
}


