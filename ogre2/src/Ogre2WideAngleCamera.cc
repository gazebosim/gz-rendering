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

#include "gz/rendering/ogre2/Ogre2WideAngleCamera.hh"

#include "gz/rendering/CameraLens.hh"
#include "gz/rendering/ogre2/Ogre2Conversions.hh"
#include "gz/rendering/ogre2/Ogre2RenderEngine.hh"
#include "gz/rendering/ogre2/Ogre2Scene.hh"

#include "gz/common/Util.hh"

#ifdef _MSC_VER
#  pragma warning(push, 0)
#endif
#include <Compositor/OgreCompositorManager2.h>
#include <Compositor/OgreCompositorWorkspace.h>
#include <Compositor/OgreCompositorWorkspaceListener.h>
#include <Compositor/Pass/PassQuad/OgreCompositorPassQuad.h>
#include <Compositor/Pass/PassScene/OgreCompositorPassSceneDef.h>
#include <OgreDepthBuffer.h>
#include <OgreImage2.h>
#include <OgrePass.h>
#include <OgreRoot.h>
#include <OgreSceneManager.h>
#include <OgreTechnique.h>
#include <OgreTextureBox.h>
#include <OgreTextureGpuManager.h>
#ifdef _MSC_VER
#  pragma warning(pop)
#endif

// clang-format off
namespace gz
{
namespace rendering
{
inline namespace GZ_RENDERING_VERSION_NAMESPACE {
/// \brief Helper class for setting up Camera and Materials when rendering
/// via Ogre2WideAngleCamera.
class GZ_RENDERING_OGRE2_HIDDEN Ogre2WideAngleCameraWorkspaceListenerPrivate :
    public Ogre::CompositorWorkspaceListener
{
  public: gz::rendering::Ogre2WideAngleCamera &owner;

  /// \brief constructor
  /// \param[in] _scene the scene manager responsible for rendering
  public: explicit Ogre2WideAngleCameraWorkspaceListenerPrivate(
        gz::rendering::Ogre2WideAngleCamera &_owner) :
    owner(_owner)
  {
  }

  /// \brief Called when each pass is about to be executed.
  /// \param[in] _pass Ogre pass which is about to execute
  public: virtual void passPreExecute(Ogre::CompositorPass *_pass) override;
};
}
}
}
// clang-format on

static const uint32_t kWideAngleNumCubemapFaces = 6u;

/// \brief Private data for the WideAngleCamera class
class gz::rendering::Ogre2WideAngleCamera::Implementation
{
  // clang-format off
  /// \brief Environment texture size
  public: uint32_t envTextureSize = 512u;

  /// \brief A single cube map texture
  public: Ogre::TextureGpu *envCubeMapTexture = nullptr;

  /// \brief Output texture
  public: Ogre::TextureGpu *ogreRenderTexture = nullptr;

  /// \brief Compositor workspace. Does all the work
  public: Ogre::CompositorWorkspace *ogreCompositorWorkspace = nullptr;

  /// \brief Main pass definition (used for visibility mask manipuluation).
  public: Ogre::CompositorPassSceneDef
  *cubePassSceneDef[kWideAngleNumCubemapFaces]{};

  /// \brief Pointer to material, used for second rendering pass
  public: Ogre::MaterialPtr compMat;

  /// \brief Camera lens description
  public: CameraLens lens;

  /// \brief Pointer to the ogre camera
  /// OgreNext rotates to face each of the 6 sides
  /// We use listeners to set FOV to the proper setting depending on the pass
  public: Ogre::Camera *ogreCamera = nullptr;

  /// \brief Dummy texture
  public: Ogre2RenderTexturePtr wideAngleTexture;

  /// \brief Event used to signal camera data
  public: gz::common::EventT<void(const unsigned char *,
              unsigned int, unsigned int, unsigned int,
              const std::string &)> newImageFrame;

  /// \brief See Ogre2WideAngleCameraWorkspaceListenerPrivate
  public: Ogre2WideAngleCameraWorkspaceListenerPrivate workspaceListener;

  explicit Implementation(gz::rendering::Ogre2WideAngleCamera &_owner) :
    workspaceListener(_owner)
  {
  }
  // clang-format on
};

using namespace gz;
using namespace rendering;

// Arbitrary values
static const uint32_t kWideAngleCameraCubemapPassId = 1276660u;
static const uint32_t kWideAngleCameraQuadPassId = 1276661u;

//////////////////////////////////////////////////
Ogre2WideAngleCamera::Ogre2WideAngleCamera() :
  dataPtr(utils::MakeUniqueImpl<Implementation>(*this))
{
}

//////////////////////////////////////////////////
Ogre2WideAngleCamera::~Ogre2WideAngleCamera()
{
  this->Destroy();
}

//////////////////////////////////////////////////
void Ogre2WideAngleCamera::Init()
{
  BaseWideAngleCamera::Init();
  this->CreateCamera();
  this->CreateRenderTexture();
  this->Reset();
}

/////////////////////////////////////////////////
void Ogre2WideAngleCamera::CreateRenderTexture()
{
  RenderTexturePtr base = this->scene->CreateRenderTexture();
  this->dataPtr->wideAngleTexture =
    std::dynamic_pointer_cast<Ogre2RenderTexture>(base);
  this->dataPtr->wideAngleTexture->SetWidth(1);
  this->dataPtr->wideAngleTexture->SetHeight(1);
}

//////////////////////////////////////////////////
void Ogre2WideAngleCamera::PreRender()
{
  BaseCamera::PreRender();
  if (!this->dataPtr->ogreRenderTexture)
    this->CreateWideAngleTexture();
}

//////////////////////////////////////////////////
void Ogre2WideAngleCamera::Destroy()
{
  auto engine = Ogre2RenderEngine::Instance();
  auto ogreRoot = engine->OgreRoot();
  Ogre::TextureGpuManager *textureMgr =
    ogreRoot->getRenderSystem()->getTextureGpuManager();

  Ogre::CompositorManager2 *ogreCompMgr = ogreRoot->getCompositorManager2();

  const Ogre::CompositorChannelVec channels = {
    this->dataPtr->ogreRenderTexture, this->dataPtr->envCubeMapTexture
  };

  if (this->dataPtr->ogreCompositorWorkspace)
  {
    ogreCompMgr->removeWorkspace(this->dataPtr->ogreCompositorWorkspace);
    this->dataPtr->ogreCompositorWorkspace = nullptr;
  }

  if (this->dataPtr->envCubeMapTexture)
  {
    textureMgr->destroyTexture(this->dataPtr->envCubeMapTexture);
    this->dataPtr->envCubeMapTexture = nullptr;
  }

  if (this->dataPtr->ogreRenderTexture)
  {
    textureMgr->destroyTexture(this->dataPtr->ogreRenderTexture);
    this->dataPtr->ogreRenderTexture = nullptr;
  }
}

//////////////////////////////////////////////////
uint32_t Ogre2WideAngleCamera::EnvTextureSize() const
{
  return this->dataPtr->envTextureSize;
}

//////////////////////////////////////////////////
void Ogre2WideAngleCamera::SetEnvTextureSize(uint32_t _size)
{
  this->dataPtr->envTextureSize = _size;
}

/////////////////////////////////////////////////
void Ogre2WideAngleCamera::CreateCamera()
{
  // Create dummy ogre camera object
  Ogre::SceneManager *ogreSceneManager = this->scene->OgreSceneManager();
  if (ogreSceneManager == nullptr)
  {
    gzerr << "Scene manager cannot be obtained" << std::endl;
    return;
  }

  this->dataPtr->ogreCamera =
    ogreSceneManager->createCamera(this->Name() + "_Camera", true, true);
  if (this->dataPtr->ogreCamera == nullptr)
  {
    gzerr << "Ogre camera cannot be created" << std::endl;
    return;
  }

  // OgreNext by default attaches the camera to the Root SceneNode
  // Detach from it and use our own node.
  this->dataPtr->ogreCamera->detachFromParent();
  this->ogreNode->attachObject(this->dataPtr->ogreCamera);

  this->dataPtr->ogreCamera->setFixedYawAxis(false);
  this->dataPtr->ogreCamera->yaw(Ogre::Degree(-90));
  this->dataPtr->ogreCamera->roll(Ogre::Degree(-90));
  this->dataPtr->ogreCamera->setAutoAspectRatio(true);

  const Ogre::Real nearPlane = static_cast<Ogre::Real>(this->NearClipPlane());
  const Ogre::Real farPlane = static_cast<Ogre::Real>(this->FarClipPlane());
  this->dataPtr->ogreCamera->setNearClipDistance(nearPlane);
  this->dataPtr->ogreCamera->setFarClipDistance(farPlane);
}

//////////////////////////////////////////////////
void Ogre2WideAngleCamera::SetupMSAA(Ogre::CompositorManager2 *_ogreCompMgr,
                                     uint8_t _msaa)
{
  GZ_ASSERT(_msaa > 1u, "Wrong API usage. Don't call this function");

  Ogre::CompositorNodeDef *nodeDef =
    _ogreCompMgr->getNodeDefinitionNonConst("WideAngleCameraCubemapPassMsaa");
  auto &textureDefs = nodeDef->getLocalTextureDefinitionsNonConst();

  GZ_ASSERT(textureDefs.size() == 1u,
            "WideAngleCamera.compositor out of sync?");
  GZ_ASSERT(textureDefs[0].getName() == "tmpMsaa",
            "WideAngleCamera.compositor out of sync?");

  textureDefs[0].fsaa = std::to_string(_msaa);
}

//////////////////////////////////////////////////
void Ogre2WideAngleCamera::RetrieveCubePassSceneDefs(
  Ogre::CompositorManager2 *_ogreCompMgr, bool _withMsaa)
{
  Ogre::CompositorNodeDef *nodeDef = _ogreCompMgr->getNodeDefinitionNonConst(
    _withMsaa ? "WideAngleCameraCubemapPassMsaa"
              : "WideAngleCameraCubemapPass");
  for (uint32_t i = 0u; i < kWideAngleNumCubemapFaces; ++i)
  {
    Ogre::CompositorTargetDef *target0 = nodeDef->getTargetPass(i);
    Ogre::CompositorPassDefVec &passes = target0->getCompositorPassesNonConst();
    GZ_ASSERT(passes.size() >= 1u,
              "WideAngleCamera.compositor is out of sync?");
    GZ_ASSERT(passes[0]->getType() == Ogre::PASS_SCENE,
              "WideAngleCamera.compositor is out of sync?");
    GZ_ASSERT(dynamic_cast<Ogre::CompositorPassSceneDef *>(passes[0]),
              "Memory corruption?");

    this->dataPtr->cubePassSceneDef[i] =
      static_cast<Ogre::CompositorPassSceneDef *>(passes[0]);
  }
}

//////////////////////////////////////////////////
void Ogre2WideAngleCamera::CreateWideAngleTexture()
{
  if (this->dataPtr->ogreCamera == nullptr)
  {
    gzerr << "Ogre camera cannot be created" << std::endl;
    return;
  }

  auto engine = Ogre2RenderEngine::Instance();
  auto ogreRoot = engine->OgreRoot();
  Ogre::TextureGpuManager *textureMgr =
    ogreRoot->getRenderSystem()->getTextureGpuManager();

  if (!this->dataPtr->ogreRenderTexture)
  {
    this->dataPtr->ogreRenderTexture =
      textureMgr->createTexture(this->Name() + "_wideAngleCamera",    //
                                Ogre::GpuPageOutStrategy::Discard,    //
                                Ogre::TextureFlags::RenderToTexture,  //
                                Ogre::TextureTypes::Type2D,           //
                                Ogre::BLANKSTRING,                    //
                                0u);

    this->dataPtr->ogreRenderTexture->setResolution(this->ImageWidth(),
                                                    this->ImageHeight());
    this->dataPtr->ogreRenderTexture->setPixelFormat(
      Ogre::PFG_RGBA8_UNORM_SRGB);
    this->dataPtr->ogreRenderTexture->_setDepthBufferDefaults(
      Ogre::DepthBuffer::POOL_NO_DEPTH, false, Ogre::PFG_UNKNOWN);
    this->dataPtr->ogreRenderTexture->scheduleTransitionTo(
      Ogre::GpuResidency::Resident);
  }

  this->dataPtr->envCubeMapTexture =
    textureMgr->createTexture(this->Name() + "_cube_wideAngleCamera",  //
                              Ogre::GpuPageOutStrategy::Discard,       //
                              Ogre::TextureFlags::RenderToTexture,     //
                              Ogre::TextureTypes::TypeCube,            //
                              Ogre::BLANKSTRING,                       //
                              0u);

  this->dataPtr->envCubeMapTexture->setResolution(
    this->dataPtr->envTextureSize, this->dataPtr->envTextureSize);
  this->dataPtr->envCubeMapTexture->setPixelFormat(Ogre::PFG_RGBA8_UNORM_SRGB);

  this->dataPtr->envCubeMapTexture->scheduleTransitionTo(
    Ogre::GpuResidency::Resident);

  // Create compositor workspace
  Ogre::CompositorManager2 *ogreCompMgr = ogreRoot->getCompositorManager2();
  Ogre::SceneManager *ogreSceneManager = this->scene->OgreSceneManager();

  const uint8_t msaa =
    Ogre2RenderTarget::TargetFSAA(static_cast<uint8_t>(this->antiAliasing));

  if (msaa > 1u)
  {
    SetupMSAA(ogreCompMgr, msaa);
  }

  this->RetrieveCubePassSceneDefs(ogreCompMgr, msaa > 1u);

  const Ogre::CompositorChannelVec channels = {
    this->dataPtr->envCubeMapTexture, this->dataPtr->ogreRenderTexture
  };

  this->dataPtr->ogreCompositorWorkspace = ogreCompMgr->addWorkspace(
    ogreSceneManager, channels, this->dataPtr->ogreCamera,
    "WideAngleCameraWorkspace", false);
  this->dataPtr->ogreCompositorWorkspace->addListener(
    &this->dataPtr->workspaceListener);
}

//////////////////////////////////////////////////
void Ogre2WideAngleCamera::Render()
{
  {
    auto engine = Ogre2RenderEngine::Instance();
    auto ogreRoot = engine->OgreRoot();
    ogreRoot->getRenderSystem()->startGpuDebuggerFrameCapture(nullptr);
  }
  const uint32_t currVisibilityMask = this->VisibilityMask();
  for (uint32_t i = 0u; i < kWideAngleNumCubemapFaces; ++i)
  {
    this->dataPtr->cubePassSceneDef[i]->mVisibilityMask = currVisibilityMask;
  }

  this->scene->StartRendering(this->dataPtr->ogreCamera);

  Ogre::vector<Ogre::TextureGpu *>::type swappedTargets;

  this->dataPtr->ogreCompositorWorkspace->setEnabled(true);

  this->dataPtr->ogreCompositorWorkspace->_validateFinalTarget();
  this->dataPtr->ogreCompositorWorkspace->_beginUpdate(false);
  this->dataPtr->ogreCompositorWorkspace->_update();
  this->dataPtr->ogreCompositorWorkspace->_endUpdate(false);

  swappedTargets.clear();
  this->dataPtr->ogreCompositorWorkspace->_swapFinalTarget(swappedTargets);

  this->dataPtr->ogreCompositorWorkspace->setEnabled(false);

  this->scene->FlushGpuCommandsAndStartNewFrame(6u, false);
  {
    auto engine = Ogre2RenderEngine::Instance();
    auto ogreRoot = engine->OgreRoot();
    ogreRoot->getRenderSystem()->endGpuDebuggerFrameCapture(nullptr);
  }
}

//////////////////////////////////////////////////
math::Vector3d Ogre2WideAngleCamera::Project3d(const math::Vector3d &_pt) const
{
  using namespace Ogre;

  // Matches CompositorPass::CubemapRotations from OgreCompositorPass.cpp
  const Quaternion kCubemapRotations[6] = {
    Quaternion(Radian(-1.570796f), Vector3(0, 1, 0)),  // +X
    Quaternion(Radian(1.570796f), Vector3(0, 1, 0)),   // -X
    Quaternion(Radian(1.570796f), Vector3(1, 0, 0)),   // +Y
    Quaternion(Radian(-1.570796f), Vector3(1, 0, 0)),  // -Y
    Quaternion(1, 0, 0, 0),                            // +Z
    Quaternion(Radian(3.1415927f), Vector3(0, 1, 0))   // -Z
  };

  this->dataPtr->ogreCamera->setAspectRatio(1.0f);
  this->dataPtr->ogreCamera->setFOVy(Ogre::Degree(90));
  const Quaternion oldCameraOrientation(
    this->dataPtr->ogreCamera->getOrientation());

  const bool isReverseDepth =
    Root::getSingleton().getRenderSystem()->isReverseDepth();

  // project world point to camera clip space.
  const Matrix4 projMatrix = this->dataPtr->ogreCamera->getProjectionMatrix();
  // const Vector4 basePos = viewProj * Vector4(Ogre2Conversions::Convert(_pt));
  const Vector3 basePos = Ogre2Conversions::Convert(_pt);

  // project onto cubemap face then onto
  gz::math::Vector3d screenPos;
  // loop through all env cameras can find the one that sees the 3d world point
  for (unsigned int i = 0u; i < kWideAngleNumCubemapFaces; ++i)
  {
    // project world point to camera clip space for each face.
    this->dataPtr->ogreCamera->setOrientation(oldCameraOrientation *
                                              kCubemapRotations[i]);
    const Matrix4 viewProj =
      projMatrix * this->dataPtr->ogreCamera->getViewMatrix();
    Vector4 pos = viewProj * Vector4(basePos);
    pos.x /= pos.w;
    pos.y /= pos.w;
    // check if point is visible
    if (std::fabs(pos.x) <= 1 && std::fabs(pos.y) <= 1 &&
        ((!isReverseDepth && pos.z > 0) || (isReverseDepth && pos.z < 1)))
    {
      // determine dir vector to projected point from env camera
      // work in y up, z forward, x right clip space
      gz::math::Vector3d dir(pos.x, pos.y, 1);
      gz::math::Quaterniond rot = gz::math::Quaterniond::Identity;

      // rotate dir vector into wide angle camera frame based on the
      // face of the cube. Note: operate in clip space so
      // left handed coordinate system rotation
      if (i == 0)
        rot = gz::math::Quaterniond(0.0, GZ_PI * 0.5, 0.0);
      else if (i == 1)
        rot = gz::math::Quaterniond(0.0, -GZ_PI * 0.5, 0.0);
      else if (i == 2)
        rot = gz::math::Quaterniond(-GZ_PI * 0.5, 0.0, 0.0);
      else if (i == 3)
        rot = gz::math::Quaterniond(GZ_PI * 0.5, 0.0, 0.0);
      else if (i == 5)
        rot = gz::math::Quaterniond(0.0, GZ_PI, 0.0);
      dir = rot * dir;
      dir.Normalize();

      // compute theta and phi from the dir vector
      // theta is angle to dir vector from z (forward)
      // phi is angle from x in x-y plane
      // direction vector (x, y, z)
      // x = sin(theta)cos(phi)
      // y = sin(theta)sin(phi)
      // z = cos(theta)
      double theta =
        std::atan2(std::sqrt(dir.X() * dir.X() + dir.Y() * dir.Y()), dir.Z());
      double phi = std::atan2(dir.Y(), dir.X());
      // this also works:
      // double theta = std::acos(dir.Z());
      // double phi = std::asin(dir.Y() / std::sin(theta));

      double f = this->Lens().F();
      double fov = this->HFOV().Radian();
      // recompute f if scale to HFOV is true
      if (this->Lens().ScaleToHFOV())
      {
        double param = (fov / 2.0) / this->Lens().C2() + this->Lens().C3();
        double funRes =
          this->Lens().ApplyMappingFunction(static_cast<float>(param));
        f = 1.0 / (this->Lens().C1() * funRes);
      }

      // Apply fisheye lens mapping function
      // r is distance of point from image center
      double r = this->Lens().C1() * f *
                 this->Lens().ApplyMappingFunction(theta / this->Lens().C2() +
                                                   this->Lens().C3());

      // compute projected x and y in clip space
      double x = cos(phi) * r;
      double y = sin(phi) * r;

      const uint32_t vpWidth = this->dataPtr->ogreRenderTexture->getWidth();
      const uint32_t vpHeight = this->dataPtr->ogreRenderTexture->getHeight();
      // env cam cube map texture is square and likely to be different size from
      // viewport. We need to adjust projected pos based on aspect ratio
      double asp = static_cast<double>(vpWidth) / static_cast<double>(vpHeight);
      y *= asp;

      // convert to screen space
      screenPos.X() = ((x / 2.0) + 0.5) * vpWidth;
      screenPos.Y() = (1 - ((y / 2.0) + 0.5)) * vpHeight;

      // r will be > 1.0 if point is not visible (outside of image)
      screenPos.Z() = r;

      this->dataPtr->ogreCamera->setOrientation(oldCameraOrientation);

      return screenPos;
    }
  }

  this->dataPtr->ogreCamera->setOrientation(oldCameraOrientation);

  return screenPos;
}

//////////////////////////////////////////////////
void Ogre2WideAngleCamera::PostRender()
{
  if (this->dataPtr->newImageFrame.ConnectionCount() <= 0u)
    return;

  const unsigned int width = this->ImageWidth();
  const unsigned int height = this->ImageHeight();

  // blit data from gpu to cpu
  Ogre::Image2 image;
  image.convertFromTexture(this->dataPtr->ogreRenderTexture, 0u, 0u);
  Ogre::TextureBox box = image.getData(0u);

  // Convert in-place from RGBA32 to RGB24 reusing the same memory region.
  // The data contained will no longer be meaningful to Image2, but that
  // class will no longer manipulate that data. We also store it contiguously
  // (which is what gazebo expects), instead of aligning rows to 4 bytes like
  // Ogre does. This saves RAM and lots of bandwidth.
  uint8_t *RESTRICT_ALIAS rgb24 =
    reinterpret_cast<uint8_t * RESTRICT_ALIAS>(box.data);
  for (size_t y = 0; y < box.height; ++y)
  {
    uint8_t *RESTRICT_ALIAS rgba32 =
      reinterpret_cast<uint8_t * RESTRICT_ALIAS>(box.at(0u, y, 0u));
    for (size_t x = 0; x < box.width; ++x)
    {
      *rgb24++ = *rgba32++;
      *rgb24++ = *rgba32++;
      *rgb24++ = *rgba32++;
      ++rgba32;
    }
  }

  this->dataPtr->newImageFrame(reinterpret_cast<uint8_t *>(box.data), width,
                               height, 3u, "PF_R8G8B");

  // Uncomment to debug wide angle cameraoutput
  // gzdbg << "wxh: " << width << " x " << height << std::endl;
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
common::ConnectionPtr Ogre2WideAngleCamera::ConnectNewWideAngleFrame(
  std::function<void(const unsigned char *, unsigned int, unsigned int,
                     unsigned int, const std::string &)>
    _subscriber)
{
  return this->dataPtr->newImageFrame.Connect(_subscriber);
}

//////////////////////////////////////////////////
RenderTargetPtr Ogre2WideAngleCamera::RenderTarget() const
{
  return this->dataPtr->wideAngleTexture;
}

//////////////////////////////////////////////////
void Ogre2WideAngleCamera::PrepareForCubemapFacePass(
  Ogre::CompositorPass *_pass)
{
  this->dataPtr->ogreCamera->setFOVy(Ogre::Degree(90));

  auto const &bgColor = this->scene->BackgroundColor();
  _pass->getRenderPassDesc()->setClearColour(
    Ogre2Conversions::Convert(bgColor));
}

//////////////////////////////////////////////////
void Ogre2WideAngleCamera::PrepareForFinalPass(Ogre::Pass *_pass)
{
  const double ratio = static_cast<double>(this->ImageWidth()) /
                       static_cast<double>(this->ImageHeight());
  const double vfov = 2.0 * atan(tan(this->HFOV().Radian() / 2.0) / ratio);
  this->dataPtr->ogreCamera->setFOVy(Ogre::Radian(Ogre::Real(vfov)));

  const float localHfov = static_cast<float>(this->HFOV().Radian());

  Ogre::GpuProgramParametersSharedPtr psParams =
    _pass->getFragmentProgramParameters();

  psParams->setNamedConstant("c1", static_cast<Ogre::Real>(this->Lens().C1()));
  psParams->setNamedConstant("c2", static_cast<Ogre::Real>(this->Lens().C2()));
  psParams->setNamedConstant("c3", static_cast<Ogre::Real>(this->Lens().C3()));

  if (this->Lens().ScaleToHFOV())
  {
    float param = (localHfov / 2) / this->Lens().C2() + this->Lens().C3();
    float funRes = this->Lens().ApplyMappingFunction(static_cast<float>(param));

    float newF = 1.0f / (this->Lens().C1() * funRes);

    psParams->setNamedConstant("f", static_cast<Ogre::Real>(newF));
  }
  else
  {
    psParams->setNamedConstant("f", static_cast<Ogre::Real>(this->Lens().F()));
  }

  auto vecFun = this->Lens().MappingFunctionAsVector3d();

  psParams->setNamedConstant("fun",
                             Ogre::Vector3(vecFun.X(), vecFun.Y(), vecFun.Z()));

  psParams->setNamedConstant(
    "cutOffAngle", static_cast<Ogre::Real>(this->Lens().CutOffAngle()));

  Ogre::GpuProgramParametersSharedPtr vsParams =
    _pass->getVertexProgramParameters();
  vsParams->setNamedConstant("ratio", static_cast<Ogre::Real>(ratio));
}

//////////////////////////////////////////////////
void Ogre2WideAngleCameraWorkspaceListenerPrivate::passPreExecute(
  Ogre::CompositorPass *_pass)
{
  const uint32_t identifier = _pass->getDefinition()->mIdentifier;
  if (identifier == kWideAngleCameraCubemapPassId)
  {
    this->owner.PrepareForCubemapFacePass(_pass);
  }
  else if (identifier == kWideAngleCameraQuadPassId)
  {
    GZ_ASSERT(
      dynamic_cast<Ogre::CompositorPassQuad *>(_pass),
      "Impossible! Corrupted memory? WideAngleCamera.compositor out of sync?");
    Ogre::CompositorPassQuad *passQuad =
      static_cast<Ogre::CompositorPassQuad *>(_pass);

    Ogre::Pass *pass = passQuad->getPass();

    this->owner.PrepareForFinalPass(pass);
  }
}
