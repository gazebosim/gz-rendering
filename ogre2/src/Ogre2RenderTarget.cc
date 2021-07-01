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

// leave this out of OgreIncludes as it conflicts with other files requiring
// gl.h
#ifdef _MSC_VER
#pragma warning(push, 0)
#endif
#include <OgreGL3PlusFBORenderTexture.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <ignition/common/Console.hh>

#include "ignition/rendering/Material.hh"

#include "ignition/rendering/ogre2/Ogre2Includes.hh"
#include "ignition/rendering/ogre2/Ogre2RenderEngine.hh"
#include "ignition/rendering/ogre2/Ogre2RenderPass.hh"
#include "ignition/rendering/ogre2/Ogre2Conversions.hh"
#include "ignition/rendering/ogre2/Ogre2Material.hh"
#include "ignition/rendering/ogre2/Ogre2RenderTarget.hh"
#include "ignition/rendering/ogre2/Ogre2Scene.hh"

namespace ignition
{
namespace rendering
{
inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {
//
/// \brief Listener for changing ogre compositor pass properties
class Ogre2RenderTargetCompositorListener :
    public Ogre::CompositorWorkspaceListener
{
  /// \brief Constructor
  /// \param[in] _target ogre render target object
  public: explicit Ogre2RenderTargetCompositorListener(
      Ogre2RenderTarget *_target)
  {
    this->ogreRenderTarget = _target;
  }

  /// \brief Destructor
  public: virtual ~Ogre2RenderTargetCompositorListener() = default;

  // Documentation inherited.
  public: virtual void passPreExecute(Ogre::CompositorPass *_pass)
  {
    if (_pass->getType() == Ogre::PASS_SCENE)
    {
      Ogre::CompositorPassScene *scenePass =
          static_cast<Ogre::CompositorPassScene *>(_pass);
      IGN_ASSERT(scenePass != nullptr, "Unable to get scene pass");
      Ogre::Viewport *vp = scenePass->getViewport();
      // make sure we do not alter the reserved visibility flags
      uint32_t f = this->ogreRenderTarget->VisibilityMask() |
          ~Ogre::VisibilityFlags::RESERVED_VISIBILITY_FLAGS;
      // apply the new visibility mask
      uint32_t flags = f & vp->getVisibilityMask();
      vp->_setVisibilityMask(flags, vp->getLightVisibilityMask());
    }
  }

  /// \brief Pointer to render target that added this listener
  private: Ogre2RenderTarget *ogreRenderTarget = nullptr;
};
}
}
}

/// \brief Private data class for Ogre2RenderTarget
class ignition::rendering::Ogre2RenderTargetPrivate
{
  /// \brief Listener for chaning compositor pass properties
  public: Ogre2RenderTargetCompositorListener *rtListener = nullptr;

  /// \brief Name of sky box material
  public: const std::string kSkyboxMaterialName = "SkyBox";

  /// \brief Name of base rendering compositor node
  public: const std::string kBaseNodeName = "PbsMaterialsRenderingNode";

  /// \brief Name of final rendering compositor node
  public: const std::string kFinalNodeName = "FinalComposition";

  /// \brief Name of shadow compositor node
  public: const std::string kShadowNodeName = "PbsMaterialsShadowNode";

  /// \brief Helper class that applies the material to the render target
  Ogre2RenderTargetMaterialPtr materialApplicator[2];

  /// \brief Pointer to the internal ogre render texture objects
  /// There's two because we ping pong postprocessing effects
  /// and the final result is always in ogreTexture[1]
  /// RenderWindows may have a 3rd texture which is the
  /// actual window
  ///
  Ogre::Texture *ogreTexture[2] = {nullptr, nullptr};
};

using namespace ignition;
using namespace rendering;

//////////////////////////////////////////////////
// Ogre2RenderTarget
//////////////////////////////////////////////////
Ogre2RenderTarget::Ogre2RenderTarget()
  : dataPtr(new Ogre2RenderTargetPrivate)
{
  this->ogreBackgroundColor = Ogre::ColourValue::Black;
  this->ogreCompositorWorkspaceDefName = "PbsMaterialsWorkspace";
}

//////////////////////////////////////////////////
Ogre2RenderTarget::~Ogre2RenderTarget()
{
  if (this->dataPtr->rtListener)
  {
    delete this->dataPtr->rtListener;
    this->dataPtr->rtListener = nullptr;
  }
}

//////////////////////////////////////////////////
void Ogre2RenderTarget::BuildCompositor()
{
  auto engine = Ogre2RenderEngine::Instance();
  auto ogreRoot = engine->OgreRoot();
  Ogre::CompositorManager2 *ogreCompMgr = ogreRoot->getCompositorManager2();

  this->UpdateBackgroundMaterial();

  bool validBackground = this->backgroundMaterial &&
      !this->backgroundMaterial->EnvironmentMap().empty();

  // The function build a similar compositor as the one defined in
  // ogre2/media/2.0/scripts/Compositors/PbsMaterials.compositor
  // but supports an extra quad pass to render the skybox cubemap if
  // sky is enabled.
  // todo(anyone) Note the definition programmatically created here
  // replaces the one defined in the script so it maybe safe to remove the
  // PbsMaterials.compositor file
  std::string wsDefName = "PbsMaterialWorkspace_" + this->Name();
  this->ogreCompositorWorkspaceDefName = wsDefName;
  if (!ogreCompMgr->hasWorkspaceDefinition(wsDefName))
  {
    // PbsMaterialsRenderingNode
    std::string nodeDefName = wsDefName + "/" + this->dataPtr->kBaseNodeName;
    Ogre::CompositorNodeDef *nodeDef =
        ogreCompMgr->addNodeDefinition(nodeDefName);

    nodeDef->addTextureSourceName(
          "rt0", 0u, Ogre::TextureDefinitionBase::TEXTURE_INPUT);
    nodeDef->addTextureSourceName(
          "rt1", 1u, Ogre::TextureDefinitionBase::TEXTURE_INPUT);

    nodeDef->setNumTargetPass(2);
    Ogre::CompositorTargetDef *rt0TargetDef =
        nodeDef->addTargetPass("rt0");

    if (validBackground)
      rt0TargetDef->setNumPasses(3);
    else
      rt0TargetDef->setNumPasses(2);
    {
      // clear pass
      Ogre::CompositorPassClearDef *passClear =
          static_cast<Ogre::CompositorPassClearDef *>(
          rt0TargetDef->addPass(Ogre::PASS_CLEAR));
      passClear->mColourValue = this->ogreBackgroundColor;

      if (validBackground)
      {
        // quad pass
        Ogre::CompositorPassQuadDef *passQuad =
            static_cast<Ogre::CompositorPassQuadDef *>(
            rt0TargetDef->addPass(Ogre::PASS_QUAD));
        passQuad->mMaterialName = this->dataPtr->kSkyboxMaterialName + "_"
            + this->Name();
        passQuad->mFrustumCorners =
            Ogre::CompositorPassQuadDef::CAMERA_DIRECTION;
      }

      // scene pass
      Ogre::CompositorPassSceneDef *passScene =
          static_cast<Ogre::CompositorPassSceneDef *>(
          rt0TargetDef->addPass(Ogre::PASS_SCENE));
      passScene->mShadowNode = this->dataPtr->kShadowNodeName;
      passScene->mIncludeOverlays = true;
    }

    nodeDef->mapOutputChannel(0, "rt0");
    nodeDef->mapOutputChannel(1, "rt1");

    // Final Composition
    std::string finalNodeDefName = wsDefName + "/" +
        this->dataPtr->kFinalNodeName;
    Ogre::CompositorNodeDef *finalNodeDef =
        ogreCompMgr->addNodeDefinition(finalNodeDefName);
    finalNodeDef->addTextureSourceName("rtN", 0,
        Ogre::TextureDefinitionBase::TEXTURE_INPUT);
    finalNodeDef->addTextureSourceName("rt_output", 1,
        Ogre::TextureDefinitionBase::TEXTURE_INPUT);

    finalNodeDef->setNumTargetPass(2);
    Ogre::CompositorTargetDef *outTargetDef =
        finalNodeDef->addTargetPass("rt_output");
    outTargetDef->setNumPasses(2);
    {
      // quad pass
      Ogre::CompositorPassQuadDef *passQuad =
          static_cast<Ogre::CompositorPassQuadDef *>(
          outTargetDef->addPass(Ogre::PASS_QUAD));
      passQuad->mMaterialName = "Ogre/Copy/4xFP32";
      passQuad->addQuadTextureSource(0, "rtN", 0);

      // scene pass
      Ogre::CompositorPassSceneDef *passScene =
          static_cast<Ogre::CompositorPassSceneDef *>(
          outTargetDef->addPass(Ogre::PASS_SCENE));
      passScene->mUpdateLodLists = false;
      passScene->mIncludeOverlays = true;
      passScene->mFirstRQ = 254;
      passScene->mLastRQ = 255;

    }
    Ogre::CompositorWorkspaceDef *workDef =
        ogreCompMgr->addWorkspaceDefinition(wsDefName);

    workDef->connectExternal(0, nodeDefName, 0);
    workDef->connectExternal(1, nodeDefName, 1);

    if (!this->IsRenderWindow())
    {
      workDef->connect(nodeDefName, finalNodeDefName);
    }
    else
    {
      // connect the last render pass to the final compositor node
      // but only input, since output goes to the render window
      workDef->connect(nodeDefName, 0, finalNodeDefName, 0);
      workDef->connectExternal(2,  finalNodeDefName, 1);
    }
  }

  auto &manager = Ogre::TextureManager::getSingleton();
  Ogre::CompositorChannelVec externalTargets(2u);
  for( size_t i = 0u; i < 2u; ++i )
  {
    // Connect them in reverse order
    const size_t srcIdx = 2u - i - 1u;
    externalTargets[i].target =
        this->dataPtr->ogreTexture[srcIdx]->getBuffer()->getRenderTarget();
    externalTargets[i].textures.push_back(
          manager.getByName(this->dataPtr->ogreTexture[srcIdx]->getName()));
  }

  this->SyncOgreTextureVars();

  this->ogreCompositorWorkspace =
      ogreCompMgr->addWorkspace(this->scene->OgreSceneManager(),
      externalTargets, this->ogreCamera,
      this->ogreCompositorWorkspaceDefName, false);

  this->dataPtr->rtListener = new Ogre2RenderTargetCompositorListener(this);
  this->ogreCompositorWorkspace->setListener(this->dataPtr->rtListener);
}

//////////////////////////////////////////////////
void Ogre2RenderTarget::DestroyCompositor()
{
  if (!this->ogreCompositorWorkspace)
    return;

  // Restore the original order so that this->ogreTexture[1] is the one with
  // FSAA (which we need for BuildCompositor to connect correctly)
  const Ogre::CompositorChannelVec &externalTargets =
      this->ogreCompositorWorkspace->getExternalRenderTargets();
  for( size_t i = 0u; i < 2u; ++i )
  {
    const size_t srcIdx = (2u - i - 1u);
    this->dataPtr->ogreTexture[srcIdx] =
        externalTargets[i].textures.front().get();
  }
  this->SyncOgreTextureVars();

  if (this->dataPtr->materialApplicator[0] &&
      this->dataPtr->materialApplicator[0]->IsSameRenderTarget(
        this->dataPtr->ogreTexture[0]->getBuffer()->getRenderTarget()))
  {
    std::swap( this->dataPtr->materialApplicator[0],
               this->dataPtr->materialApplicator[1] );
  }
  this->materialApplicator = this->dataPtr->materialApplicator[0];

  auto engine = Ogre2RenderEngine::Instance();
  auto ogreRoot = engine->OgreRoot();
  Ogre::CompositorManager2 *ogreCompMgr = ogreRoot->getCompositorManager2();
  this->ogreCompositorWorkspace->setListener(nullptr);
  ogreCompMgr->removeWorkspace(this->ogreCompositorWorkspace);
  ogreCompMgr->removeWorkspaceDefinition(this->ogreCompositorWorkspaceDefName);
  ogreCompMgr->removeNodeDefinition(this->ogreCompositorWorkspaceDefName +
      "/" + this->dataPtr->kBaseNodeName);
  ogreCompMgr->removeNodeDefinition(this->ogreCompositorWorkspaceDefName +
      "/" + this->dataPtr->kFinalNodeName);

  this->ogreCompositorWorkspace = nullptr;
  delete this->dataPtr->rtListener;
  this->dataPtr->rtListener = nullptr;
}

//////////////////////////////////////////////////
void Ogre2RenderTarget::RebuildCompositor()
{
  this->DestroyCompositor();
  this->BuildCompositor();
}

//////////////////////////////////////////////////
void Ogre2RenderTarget::Copy(Image &_image) const
{
  // TODO(anyone) handle Bayer conversions
  // TODO(anyone) handle ogre version differences

  if (_image.Width() != this->width || _image.Height() != this->height)
  {
    ignerr << "Invalid image dimensions" << std::endl;
    return;
  }

  void *data = _image.Data();
  Ogre::PixelFormat imageFormat = Ogre2Conversions::Convert(_image.Format());
  Ogre::PixelBox ogrePixelBox(this->width, this->height, 1, imageFormat, data);
  this->RenderTarget()->copyContentsToMemory(ogrePixelBox);
}

//////////////////////////////////////////////////
Ogre::Camera *Ogre2RenderTarget::Camera() const
{
  return this->ogreCamera;
}

//////////////////////////////////////////////////
void Ogre2RenderTarget::SetCamera(Ogre::Camera *_camera)
{
  this->ogreCamera = _camera;
  this->targetDirty = true;
}

//////////////////////////////////////////////////
math::Color Ogre2RenderTarget::BackgroundColor() const
{
  return Ogre2Conversions::Convert(this->ogreBackgroundColor);
}

//////////////////////////////////////////////////
void Ogre2RenderTarget::SetBackgroundColor(math::Color _color)
{
  this->ogreBackgroundColor = Ogre2Conversions::Convert(_color);
  this->colorDirty = true;
}

//////////////////////////////////////////////////
void Ogre2RenderTarget::SetBackgroundMaterial(MaterialPtr _material)
{
  this->backgroundMaterial = _material;
  this->backgroundMaterialDirty = true;
  this->targetDirty = true;
}

//////////////////////////////////////////////////
MaterialPtr Ogre2RenderTarget::BackgroundMaterial() const
{
  return this->backgroundMaterial;
}

//////////////////////////////////////////////////
unsigned int Ogre2RenderTarget::AntiAliasing() const
{
  return this->antiAliasing;
}

//////////////////////////////////////////////////
void Ogre2RenderTarget::SetAntiAliasing(unsigned int _aa)
{
  this->antiAliasing = _aa;
  this->targetDirty = true;
}

//////////////////////////////////////////////////
void Ogre2RenderTarget::PreRender()
{
  BaseRenderTarget::PreRender();
  this->UpdateBackgroundColor();

  if (this->material)
  {
    this->material->PreRender();
  }

  this->UpdateRenderPassChain();
}

//////////////////////////////////////////////////
void Ogre2RenderTarget::PostRender()
{
  // do nothing by default
}

//////////////////////////////////////////////////
void Ogre2RenderTarget::Render()
{
  // TODO(anyone)
  // There is current not an easy solution to manually updating
  // render textures:
  // https://forums.ogre3d.org/viewtopic.php?t=84687
  this->ogreCompositorWorkspace->setEnabled(true);
  auto engine = Ogre2RenderEngine::Instance();
  engine->OgreRoot()->renderOneFrame();
  this->ogreCompositorWorkspace->setEnabled(false);

  // The code below for manual updating render textures was suggested in ogre
  // forum but it does not seem to work
  // this->scene->OgreSceneManager()->updateSceneGraph();
  // this->ogreCompositorWorkspace->_validateFinalTarget();
  // engine->OgreRoot()->getRenderSystem()->_beginFrameOnce();
  // this->ogreCompositorWorkspace->_beginUpdate(false);
  // this->ogreCompositorWorkspace->_update();
  // this->ogreCompositorWorkspace->_endUpdate(false);

  // this->scene->OgreSceneManager()->_frameEnded();
  // for (size_t i=0; i < Ogre::HLMS_MAX; ++i)
  // {
  //   Ogre::Hlms *hlms = engine->OgreRoot()->getHlmsManager()->getHlms(
  //       static_cast<Ogre::HlmsTypes>(i));
  //   if(hlms)
  //     hlms->frameEnded();
  // }
  // engine->OgreRoot()->getRenderSystem()->_update();
}

//////////////////////////////////////////////////
bool Ogre2RenderTarget::IsRenderWindow() const
{
  const Ogre2RenderWindow *asWindow =
      dynamic_cast<const Ogre2RenderWindow*>(this);
  if (asWindow)
    return true;

  return false;
}

//////////////////////////////////////////////////
void Ogre2RenderTarget::DestroyTargetImpl()
{
  if (nullptr == this->dataPtr->ogreTexture[0])
    return;

  this->DestroyCompositor();

  auto &manager = Ogre::TextureManager::getSingleton();

  this->materialApplicator.reset();

  for( size_t i = 0u; i < 2u; ++i )
  {
    manager.unload(this->dataPtr->ogreTexture[i]->getName());
    manager.remove(this->dataPtr->ogreTexture[i]->getName());

    // TODO(anyone) there is memory leak when a render texture is destroyed.
    // The RenderSystem::_cleanupDepthBuffers method used in ogre1 does not
    // seem to work in ogre2

    this->dataPtr->materialApplicator[i].reset();
    this->dataPtr->ogreTexture[i] = nullptr;
  }

  this->SyncOgreTextureVars();
}

//////////////////////////////////////////////////
void Ogre2RenderTarget::BuildTargetImpl()
{
  Ogre::TextureManager &manager = Ogre::TextureManager::getSingleton();
  Ogre::PixelFormat ogreFormat = Ogre2Conversions::Convert(this->format);

  // check if target fsaa is supported
  unsigned int fsaa = 0;
  std::vector<unsigned int> fsaaLevels =
      Ogre2RenderEngine::Instance()->FSAALevels();
  unsigned int targetFSAA = this->antiAliasing;
  auto const it = std::find(fsaaLevels.begin(), fsaaLevels.end(), targetFSAA);
  if (it != fsaaLevels.end())
  {
    fsaa = targetFSAA;
  }
  else
  {
    // output warning but only do it once
    static bool ogre2FSAAWarn = false;
    if (ogre2FSAAWarn)
    {
      ignwarn << "Anti-aliasing level of '" << this->antiAliasing << "' "
              << "is not supported. Setting to 0" << std::endl;
      ogre2FSAAWarn = true;
    }
  }

  for( size_t i = 0u; i < 2u; ++i )
  {
    // Ogre 2 PBS expects gamma correction to be enabled
    // Only the second target uses FSAA.
    // Note: It's not guaranteed the 2nd target will remain
    // the one using FSAA
    this->dataPtr->ogreTexture[i] = (manager.createManual(
        this->name + std::to_string(i), "General",
        Ogre::TEX_TYPE_2D, this->width, this->height, 0, ogreFormat,
        Ogre::TU_RENDERTARGET, 0, true, i == 1u ? fsaa : 0)).get();
  }

  this->SyncOgreTextureVars();
}

//////////////////////////////////////////////////
unsigned int Ogre2RenderTarget::GLIdImpl() const
{
  if (!this->dataPtr->ogreTexture[0])
    return 0;

  GLuint texId;
  this->dataPtr->ogreTexture[1]->getCustomAttribute("GLID", &texId);

  return static_cast<unsigned int>(texId);
}

//////////////////////////////////////////////////
Ogre::RenderTarget *Ogre2RenderTarget::RenderTargetImpl() const
{
  return this->dataPtr->ogreTexture[1]->getBuffer()->getRenderTarget();
}

//////////////////////////////////////////////////
uint32_t Ogre2RenderTarget::VisibilityMask() const
{
  return this->visibilityMask;
}

//////////////////////////////////////////////////
void Ogre2RenderTarget::SetVisibilityMask(uint32_t _mask)
{
  this->visibilityMask = _mask;
}

//////////////////////////////////////////////////
void Ogre2RenderTarget::UpdateBackgroundColor()
{
  if (this->colorDirty)
  {
    // set background color in compositor clear pass def
    auto nodeSeq = this->ogreCompositorWorkspace->getNodeSequence();
    auto pass = nodeSeq[0]->_getPasses()[0]->getDefinition();
    auto clearPass = dynamic_cast<const Ogre::CompositorPassClearDef *>(pass);
    const_cast<Ogre::CompositorPassClearDef *>(clearPass)->mColourValue =
        this->ogreBackgroundColor;

    this->colorDirty = false;
  }
}

//////////////////////////////////////////////////
void Ogre2RenderTarget::UpdateBackgroundMaterial()
{
  if (!this->backgroundMaterialDirty)
    return;

  bool validBackground = this->backgroundMaterial &&
      !this->backgroundMaterial->EnvironmentMap().empty();

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
        ignerr << "Unable to find skybox material" << std::endl;
        return;
      }
      mat = skyboxMat->clone(skyMatName);
    }
    Ogre::TextureUnitState *texUnit =
        mat->getTechnique(0u)->getPass(0u)->getTextureUnitState(0u);
    texUnit->setTextureName(this->backgroundMaterial->EnvironmentMap(),
        Ogre::TEX_TYPE_CUBE_MAP);
  }

  this->backgroundMaterialDirty = false;
}

//////////////////////////////////////////////////
void Ogre2RenderTarget::UpdateRenderPassChain()
{
  UpdateRenderPassChain(this->ogreCompositorWorkspace,
      this->ogreCompositorWorkspaceDefName,
      this->ogreCompositorWorkspaceDefName + "/" +
      this->dataPtr->kBaseNodeName,
      this->ogreCompositorWorkspaceDefName + "/" +
      this->dataPtr->kFinalNodeName,
      this->renderPasses,
      this->renderPassDirty,
      &this->dataPtr->ogreTexture,
      this->IsRenderWindow());

  // this->dataPtr->ogreTexture[0] may have changed
  if (this->dataPtr->materialApplicator[0] &&
      this->dataPtr->materialApplicator[0]->IsSameRenderTarget(
        this->dataPtr->ogreTexture[0]->getBuffer()->getRenderTarget()))
  {
    std::swap( this->dataPtr->materialApplicator[0],
               this->dataPtr->materialApplicator[1] );
  }
  this->materialApplicator = this->dataPtr->materialApplicator[0];

  this->SyncOgreTextureVars();

  this->renderPassDirty = false;
}

//////////////////////////////////////////////////
void Ogre2RenderTarget::UpdateRenderPassChain(
    Ogre::CompositorWorkspace * /*_workspace*/,
    const std::string & /*_workspaceDefName*/,
    const std::string & /*_baseNode*/, const std::string & /*_finalNode*/,
    const std::vector<RenderPassPtr> & /*_renderPasses*/,
    bool /*_recreateNodes*/)
{
  ignwarn << "Warning: This Ogre2RenderTarget::UpdateRenderPassChain "
          << "overload is deprecated" << std::endl;
}

//////////////////////////////////////////////////
void Ogre2RenderTarget::UpdateRenderPassChain(
    Ogre::CompositorWorkspace *_workspace, const std::string &_workspaceDefName,
    const std::string &_baseNode, const std::string &_finalNode,
    const std::vector<RenderPassPtr> &_renderPasses,
    bool _recreateNodes, Ogre::Texture *(*_ogreTextures)[2],
    bool _isRenderWindow)
{
  if (!_workspace || _workspaceDefName.empty() ||
      _baseNode.empty() || _finalNode.empty() || _renderPasses.empty())
    return;

  // check pass enabled state and update connections if necessary.
  // If render pass is dirty then skip the enabled state check since the whole
  // workspace nodes and connections will be recreated
  bool updateConnection = false;
  if (!_recreateNodes)
  {
    auto nodeSeq = _workspace->getNodeSequence();

    // set node instance to render pass and update enabled state
    for (const auto &pass : _renderPasses)
    {
      Ogre2RenderPass *ogre2RenderPass =
          dynamic_cast<Ogre2RenderPass *>(pass.get());
      Ogre::CompositorNode *node =
          _workspace->findNodeNoThrow(
          ogre2RenderPass->OgreCompositorNodeDefinitionName());

      // check if we need to create all nodes or just update the connections.
      // if node does not exist then it means it either has not been added to
      // the chain yet or it was removed because it was disabled.
      // In both cases, we need to recreate the nodes and connections
      if (!node && ogre2RenderPass->IsEnabled())
      {
        _recreateNodes = true;
      }
      else if (node && node->getEnabled() != ogre2RenderPass->IsEnabled())
      {
        node->setEnabled(ogre2RenderPass->IsEnabled());
        updateConnection = true;
      }
    }
  }

  if (!_recreateNodes && !updateConnection)
    return;

  auto engine = Ogre2RenderEngine::Instance();
  auto ogreRoot = engine->OgreRoot();
  Ogre::CompositorManager2 *ogreCompMgr = ogreRoot->getCompositorManager2();

  Ogre::CompositorWorkspaceDef *workspaceDef =
    ogreCompMgr->getWorkspaceDefinition(_workspaceDefName);

  auto nodeSeq = _workspace->getNodeSequence();

  // The first node and final node in the workspace are defined in
  // PbsMaterials.compositor
  // the first node is the base scene pass node
  std::string outNodeDefName = _baseNode;
  // the final compositor node
  const std::string finalNodeDefName = _finalNode;
  std::string inNodeDefName;

  // if new nodes need to be added then clear everything,
  // otherwise clear only the node connections
  if (_recreateNodes)
    workspaceDef->clearAll();
  else
    workspaceDef->clearAllInterNodeConnections();

  int numActiveNodes = 0;

  // chain the render passes by connecting all the ogre compositor nodes
  // in between the base scene pass node and the final compositor node
  for (const auto &pass : _renderPasses)
  {
    Ogre2RenderPass *ogre2RenderPass =
        dynamic_cast<Ogre2RenderPass *>(pass.get());
    ogre2RenderPass->CreateRenderPass();
    inNodeDefName = ogre2RenderPass->OgreCompositorNodeDefinitionName();
    // only connect passes that are enabled
    if (!inNodeDefName.empty() && ogre2RenderPass->IsEnabled())
    {
      workspaceDef->connect(outNodeDefName, inNodeDefName);
      outNodeDefName = inNodeDefName;
      ++numActiveNodes;
    }
  }

  workspaceDef->connectExternal(0, _baseNode, 0);
  workspaceDef->connectExternal(1, _baseNode, 1);

  if (!_isRenderWindow)
  {
    // connect the last render pass to the final compositor node
    workspaceDef->connect(outNodeDefName, finalNodeDefName);

    // We must ensure the output is always in ogreTextures[1]
    const bool bMustSwapRts = (numActiveNodes & 0x01) == 0u;

    const Ogre::CompositorChannelVec &externalTargets =
        _workspace->getExternalRenderTargets();
    for( size_t i = 0u; i < 2u; ++i )
    {
      const size_t srcIdx = bMustSwapRts ? (2u - i - 1u) : i;
      (*_ogreTextures)[srcIdx] = externalTargets[i].textures.front().get();
    }
  }
  else
  {
    // connect the last render pass to the final compositor node
    // but only input, since output goes to the render window
    workspaceDef->connect(outNodeDefName, 0, finalNodeDefName, 0);
    workspaceDef->connectExternal(2, _finalNode, 1);
  }


  // if new node definitions were added then recreate all the compositor nodes,
  // otherwise update the connections
  if (_recreateNodes)
  {
    // clearAll requires the output to be connected again.
    _workspace->recreateAllNodes();
  }
  else
  {
    _workspace->reconnectAllNodes();
  }
}

//////////////////////////////////////////////////
void Ogre2RenderTarget::UpdateShadowNode()
{
  ignwarn << "Ogre2RenderTarget::UpdateShadowNode() is deprecated and "
             "replaced by Ogre2Scene:UpdateShadowNode()"
          << std::endl;
}

//////////////////////////////////////////////////
void Ogre2RenderTarget::RebuildImpl()
{
  this->RebuildTarget();
  this->RebuildMaterial();
  this->RebuildCompositor();
}

//////////////////////////////////////////////////
void Ogre2RenderTarget::SetMaterial(MaterialPtr _material)
{
  this->material = _material;

  // Have to rebuild the target so there is something to apply the applicator to
  this->targetDirty = true;
}

//////////////////////////////////////////////////
void Ogre2RenderTarget::SetShadowsNodeDefDirty()
{
  this->DestroyCompositor();

  // Have to rebuild the target so there is something to apply the applicator to
  this->targetDirty = true;
}

//////////////////////////////////////////////////
void Ogre2RenderTarget::RebuildMaterial()
{
  if (this->material)
  {
    Ogre2Material *ogreMaterial = dynamic_cast<Ogre2Material *>(
        this->material.get());
    Ogre::MaterialPtr matPtr = ogreMaterial->Material();

    Ogre::SceneManager *sceneMgr = this->scene->OgreSceneManager();
    for( size_t i = 0u; i < 2u; ++i )
    {
      Ogre::RenderTarget *target =
          this->dataPtr->ogreTexture[i]->getBuffer()->getRenderTarget();
      this->dataPtr->materialApplicator[i].reset(
            new Ogre2RenderTargetMaterial(sceneMgr, target, matPtr.get()));
    }

    this->materialApplicator = this->dataPtr->materialApplicator[0];
  }
}

//////////////////////////////////////////////////
void Ogre2RenderTarget::SyncOgreTextureVars()
{
  Ogre2RenderTexture *asRenderTexture =
      dynamic_cast<Ogre2RenderTexture*>(this);
  if (asRenderTexture)
   asRenderTexture->SetOgreTexture(this->dataPtr->ogreTexture[1]);
}

//////////////////////////////////////////////////
// Ogre2RenderTexture
//////////////////////////////////////////////////
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
Ogre2RenderTexture::Ogre2RenderTexture()
{
}
#pragma GCC diagnostic pop

//////////////////////////////////////////////////
Ogre2RenderTexture::~Ogre2RenderTexture()
{
}

//////////////////////////////////////////////////
void Ogre2RenderTexture::Destroy()
{
  this->DestroyTarget();
}

//////////////////////////////////////////////////
void Ogre2RenderTexture::RebuildTarget()
{
  this->DestroyTarget();
  this->BuildTarget();
}

//////////////////////////////////////////////////
void Ogre2RenderTexture::DestroyTarget()
{
  Ogre2RenderTarget::DestroyTargetImpl();
}

//////////////////////////////////////////////////
void Ogre2RenderTexture::BuildTarget()
{
  Ogre2RenderTarget::BuildTargetImpl();
}

//////////////////////////////////////////////////
unsigned int Ogre2RenderTexture::GLId() const
{
  return Ogre2RenderTarget::GLIdImpl();
}

//////////////////////////////////////////////////
void Ogre2RenderTexture::PreRender()
{
  Ogre2RenderTarget::PreRender();
}

//////////////////////////////////////////////////
void Ogre2RenderTexture::PostRender()
{
  Ogre2RenderTarget::PostRender();
}

//////////////////////////////////////////////////
Ogre::RenderTarget *Ogre2RenderTexture::RenderTarget() const
{
  return Ogre2RenderTarget::RenderTargetImpl();
}

//////////////////////////////////////////////////
void Ogre2RenderTexture::SetOgreTexture(Ogre::Texture *_ogreTexture)
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
  this->ogreTexture = _ogreTexture;
#pragma GCC diagnostic pop
}

//////////////////////////////////////////////////
// Ogre2RenderWindow
//////////////////////////////////////////////////
Ogre2RenderWindow::Ogre2RenderWindow()
{
}

//////////////////////////////////////////////////
Ogre2RenderWindow::~Ogre2RenderWindow()
{
}

//////////////////////////////////////////////////
bool Ogre2RenderWindow::IsRenderWindow() const
{
  return true;
}

//////////////////////////////////////////////////
Ogre::RenderTarget *Ogre2RenderWindow::RenderTarget() const
{
  return this->ogreRenderWindow;
}

//////////////////////////////////////////////////
void Ogre2RenderWindow::Destroy()
{
  // TODO(anyone)
}

//////////////////////////////////////////////////
void Ogre2RenderWindow::RebuildTarget()
{
  // TODO(anyone): determine when to rebuild
  // ie. only when ratio or handle changes!
  // e.g. sizeDirty?
  if (!this->ogreRenderWindow)
    this->BuildTarget();

  Ogre::RenderWindow *window =
      dynamic_cast<Ogre::RenderWindow *>(this->ogreRenderWindow);
  window->resize(this->width, this->height);
  window->windowMovedOrResized();
}

//////////////////////////////////////////////////
void Ogre2RenderWindow::BuildTarget()
{
  auto engine = Ogre2RenderEngine::Instance();
  std::string renderTargetName =
      engine->CreateRenderWindow(this->handle,
          this->width,
          this->height,
          this->ratio,
          this->antiAliasing);
  this->ogreRenderWindow =
      engine->OgreRoot()->getRenderTarget(renderTargetName);
}
