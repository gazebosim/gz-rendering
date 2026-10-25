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
  #ifndef NOMINMAX
    #define NOMINMAX
  #endif
  #include <windows.h>
#endif

#include <algorithm>
#include <cstdint>
#include <math.h>
#include <gz/math/Helpers.hh>
#include <gz/math/Matrix4.hh>

#include <gz/common/Profiler.hh>

#include "gz/rendering/RenderTypes.hh"
#include "gz/rendering/ogre2/Ogre2Conversions.hh"
#include "gz/rendering/ogre2/Ogre2DepthCamera.hh"
#include "gz/rendering/ogre2/Ogre2GaussianNoisePass.hh"
#include "gz/rendering/ogre2/Ogre2ParticleEmitter.hh"
#include "gz/rendering/ogre2/Ogre2RenderEngine.hh"
#include "gz/rendering/ogre2/Ogre2RenderTarget.hh"
#include "gz/rendering/ogre2/Ogre2RenderTypes.hh"
#include "gz/rendering/ogre2/Ogre2Scene.hh"
#include "gz/rendering/ogre2/Ogre2Sensor.hh"

#include "Ogre2GpuReadbackTicket.hh"
#include "Ogre2ParticleNoiseListener.hh"

#ifdef _MSC_VER
  #pragma warning(push, 0)
#endif
#include <Compositor/OgreCompositorManager2.h>
#include <Compositor/OgreCompositorNode.h>
#include <Compositor/OgreCompositorWorkspace.h>
#include <Compositor/Pass/PassClear/OgreCompositorPassClearDef.h>
#include <Compositor/Pass/PassQuad/OgreCompositorPassQuadDef.h>
#include <Compositor/Pass/PassScene/OgreCompositorPassSceneDef.h>
#include <OgreRoot.h>
#include <OgreSceneManager.h>
#include <OgreTechnique.h>
#ifdef _MSC_VER
  #pragma warning(pop)
#endif

namespace gz
{
namespace rendering
{
inline namespace GZ_RENDERING_VERSION_NAMESPACE {
//
/// \brief Gaussian noise render pass for depth cameras
/// The class implementation is very similar to Ogre2GaussianNoisePass but
/// uses a different shader material for apply noise to depth cameras
/// This class is added here since we can not modify Ogre2GaussianNoisePass
/// as it would break ABI
class Ogre2DepthGaussianNoisePass : public Ogre2GaussianNoisePass
{
  /// \brief Constructor
  public: Ogre2DepthGaussianNoisePass() {}

  /// \brief Destructor
  public: virtual ~Ogre2DepthGaussianNoisePass() {}

  // Documentation inherited.
  public: virtual void PreRender(const CameraPtr &_camera) override;

  // Documentation inherited.
  public: void CreateRenderPass() override;

  /// brief Pointer to the Gaussian noise ogre material
  private: Ogre::Material *gaussianNoiseMat = nullptr;
};
}
}
}

/// \internal
/// \brief Private data for the Ogre2DepthCamera class
class gz::rendering::Ogre2DepthCameraPrivate
{
  /// \brief The depth buffer - also the outgoing point cloud data used
  /// by newRgbPointCloud event
  public: float *depthBuffer = nullptr;

  /// \brief Outgoing depth data, used by newDepthFrame event.
  public: float *depthImage = nullptr;

  /// \brief Persistent GPU->CPU readback ticket used by the non-legacy
  /// PostRender path.
  public: Ogre2GpuReadbackTicket depthReadback;

  /// \brief Persistent readback of the point cloud of point cloud frames,
  /// whose depth frame depthReadback reads back from its own target.
  public: Ogre2GpuReadbackTicket pointReadback;

  /// \brief True if IssueReadback() issued the readbacks of the last
  /// Render(), which PostRender() then maps.
  public: bool readbackIssued = false;

  /// \brief Final quad pass writing the xyz + colour output.
  public: Ogre::CompositorPassDef *finalPassDef = nullptr;

  /// \brief Final quad pass writing the depth-only output (x of the xyz
  /// output) into an R32 target, used without point cloud subscribers.
  public: Ogre::CompositorPassDef *depthOnlyPassDef = nullptr;

  /// \brief Quad pass of the base node writing the xyz + colour output.
  public: Ogre::CompositorPassDef *xyzPassDef = nullptr;

  /// \brief Quad pass of the base node writing the depth-only output
  /// directly, the xyz and the final depth-only passes in one. Used
  /// without point cloud subscribers when no render pass (noise) sits
  /// between the base and the final node.
  public: Ogre::CompositorPassDef *mergedPassDef = nullptr;

  /// \brief Final node pass packing the xyz output into rt_points.
  public: Ogre::CompositorPassDef *packPassDef = nullptr;

  /// \brief Material of the pass packing the xyz output.
  public: Ogre::MaterialPtr packMaterial;

  /// \brief Point cloud format requested by SetPointCloudFormat().
  public: PixelFormat pointFormat = PF_FLOAT32_RGBA;

  /// \brief True if point cloud frames pack the xyz output into rt_points
  /// (PF_FLOAT32_RGB point clouds); set by CreateDepthTexture.
  public: bool packedPoints = false;

  /// \brief True if the point cloud event had connections at the last
  /// PreRender, which chose the colour passes.
  public: bool pointsAtPreRender = false;

  /// \brief True if the last Render set up a depth-only frame.
  public: bool renderDepthOnly = false;

  /// \brief True if the last Render set up the merged depth-only pass.
  public: bool renderMerged = false;

  /// \brief Issue the persistent-ticket downloads of the last Render: the
  /// depth-only output, and for a point cloud frame the point cloud.
  /// \return False if none was issued.
  public: bool Download()
  {
    auto defined = [this](const std::string &_node, const char *_name)
    {
      Ogre::CompositorNode *node =
          this->ogreCompositorWorkspace->findNodeNoThrow(_node);
      return node ? node->getDefinedTexture(_name) : nullptr;
    };
    Ogre::TextureGpu *depth = this->renderMerged ?
        defined(this->ogreCompositorBaseNodeDef, "depthOnlyTexture") :
        defined(this->ogreCompositorFinalNodeDef, "rt_depth");
    Ogre::TextureGpu *points = this->renderDepthOnly ? nullptr :
        (this->packedPoints ?
        defined(this->ogreCompositorFinalNodeDef, "rt_points") :
        this->ogreDepthTexture[1]);
    this->depthDownloaded = depth && this->depthReadback.Download(depth);
    this->pointsDownloaded = points && this->pointReadback.Download(points);
    return this->depthDownloaded || this->pointsDownloaded;
  }

  /// \brief True if the last Download() issued the depth download.
  public: bool depthDownloaded = false;

  /// \brief True if the last Download() issued the point cloud download.
  public: bool pointsDownloaded = false;

  /// \brief maximum value used for data outside sensor range
  public: float dataMaxVal = gz::math::INF_D;

  /// \brief minimum value used for data outside sensor range
  public: float dataMinVal = -gz::math::INF_D;

  /// \brief 1st pass compositor workspace definition
  public: std::string ogreCompositorWorkspaceDef;

  /// \brief 1st pass compositor node definition
  public: std::string ogreCompositorBaseNodeDef;

  /// \brief Final pass compositor node definition
  public: std::string ogreCompositorFinalNodeDef;

  /// \brief Compositor workspace.
  public: Ogre::CompositorWorkspace *ogreCompositorWorkspace = nullptr;

  /// \brief Output texture with depth and color data
  public: Ogre::TextureGpu *ogreDepthTexture[2];

  /// \brief Dummy render texture for the depth data
  public: RenderTexturePtr depthTexture;

  /// \brief The depth material
  public: Ogre::MaterialPtr depthMaterial;

  /// \brief The depth material in final pass
  public: Ogre::MaterialPtr depthFinalMaterial;

  /// \brief A chain of render passes applied to the render target
  public: std::vector<RenderPassPtr> renderPasses;

  /// \brief Flag to indicate if render pass need to be rebuilt
  public: bool renderPassDirty = false;

  /// \brief Event used to signal rgb point cloud data
  public: gz::common::EventT<void(const float *,
              unsigned int, unsigned int, unsigned int,
              const std::string &)> newRgbPointCloud;

  /// \brief Event used to signal depth data
  public: gz::common::EventT<void(const float *,
              unsigned int, unsigned int, unsigned int,
              const std::string &)> newDepthFrame;

  /// \brief standard deviation of particle noise
  public: double particleStddev = 0.01;

  /// \brief Listener for setting particle noise value based on particle
  /// emitter region
  public: std::unique_ptr<Ogre2ParticleNoiseListener> particleNoiseListener;

  /// \brief Particle scatter ratio. This is used to determine the ratio of
  /// particles that will detected by the depth camera
  public: double particleScatterRatio = 0.1;

  /// \brief Name of sky box material
  public: const std::string kSkyboxMaterialName = "SkyBox";

  /// \brief Name of shadow compositor node
  public: const std::string kShadowNodeName = "PbsMaterialsShadowNode";

  /// \brief Execution mask for this workspace
  /// If RGB point color data are requested, the execution mask of the color
  /// target will be updated to match the workspace's execution mask so that
  /// these passes are executed, otherwise they will be skipped for performance
  /// improvement.
  public: const uint8_t kDepthExecutionMask = 0xEF;

  /// \brief Pointer to the color target definition in the workspace
  public: Ogre::CompositorTargetDef *colorTargetDef{nullptr};

  /// \brief Pointer to the particle target definition in the workspace
  public: Ogre::CompositorTargetDef *particleTargetDef{nullptr};
};

using namespace gz;
using namespace rendering;

//////////////////////////////////////////////////
void Ogre2DepthGaussianNoisePass::PreRender(const CameraPtr &/*_camera*/)
{
  GZ_PROFILE("Ogre2DepthGaussianNoisePass::PreRender");
  // This function is similar to Ogre2GaussianNoisePass but duplicated here
  // for Ogre2DepthCamera

  if (!this->gaussianNoiseMat)
    return;

  if (!this->enabled)
    return;

  Ogre::Vector3 offsets(math::Rand::DblUniform(0.0, 1.0),
                        math::Rand::DblUniform(0.0, 1.0),
                        math::Rand::DblUniform(0.0, 1.0));

  Ogre::Pass *pass = this->gaussianNoiseMat->getTechnique(0)->getPass(0);
  Ogre::GpuProgramParametersSharedPtr psParams =
      pass->getFragmentProgramParameters();
  psParams->setNamedConstant("offsets", offsets);
  psParams->setNamedConstant("mean", static_cast<Ogre::Real>(this->mean));
  psParams->setNamedConstant("stddev",
      static_cast<Ogre::Real>(this->stdDev));
}

//////////////////////////////////////////////////
void Ogre2DepthGaussianNoisePass::CreateRenderPass()
{
  // This function is similar to Ogre2GaussianNoisePass but duplicated here
  // for Ogre2DepthCamera.

  static int gaussianDepthNodeCounter = 0;

  auto engine = Ogre2RenderEngine::Instance();
  auto ogreRoot = engine->OgreRoot();
  Ogre::CompositorManager2 *ogreCompMgr = ogreRoot->getCompositorManager2();

  std::string nodeDefName = "GaussianDepthNoiseNode_"
      + std::to_string(gaussianDepthNodeCounter);

  if (ogreCompMgr->hasNodeDefinition(nodeDefName))
    return;

  // The GaussianNoise material is defined in script (gaussian_noise.material).
  // clone the material
  std::string matName = "GaussianNoiseDepth";
  Ogre::MaterialPtr ogreMat =
      Ogre::MaterialManager::getSingleton().getByName(matName);
  if (!ogreMat)
  {
    gzerr << "Gaussian noise material not found: '" << matName << "'"
           << std::endl;
    return;
  }
  if (!ogreMat->isLoaded())
    ogreMat->load();
  std::string materialName = matName + "_" +
      std::to_string(gaussianDepthNodeCounter);
  this->gaussianNoiseMat = ogreMat->clone(materialName).get();

  this->ogreCompositorNodeDefName = nodeDefName;
  gaussianDepthNodeCounter++;

  Ogre::CompositorNodeDef *nodeDef =
      ogreCompMgr->addNodeDefinition(nodeDefName);

  // Input texture
  nodeDef->addTextureSourceName("rt_input", 0,
      Ogre::TextureDefinitionBase::TEXTURE_INPUT);
  nodeDef->addTextureSourceName("rt_output", 1,
      Ogre::TextureDefinitionBase::TEXTURE_INPUT);

  // rt_input target
  nodeDef->setNumTargetPass(1);
  Ogre::CompositorTargetDef *inputTargetDef =
      nodeDef->addTargetPass("rt_output");
  inputTargetDef->setNumPasses(1);
  {
    // quad pass
    Ogre::CompositorPassQuadDef *passQuad =
        static_cast<Ogre::CompositorPassQuadDef *>(
        inputTargetDef->addPass(Ogre::PASS_QUAD));
    passQuad->setAllLoadActions(Ogre::LoadAction::Clear);

    passQuad->mMaterialName = materialName;
    passQuad->addQuadTextureSource(0, "rt_input");
  }
  nodeDef->mapOutputChannel(0, "rt_output");
  nodeDef->mapOutputChannel(1, "rt_input");
}

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
  this->RemoveAllRenderPasses();
  this->dataPtr->depthReadback.Destroy();
  this->dataPtr->pointReadback.Destroy();

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

  if (!this->ogreCamera)
    return;

  auto engine = Ogre2RenderEngine::Instance();
  auto ogreRoot = engine->OgreRoot();
  Ogre::CompositorManager2 *ogreCompMgr = ogreRoot->getCompositorManager2();

  // remove depth texture, material, compositor
  for (size_t i = 0u; i < 2u; ++i)
  {
    if (this->dataPtr->ogreDepthTexture[i])
    {
      ogreRoot->getRenderSystem()->getTextureGpuManager()->destroyTexture(
            this->dataPtr->ogreDepthTexture[i]);
      this->dataPtr->ogreDepthTexture[i] = nullptr;
    }
  }
  if (this->dataPtr->ogreCompositorWorkspace)
  {
    ogreCompMgr->removeWorkspace(
        this->dataPtr->ogreCompositorWorkspace);
    this->dataPtr->colorTargetDef = nullptr;
    this->dataPtr->particleTargetDef = nullptr;
  }

  if (this->dataPtr->depthMaterial)
  {
    Ogre::MaterialManager::getSingleton().remove(
        this->dataPtr->depthMaterial->getName());
    this->dataPtr->depthMaterial.setNull();
  }
  if (this->dataPtr->depthFinalMaterial)
  {
    Ogre::MaterialManager::getSingleton().remove(
        this->dataPtr->depthFinalMaterial->getName());
    this->dataPtr->depthFinalMaterial.setNull();
  }
  if (this->dataPtr->packMaterial)
  {
    Ogre::MaterialManager::getSingleton().remove(
        this->dataPtr->packMaterial->getName());
    this->dataPtr->packMaterial.setNull();
  }

  if (!this->dataPtr->ogreCompositorWorkspaceDef.empty())
  {
    ogreCompMgr->removeWorkspaceDefinition(
        this->dataPtr->ogreCompositorWorkspaceDef);
    ogreCompMgr->removeNodeDefinition(
        this->dataPtr->ogreCompositorBaseNodeDef);
    ogreCompMgr->removeNodeDefinition(
        this->dataPtr->ogreCompositorFinalNodeDef);
  }

  if (this->dataPtr->particleNoiseListener)
  {
    this->ogreCamera->removeListener(
        this->dataPtr->particleNoiseListener.get());
    this->dataPtr->particleNoiseListener.reset();
  }

  Ogre::SceneManager *ogreSceneManager;
  ogreSceneManager = this->scene->OgreSceneManager();
  if (ogreSceneManager == nullptr)
  {
    gzerr << "Scene manager cannot be obtained" << std::endl;
  }
  else
  {
    if (ogreSceneManager->findCameraNoThrow(this->name) != nullptr)
    {
      ogreSceneManager->destroyCamera(this->ogreCamera);
      this->ogreCamera = nullptr;
    }
  }

  BaseDepthCamera::Destroy();
}

//////////////////////////////////////////////////
void Ogre2DepthCamera::CreateCamera()
{
  // create ogre camera object
  Ogre::SceneManager *ogreSceneManager;
  ogreSceneManager = this->scene->OgreSceneManager();
  if (ogreSceneManager == nullptr)
  {
    gzerr << "Scene manager cannot be obtained" << std::endl;
    return;
  }

  this->ogreCamera = ogreSceneManager->createCamera(this->name);
  if (this->ogreCamera == nullptr)
  {
    gzerr << "Ogre camera cannot be created" << std::endl;
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
  GZ_PROFILE("Ogre2DepthCamera::CreateDepthTexture");
  // set aspect ratio and fov
  const double aspectRatio = this->AspectRatio();
  const double angle = this->HFOV().Radian();
  const double vfov =
    this->LimitFOV(2.0 * atan(tan(angle / 2.0) / aspectRatio));
  this->ogreCamera->setFOVy(Ogre::Radian((Ogre::Real)vfov));
  this->ogreCamera->setAspectRatio((Ogre::Real)aspectRatio);

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
  Ogre::Vector2 projectionAB = this->ogreCamera->getProjectionParamsAB();
  double projectionA = projectionAB.x;
  double projectionB = projectionAB.y;
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
  psParams->setNamedConstant("particleStddev",
    static_cast<float>(this->dataPtr->particleStddev));
  // The merged depth-only pass shares this material (Render sets
  // depthOnly); only one of the two runs in a frame.
  psParams->setNamedConstant("depthOnly", 0.0f);
  psParams->setNamedConstant("hasParticles", 0.0f);

  std::string matDepthFinalName = "DepthCameraFinal";
  Ogre::MaterialPtr matDepthFinal =
      Ogre::MaterialManager::getSingleton().getByName(matDepthFinalName);
  this->dataPtr->depthFinalMaterial = matDepthFinal->clone(
      this->Name() + "_" + matDepthFinalName);
  this->dataPtr->depthFinalMaterial->load();
  Ogre::Pass *passFinal =
      this->dataPtr->depthFinalMaterial->getTechnique(0)->getPass(0);
  Ogre::GpuProgramParametersSharedPtr psParamsFinal =
      passFinal->getFragmentProgramParameters();
  psParamsFinal->setNamedConstant("near",
      static_cast<float>(this->NearClipPlane()));
  psParamsFinal->setNamedConstant("far",
      static_cast<float>(this->FarClipPlane()));
  psParamsFinal->setNamedConstant("max",
      static_cast<float>(this->dataPtr->dataMaxVal));
  psParamsFinal->setNamedConstant("min",
      static_cast<float>(this->dataPtr->dataMinVal));
  psParamsFinal->setNamedConstant("packXyz", 0.0f);

  // A copy of it for the pass packing the xyz output.
  // ponytail: rows must pack into whole RGBA texels (width multiple of 4);
  // other widths keep PF_FLOAT32_RGBA point clouds.
  this->dataPtr->packedPoints =
      this->dataPtr->pointFormat == PF_FLOAT32_RGB &&
      this->ImageWidth() % 4u == 0u;
  if (this->dataPtr->packedPoints)
  {
    this->dataPtr->packMaterial = this->dataPtr->depthFinalMaterial->clone(
        this->Name() + "_" + matDepthFinalName + "_pack");
    this->dataPtr->packMaterial->load();
    this->dataPtr->packMaterial->getTechnique(0)->getPass(0)
        ->getFragmentProgramParameters()->setNamedConstant("packXyz", 1.0f);
  }

  // create background material is specified
  MaterialPtr backgroundMaterial = this->Scene()->BackgroundMaterial();
  bool validBackground = backgroundMaterial &&
      !backgroundMaterial->EnvironmentMap().empty();

  // let depth camera shader know if there is background material
  // This is needed for manual clipping of color pixel values.
  psParams->setNamedConstant("hasBackground",
      static_cast<int>(validBackground));

  if (validBackground)
  {
    Ogre::MaterialManager &matManager = Ogre::MaterialManager::getSingleton();
    std::string skyMatName = this->dataPtr->kSkyboxMaterialName + "_"
        + this->Name();
    auto mat = matManager.getByName(skyMatName);
    if (!mat)
    {
      auto skyboxMat = matManager.getByName(this->dataPtr->kSkyboxMaterialName);
      if (!skyboxMat)
      {
        gzerr << "Unable to find skybox material" << std::endl;
        return;
      }
      mat = skyboxMat->clone(skyMatName);
    }
    Ogre::TextureUnitState *texUnit =
        mat->getTechnique(0u)->getPass(0u)->getTextureUnitState(0u);
    texUnit->setTextureName(backgroundMaterial->EnvironmentMap(),
        Ogre::TextureTypes::TypeCube);
  }

  // Create depth camera compositor
  auto engine = Ogre2RenderEngine::Instance();
  auto ogreRoot = engine->OgreRoot();
  Ogre::CompositorManager2 *ogreCompMgr = ogreRoot->getCompositorManager2();

  std::string wsDefName = "DepthCameraWorkspace_" + this->Name();
  this->dataPtr->ogreCompositorWorkspaceDef = wsDefName;
  if (!ogreCompMgr->hasWorkspaceDefinition(wsDefName))
  {
    // The depth camera compositor does a few passes in order to simulate
    // particles effects in depth / point cloud image data
    //
    // render scene (color) with particles, c1
    // render scene (depth) without particles, d1
    // render scene (grayscale) with particles only, g2
    // render scene (depth) with particles only, d2
    //
    //   if g2 is non-zero // pixel with particle
    //     if d2 < d1 // particle is in view
    //       apply noise and scatterbility to d2
    //       set depth data to d2
    //     else
    //       set depth data to d1
    //   set color data to c1

    // We need to programmatically create the compositor because we need to
    // configure it to use the cloned depth material created earlier.
    // The compositor node definition is equivalent to the following:
    //
    // compositor_node DepthCamera
    // {
    //   texture rt0 target_width target_height PF_FLOAT32_RGBA
    //   texture rt1 target_width target_height PF_FLOAT32_RGBA
    //   texture colorTexture target_width target_height PF_R8G8B8
    //       depth_texture depth_format PF_D32_FLOAT
    //   texture depthTexture target_width target_height PF_D32_FLOAT
    //   texture particleTexture target_width target_height PFG_R8_UNORM
    //   // particleDepthTexture shares same depth buffer as particleTexture
    //   texture particleDepthTexture target_width target_height PFG_D32_FLOAT
    //
    //   rtv particleTexture
    //   {
    //     depth particleDepthTexture
    //   }
    //
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
    //   target depthTexture
    //   {
    //     pass clear
    //     {
    //       colour_value 0.0 0.0 0.0 1.0
    //     }
    //     pass render_scene
    //     {
    //       visibility_mask 0x11011111
    //     }
    //   }
    //   target particleTexture
    //   {
    //     pass clear
    //     {
    //     }
    //     pass render_scene
    //     {
    //       visibility_mask 0x00100000
    //     }
    //   }
    //   target rt0
    //   {
    //     pass clear
    //     {
    //     }
    //     pass render_quad
    //     {
    //       material DepthCamera // Use copy instead of original
    //       input 0 depthTexture
    //       input 1 colorTexture
    //       quad_normals camera_far_corners_view_space
    //     }
    //   }
    //   out 0 rt0
    //   out 1 rt1
    // }

    std::string baseNodeDefName = wsDefName + "/BaseNode";
    this->dataPtr->ogreCompositorBaseNodeDef = baseNodeDefName;
    Ogre::CompositorNodeDef *baseNodeDef =
        ogreCompMgr->addNodeDefinition(baseNodeDefName);

    baseNodeDef->addTextureSourceName(
          "rt0", 0u, Ogre::TextureDefinitionBase::TEXTURE_INPUT);
    baseNodeDef->addTextureSourceName(
          "rt1", 1u, Ogre::TextureDefinitionBase::TEXTURE_INPUT);

    Ogre::TextureDefinitionBase::TextureDefinition *depthTexDef =
        baseNodeDef->addTextureDefinition("depthTexture");
    depthTexDef->textureType = Ogre::TextureTypes::Type2D;
    depthTexDef->width = 0;
    depthTexDef->height = 0;
    depthTexDef->depthOrSlices = 1;
    depthTexDef->numMipmaps = 0;
    depthTexDef->widthFactor = 1;
    depthTexDef->heightFactor = 1;
    depthTexDef->format = Ogre::PFG_D32_FLOAT;
    depthTexDef->textureFlags &= ~Ogre::TextureFlags::Uav;
    depthTexDef->depthBufferId = Ogre::DepthBuffer::POOL_DEFAULT;
    depthTexDef->depthBufferFormat = Ogre::PFG_UNKNOWN;
    depthTexDef->fsaa = "0";

    Ogre::RenderTargetViewDef *rtvDepth =
      baseNodeDef->addRenderTextureView("depthTexture");
    rtvDepth->setForTextureDefinition("depthTexture", depthTexDef );

    Ogre::TextureDefinitionBase::TextureDefinition *colorTexDef =
        baseNodeDef->addTextureDefinition("colorTexture");
    colorTexDef->textureType = Ogre::TextureTypes::Type2D;
    colorTexDef->width = 0;
    colorTexDef->height = 0;
    colorTexDef->depthOrSlices = 1;
    colorTexDef->numMipmaps = 0;
    colorTexDef->widthFactor = 1;
    colorTexDef->heightFactor = 1;
    colorTexDef->format = Ogre::PFG_RGBA8_UNORM_SRGB;
    // Note we are using low level materials in quad pass so also had to perform
    // gamma correction in the fragment shaders (depth_camera_fs.glsl)
    colorTexDef->textureFlags &= ~Ogre::TextureFlags::Uav;
    colorTexDef->depthBufferId = Ogre::DepthBuffer::POOL_DEFAULT;
    colorTexDef->depthBufferFormat = Ogre::PFG_D32_FLOAT;
    colorTexDef->preferDepthTexture = true;
    colorTexDef->fsaa = "0";
    // keep_content: depth-only frames neither render nor clear it, and the
    // depth shader samples it (without using it); a discardable texture
    // would start every frame undefined
    colorTexDef->textureFlags &= ~Ogre::TextureFlags::DiscardableContent;

    Ogre::RenderTargetViewDef *rtvColor =
      baseNodeDef->addRenderTextureView("colorTexture");
    rtvColor->setForTextureDefinition("colorTexture", colorTexDef);

    Ogre::TextureDefinitionBase::TextureDefinition *particleTexDef =
        baseNodeDef->addTextureDefinition("particleTexture");
    particleTexDef->textureType = Ogre::TextureTypes::Type2D;
    particleTexDef->width = 0;
    particleTexDef->height = 0;
    particleTexDef->depthOrSlices = 1;
    particleTexDef->numMipmaps = 0;
    particleTexDef->widthFactor = 0.5;
    particleTexDef->heightFactor = 0.5;
    particleTexDef->format = Ogre::PFG_R8_UNORM;
    particleTexDef->textureFlags &= ~Ogre::TextureFlags::Uav;
    particleTexDef->depthBufferId = Ogre::DepthBuffer::POOL_DEFAULT;
    particleTexDef->depthBufferFormat = Ogre::PFG_UNKNOWN;
    particleTexDef->preferDepthTexture = false;
    particleTexDef->fsaa = "0";
    // keep_content: without particles it is neither rendered nor cleared
    // (see hasParticles), like colorTexture
    particleTexDef->textureFlags &= ~Ogre::TextureFlags::DiscardableContent;

    Ogre::TextureDefinitionBase::TextureDefinition *particleDepthTexDef =
        baseNodeDef->addTextureDefinition("particleDepthTexture");
    particleDepthTexDef->textureType = Ogre::TextureTypes::Type2D;
    particleDepthTexDef->width = 0;
    particleDepthTexDef->height = 0;
    particleDepthTexDef->depthOrSlices = 1;
    particleDepthTexDef->numMipmaps = 0;
    particleDepthTexDef->widthFactor = 0.5;
    particleDepthTexDef->heightFactor = 0.5;
    particleDepthTexDef->format = Ogre::PFG_D32_FLOAT;
    particleDepthTexDef->depthBufferId = Ogre::DepthBuffer::POOL_NON_SHAREABLE;
    particleDepthTexDef->depthBufferFormat = Ogre::PFG_UNKNOWN;
    particleDepthTexDef->fsaa = "0";
    particleDepthTexDef->textureFlags &= ~Ogre::TextureFlags::Uav;
    particleDepthTexDef->textureFlags &=
        ~Ogre::TextureFlags::DiscardableContent;

    // Auto setup the RTV then manually override the depth buffer so
    // it uses the one we created (and thus we can sample from it later)
    Ogre::RenderTargetViewDef *rtvParticleTexture =
      baseNodeDef->addRenderTextureView("particleTexture");
    rtvParticleTexture->setForTextureDefinition("particleTexture",
                                                particleTexDef);
    rtvParticleTexture->depthAttachment.textureName = "particleDepthTexture";

    // Depth-only output of the merged pass, same as the final node's
    // rt_depth
    Ogre::TextureDefinitionBase::TextureDefinition *mergedTexDef =
        baseNodeDef->addTextureDefinition("depthOnlyTexture");
    mergedTexDef->textureType = Ogre::TextureTypes::Type2D;
    mergedTexDef->width = 0;
    mergedTexDef->height = 0;
    mergedTexDef->depthOrSlices = 1;
    mergedTexDef->numMipmaps = 0;
    mergedTexDef->widthFactor = 1;
    mergedTexDef->heightFactor = 1;
    mergedTexDef->format = Ogre::PFG_R32_UINT;
    mergedTexDef->textureFlags &= ~Ogre::TextureFlags::Uav;
    mergedTexDef->depthBufferId = Ogre::DepthBuffer::POOL_NO_DEPTH;
    mergedTexDef->depthBufferFormat = Ogre::PFG_UNKNOWN;
    mergedTexDef->preferDepthTexture = false;
    mergedTexDef->fsaa = "0";
    Ogre::RenderTargetViewDef *rtvMerged =
      baseNodeDef->addRenderTextureView("depthOnlyTexture");
    rtvMerged->setForTextureDefinition("depthOnlyTexture", mergedTexDef);

    baseNodeDef->setNumTargetPass(5);
    this->dataPtr->colorTargetDef =
        baseNodeDef->addTargetPass("colorTexture");
    if (validBackground)
      this->dataPtr->colorTargetDef->setNumPasses(4);
    else
      this->dataPtr->colorTargetDef->setNumPasses(3);
    {
      // clear pass
      Ogre::CompositorPassClearDef *passClear =
          static_cast<Ogre::CompositorPassClearDef *>(
          this->dataPtr->colorTargetDef->addPass(Ogre::PASS_CLEAR));
      passClear->mExecutionMask = this->dataPtr->kDepthExecutionMask;

      // scene pass - opaque
      {
        Ogre::CompositorPassSceneDef *passScene =
            static_cast<Ogre::CompositorPassSceneDef *>(
            this->dataPtr->colorTargetDef->addPass(Ogre::PASS_SCENE));
        passScene->mShadowNode = this->dataPtr->kShadowNodeName;
        passScene->setVisibilityMask(GZ_VISIBILITY_ALL);
        passScene->mIncludeOverlays = false;
        passScene->mFirstRQ = 0u;
        passScene->mLastRQ = 2u;
        if (validBackground)
        {
          passScene->setAllLoadActions(Ogre::LoadAction::DontCare);
          passScene->mLoadActionDepth = Ogre::LoadAction::Clear;
          passScene->mLoadActionStencil = Ogre::LoadAction::Clear;
        }
        else
        {
          passScene->setAllLoadActions(Ogre::LoadAction::Clear);
          passScene->setAllClearColours(
              Ogre2Conversions::Convert(this->Scene()->BackgroundColor()));

        }
        passScene->mExecutionMask = ~this->dataPtr->kDepthExecutionMask;
      }

      // render background, e.g. sky, after opaque stuff
      if (validBackground)
      {
        // quad pass
        Ogre::CompositorPassQuadDef *passQuad =
            static_cast<Ogre::CompositorPassQuadDef *>(
            this->dataPtr->colorTargetDef->addPass(Ogre::PASS_QUAD));
        passQuad->mMaterialName = this->dataPtr->kSkyboxMaterialName + "_"
            + this->Name();
        passQuad->mFrustumCorners =
            Ogre::CompositorPassQuadDef::CAMERA_DIRECTION;
        passQuad->mExecutionMask = ~this->dataPtr->kDepthExecutionMask;
      }

      // scene pass - transparent stuff
      {
        Ogre::CompositorPassSceneDef *passScene =
            static_cast<Ogre::CompositorPassSceneDef *>(
            this->dataPtr->colorTargetDef->addPass(Ogre::PASS_SCENE));
        passScene->setVisibilityMask(GZ_VISIBILITY_ALL);
        // todo(anyone) PbsMaterialsShadowNode is hardcoded.
        // Although this may be just fine
        passScene->mShadowNode = this->dataPtr->kShadowNodeName;
        passScene->mFirstRQ = 2u;
        passScene->mExecutionMask = ~this->dataPtr->kDepthExecutionMask;
      }
    }

    Ogre::CompositorTargetDef *depthTargetDef =
        baseNodeDef->addTargetPass("depthTexture");
    depthTargetDef->setNumPasses(1);
    {
      // scene pass
      Ogre::CompositorPassSceneDef *passScene =
          static_cast<Ogre::CompositorPassSceneDef *>(
          depthTargetDef->addPass(Ogre::PASS_SCENE));
      passScene->setAllLoadActions(Ogre::LoadAction::Clear);
      passScene->setAllClearColours(Ogre::ColourValue(
        this->FarClipPlane(),
        this->FarClipPlane(),
        this->FarClipPlane()));
      // depth texture does not contain particles
      passScene->setVisibilityMask(
        GZ_VISIBILITY_ALL & ~Ogre2ParticleEmitter::kParticleVisibilityFlags);
      passScene->mEnableForwardPlus = false;
      passScene->setLightVisibilityMask(0x0);
    }

    // Ogre::CompositorTargetDef *particleTargetDef =
    this->dataPtr->particleTargetDef =
        baseNodeDef->addTargetPass("particleTexture");
    this->dataPtr->particleTargetDef->setNumPasses(2);
    {
      // clear pass
      Ogre::CompositorPassClearDef *passClear =
          static_cast<Ogre::CompositorPassClearDef *>(
          this->dataPtr->particleTargetDef->addPass(Ogre::PASS_CLEAR));
      passClear->setAllClearColours(Ogre::ColourValue::Black);
      // never runs: without particles the shader reads the particle target
      // as empty (hasParticles), and the scene pass clears it
      passClear->mExecutionMask = ~this->dataPtr->kDepthExecutionMask;

      // scene pass
      Ogre::CompositorPassSceneDef *passScene =
          static_cast<Ogre::CompositorPassSceneDef *>(
          this->dataPtr->particleTargetDef->addPass(Ogre::PASS_SCENE));
      passScene->setAllLoadActions(Ogre::LoadAction::Clear);
      passScene->setAllClearColours(Ogre::ColourValue::Black);
      passScene->setVisibilityMask(
        Ogre2ParticleEmitter::kParticleVisibilityFlags);
      passScene->mEnableForwardPlus = false;
      passScene->setLightVisibilityMask(0x0);
      passScene->mExecutionMask = ~this->dataPtr->kDepthExecutionMask;
    }

    // rt0 target - converts depth to xyz
    Ogre::CompositorTargetDef *inTargetDef =
        baseNodeDef->addTargetPass("rt0");
    inTargetDef->setNumPasses(1);
    {
      // quad pass
      Ogre::CompositorPassQuadDef *passQuad =
          static_cast<Ogre::CompositorPassQuadDef *>(
          inTargetDef->addPass(Ogre::PASS_QUAD));
      passQuad->setAllLoadActions(Ogre::LoadAction::Clear);
      passQuad->setAllClearColours(Ogre::ColourValue(
        this->FarClipPlane(),
        this->FarClipPlane(),
        this->FarClipPlane()));

      passQuad->mMaterialName = this->dataPtr->depthMaterial->getName();
      passQuad->addQuadTextureSource(0, "depthTexture");
      passQuad->addQuadTextureSource(1, "colorTexture");
      passQuad->addQuadTextureSource(2, "particleTexture");
      passQuad->addQuadTextureSource(3, "particleDepthTexture");
      passQuad->mFrustumCorners =
          Ogre::CompositorPassQuadDef::VIEW_SPACE_CORNERS;
      this->dataPtr->xyzPassDef = passQuad;
    }

    // depthOnlyTexture target - xyz and final depth-only pass in one;
    // disabled until Render chooses it
    Ogre::CompositorTargetDef *mergedTargetDef =
        baseNodeDef->addTargetPass("depthOnlyTexture");
    mergedTargetDef->setNumPasses(1);
    {
      Ogre::CompositorPassQuadDef *passQuad =
          static_cast<Ogre::CompositorPassQuadDef *>(
          mergedTargetDef->addPass(Ogre::PASS_QUAD));
      // the quad covers every pixel
      passQuad->setAllLoadActions(Ogre::LoadAction::DontCare);
      passQuad->mMaterialName = this->dataPtr->depthMaterial->getName();
      passQuad->addQuadTextureSource(0, "depthTexture");
      passQuad->addQuadTextureSource(1, "colorTexture");
      passQuad->addQuadTextureSource(2, "particleTexture");
      passQuad->addQuadTextureSource(3, "particleDepthTexture");
      passQuad->mFrustumCorners =
          Ogre::CompositorPassQuadDef::VIEW_SPACE_CORNERS;
      passQuad->mExecutionMask = ~this->dataPtr->kDepthExecutionMask;
      this->dataPtr->mergedPassDef = passQuad;
    }

    baseNodeDef->mapOutputChannel(0, "rt0");
    baseNodeDef->mapOutputChannel(1, "rt1");

    // Programmatically create the final pass node and use the cloned final
    // depth material created earlier.
    // The compositor node definition is equivalent to the following:
    //
    // compositor_node DepthCameraFinal
    // {
    //   in 0 rt_output
    //   in 1 rt_input
    //
    //   target rt_output
    //   {
    //     pass clear
    //     {
    //     }
    //     pass render_quad
    //     {
    //       material DepthCameraFinal // Use copy instead of original
    //       input 0 rt_input
    //     }
    //   }
    // }

    std::string finalNodeDefName = wsDefName + "/FinalNode";
    this->dataPtr->ogreCompositorFinalNodeDef = finalNodeDefName;
    Ogre::CompositorNodeDef *finalNodeDef =
        ogreCompMgr->addNodeDefinition(finalNodeDefName);

    finalNodeDef->addTextureSourceName("rt_input", 0,
        Ogre::TextureDefinitionBase::TEXTURE_INPUT);
    // output texture
    finalNodeDef->addTextureSourceName("rt_output", 1,
        Ogre::TextureDefinitionBase::TEXTURE_INPUT);

    // Depth-only output: the final pass writes into a single channel uint
    // target too, which keeps the x (depth) bits of its uvec4 output. It is
    // read back instead of rt_output when there are no point cloud
    // subscribers: a quarter of the bytes.
    Ogre::TextureDefinitionBase::TextureDefinition *depthOnlyTexDef =
        finalNodeDef->addTextureDefinition("rt_depth");
    depthOnlyTexDef->textureType = Ogre::TextureTypes::Type2D;
    depthOnlyTexDef->width = 0;
    depthOnlyTexDef->height = 0;
    depthOnlyTexDef->depthOrSlices = 1;
    depthOnlyTexDef->numMipmaps = 0;
    depthOnlyTexDef->widthFactor = 1;
    depthOnlyTexDef->heightFactor = 1;
    depthOnlyTexDef->format = Ogre::PFG_R32_UINT;
    depthOnlyTexDef->textureFlags &= ~Ogre::TextureFlags::Uav;
    depthOnlyTexDef->depthBufferId = Ogre::DepthBuffer::POOL_NO_DEPTH;
    depthOnlyTexDef->depthBufferFormat = Ogre::PFG_UNKNOWN;
    depthOnlyTexDef->preferDepthTexture = false;
    depthOnlyTexDef->fsaa = "0";
    Ogre::RenderTargetViewDef *rtvDepthOnly =
      finalNodeDef->addRenderTextureView("rt_depth");
    rtvDepthOnly->setForTextureDefinition("rt_depth", depthOnlyTexDef);

    // Packed xyz output: the x, y, z of the xyz + colour output, rows
    // without padding, 4 floats per texel: an xyz PointCloudPacked's data.
    if (this->dataPtr->packedPoints)
    {
      Ogre::TextureDefinitionBase::TextureDefinition *pointsTexDef =
          finalNodeDef->addTextureDefinition("rt_points");
      pointsTexDef->textureType = Ogre::TextureTypes::Type2D;
      pointsTexDef->width = this->ImageWidth() * 3u / 4u;
      pointsTexDef->height = this->ImageHeight();
      pointsTexDef->depthOrSlices = 1;
      pointsTexDef->numMipmaps = 0;
      pointsTexDef->format = Ogre::PFG_RGBA32_UINT;
      pointsTexDef->textureFlags &= ~Ogre::TextureFlags::Uav;
      pointsTexDef->depthBufferId = Ogre::DepthBuffer::POOL_NO_DEPTH;
      pointsTexDef->depthBufferFormat = Ogre::PFG_UNKNOWN;
      pointsTexDef->preferDepthTexture = false;
      pointsTexDef->fsaa = "0";
      Ogre::RenderTargetViewDef *rtvPoints =
        finalNodeDef->addRenderTextureView("rt_points");
      rtvPoints->setForTextureDefinition("rt_points", pointsTexDef);
    }

    finalNodeDef->setNumTargetPass(this->dataPtr->packedPoints ? 3 : 2);
    // rt_output target - converts depth to xyz
    Ogre::CompositorTargetDef *outputTargetDef =
        finalNodeDef->addTargetPass("rt_output");
    outputTargetDef->setNumPasses(1);
    {
      // quad pass
      Ogre::CompositorPassQuadDef *passQuad =
          static_cast<Ogre::CompositorPassQuadDef *>(
          outputTargetDef->addPass(Ogre::PASS_QUAD));
      passQuad->setAllLoadActions(Ogre::LoadAction::Clear);
      passQuad->setAllClearColours(Ogre::ColourValue(
        this->FarClipPlane(),
        this->FarClipPlane(),
        this->FarClipPlane()));

      passQuad->mMaterialName = this->dataPtr->depthFinalMaterial->getName();
      passQuad->addQuadTextureSource(0, "rt_input");
      this->dataPtr->finalPassDef = passQuad;
    }
    // rt_depth target - same pass, depth only; disabled until Render
    // finds no point cloud subscribers
    Ogre::CompositorTargetDef *depthOnlyTargetDef =
        finalNodeDef->addTargetPass("rt_depth");
    depthOnlyTargetDef->setNumPasses(1);
    {
      Ogre::CompositorPassQuadDef *passQuad =
          static_cast<Ogre::CompositorPassQuadDef *>(
          depthOnlyTargetDef->addPass(Ogre::PASS_QUAD));
      // the quad covers every pixel
      passQuad->setAllLoadActions(Ogre::LoadAction::DontCare);
      passQuad->mMaterialName = this->dataPtr->depthFinalMaterial->getName();
      passQuad->addQuadTextureSource(0, "rt_input");
      passQuad->mExecutionMask = ~this->dataPtr->kDepthExecutionMask;
      this->dataPtr->depthOnlyPassDef = passQuad;
    }
    // rt_points target - packs rt_output; disabled until Render finds
    // point cloud subscribers
    if (this->dataPtr->packedPoints)
    {
      Ogre::CompositorTargetDef *pointsTargetDef =
          finalNodeDef->addTargetPass("rt_points");
      pointsTargetDef->setNumPasses(1);
      Ogre::CompositorPassQuadDef *passQuad =
          static_cast<Ogre::CompositorPassQuadDef *>(
          pointsTargetDef->addPass(Ogre::PASS_QUAD));
      // the quad covers every pixel
      passQuad->setAllLoadActions(Ogre::LoadAction::DontCare);
      passQuad->mMaterialName = this->dataPtr->packMaterial->getName();
      passQuad->addQuadTextureSource(0, "rt_output");
      passQuad->mExecutionMask = ~this->dataPtr->kDepthExecutionMask;
      this->dataPtr->packPassDef = passQuad;
    }
    finalNodeDef->mapOutputChannel(0, "rt_output");

    // Finally create the workspace.
    // The compositor workspace definition is equivalent to the following:
    //
    // workspace DepthCameraWorkspace
    // {
    //   connect_output DepthCameraFinal 0
    //   connect DepthCamera 0 DepthCameraFinal 1
    // }
    Ogre::CompositorWorkspaceDef *workDef =
        ogreCompMgr->addWorkspaceDefinition(wsDefName);

    workDef->connectExternal(0, baseNodeDefName, 0);
    workDef->connectExternal(1, baseNodeDefName, 1);
    workDef->connect(baseNodeDefName, finalNodeDefName);
  }
  Ogre::CompositorWorkspaceDef *wsDef =
      ogreCompMgr->getWorkspaceDefinition(wsDefName);

  if (!wsDef)
  {
    gzerr << "Unable to add workspace definition [" << wsDefName << "] "
           << " for " << this->Name();
  }

  Ogre::TextureGpuManager *textureMgr =
    ogreRoot->getRenderSystem()->getTextureGpuManager();
  // create render texture - these textures pack the range data
  for (size_t i = 0u; i < 2u; ++i)
  {
    this->dataPtr->ogreDepthTexture[i] =
        textureMgr->createTexture(
          this->Name() + "_depth" + std::to_string(i),
          Ogre::GpuPageOutStrategy::SaveToSystemRam,
          Ogre::TextureFlags::RenderToTexture,
          Ogre::TextureTypes::Type2D);

      this->dataPtr->ogreDepthTexture[i]->setResolution(
        this->ImageWidth(), this->ImageHeight());
      this->dataPtr->ogreDepthTexture[i]->setNumMipmaps(1u);
      this->dataPtr->ogreDepthTexture[i]->setPixelFormat(
        Ogre::PFG_RGBA32_UINT);

      this->dataPtr->ogreDepthTexture[i]->scheduleTransitionTo(
        Ogre::GpuResidency::Resident);
  }

  this->CreateWorkspaceInstance();
}

//////////////////////////////////////////////////
void Ogre2DepthCamera::CreateWorkspaceInstance()
{
  auto engine = Ogre2RenderEngine::Instance();
  auto ogreRoot = engine->OgreRoot();
  Ogre::CompositorManager2 *ogreCompMgr = ogreRoot->getCompositorManager2();

  Ogre::CompositorChannelVec externalTargets(2u);

  externalTargets[0] = this->dataPtr->ogreDepthTexture[0];
  externalTargets[1] = this->dataPtr->ogreDepthTexture[1];

  // create compositor worksspace
  this->dataPtr->ogreCompositorWorkspace =
      ogreCompMgr->addWorkspace(
          this->scene->OgreSceneManager(),
          externalTargets,
          this->ogreCamera,
          this->dataPtr->ogreCompositorWorkspaceDef,
          false, -1, 0, 0, Ogre::Vector4::ZERO, 0x00,
          this->dataPtr->kDepthExecutionMask);

  this->dataPtr->ogreCompositorWorkspace->addListener(
    engine->TerraWorkspaceListener());

  // add the listener
  Ogre::CompositorNode *node =
      this->dataPtr->ogreCompositorWorkspace->getNodeSequence()[0];
  auto channelsTex = node->getLocalTextures();

  for (const auto &c : channelsTex)
  {
    if (c->getPixelFormat() == Ogre::PFG_R8_UNORM)
    {
      // add particle noise / scatter effects listener so we can set the
      // amount of noise based on size of emitter
      this->dataPtr->particleNoiseListener.reset(
          new Ogre2ParticleNoiseListener(this->scene,
          this->dataPtr->depthMaterial));
      this->ogreCamera->addListener(
            this->dataPtr->particleNoiseListener.get());
      break;
    }
  }
}

/////////////////////////////////////////////////
math::Matrix4d Ogre2DepthCamera::ProjectionMatrix() const
{
  return Ogre2Conversions::Convert(this->ogreCamera->getProjectionMatrix());
}

/////////////////////////////////////////////////
void Ogre2DepthCamera::SetProjectionMatrix(const math::Matrix4d &_matrix)
{
  BaseDepthCamera::SetProjectionMatrix(_matrix);
  this->ogreCamera->setCustomProjectionMatrix(
      true, Ogre2Conversions::Convert(this->projectionMatrix));
}

//////////////////////////////////////////////////
void Ogre2DepthCamera::Render()
{
  GZ_PROFILE("Ogre2DepthCamera::Render");
  // A readback issued for an earlier frame is not this frame's.
  this->dataPtr->readbackIssued = false;
  // Depth-only or point cloud frame: chosen from the point cloud
  // connections at Render time, not at PreRender. A sensor connects its
  // point cloud callback after the scene's PreRender (gz-sim:
  // Scene::PreRender, then the sensors' Update), and that frame must
  // deliver a point cloud.
  // Without point cloud subscribers the final pass writes depth only, and
  // PostRender reads back that instead of the xyz + colour output.
  // (CreateDepthTexture sets all the pass defs used here together)
  if (this->dataPtr->finalPassDef)
  {
    // (the legacy readback reads the xyz + colour output)
    const bool depthOnly =
        this->dataPtr->newRgbPointCloud.ConnectionCount() == 0u &&
        !Ogre2UseLegacyReadback();
    // Without render passes (noise) between the base and the final node,
    // one quad pass of the base node writes the depth-only output: the
    // xyz pass followed by the final depth-only pass, in one pass.
    const bool merged = depthOnly &&
        std::none_of(this->dataPtr->renderPasses.begin(),
            this->dataPtr->renderPasses.end(),
            [](const RenderPassPtr &_pass) { return _pass->IsEnabled(); });
    auto enable = [this](Ogre::CompositorPassDef *_def, bool _on)
    {
      _def->mExecutionMask = _on ? this->dataPtr->kDepthExecutionMask :
          ~this->dataPtr->kDepthExecutionMask;
    };
    enable(this->dataPtr->finalPassDef, !depthOnly);
    // Point cloud frames read the depth frame back from the final
    // depth-only pass's target too: cheaper than gathering it from the point
    // cloud on the CPU, which only the legacy readback does.
    enable(this->dataPtr->depthOnlyPassDef,
        !merged && !Ogre2UseLegacyReadback());
    enable(this->dataPtr->xyzPassDef, !merged);
    enable(this->dataPtr->mergedPassDef, merged);
    this->dataPtr->depthMaterial->getTechnique(0)->getPass(0)
        ->getFragmentProgramParameters()->setNamedConstant(
        "depthOnly", merged ? 1.0f : 0.0f);
    if (this->dataPtr->packPassDef)
      enable(this->dataPtr->packPassDef, !depthOnly);
    this->dataPtr->renderDepthOnly = depthOnly;
    this->dataPtr->renderMerged = merged;

    // A subscriber that connected after PreRender: PreRender set up the
    // colour target as for no subscribers, without its scene passes (which
    // also keeps the particle noise's random draws as they were), and such
    // a frame used to read back the cleared target. Clear it for this point
    // cloud frame too, instead of reading back stale colour.
    if (!depthOnly && !this->dataPtr->pointsAtPreRender)
    {
      this->dataPtr->colorTargetDef->getCompositorPassesNonConst()[0]
          ->mExecutionMask = this->dataPtr->kDepthExecutionMask;
    }
  }

  // Our shaders rely on clamped values so enable it for this sensor
  //
  // TODO(anyone): Matias N. Goldberg (dark_sylinc) insists this is a hack
  // and something is wrong. We should not need depth clamp. Depth clamp is
  // just masking a bug
  const bool bOldDepthClamp = this->ogreCamera->getNeedsDepthClamp();
  this->ogreCamera->_setNeedsDepthClamp(true);

  this->scene->StartRendering(this->ogreCamera);

  // update the compositors
  this->dataPtr->ogreCompositorWorkspace->_validateFinalTarget();
  this->dataPtr->ogreCompositorWorkspace->_beginUpdate(false);
  this->dataPtr->ogreCompositorWorkspace->_update();
  this->dataPtr->ogreCompositorWorkspace->_endUpdate(false);

  Ogre::vector<Ogre::TextureGpu*>::type swappedTargets;
  swappedTargets.reserve(2u);
  this->dataPtr->ogreCompositorWorkspace->_swapFinalTarget(swappedTargets);

  this->scene->FlushGpuCommandsAndStartNewFrame(1u, false);

  this->ogreCamera->_setNeedsDepthClamp(bOldDepthClamp);
}

//////////////////////////////////////////////////
void Ogre2DepthCamera::PreRender()
{
  GZ_PROFILE("Ogre2DepthCamera::PreRender");
  if (!this->dataPtr->ogreDepthTexture[0])
    this->CreateDepthTexture();

  if (!this->dataPtr->ogreCompositorWorkspace)
    this->CreateWorkspaceInstance();

  // Colour scene passes only with rgb point cloud connections
  this->dataPtr->pointsAtPreRender =
      this->dataPtr->newRgbPointCloud.ConnectionCount() > 0u;
  const bool hasParticles =
      this->scene->OgreSceneManager()->getMovableObjectIterator(
      Ogre::ParticleSystemFactory::FACTORY_TYPE_NAME).hasMoreElements();
  // Packed xyz point clouds need no colour. With particles the colour
  // passes run anyway whenever there are point cloud subscribers: the
  // particle noise listener draws a random number in every scene pass that
  // sees particles, so skipping them would change the depth noise.
  const bool needColor = this->dataPtr->pointsAtPreRender &&
      (hasParticles || !this->dataPtr->packedPoints);
  if (this->dataPtr->colorTargetDef)
  {
    Ogre::CompositorPassDefVec &colorPasses =
        this->dataPtr->colorTargetDef->getCompositorPassesNonConst();
    GZ_ASSERT(colorPasses.size() > 2u,
        "Ogre2DepthCamera color target should contain more than 2 passes");
    GZ_ASSERT(colorPasses[0]->getType() == Ogre::PASS_CLEAR,
        "Ogre2DepthCamera color target should start with a clear pass");
    // Without them nothing reads the colour target except the legacy
    // readback, so only that needs it cleared.
    colorPasses[0]->mExecutionMask =
      (!needColor && Ogre2UseLegacyReadback()) ?
      this->dataPtr->kDepthExecutionMask : ~this->dataPtr->kDepthExecutionMask;
    for (size_t i = 1; i < colorPasses.size(); ++i)
    {
      colorPasses[i]->mExecutionMask = needColor ?
          this->dataPtr->kDepthExecutionMask :
          ~this->dataPtr->kDepthExecutionMask;
    }
  }

  // Particle scene pass only if there are particles
  if (this->dataPtr->particleTargetDef)
  {
    Ogre::CompositorPassDefVec &particlePasses =
        this->dataPtr->particleTargetDef->getCompositorPassesNonConst();
    GZ_ASSERT(particlePasses.size() == 2u,
        "Ogre2DepthCamera particle target should 2 passes");
    GZ_ASSERT(particlePasses[0]->getType() == Ogre::PASS_CLEAR,
        "Ogre2DepthCamera particle target should start with a clear pass");
    GZ_ASSERT(particlePasses[1]->getType() == Ogre::PASS_SCENE,
        "Ogre2DepthCamera particle target should end with a scene pass");
    particlePasses[1]->mExecutionMask =
      (hasParticles) ? this->dataPtr->kDepthExecutionMask :
                       ~this->dataPtr->kDepthExecutionMask;
    this->dataPtr->depthMaterial->getTechnique(0)->getPass(0)
        ->getFragmentProgramParameters()->setNamedConstant(
        "hasParticles", hasParticles ? 1.0f : 0.0f);
  }

  // update depth camera render passes
  Ogre2RenderTarget::UpdateRenderPassChain(
      this->dataPtr->ogreCompositorWorkspace,
      this->dataPtr->ogreCompositorWorkspaceDef,
      this->dataPtr->ogreCompositorBaseNodeDef,
      this->dataPtr->ogreCompositorFinalNodeDef,
      this->dataPtr->renderPasses,
      this->dataPtr->renderPassDirty,
      &this->dataPtr->ogreDepthTexture,
      false);

  {
    CameraPtr camera =
        std::dynamic_pointer_cast<Camera>(this->shared_from_this());
    for (auto &pass : this->dataPtr->renderPasses)
      pass->PreRender(camera);
  }

  // add the particle noise listener again if worksapce is recreated due to
  // dirty render pass
  if (this->dataPtr->renderPassDirty)
  {
    Ogre::CompositorNode *node =
        this->dataPtr->ogreCompositorWorkspace->getNodeSequence()[0];
    auto channelsTex = node->getLocalTextures();

    for (const auto &c : channelsTex)
    {
      if (c->getPixelFormat() == Ogre::PFG_RGB8_UNORM)
      {
        // add particle noise / scatter effects listener so we can set the
        // amount of noise based on size of emitter
        this->dataPtr->particleNoiseListener.reset(
            new Ogre2ParticleNoiseListener(this->scene,
              this->dataPtr->depthMaterial));
        this->ogreCamera->addListener(
          this->dataPtr->particleNoiseListener.get());
        break;
      }
    }
  }

  this->dataPtr->renderPassDirty = false;
}

//////////////////////////////////////////////////
void Ogre2DepthCamera::PostRender()
{
  GZ_PROFILE("Ogre2DepthCamera::PostRender");
  unsigned int width = this->ImageWidth();
  unsigned int height = this->ImageHeight();

  PixelFormat format = PF_FLOAT32_RGBA;

  int len = width * height;
  unsigned int channelCount = PixelUtil::ChannelCount(format);
  unsigned int bytesPerChannel = PixelUtil::BytesPerChannel(format);

  if (!this->dataPtr->depthBuffer)
    this->dataPtr->depthBuffer = new float[len * channelCount];
  if (!this->dataPtr->depthImage)
    this->dataPtr->depthImage = new float[len * channelCount];

  if (Ogre2UseLegacyReadback())
  {
    // Legacy A/B control: original Ogre::Image2 path, kept verbatim.
    Ogre::Image2 image;
    image.convertFromTexture(this->dataPtr->ogreDepthTexture[1], 0u, 0u);
    Ogre::TextureBox box = image.getData(0);
    float *depthBufferTmp = static_cast<float *>(box.data);

    // copy data row by row. The texture box may not be a contiguous region of
    // a texture
    for (unsigned int i = 0; i < height; ++i)
    {
      unsigned int rawDataRowIdx = i * box.bytesPerRow / bytesPerChannel;
      unsigned int rowIdx = i * width * channelCount;
      memcpy(&this->dataPtr->depthBuffer[rowIdx],
          &depthBufferTmp[rawDataRowIdx],
          width * channelCount * bytesPerChannel);
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
  }
  else
  {
    // Persistent-ticket path. Every frame reads back the R32 depth target,
    // a point cloud frame also the xyz + colour output. The depth frame goes
    // straight from the mapped memory to its subscribers when its rows are
    // contiguous; the point cloud is copied into the point cloud buffer.
    if (!this->dataPtr->readbackIssued)
      this->dataPtr->Download();
    this->dataPtr->readbackIssued = false;
    // (only the tickets this frame downloaded to: the other one may hold an
    // older frame)
    Ogre::TextureBox depthBox;
    Ogre::TextureBox pointBox;
    if (this->dataPtr->depthDownloaded)
      depthBox = this->dataPtr->depthReadback.MapDownloaded();
    if (this->dataPtr->pointsDownloaded)
      pointBox = this->dataPtr->pointReadback.MapDownloaded();
    if (!depthBox.data && !pointBox.data)
    {
      gzerr << "Ogre2DepthCamera: GPU readback failed; dropping frame"
            << std::endl;
    }

    // The point cloud buffer is also DepthData(), so it is filled before
    // either event fires.
    const unsigned int pointChannels =
        this->dataPtr->packedPoints ? 3u : channelCount;
    if (pointBox.data)
    {
      const size_t rowBytes = width * pointChannels * sizeof(float);
      char *dst = reinterpret_cast<char *>(this->dataPtr->depthBuffer);
      for (unsigned int i = 0; i < height; ++i)
      {
        memcpy(dst + i * rowBytes, static_cast<const char *>(pointBox.data) +
            i * pointBox.bytesPerRow, rowBytes);
      }
    }
    if (depthBox.data)
    {
      const float *depth = static_cast<const float *>(depthBox.data);
      if (depthBox.bytesPerRow != width * sizeof(float))
      {
        for (unsigned int i = 0; i < height; ++i)
        {
          memcpy(&this->dataPtr->depthImage[i * width],
              static_cast<const char *>(depthBox.data) +
              i * depthBox.bytesPerRow, width * sizeof(float));
        }
        depth = this->dataPtr->depthImage;
      }
      this->dataPtr->newDepthFrame(depth, width, height, 1, "FLOAT32");
    }
    if (pointBox.data &&
        this->dataPtr->newRgbPointCloud.ConnectionCount() > 0u)
    {
      this->dataPtr->newRgbPointCloud(this->dataPtr->depthBuffer, width,
          height, pointChannels,
          this->dataPtr->packedPoints ? "PF_FLOAT32_RGB" : "PF_FLOAT32_RGBA");
    }
    this->dataPtr->depthReadback.Unmap();
    this->dataPtr->pointReadback.Unmap();
    return;
  }

  this->dataPtr->newDepthFrame(
        this->dataPtr->depthImage, width, height, 1, "FLOAT32");

  // point cloud data
  if (this->dataPtr->newRgbPointCloud.ConnectionCount() > 0u)
  {
    this->dataPtr->newRgbPointCloud(
        this->dataPtr->depthBuffer, width, height, channelCount,
        "PF_FLOAT32_RGBA");
  }
}

//////////////////////////////////////////////////
bool Ogre2DepthCamera::IssueReadback()
{
  if (Ogre2UseLegacyReadback())
    return false;
  this->dataPtr->readbackIssued = this->dataPtr->Download();
  return this->dataPtr->readbackIssued;
}

//////////////////////////////////////////////////
bool Ogre2DepthCamera::SetPointCloudFormat(PixelFormat _format)
{
  // PF_FLOAT32_RGB point clouds are packed for the persistent-ticket
  // readback, which the legacy readback path doesn't use.
  if (_format == PF_FLOAT32_RGB && !Ogre2UseLegacyReadback())
  {
    this->dataPtr->pointFormat = PF_FLOAT32_RGB;
    return true;
  }
  this->dataPtr->pointFormat = PF_FLOAT32_RGBA;
  return _format == PF_FLOAT32_RGBA;
}

//////////////////////////////////////////////////
PixelFormat Ogre2DepthCamera::PointCloudFormat() const
{
  return this->dataPtr->pointFormat;
}

//////////////////////////////////////////////////
const float *Ogre2DepthCamera::DepthData() const
{
  return this->dataPtr->depthBuffer;
}

//////////////////////////////////////////////////
common::ConnectionPtr Ogre2DepthCamera::ConnectNewDepthFrame(
    std::function<void(const float *, unsigned int, unsigned int,
      unsigned int, const std::string &)>  _subscriber)
{
  return this->dataPtr->newDepthFrame.Connect(_subscriber);
}

//////////////////////////////////////////////////
common::ConnectionPtr Ogre2DepthCamera::ConnectNewRgbPointCloud(
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
  return std::min(std::max(0.001, _fov), GZ_PI * 0.999);
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

//////////////////////////////////////////////////
void Ogre2DepthCamera::SetShadowsDirty()
{
  this->SetShadowsNodeDefDirty();
}

//////////////////////////////////////////////////
void Ogre2DepthCamera::SetShadowsNodeDefDirty()
{
  if (!this->dataPtr->ogreCompositorWorkspace)
    return;

  auto engine = Ogre2RenderEngine::Instance();
  auto ogreRoot = engine->OgreRoot();
  Ogre::CompositorManager2 *ogreCompMgr = ogreRoot->getCompositorManager2();

  ogreCompMgr->removeWorkspace(this->dataPtr->ogreCompositorWorkspace);
  this->dataPtr->ogreCompositorWorkspace = nullptr;
  if (this->dataPtr->particleNoiseListener)
  {
    this->ogreCamera->removeListener(
        this->dataPtr->particleNoiseListener.get());
  }
}

//////////////////////////////////////////////////
void Ogre2DepthCamera::AddRenderPass(const RenderPassPtr &_pass)
{
  // hack: check and only allow gaussian noise for depth cameras
  // We create a new depth gaussion noise render pass object
  // (class declared in this src file) so that we can change the shader material
  // to use for applying noise to depth data.
  // The proper solution would be to either add a new DepthGaussianNoisePass
  // class or extend the Ogre2GaussianNoisePass to handle both color and
  // depth cameras
  std::shared_ptr<Ogre2GaussianNoisePass> pass =
      std::dynamic_pointer_cast<Ogre2GaussianNoisePass>(_pass);
  if (!pass)
  {
    gzerr << "Depth camera currently only supports a gaussian noise pass"
           << std::endl;
    return;
  }

  // create new depth noise pass
  std::shared_ptr<Ogre2DepthGaussianNoisePass> depthNoisePass =
    std::make_shared<Ogre2DepthGaussianNoisePass>();
  depthNoisePass->SetMean(pass->Mean());
  depthNoisePass->SetStdDev(pass->StdDev());

  this->dataPtr->renderPasses.push_back(depthNoisePass);
  this->dataPtr->renderPassDirty = true;
}

//////////////////////////////////////////////////
Ogre::Camera *Ogre2DepthCamera::OgreCamera() const
{
  return this->ogreCamera;
}
