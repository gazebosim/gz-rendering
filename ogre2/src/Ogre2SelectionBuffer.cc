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

#include <memory>
#include <ignition/math/Color.hh>

#include "ignition/common/Console.hh"
#include "ignition/rendering/RenderTypes.hh"
#include "ignition/rendering/ogre2/Ogre2Conversions.hh"
#include "ignition/rendering/ogre2/Ogre2MaterialSwitcher.hh"
#include "ignition/rendering/ogre2/Ogre2RenderEngine.hh"
#include "ignition/rendering/ogre2/Ogre2RenderTarget.hh"
#include "ignition/rendering/ogre2/Ogre2Scene.hh"
#include "ignition/rendering/ogre2/Ogre2SelectionBuffer.hh"

#ifdef _MSC_VER
  #pragma warning(push, 0)
#endif
#include <Compositor/OgreCompositorManager2.h>
#include <Compositor/OgreCompositorWorkspace.h>
#include <Compositor/Pass/PassClear/OgreCompositorPassClearDef.h>
#include <Compositor/Pass/PassQuad/OgreCompositorPassQuadDef.h>
#include <Compositor/Pass/PassScene/OgreCompositorPassSceneDef.h>
#include <OgreCamera.h>
#include <OgreDepthBuffer.h>
#include <OgreHardwarePixelBuffer.h>
#include <OgreItem.h>
#include <OgrePass.h>
#include <OgreRenderTexture.h>
#include <OgreRoot.h>
#include <OgreSceneManager.h>
#include <OgreTechnique.h>
#include <OgreTextureManager.h>
#include <OgreViewport.h>
#ifdef _MSC_VER
  #pragma warning(pop)
#endif

using namespace ignition;
using namespace rendering;

class ignition::rendering::Ogre2SelectionBufferPrivate
{
  /// \brief This is a material listener and a RenderTargetListener.
  /// The material switcher is applied to only the selection camera
  /// and not applied globally to all targets. The class associates a
  /// color to an ogre entity
  public: std::unique_ptr<Ogre2MaterialSwitcher> materialSwitcher;

  /// \brief Ogre scene manager
  public: Ogre2ScenePtr scene;

  /// \brief Ogre scene manager
  public: Ogre::SceneManager *sceneMgr = nullptr;

  /// \brief Pointer to the camera that will be used as the reference
  /// for selection
  public: Ogre::Camera *camera = nullptr;

  /// \brief Selection buffer's render to texture camera
  public: Ogre::Camera *selectionCamera  = nullptr;

  /// \brief Ogre texture
  public: Ogre::TexturePtr texture;

  /// \brief Ogre render texture
  // public: Ogre2RenderTexturePtr renderTexture;
  public: Ogre::RenderTexture *renderTexture = nullptr;

  /// \brief Ogre's compositor workspace - the main interface to render
  /// into a render target or render texture.
  public: Ogre::CompositorWorkspace *ogreCompositorWorkspace = nullptr;

  /// \brief Render texture data buffer
  public: float *buffer = nullptr;

  /// \brief Ogre pixel box that contains description of the data buffer
  public: Ogre::PixelBox *pixelBox = nullptr;

  /// \brief The selection buffer material
  public: Ogre::MaterialPtr selectionMaterial;
};

/////////////////////////////////////////////////
Ogre2SelectionBuffer::Ogre2SelectionBuffer(const std::string &_cameraName,
    Ogre2ScenePtr _scene): dataPtr(new Ogre2SelectionBufferPrivate)
{
  this->dataPtr->scene = _scene;
  this->dataPtr->sceneMgr = _scene->OgreSceneManager();

  this->dataPtr->camera = this->dataPtr->sceneMgr->findCameraNoThrow(
      _cameraName);
  if (!this->dataPtr->camera)
  {
    ignerr << "No camera found. Unable to create Ogre 2 selection buffer "
           << std::endl;
    return;
  }

  std::string selectionCameraName = _cameraName + "_selection_buffer";
  this->dataPtr->selectionCamera =
      this->dataPtr->sceneMgr->createCamera(selectionCameraName);

  this->dataPtr->materialSwitcher.reset(
      new Ogre2MaterialSwitcher(this->dataPtr->scene));
  this->CreateRTTBuffer();
}

/////////////////////////////////////////////////
Ogre2SelectionBuffer::~Ogre2SelectionBuffer()
{
  this->DeleteRTTBuffer();

  // remove selection buffer camera
  this->dataPtr->sceneMgr->destroyCamera(this->dataPtr->selectionCamera);
}

/////////////////////////////////////////////////
void Ogre2SelectionBuffer::Update()
{
  if (!this->dataPtr->renderTexture)
    return;

  this->dataPtr->materialSwitcher->Reset();

  // manual update
  this->dataPtr->ogreCompositorWorkspace->setEnabled(true);
  auto engine = Ogre2RenderEngine::Instance();
  engine->OgreRoot()->renderOneFrame();
  this->dataPtr->ogreCompositorWorkspace->setEnabled(false);

  this->dataPtr->renderTexture->copyContentsToMemory(*this->dataPtr->pixelBox,
      Ogre::RenderTarget::FB_FRONT);
}

/////////////////////////////////////////////////
void Ogre2SelectionBuffer::DeleteRTTBuffer()
{
  if (this->dataPtr->selectionMaterial)
  {
    Ogre::MaterialManager::getSingleton().remove(
        this->dataPtr->selectionMaterial->getName());
  }

  auto &manager = Ogre::TextureManager::getSingleton();
  manager.unload(this->dataPtr->texture->getName());
  manager.remove(this->dataPtr->texture->getName());

  if (this->dataPtr->buffer)
  {
    delete [] this->dataPtr->buffer;
    this->dataPtr->buffer = nullptr;
  }
  if (this->dataPtr->pixelBox)
    delete this->dataPtr->pixelBox;
}

/////////////////////////////////////////////////
void Ogre2SelectionBuffer::CreateRTTBuffer()
{
  // create a 1x1 pixel buffer
  unsigned int width = 1;
  unsigned int height = 1;
  Ogre::PixelFormat format = Ogre::PF_FLOAT32_RGBA;
  this->dataPtr->texture = Ogre::TextureManager::getSingleton().createManual(
        "SelectionPassTex",
        Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
        Ogre::TEX_TYPE_2D, width, height, 0, Ogre::PF_FLOAT32_RGBA,
        Ogre::TU_RENDERTARGET);

  this->dataPtr->renderTexture =
      this->dataPtr->texture->getBuffer()->getRenderTarget();

  // Load selection material
  // The SelectionBuffer material is defined in script
  // (selection_buffer.material).
  std::string matSelectionName = "SelectionBuffer";
  Ogre::MaterialPtr matSelection =
      Ogre::MaterialManager::getSingleton().getByName(matSelectionName);
  this->dataPtr->selectionMaterial = matSelection->clone(
      this->dataPtr->camera->getName() + "_" + matSelectionName);
  this->dataPtr->selectionMaterial->load();
  Ogre::Pass *p = this->dataPtr->selectionMaterial->getTechnique(0)->getPass(0);
  Ogre::GpuProgramParametersSharedPtr psParams =
      p->getFragmentProgramParameters();

  // Set the uniform variables (selection_buffer_fs.glsl).
  // The projectParams is used to linearize depth buffer data
  double nearPlane = this->dataPtr->camera->getNearClipDistance();
  double farPlane = this->dataPtr->camera->getFarClipDistance();

  this->dataPtr->selectionCamera->setNearClipDistance(nearPlane);
  this->dataPtr->selectionCamera->setFarClipDistance(farPlane);

  // \todo(anyone) change the code below when merging forward to fortress that
  // uses ogre 2.2 otherwise the depth values will be incorrect due to
  // reverse-z depth buffer
  // Ogre::Vector2 projectionAB =
  //   this->dataPtr->camera->getProjectionParamsAB();
  // double projectionA = projectionAB.x
  // double projectionB = projectionAB.y
  double projectionA = farPlane /
      (farPlane - nearPlane);
  double projectionB = (-farPlane * nearPlane) /
      (farPlane - nearPlane);
  projectionB /= farPlane;
  psParams->setNamedConstant("projectionParams",
      Ogre::Vector2(projectionA, projectionB));
  psParams->setNamedConstant("far",
      static_cast<float>(farPlane));
  psParams->setNamedConstant("inf",
      static_cast<float>(math::INF_F));

  // create compositor workspace for rendering
  auto engine = Ogre2RenderEngine::Instance();
  auto ogreRoot = engine->OgreRoot();
  Ogre::CompositorManager2 *ogreCompMgr = ogreRoot->getCompositorManager2();

  const Ogre::String workspaceName = "SelectionBufferWorkspace" +
      this->dataPtr->camera->getName();

  Ogre::CompositorNodeDef *nodeDef =
      ogreCompMgr->addNodeDefinition("AutoGen " + Ogre::IdString(workspaceName +
      "/Node").getReleaseText());

  Ogre::TextureDefinitionBase::TextureDefinition *colorTexDef =
      nodeDef->addTextureDefinition("colorTexture");
  colorTexDef->textureType = Ogre::TEX_TYPE_2D;
  colorTexDef->width = 0;
  colorTexDef->height = 0;
  colorTexDef->depth = 1;
  colorTexDef->numMipmaps = 0;
  colorTexDef->widthFactor = 1;
  colorTexDef->heightFactor = 1;
  colorTexDef->formatList = {Ogre::PF_FLOAT32_RGBA};
  colorTexDef->fsaa = 0;
  colorTexDef->uav = false;
  colorTexDef->automipmaps = false;
  colorTexDef->depthBufferId = Ogre::DepthBuffer::POOL_DEFAULT;
  colorTexDef->depthBufferFormat = Ogre::PF_D32_FLOAT;
  colorTexDef->preferDepthTexture = true;
  colorTexDef->fsaaExplicitResolve = false;

  Ogre::TextureDefinitionBase::TextureDefinition *depthTexDef =
      nodeDef->addTextureDefinition("depthTexture");
  depthTexDef->textureType = Ogre::TEX_TYPE_2D;
  depthTexDef->width = 0;
  depthTexDef->height = 0;
  depthTexDef->depth = 1;
  depthTexDef->numMipmaps = 0;
  depthTexDef->widthFactor = 1;
  depthTexDef->heightFactor = 1;
  depthTexDef->formatList = {Ogre::PF_D32_FLOAT};
  depthTexDef->fsaa = 0;
  depthTexDef->uav = false;
  depthTexDef->automipmaps = false;
  depthTexDef->hwGammaWrite = Ogre::TextureDefinitionBase::BoolFalse;
  depthTexDef->depthBufferId = Ogre::DepthBuffer::POOL_DEFAULT;
  depthTexDef->depthBufferFormat = Ogre::PF_UNKNOWN;
  depthTexDef->fsaaExplicitResolve = false;

  // Input texture
  nodeDef->addTextureSourceName("rt", 0,
      Ogre::TextureDefinitionBase::TEXTURE_INPUT);

  nodeDef->setNumTargetPass(2);
  Ogre::CompositorTargetDef *colorTargetDef =
      nodeDef->addTargetPass("colorTexture");
  colorTargetDef->setNumPasses(2);
  {
    // clear pass
    Ogre::CompositorPassClearDef *passClear =
        static_cast<Ogre::CompositorPassClearDef *>(
        colorTargetDef->addPass(Ogre::PASS_CLEAR));
    passClear->mColourValue = Ogre::ColourValue(0.0f, 0.0f, 0.0f, 1.0f);
    // scene pass
    Ogre::CompositorPassSceneDef *passScene =
        static_cast<Ogre::CompositorPassSceneDef *>(
        colorTargetDef->addPass(Ogre::PASS_SCENE));
    passScene->mVisibilityMask = IGN_VISIBILITY_SELECTABLE;
  }

  Ogre::CompositorTargetDef *targetDef = nodeDef->addTargetPass("rt");
  targetDef->setNumPasses(2);
  {
    {
      Ogre::CompositorPassClearDef *passClear =
          static_cast<Ogre::CompositorPassClearDef *>(
          targetDef->addPass(Ogre::PASS_CLEAR));
      passClear->mColourValue = Ogre::ColourValue(0.0f, 0.0f, 0.0f, 1.0f);
    }
    // quad pass
    Ogre::CompositorPassQuadDef *passQuad =
        static_cast<Ogre::CompositorPassQuadDef *>(
        targetDef->addPass(Ogre::PASS_QUAD));
    passQuad->mMaterialName = this->dataPtr->selectionMaterial->getName();
    passQuad->addQuadTextureSource(0, "colorTexture", 0);
    passQuad->addQuadTextureSource(1, "depthTexture", 0);
    passQuad->mFrustumCorners =
        Ogre::CompositorPassQuadDef::VIEW_SPACE_CORNERS;
  }

  Ogre::CompositorWorkspaceDef *workDef =
      ogreCompMgr->addWorkspaceDefinition(workspaceName);
  workDef->connectExternal(0, nodeDef->getName(), 0);

  this->dataPtr->ogreCompositorWorkspace =
      ogreCompMgr->addWorkspace(this->dataPtr->scene->OgreSceneManager(),
      this->dataPtr->renderTexture,
      this->dataPtr->selectionCamera, workspaceName, false);

  // add the listener
  Ogre::CompositorNode *node =
      this->dataPtr->ogreCompositorWorkspace->getNodeSequence()[0];
  auto channelsTex = node->getLocalTextures();

  for (auto &c : channelsTex)
  {
    if (c.textures[0]->getSrcFormat() == Ogre::PF_FLOAT32_RGBA)
    {
      c.target->addListener(
          this->dataPtr->materialSwitcher.get());
      break;
    }
  }

  // buffer to store render texture data. Ensure it's at least 4 channels
  size_t bufferSize = std::max<size_t>(
      Ogre::PixelUtil::getMemorySize(width, height, 1, format),
      4u);
  this->dataPtr->buffer = new float[width * height * 4u];
  memset(this->dataPtr->buffer, 0, bufferSize);
  this->dataPtr->pixelBox = new Ogre::PixelBox(width,
      height, 1, format, this->dataPtr->buffer);
}

/////////////////////////////////////////////////
Ogre::Item *Ogre2SelectionBuffer::OnSelectionClick(const int _x, const int _y)
{
  Ogre::Item *item = nullptr;
  math::Vector3d point;
  this->ExecuteQuery(_x, _y, item, point);
  return item;
}

/////////////////////////////////////////////////
bool Ogre2SelectionBuffer::ExecuteQuery(const int _x, const int _y,
    Ogre::Item *&_item, math::Vector3d &_point)
{
  if (!this->dataPtr->renderTexture)
    return false;

  if (!this->dataPtr->camera)
    return false;

  Ogre::Viewport *vp = this->dataPtr->camera->getLastViewport();

  if (!vp)
    return false;

  Ogre::RenderTarget *rt = vp->getTarget();

  if (!rt)
    return false;

  const unsigned int targetWidth = rt->getWidth();
  const unsigned int targetHeight = rt->getHeight();

  if (_x < 0 || _y < 0 || _x >= static_cast<int>(targetWidth)
      || _y >= static_cast<int>(targetHeight))
    return false;

  // 1x1 selection buffer, adapted from rviz
  // http://docs.ros.org/indigo/api/rviz/html/c++/selection__manager_8cpp.html
  unsigned int width = 1;
  unsigned int height = 1;
  float x1 = static_cast<float>(_x) /
      static_cast<float>(targetWidth - 1) - 0.5f;
  float y1 = static_cast<float>(_y) /
      static_cast<float>(targetHeight - 1) - 0.5f;
  float x2 = static_cast<float>(_x+width) /
      static_cast<float>(targetWidth - 1) - 0.5f;
  float y2 = static_cast<float>(_y+height) /
      static_cast<float>(targetHeight - 1) - 0.5f;
  Ogre::Matrix4 scaleMatrix = Ogre::Matrix4::IDENTITY;
  Ogre::Matrix4 transMatrix = Ogre::Matrix4::IDENTITY;
  scaleMatrix[0][0] = 1.0 / (x2-x1);
  scaleMatrix[1][1] = 1.0 / (y2-y1);
  transMatrix[0][3] -= x1+x2;
  transMatrix[1][3] += y1+y2;
  Ogre::Matrix4 customProjectionMatrix =
      scaleMatrix * transMatrix * this->dataPtr->camera->getProjectionMatrix();

  // There is a bug in ogre 2.1 that produces incorrect frustum
  // extents when a custom projection matrix is set.
  // So we manually compute the frusm extents ourselves
  // \todo(anyone) The bug should be fixed in ogre 2.2 so we should be
  // able to uncomment the setCustomProjectionMatrix call below
  // and remove the extents set by setFrustumExtents
  // this->dataPtr->selectionCamera->setCustomProjectionMatrix(true,
  //      customProjectionMatrix);
  Ogre::Matrix4 invProj = customProjectionMatrix.inverse();
  Ogre::Vector4 topLeft(-1.0f, 1.0f, -1.0f, 1.0f);
  Ogre::Vector4 bottomRight(1.0f, -1.0f, -1.0f, 1.0f);
  topLeft = invProj * topLeft;
  bottomRight = invProj * bottomRight;
  float left = topLeft.x / topLeft.w;
  float top = topLeft.y / topLeft.w;
  float right = bottomRight.x / bottomRight.w;
  float bottom = bottomRight.y / bottomRight.w;
  this->dataPtr->selectionCamera->setFrustumExtents(left, right, top, bottom);

  this->dataPtr->selectionCamera->setPosition(
      this->dataPtr->camera->getDerivedPosition());
  this->dataPtr->selectionCamera->setOrientation(
      this->dataPtr->camera->getDerivedOrientation());
  Ogre::Viewport* renderViewport =
      this->dataPtr->renderTexture->getViewport(0);
  renderViewport->setDimensions(0, 0, width, height);

  // update render texture
  this->Update();

  if (!this->dataPtr->buffer)
  {
    ignerr << "Selection buffer is null." << std::endl;
    return false;
  }

  float color = this->dataPtr->buffer[3];
  uint32_t *rgba = reinterpret_cast<uint32_t *>(&color);
  unsigned int r = *rgba >> 24 & 0xFF;
  unsigned int g = *rgba >> 16 & 0xFF;
  unsigned int b = *rgba >> 8 & 0xFF;

  math::Vector3d point(this->dataPtr->buffer[0], this->dataPtr->buffer[1],
      this->dataPtr->buffer[2]);
  auto rot = Ogre2Conversions::Convert(
      this->dataPtr->camera->getParentSceneNode()->_getDerivedOrientation());
  auto pos = Ogre2Conversions::Convert(
      this->dataPtr->camera->getParentSceneNode()->_getDerivedPosition());
  math::Pose3d p(pos, rot);
  point = rot * point + pos;

  ignition::math::Color cv;
  cv.A(1.0);
  cv.R(r / 255.0);
  cv.G(g / 255.0);
  cv.B(b / 255.0);

  const std::string &entName =
    this->dataPtr->materialSwitcher->EntityName(cv);

  if (entName.empty())
  {
    return false;
  }
  else
  {
    auto collection = this->dataPtr->sceneMgr->findMovableObjects(
        Ogre::ItemFactory::FACTORY_TYPE_NAME, entName);
    if (collection.empty())
      return false;
    else
    {
      _item = dynamic_cast<Ogre::Item *>(collection[0]);
      _point = point;
      return true;
    }
  }
}
