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
      uint32_t f = ogreRenderTarget->VisibilityMask() |
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
  this->UpdateShadowNode();

  auto engine = Ogre2RenderEngine::Instance();
  auto ogreRoot = engine->OgreRoot();
  Ogre::CompositorManager2 *ogreCompMgr = ogreRoot->getCompositorManager2();

  this->ogreCompositorWorkspace =
      ogreCompMgr->addWorkspace(this->scene->OgreSceneManager(),
      this->RenderTarget(), this->ogreCamera,
      this->ogreCompositorWorkspaceDefName, false);

  this->dataPtr->rtListener = new Ogre2RenderTargetCompositorListener(this);
  this->ogreCompositorWorkspace->setListener(this->dataPtr->rtListener);
}

//////////////////////////////////////////////////
void Ogre2RenderTarget::DestroyCompositor()
{
  if (!this->ogreCompositorWorkspace)
    return;

  auto engine = Ogre2RenderEngine::Instance();
  auto ogreRoot = engine->OgreRoot();
  Ogre::CompositorManager2 *ogreCompMgr = ogreRoot->getCompositorManager2();
  this->ogreCompositorWorkspace->setListener(nullptr);
  ogreCompMgr->removeWorkspace(this->ogreCompositorWorkspace);
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
void Ogre2RenderTarget::UpdateRenderPassChain()
{
  UpdateRenderPassChain(this->ogreCompositorWorkspace,
      this->ogreCompositorWorkspaceDefName,
      "PbsMaterialsRenderingNode", "FinalComposition",
      this->renderPasses, this->renderPassDirty);

  this->renderPassDirty = false;
}

//////////////////////////////////////////////////
void Ogre2RenderTarget::UpdateRenderPassChain(
    Ogre::CompositorWorkspace *_workspace, const std::string &_workspaceDefName,
    const std::string &_baseNode, const std::string &_finalNode,
    const std::vector<RenderPassPtr> &_renderPasses,
    bool _recreateNodes)
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
      // if node does not exist then it means it has not been added to the
      // chain yet, in which case, we need to recreate the nodes and
      // connections
      if (!node)
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
  std::string finalNodeDefName = _finalNode;
  std::string inNodeDefName;

  // if new nodes need to be added then clear everything,
  // otherwise clear only the node connections
  if (_recreateNodes)
    workspaceDef->clearAll();
  else
    workspaceDef->clearAllInterNodeConnections();

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
    }
  }

  // connect the last render pass to the final compositor node
  workspaceDef->connect(outNodeDefName, 0,  finalNodeDefName, 1);

  // if new node definitions were added then recreate all the compositor nodes,
  // otherwise update the connections
  if (_recreateNodes)
  {
    // clearAll requires the output to be connected again.
    workspaceDef->connectExternal(0, finalNodeDefName, 0);
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
  if (!this->scene->ShadowsDirty())
    return;

  unsigned int spotPointLightCount = 0;
  unsigned int dirLightCount = 0;

  for (unsigned int i = 0; i < this->scene->LightCount(); ++i)
  {
    LightPtr light = this->scene->LightByIndex(i);
    if (light->CastShadows())
    {
      if (std::dynamic_pointer_cast<DirectionalLight>(light))
        dirLightCount++;
      else
        spotPointLightCount++;
    }
  }

  // limit number of shadow maps
  // shaders dynamically generated by ogre produce compile error at runtime if
  // the number of shadow maps exceeds certain number. The error seems to
  // suggest that the number of uniform variables has exceeded the max number
  // allowed
  unsigned int maxShadowMaps = 25u;
  if (dirLightCount * 3 + spotPointLightCount > maxShadowMaps)
  {
    dirLightCount = std::min(static_cast<unsigned int>(maxShadowMaps / 3),
        dirLightCount);
    spotPointLightCount = std::min(
        std::max(maxShadowMaps - dirLightCount * 3, 0u), spotPointLightCount);
    ignwarn << "Number of shadow-casting lights exceeds the limit supported by "
            << "the underlying rendering engine ogre2. Limiting to "
            << dirLightCount << " directional lights and "
            << spotPointLightCount << " point / spot lights" << std::endl;
  }

  auto engine = Ogre2RenderEngine::Instance();
  Ogre::CompositorManager2 *compositorManager =
      engine->OgreRoot()->getCompositorManager2();

  Ogre::ShadowNodeHelper::ShadowParamVec shadowParams;
  Ogre::ShadowNodeHelper::ShadowParam shadowParam;

  // directional lights
  unsigned int atlasId = 0u;
  unsigned int texSize = 2048u;
  unsigned int halfTexSize = texSize * 0.5;
  for (unsigned int i = 0; i < dirLightCount; ++i)
  {
    shadowParam.technique = Ogre::SHADOWMAP_PSSM;
    shadowParam.atlasId = atlasId;
    shadowParam.numPssmSplits = 3u;
    shadowParam.resolution[0].x = texSize;
    shadowParam.resolution[0].y = texSize;
    shadowParam.resolution[1].x = halfTexSize;
    shadowParam.resolution[1].y = halfTexSize;
    shadowParam.resolution[2].x = halfTexSize;
    shadowParam.resolution[2].y = halfTexSize;
    shadowParam.atlasStart[0].x = 0u;
    shadowParam.atlasStart[0].y = 0u;
    shadowParam.atlasStart[1].x = 0u;
    shadowParam.atlasStart[1].y = texSize;
    shadowParam.atlasStart[2].x = halfTexSize;
    shadowParam.atlasStart[2].y = texSize;
    shadowParam.supportedLightTypes = 0u;
    shadowParam.addLightType(Ogre::Light::LT_DIRECTIONAL);
    shadowParams.push_back(shadowParam);
    atlasId++;
  }

  // others
  unsigned int maxTexSize = 8192u;
  unsigned int rowIdx = 0;
  unsigned int colIdx = 0;
  unsigned int rowSize = maxTexSize / texSize;
  unsigned int colSize = rowSize;

  for (unsigned int i = 0; i < spotPointLightCount; ++i)
  {
    shadowParam.technique = Ogre::SHADOWMAP_FOCUSED;
    shadowParam.atlasId = atlasId;
    shadowParam.resolution[0].x = texSize;
    shadowParam.resolution[0].y = texSize;
    shadowParam.atlasStart[0].x = colIdx * texSize;
    shadowParam.atlasStart[0].y = rowIdx * texSize;

    shadowParam.supportedLightTypes = 0u;
    shadowParam.addLightType(Ogre::Light::LT_DIRECTIONAL);
    shadowParam.addLightType(Ogre::Light::LT_POINT);
    shadowParam.addLightType(Ogre::Light::LT_SPOTLIGHT);
    shadowParams.push_back(shadowParam);

    colIdx++;
    colIdx = colIdx % colSize;
    if (colIdx == 0u)
      rowIdx++;

    // check if we've filled the current texture atlas
    // if so, increment atlas id to indicate we want a new texture
    if (rowIdx >= rowSize)
    {
      atlasId++;
      colIdx = 0;
      rowIdx = 0;
    }
  }

  std::string shadowNodeDefName = "PbsMaterialsShadowNode";
  if (compositorManager->hasShadowNodeDefinition(shadowNodeDefName))
    compositorManager->removeShadowNodeDefinition(shadowNodeDefName);

  this->CreateShadowNodeWithSettings(compositorManager, shadowNodeDefName,
      shadowParams);

  this->scene->SetShadowsDirty(false);
}

////////////////////////////////////////////////////
void Ogre2RenderTarget::CreateShadowNodeWithSettings(
    Ogre::CompositorManager2 *_compositorManager,
    const std::string &_shadowNodeName,
    const Ogre::ShadowNodeHelper::ShadowParamVec &_shadowParams)
{
  Ogre::uint32 pointLightCubemapResolution = 1024u;
  Ogre::Real pssmLambda = 0.95f;
  Ogre::Real splitPadding = 1.0f;
  Ogre::Real splitBlend = 0.125f;
  Ogre::Real splitFade = 0.313f;

  const Ogre::uint32 spotMask           = 1u << Ogre::Light::LT_SPOTLIGHT;
  const Ogre::uint32 directionalMask    = 1u << Ogre::Light::LT_DIRECTIONAL;
  const Ogre::uint32 pointMask          = 1u << Ogre::Light::LT_POINT;
  const Ogre::uint32 spotAndDirMask = spotMask | directionalMask;

  typedef Ogre::vector<Ogre::ShadowNodeHelper::Resolution>::type ResolutionVec;

  size_t numExtraShadowMapsForPssmSplits = 0;
  size_t numTargetPasses = 0;
  ResolutionVec atlasResolutions;

  // Validation and data gathering
  bool hasPointLights = false;

  Ogre::ShadowNodeHelper::ShadowParamVec::const_iterator itor =
      _shadowParams.begin();
  Ogre::ShadowNodeHelper::ShadowParamVec::const_iterator end =
      _shadowParams.end();

  while (itor != end)
  {
    if (itor->technique == Ogre::SHADOWMAP_PSSM)
    {
      numExtraShadowMapsForPssmSplits = itor->numPssmSplits - 1u;
      // 1 per PSSM split
      numTargetPasses += numExtraShadowMapsForPssmSplits + 1u;
    }

    if (itor->atlasId >= atlasResolutions.size())
      atlasResolutions.resize(itor->atlasId + 1u);

    Ogre::ShadowNodeHelper::Resolution &resolution =
        atlasResolutions[itor->atlasId];

    const size_t numSplits = itor->technique == Ogre::SHADOWMAP_PSSM ?
        itor->numPssmSplits : 1u;
    for (size_t i = 0; i < numSplits; ++i)
    {
      resolution.x = std::max(resolution.x,
          itor->atlasStart[i].x + itor->resolution[i].x);
      resolution.y = std::max(resolution.y,
          itor->atlasStart[i].y + itor->resolution[i].y);
    }

    if (itor->supportedLightTypes & pointMask)
    {
      hasPointLights = true;
      // 6 target passes per cubemap + 1 for copy
      numTargetPasses += 7u;
    }
    if (itor->supportedLightTypes & spotAndDirMask &&
        itor->technique != Ogre::SHADOWMAP_PSSM)
    {
      // 1 per directional/spot light (for non-PSSM techniques)
      numTargetPasses += 1u;
    }
    ++itor;
  }

  // One clear for each atlas
  numTargetPasses += atlasResolutions.size();
  // Create the shadow node definition
  Ogre::CompositorShadowNodeDef *shadowNodeDef =
      _compositorManager->addShadowNodeDefinition(_shadowNodeName);

  const size_t numTextures = atlasResolutions.size();
  {
    // Define the atlases (textures)
    shadowNodeDef->setNumLocalTextureDefinitions(
        numTextures + (hasPointLights ? 1u : 0u));
    for (size_t i = 0; i < numTextures; ++i)
    {
      const Ogre::ShadowNodeHelper::Resolution &atlasRes = atlasResolutions[i];
      Ogre::TextureDefinitionBase::TextureDefinition *texDef =
          shadowNodeDef->addTextureDefinition(
          "atlas" + Ogre::StringConverter::toString(i));

      texDef->width = std::max(atlasRes.x, 1u);
      texDef->height = std::max(atlasRes.y, 1u);
      texDef->formatList.push_back(Ogre::PF_D32_FLOAT);
      texDef->depthBufferId = Ogre::DepthBuffer::POOL_NON_SHAREABLE;
      texDef->depthBufferFormat = Ogre::PF_D32_FLOAT;
      texDef->preferDepthTexture = false;
      texDef->fsaa = false;
    }

    // Define the cubemap needed by point lights
    if (hasPointLights)
    {
      Ogre::TextureDefinitionBase::TextureDefinition *texDef =
          shadowNodeDef->addTextureDefinition("tmpCubemap");

      texDef->width   = pointLightCubemapResolution;
      texDef->height  = pointLightCubemapResolution;
      texDef->depth   = 6u;
      texDef->textureType = Ogre::TEX_TYPE_CUBE_MAP;
      texDef->formatList.push_back(Ogre::PF_FLOAT32_R);
      texDef->depthBufferId = 1u;
      texDef->depthBufferFormat = Ogre::PF_D32_FLOAT;
      texDef->preferDepthTexture = false;
      texDef->fsaa = false;
    }
  }

  // Create the shadow maps
  const size_t numShadowMaps =
      _shadowParams.size() + numExtraShadowMapsForPssmSplits;
  shadowNodeDef->setNumShadowTextureDefinitions(numShadowMaps);

  itor = _shadowParams.begin();

  while (itor != end)
  {
    const size_t lightIdx = itor - _shadowParams.begin();
    const Ogre::ShadowNodeHelper::ShadowParam &shadowParam = *itor;

    const Ogre::ShadowNodeHelper::Resolution &texResolution =
        atlasResolutions[shadowParam.atlasId];

    const size_t numSplits =
        shadowParam.technique == Ogre::SHADOWMAP_PSSM ?
        shadowParam.numPssmSplits : 1u;

    for (size_t j = 0; j < numSplits; ++j)
    {
      Ogre::Vector2 uvOffset(
          shadowParam.atlasStart[j].x, shadowParam.atlasStart[j].y);
      Ogre::Vector2 uvLength(
          shadowParam.resolution[j].x, shadowParam.resolution[j].y);

      uvOffset /= Ogre::Vector2(texResolution.x, texResolution.y);
      uvLength /= Ogre::Vector2(texResolution.x, texResolution.y);

      const Ogre::String texName =
          "atlas" + Ogre::StringConverter::toString(shadowParam.atlasId);

      Ogre::ShadowTextureDefinition *shadowTexDef =
          shadowNodeDef->addShadowTextureDefinition(lightIdx, j, texName,
          0, uvOffset, uvLength, 0);
      shadowTexDef->shadowMapTechnique = shadowParam.technique;
      shadowTexDef->pssmLambda = pssmLambda;
      shadowTexDef->splitPadding = splitPadding;
      shadowTexDef->splitBlend = splitBlend;
      shadowTexDef->splitFade = splitFade;
      shadowTexDef->numSplits = numSplits;
    }
    ++itor;
  }

  shadowNodeDef->setNumTargetPass(numTargetPasses);

  // Create the passes for each atlas
  for (size_t atlasId = 0; atlasId < numTextures; ++atlasId)
  {
    const Ogre::String texName =
        "atlas" + Ogre::StringConverter::toString(atlasId);
    {
      // Atlas clear pass
      Ogre::CompositorTargetDef *targetDef =
          shadowNodeDef->addTargetPass(texName);
      targetDef->setNumPasses(1u);

      Ogre::CompositorPassDef *passDef = targetDef->addPass(Ogre::PASS_CLEAR);
      Ogre::CompositorPassClearDef *passClear =
          static_cast<Ogre::CompositorPassClearDef *>(passDef);
      passClear->mColourValue = Ogre::ColourValue::White;
      passClear->mDepthValue = 1.0f;
    }

    // Pass scene for directional and spot lights first
    size_t shadowMapIdx = 0;
    itor = _shadowParams.begin();
    while (itor != end)
    {
      const Ogre::ShadowNodeHelper::ShadowParam &shadowParam = *itor;
      const size_t numSplits = shadowParam.technique == Ogre::SHADOWMAP_PSSM ?
          shadowParam.numPssmSplits : 1u;
      if (shadowParam.atlasId == atlasId &&
          shadowParam.supportedLightTypes & spotAndDirMask)
      {
        size_t currentShadowMapIdx = shadowMapIdx;
        for (size_t i = 0; i < numSplits; ++i)
        {
          Ogre::CompositorTargetDef *targetDef =
              shadowNodeDef->addTargetPass(texName);
          targetDef->setShadowMapSupportedLightTypes(
              shadowParam.supportedLightTypes & spotAndDirMask);
          targetDef->setNumPasses(1u);

          Ogre::CompositorPassDef *passDef =
              targetDef->addPass(Ogre::PASS_SCENE);
          Ogre::CompositorPassSceneDef *passScene =
              static_cast<Ogre::CompositorPassSceneDef *>(passDef);

          passScene->mShadowMapIdx = currentShadowMapIdx + i;
          passScene->mIncludeOverlays = false;
        }
      }
      shadowMapIdx += numSplits;
      ++itor;
    }

    // Pass scene for point lights last
    shadowMapIdx = 0;
    itor = _shadowParams.begin();
    while (itor != end)
    {
      const Ogre::ShadowNodeHelper::ShadowParam &shadowParam = *itor;
      if (shadowParam.atlasId == atlasId &&
          shadowParam.supportedLightTypes & pointMask)
      {
        // Render to cubemap, each face clear + render
        for (Ogre::uint32 i = 0; i < 6u; ++i)
        {
          Ogre::CompositorTargetDef *targetDef =
              shadowNodeDef->addTargetPass("tmpCubemap", i);
          targetDef->setNumPasses(2u);
          targetDef->setShadowMapSupportedLightTypes(
              shadowParam.supportedLightTypes & pointMask);
          {
            // Clear pass
            Ogre::CompositorPassDef *passDef =
                targetDef->addPass(Ogre::PASS_CLEAR);
            Ogre::CompositorPassClearDef *passClear =
                static_cast<Ogre::CompositorPassClearDef *>(passDef);
            passClear->mColourValue = Ogre::ColourValue::White;
            passClear->mDepthValue = 1.0f;
            passClear->mShadowMapIdx = shadowMapIdx;
          }

          {
            // Scene pass
            Ogre::CompositorPassDef *passDef =
                targetDef->addPass(Ogre::PASS_SCENE);
            Ogre::CompositorPassSceneDef *passScene =
                static_cast<Ogre::CompositorPassSceneDef *>(passDef);
            passScene->mCameraCubemapReorient = true;
            passScene->mShadowMapIdx = shadowMapIdx;
            passScene->mIncludeOverlays = false;
          }
        }

        // Copy to the atlas using a pass quad
        // (Cubemap -> DPSM / Dual Paraboloid).
        Ogre::CompositorTargetDef *targetDef =
            shadowNodeDef->addTargetPass(texName);
        targetDef->setShadowMapSupportedLightTypes(
            shadowParam.supportedLightTypes & pointMask);
        targetDef->setNumPasses(1u);
        Ogre::CompositorPassDef *passDef = targetDef->addPass(Ogre::PASS_QUAD);
        Ogre::CompositorPassQuadDef *passQuad =
            static_cast<Ogre::CompositorPassQuadDef *>(passDef);
        passQuad->mMaterialIsHlms = false;
        passQuad->mMaterialName = "Ogre/DPSM/CubeToDpsm";
        passQuad->addQuadTextureSource(0, "tmpCubemap", 0);
        passQuad->mShadowMapIdx = shadowMapIdx;
      }
      const size_t numSplits = shadowParam.technique ==
          Ogre::SHADOWMAP_PSSM ? shadowParam.numPssmSplits : 1u;
      shadowMapIdx += numSplits;
      ++itor;
    }
  }
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
void Ogre2RenderTarget::RebuildMaterial()
{
  if (this->material)
  {
    Ogre2Material *ogreMaterial = dynamic_cast<Ogre2Material *>(
        this->material.get());
    Ogre::MaterialPtr matPtr = ogreMaterial->Material();

    Ogre::SceneManager *sceneMgr = this->scene->OgreSceneManager();
    Ogre::RenderTarget *target = this->RenderTarget();
    this->materialApplicator.reset(new Ogre2RenderTargetMaterial(
        sceneMgr, target, matPtr.get()));
  }
}

//////////////////////////////////////////////////
// Ogre2RenderTexture
//////////////////////////////////////////////////
Ogre2RenderTexture::Ogre2RenderTexture()
{
}

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
Ogre::RenderTarget *Ogre2RenderTexture::RenderTarget() const
{
  return this->ogreTexture->getBuffer()->getRenderTarget();
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
  if (nullptr == this->ogreTexture)
    return;

  auto &manager = Ogre::TextureManager::getSingleton();
  manager.unload(this->ogreTexture->getName());
  manager.remove(this->ogreTexture->getName());

  // TODO(anyone) there is memory leak when a render texture is destroyed.
  // The RenderSystem::_cleanupDepthBuffers method used in ogre1 does not
  // seem to work in ogre2

  this->ogreTexture = nullptr;
}

//////////////////////////////////////////////////
void Ogre2RenderTexture::BuildTarget()
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

  // Ogre 2 PBS expects gamma correction to be enabled
  this->ogreTexture = (manager.createManual(this->name, "General",
      Ogre::TEX_TYPE_2D, this->width, this->height, 0, ogreFormat,
      Ogre::TU_RENDERTARGET, 0, true, fsaa)).get();
}

//////////////////////////////////////////////////
unsigned int Ogre2RenderTexture::GLId() const
{
  if (!this->ogreTexture)
    return 0;

  GLuint texId;
  this->ogreTexture->getCustomAttribute("GLID", &texId);

  return static_cast<unsigned int>(texId);
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
