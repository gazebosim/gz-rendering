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

#include "ignition/common/Console.hh"
#include "ignition/rendering/ogre2/Ogre2Camera.hh"
#include "ignition/rendering/ogre2/Ogre2Conversions.hh"
#include "ignition/rendering/ogre2/Ogre2Includes.hh"
#include "ignition/rendering/ogre2/Ogre2RenderTarget.hh"
#include "ignition/rendering/ogre2/Ogre2Scene.hh"
#include "ignition/rendering/ogre2/Ogre2SelectionBuffer.hh"
#include "ignition/rendering/RenderTypes.hh"
#include "ignition/rendering/ogre2/Ogre2RenderTypes.hh"
#include "ignition/rendering/Utils.hh"
#include "ignition/rendering/ogre2/Ogre2RenderEngine.hh"
#include "ignition/rendering/ogre2/Ogre2BoundingBoxCamera.hh"
#include "ignition/rendering/ogre2/Ogre2Visual.hh"
#include "ignition/math/Color.hh"
#include <random>
#include <limits>

#include <OgreGL3PlusAsyncTicket.h>
#include <OgreBitwise.h>
#include <OgreWireAabb.h>
#include <OgreWireBoundingBox.h>

using namespace ignition;
using namespace rendering;


namespace ignition
{
  namespace rendering
  {
    inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {

    /// \brief Helper class to assign ogre Id & label to each pixels
    /// used in checking bounding boxes visibility in case of full mode
    /// and to get bounding boxes boundaries in case of visible mode
    class BoundingBoxMaterialSwitcher : public Ogre::RenderTargetListener
    {
      /// \brief Constructor
      public: explicit BoundingBoxMaterialSwitcher(Ogre2ScenePtr _scene);

      /// \brief Destructor
      public: ~BoundingBoxMaterialSwitcher();

      /// \brief Ogre's pre render update callback
      /// \param[in] _evt Ogre render target event containing information about
      /// the source render target.
      public: virtual void preRenderTargetUpdate(
                  const Ogre::RenderTargetEvent &_evt);

      /// \brief Ogre's post render update callback
      /// \param[in] _evt Ogre render target event containing information about
      /// the source render target.
      public: virtual void postRenderTargetUpdate(
                  const Ogre::RenderTargetEvent &_evt);

      /// \brief A map of ogre sub item pointer to their original hlms material
      private: std::map<Ogre::SubItem *, Ogre::HlmsDatablock *> datablockMap;

      /// \brief Ogre v1 material consisting of a shader that changes the
      /// appearance of item to use a unique color for mouse picking
      private: Ogre::MaterialPtr plainMaterial;

      /// \brief Ogre v1 material consisting of a shader that changes the
      /// appearance of item to use a unique color for mouse picking. In
      /// addition, the depth check and depth write properties disabled.
      private: Ogre::MaterialPtr plainOverlayMaterial;

      /// \brief User Data Key to set the label
      private: std::string labelKey = "label";

      /// \brief Label for background pixels in the ogre Ids map
      private: uint32_t backgroundLabel {255};

      /// \brief Ogre2 Scene
      private: Ogre2ScenePtr scene;

      friend class Ogre2BoundingBoxCamera;
    };
    }
  }
}

/////////////////////////////////////////////////
BoundingBoxMaterialSwitcher::BoundingBoxMaterialSwitcher(Ogre2ScenePtr _scene)
{
  this->scene = _scene;

  // plain material to switch item's material
  Ogre::ResourcePtr res =
    Ogre::MaterialManager::getSingleton().load("ign-rendering/plain_color",
        Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

  this->plainMaterial = res.staticCast<Ogre::Material>();
  this->plainMaterial->load();

  // plain overlay material
  this->plainOverlayMaterial =
      this->plainMaterial->clone("plain_color_overlay");
  if (!this->plainOverlayMaterial->getTechnique(0) ||
      !this->plainOverlayMaterial->getTechnique(0)->getPass(0))
  {
    ignerr << "Problem creating selection buffer overlay material"
        << std::endl;
    return;
  }
  Ogre::Pass *overlayPass =
      this->plainOverlayMaterial->getTechnique(0)->getPass(0);
  Ogre::HlmsMacroblock macroblock(*overlayPass->getMacroblock());
  macroblock.mDepthCheck = false;
  macroblock.mDepthWrite = false;
  overlayPass->setMacroblock(macroblock);
}

/////////////////////////////////////////////////
BoundingBoxMaterialSwitcher::~BoundingBoxMaterialSwitcher()
{
}

////////////////////////////////////////////////
void BoundingBoxMaterialSwitcher::preRenderTargetUpdate(
    const Ogre::RenderTargetEvent &/*_evt*/)
{
  this->datablockMap.clear();
  auto itor = this->scene->OgreSceneManager()->getMovableObjectIterator(
      Ogre::ItemFactory::FACTORY_TYPE_NAME);

  while (itor.hasMoreElements())
  {
    Ogre::MovableObject *object = itor.peekNext();
    Ogre::Item *item = static_cast<Ogre::Item *>(object);

    // get visual from ogre item
    Ogre::Any userAny = item->getUserObjectBindings().getUserAny();

    if (!userAny.isEmpty() && userAny.getType() == typeid(unsigned int))
    {
      VisualPtr visual;
      try
      {
        visual = this->scene->VisualById(Ogre::any_cast<unsigned int>(userAny));
      }
      catch(Ogre::Exception &e)
      {
        ignerr << "Ogre Error:" << e.getFullDescription() << "\n";
      }
      Ogre2VisualPtr ogreVisual = std::dynamic_pointer_cast<Ogre2Visual>(
        visual);

      // get class user data
      Variant labelAny = ogreVisual->UserData(this->labelKey);

      int label = this->backgroundLabel;
      try
      {
        label = std::get<int>(labelAny);
      }
      catch(std::bad_variant_access &e)
      {
        // items with no class are considered background
        label = this->backgroundLabel;
      }

      for (unsigned int i = 0; i < item->getNumSubItems(); i++)
      {
        // save subitems material
        Ogre::SubItem *subItem = item->getSubItem(i);
        Ogre::HlmsDatablock *datablock = subItem->getDatablock();
        this->datablockMap[subItem] = datablock;

        // for full bbox, each pixel contains 1 channel for label
        // and 2 channels stores ogreId
        uint32_t ogreId = item->getId();

        float labelColor = label / 255.0;
        float ogreId1 = (ogreId / 256) / 255.0;
        float ogreId2 = (ogreId % 256) / 255.0;

        subItem->setCustomParameter(1, Ogre::Vector4(
        labelColor, ogreId1, ogreId2, 1.0));

        if (!datablock->getMacroblock()->mDepthWrite &&
            !datablock->getMacroblock()->mDepthCheck)
          subItem->setMaterial(this->plainOverlayMaterial);
        else
          subItem->setMaterial(this->plainMaterial);
      }
    }
    itor.moveNext();
  }
}

/////////////////////////////////////////////////
void BoundingBoxMaterialSwitcher::postRenderTargetUpdate(
    const Ogre::RenderTargetEvent &/*_evt*/)
{
  // restore item to use hlms material
  auto itor = this->scene->OgreSceneManager()->getMovableObjectIterator(
      Ogre::ItemFactory::FACTORY_TYPE_NAME);
  while (itor.hasMoreElements())
  {
    Ogre::MovableObject *object = itor.peekNext();
    Ogre::Item *item = static_cast<Ogre::Item *>(object);
    for (unsigned int i = 0; i < item->getNumSubItems(); i++)
    {
      Ogre::SubItem *subItem = item->getSubItem(i);
      auto it = this->datablockMap.find(subItem);
      if (it != this->datablockMap.end())
        subItem->setDatablock(it->second);
    }
    itor.moveNext();
  }
}

/////////////////////////////////////////////////
class ignition::rendering::Ogre2BoundingBoxCameraPrivate
{
  /// \brief Material Switcher to switch item's material with ogre Ids
  /// For bounding boxes visibility checking & finding boundaires
  public: BoundingBoxMaterialSwitcher *materialSwitcher {nullptr};

  /// \brief Compositor Manager to create workspace
  public: Ogre::CompositorManager2 *ogreCompositorManager {nullptr};

  /// \brief Workspace to interface with render texture
  public: Ogre::CompositorWorkspace *ogreCompositorWorkspace {nullptr};

  /// \brief Workspace Definition
  public: std::string workspaceDefinition;

  /// \brief Render Texture to store the ogreIds map
  public: Ogre::RenderTexture *ogreRenderTexture {nullptr};

  /// \brief Texture to create the render texture from.
  public: Ogre::TexturePtr ogreTexture;

  /// \brief Pixel Box to copy render texture data to a buffer
  public: Ogre::PixelBox *pixelBox {nullptr};

  /// \brief buffer to store render texture data & to be sent to listeners
  public: uint8_t *buffer = nullptr;

  /// \brief dummy render texture to set image dims
  public: Ogre2RenderTexturePtr dummyTexture {nullptr};

  /// \brief New BoundingBox Frame Event to notify listeners with new data
  public: ignition::common::EventT<void(const std::vector<BoundingBox> &)>
        newBoundingBoxes;

  /// \brief Image / Render Texture Format
  public: Ogre::PixelFormat format = Ogre::PF_R8G8B8;

  /// \brief map ogreId id to bounding box
  /// key: ogreId, value: bounding box contains max & min boundaries
  public: std::map<uint32_t, BoundingBox *> boundingboxes;

  /// \brief keep track of visible bounding boxes (used in filtering)
  /// key: ogreId, value: label id
  public: std::map<uint32_t, uint32_t> visibleBoxesLabel;

  /// \brief output bounding boxes to nofity listeners
  public: std::vector<BoundingBox> output_boxes;

  public: BoundingBoxType type {BoundingBoxType::VisibleBox};
};

/////////////////////////////////////////////////
Ogre2BoundingBoxCamera::Ogre2BoundingBoxCamera() :
  dataPtr(new Ogre2BoundingBoxCameraPrivate())
{
}

/////////////////////////////////////////////////
Ogre2BoundingBoxCamera::~Ogre2BoundingBoxCamera()
{
  this->Destroy();
}

/////////////////////////////////////////////////
void Ogre2BoundingBoxCamera::Init()
{
  BaseCamera::Init();
  this->CreateCamera();
  this->CreateRenderTexture();

  this->dataPtr->materialSwitcher =
    new BoundingBoxMaterialSwitcher(this->scene);
}

/////////////////////////////////////////////////
void Ogre2BoundingBoxCamera::CreateCamera()
{
  auto ogreScene = this->scene->OgreSceneManager();
  if (ogreScene == nullptr)
  {
    ignerr << "Scene manager cannot be obtained" << std::endl;
    return;
  }

  this->ogreCamera = ogreScene->createCamera(this->Name());
  if (this->ogreCamera == nullptr)
  {
    ignerr << "Ogre camera cannot be created" << std::endl;
    return;
  }

  this->ogreCamera->detachFromParent();
  this->ogreNode->attachObject(this->ogreCamera);

  // rotate to ignition gazebo coord.
  this->ogreCamera->yaw(Ogre::Degree(-90));
  this->ogreCamera->roll(Ogre::Degree(-90));
  this->ogreCamera->setFixedYawAxis(false);

  this->ogreCamera->setAutoAspectRatio(true);
  this->ogreCamera->setRenderingDistance(100);
  this->ogreCamera->setProjectionType(Ogre::ProjectionType::PT_PERSPECTIVE);
  this->ogreCamera->setCustomProjectionMatrix(false);
}

/////////////////////////////////////////////////
void Ogre2BoundingBoxCamera::Destroy()
{
  BaseCamera::Destroy();
  Camera::Destroy();

  delete this->dataPtr->ogreCompositorWorkspace;
  delete this->dataPtr->ogreCompositorManager;

  auto &manager = Ogre::TextureManager::getSingleton();
  manager.unload(this->dataPtr->ogreTexture->getName());
  manager.remove(this->dataPtr->ogreTexture->getName());
}

/////////////////////////////////////////////////
void Ogre2BoundingBoxCamera::PreRender()
{
  if (!this->dataPtr->ogreRenderTexture)
    this->CreateBoundingBoxTexture();

  this->dataPtr->output_boxes.clear();
}

/////////////////////////////////////////////////
void Ogre2BoundingBoxCamera::CreateBoundingBoxTexture()
{
  // Camera Parameters
  this->ogreCamera->setNearClipDistance(this->NearClipPlane());
  this->ogreCamera->setFarClipDistance(this->FarClipPlane());
  this->ogreCamera->setAspectRatio(this->AspectRatio());
  double vfov = 2.0 * atan(tan(this->HFOV().Radian() / 2.0) /
    this->AspectRatio());
  this->ogreCamera->setFOVy(Ogre::Radian(vfov));

  auto width = this->ImageWidth();
  auto height = this->ImageHeight();

  // texture
  this->dataPtr->ogreTexture =
    Ogre::TextureManager::getSingleton().createManual(
    "BoundingBoxCameraTexture",
    Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
    Ogre::TEX_TYPE_2D, width, height, 0, this->dataPtr->format,
    Ogre::TU_RENDERTARGET
  );

  // render texture
  auto hardwareBuffer = this->dataPtr->ogreTexture->getBuffer();
  this->dataPtr->ogreRenderTexture = hardwareBuffer->getRenderTarget();

  // Switch material to OGRE Ids map to use it to get the visible bboxes
  // or to check visiblity in full bboxes
  this->dataPtr->ogreRenderTexture->addListener(
    this->dataPtr->materialSwitcher);

  // workspace
  auto engine = Ogre2RenderEngine::Instance();
  auto ogreRoot = engine->OgreRoot();
  this->dataPtr->ogreCompositorManager = ogreRoot->getCompositorManager2();

  this->dataPtr->workspaceDefinition = "BoundingBoxCameraWorkspace_" +
    this->Name();

  uint32_t background = this->dataPtr->materialSwitcher->backgroundLabel;
  auto backgroundColor = Ogre::ColourValue(background, background, background);

  // basic workspace consist of clear pass with the givin color &
  // a render scene pass to the givin render texture
  this->dataPtr->ogreCompositorManager->createBasicWorkspaceDef(
    this->dataPtr->workspaceDefinition,
    backgroundColor
    );

  // connect the compositor with the render texture to render the final output
  this->dataPtr->ogreCompositorWorkspace =
    this->dataPtr->ogreCompositorManager->addWorkspace(
      this->scene->OgreSceneManager(),
      this->dataPtr->ogreRenderTexture,
      this->ogreCamera,
      this->dataPtr->workspaceDefinition,
      false
    );

  // set visibility mask
  auto node = this->dataPtr->ogreCompositorWorkspace->getNodeSequence()[0];
  auto pass = node->_getPasses()[1]->getDefinition();
  auto renderScenePass =
    dynamic_cast<const Ogre::CompositorPassSceneDef *>(pass);
  const_cast<Ogre::CompositorPassSceneDef *>(
    renderScenePass)->setVisibilityMask(IGN_VISIBILITY_ALL);

  // buffer to store render texture data
  auto bufferSize = Ogre::PixelUtil::getMemorySize(
    width, height, 1, this->dataPtr->format);
  this->dataPtr->buffer = new uint8_t[bufferSize];
  this->dataPtr->pixelBox = new Ogre::PixelBox(width, height, 1,
    this->dataPtr->format, this->dataPtr->buffer);
}

/////////////////////////////////////////////////
void Ogre2BoundingBoxCamera::Render()
{
  this->dataPtr->ogreCompositorWorkspace->setEnabled(true);
  auto engine = Ogre2RenderEngine::Instance();
  auto ogreRoot = engine->OgreRoot();
  ogreRoot->renderOneFrame();
  this->dataPtr->ogreCompositorWorkspace->setEnabled(false);
}

/////////////////////////////////////////////////
void Ogre2BoundingBoxCamera::PostRender()
{
  // copy render texture data to the pixel box & its buffer
  this->dataPtr->ogreRenderTexture->copyContentsToMemory(
    *this->dataPtr->pixelBox,
    Ogre::RenderTarget::FB_FRONT
  );

  // return if no one is listening to the new frame
  if (this->dataPtr->newBoundingBoxes.ConnectionCount() == 0)
    return;

  std::vector<BoundingBox> boxes;

  if (this->dataPtr->type == BoundingBoxType::VisibleBox)
    this->VisibleBoundingBoxes();
  else if (this->dataPtr->type == BoundingBoxType::FullBox)
    this->FullBoundingBoxes();

  for (auto box : this->dataPtr->boundingboxes)
    delete box.second;

  this->dataPtr->boundingboxes.clear();
  this->dataPtr->visibleBoxesLabel.clear();

  this->dataPtr->newBoundingBoxes(this->dataPtr->output_boxes);
}

void Ogre2BoundingBoxCamera::VisibleBoundingBoxes()
{
  uint32_t width = this->ImageWidth();
  uint32_t height = this->ImageHeight();
  uint32_t channelCount = 3;

  // find item's boundaries from panoptic BoundingBox
  for (uint32_t y = 0; y < height; y++)
  {
    for (uint32_t x = 0; x < width; x++)
    {
      auto index = (y * width + x) * channelCount;

      uint32_t label = this->dataPtr->buffer[index + 2];
      uint32_t ogreId1 = this->dataPtr->buffer[index + 1];
      uint32_t ogreId2 = this->dataPtr->buffer[index + 0];

      if (label != this->dataPtr->materialSwitcher->backgroundLabel)
      {
        // get the OGRE id of 16 bit value
        uint32_t ogreId = ogreId1 * 256 + ogreId2;

        BoundingBox *box;

        // create new boxes when its first pixel appears
        if (!this->dataPtr->boundingboxes.count(ogreId))
        {
          box = new BoundingBox();
          box->label = label;
          box->minX = width;
          box->minY = height;
          box->maxX = 0;
          box->maxY = 0;
          this->dataPtr->boundingboxes[ogreId] = box;
        }
        else
          box = this->dataPtr->boundingboxes[ogreId];

        box->minX = std::min<uint32_t>(box->minX, x);
        box->minY = std::min<uint32_t>(box->minY, y);
        box->maxX = std::max<uint32_t>(box->maxX, x);
        box->maxY = std::max<uint32_t>(box->maxY, y);
      }
    }
  }

  for (auto box : this->dataPtr->boundingboxes)
    this->dataPtr->output_boxes.push_back(*box.second);
}

void Ogre2BoundingBoxCamera::FullBoundingBoxes()
{
  uint32_t width = this->ImageWidth();
  uint32_t height = this->ImageHeight();
  uint32_t channelCount = 3;

  // Filter bounding boxes
  for (uint32_t y = 0; y < height; y++)
  {
    for (uint32_t x = 0; x < width; x++)
    {
      auto index = (y * width + x) * channelCount;

      uint32_t label = this->dataPtr->buffer[index + 2];

      if (label != this->dataPtr->materialSwitcher->backgroundLabel)
      {
        // get the ogre id encoded in 16 bit value
        uint32_t ogreId1 = this->dataPtr->buffer[index + 1];
        uint32_t ogreId2 = this->dataPtr->buffer[index + 0];
        uint32_t ogreId = ogreId1 * 256 + ogreId2;

        // mark the ogreId as visible not to filter its bbox
        if (!this->dataPtr->visibleBoxesLabel.count(ogreId))
          this->dataPtr->visibleBoxesLabel[ogreId] = label;
      }
    }
  }

  Ogre::Matrix4 viewMatrix = this->ogreCamera->getViewMatrix();
  Ogre::Matrix4 projMatrix = this->ogreCamera->getProjectionMatrix();

  auto itor = this->scene->OgreSceneManager()->getMovableObjectIterator(
      Ogre::ItemFactory::FACTORY_TYPE_NAME);
  while (itor.hasMoreElements())
  {
    Ogre::MovableObject *object = itor.peekNext();
    Ogre::Item *item = static_cast<Ogre::Item *>(object);
    Ogre::MeshPtr mesh = item->getMesh();

    uint32_t ogreId = item->getId();

    // Skip the items which is hidden in the ogreId map
    if (!this->dataPtr->visibleBoxesLabel.count(ogreId))
    {
      itor.moveNext();
      continue;
    }

    // get attached node
    Ogre::Node *node = item->getParentNode();

    Ogre::Vector3 position = node->getPosition();
    Ogre::Quaternion oreintation = node->getOrientation();
    Ogre::Vector3 scale = node->getScale();

    Ogre::Aabb aabb = item->getWorldAabb();
    Ogre::AxisAlignedBox worldAabb;
    worldAabb.setExtents(aabb.getMinimum(), aabb.getMaximum());

    if (!this->ogreCamera->isVisible(worldAabb))
    {
      itor.moveNext();
      continue;
    }

    // this->ogreCamera.in
    Ogre::Vector3 minVertex;
    Ogre::Vector3 maxVertex;

    this->MeshMinimalBox(
      mesh,
      viewMatrix,
      projMatrix,
      minVertex,
      maxVertex,
      position,
      oreintation,
      scale
    );

    if ((abs(minVertex.x) > 1 && abs(maxVertex.x) > 1) ||
        (abs(minVertex.y) > 1 && abs(maxVertex.y) > 1))
    {
      itor.moveNext();
      continue;
    }

    this->ConvertToScreenCoord(minVertex, maxVertex);

    BoundingBox *box = new BoundingBox();
    box->minX = minVertex.x;
    box->maxX = maxVertex.x;
    // swap min & max of y as image coord is inverted in y
    box->minY = maxVertex.y;
    box->maxY = minVertex.y;

    this->dataPtr->boundingboxes[ogreId] = box;

    itor.moveNext();
  }

  for (auto box : this->dataPtr->boundingboxes)
  {
    uint32_t ogreId = box.first;
    uint32_t label = this->dataPtr->visibleBoxesLabel[ogreId];

    box.second->label = label;
    this->dataPtr->output_boxes.push_back(*box.second);
  }
}

/////////////////////////////////////////////////
void Ogre2BoundingBoxCamera::MeshMinimalBox(
  const Ogre::MeshPtr mesh,
  const Ogre::Matrix4 &viewMatrix,
  const Ogre::Matrix4 &projMatrix,
  Ogre::Vector3 &minVertex,
  Ogre::Vector3 &maxVertex,
  const Ogre::Vector3 &position,
  const Ogre::Quaternion &oreintation,
  const Ogre::Vector3 &scale
  )
{
  minVertex.x = INT32_MAX;
  minVertex.y = INT32_MAX;
  minVertex.z = INT32_MAX;
  maxVertex.x = INT32_MIN;
  maxVertex.y = INT32_MIN;
  maxVertex.z = INT32_MIN;

  auto subMeshes = mesh->getSubMeshes();

  for (auto subMesh : subMeshes)
  {
    Ogre::VertexArrayObjectArray vaos = subMesh->mVao[0];

    if (!vaos.empty())
    {
      // Get the first LOD level
      Ogre::VertexArrayObject *vao = vaos[0];

      // request async read from buffer
      Ogre::VertexArrayObject::ReadRequestsArray requests;
      requests.push_back(Ogre::VertexArrayObject::ReadRequests(
        Ogre::VES_POSITION));
      vao->readRequests(requests);
      vao->mapAsyncTickets(requests);

      unsigned int subMeshVerticiesNum =
        requests[0].vertexBuffer->getNumElements();
      for (size_t i = 0; i < subMeshVerticiesNum; ++i)
      {
        Ogre::Vector3 vec;
        if (requests[0].type == Ogre::VET_HALF4)
        {
          const Ogre::uint16* vertex = reinterpret_cast<const Ogre::uint16*>
            (requests[0].data);
          vec.x = Ogre::Bitwise::halfToFloat(vertex[0]);
          vec.y = Ogre::Bitwise::halfToFloat(vertex[1]);
          vec.z = Ogre::Bitwise::halfToFloat(vertex[2]);
        }
        else if (requests[0].type == Ogre::VET_FLOAT3)
        {
          const float* vertex =
            reinterpret_cast<const float*>(requests[0].data);
          vec.x = *vertex++;
          vec.y = *vertex++;
          vec.z = *vertex++;
        }
        else
          ignerr << "Vertex Buffer type error" << std::endl;

        vec = (oreintation * (vec * scale)) + position;

        vec = projMatrix * viewMatrix * vec;

        // homogenous
        vec.x /= vec.z;
        vec.y /= vec.z;

        minVertex.x = std::min(minVertex.x, vec.x);
        minVertex.y = std::min(minVertex.y, vec.y);
        minVertex.z = std::min(minVertex.z, vec.z);

        maxVertex.x = std::max(maxVertex.x, vec.x);
        maxVertex.y = std::max(maxVertex.y, vec.y);
        maxVertex.z = std::max(maxVertex.z, vec.z);

        // get the next element
        requests[0].data += requests[0].vertexBuffer->getBytesPerElement();
      }
      vao->unmapAsyncTickets(requests);
    }
  }
}

/////////////////////////////////////////////////
void Ogre2BoundingBoxCamera::DrawBoundingBox(
  unsigned char *_data, BoundingBox &_box)
{
  math::Vector2 minVertex(_box.minX, _box.minY);
  math::Vector2 maxVertex(_box.maxX, _box.maxY);

  uint32_t width = this->ImageWidth();

  std::vector<uint32_t> x_values =
    {uint(minVertex.X()), uint32_t(maxVertex.X())};
  std::vector<uint32_t> y_values =
    {uint(minVertex.Y()), uint32_t(maxVertex.Y())};

  for (uint32_t i = minVertex.Y(); i < maxVertex.Y(); i++)
  {
    for (auto j : x_values)
    {
      auto index = (i * width + j) * 3;
      _data[index] = 0;
      _data[index + 1] = 255;
      _data[index + 2] = 0;
    }
  }
  for (auto i : y_values)
  {
    for (uint32_t j = minVertex.X(); j < maxVertex.X(); j++)
    {
      auto index = (i * width + j) * 3;
      _data[index] = 0;
      _data[index + 1] = 255;
      _data[index + 2] = 0;
    }
  }
}

/////////////////////////////////////////////////
void Ogre2BoundingBoxCamera::ConvertToScreenCoord(
  Ogre::Vector3 &minVertex, Ogre::Vector3 &maxVertex)
{
  uint32_t width = this->ImageWidth();
  uint32_t height = this->ImageHeight();

  // clip the values outside the frustum range
  minVertex.x = std::clamp<double>(minVertex.x, -1.0, 1.0);
  minVertex.y = std::clamp<double>(minVertex.y, -1.0, 1.0);
  maxVertex.x = std::clamp<double>(maxVertex.x, -1.0, 1.0);
  maxVertex.y = std::clamp<double>(maxVertex.y, -1.0, 1.0);

  // convert from [-1, 1] range to [0, 1] range & multiply by screen dims
  minVertex.x = uint32_t((minVertex.x + 1.0) / 2 * width );
  minVertex.y = uint32_t((1.0 - minVertex.y) / 2 * height);
  maxVertex.x = uint32_t((maxVertex.x + 1.0) / 2 * width );
  maxVertex.y = uint32_t((1.0 - maxVertex.y) / 2 * height);

  // clip outside screen boundries
  minVertex.x = std::max<uint32_t>(0, minVertex.x);
  minVertex.y = std::max<uint32_t>(0, minVertex.y);
  maxVertex.x = std::min<uint32_t>(maxVertex.x, width - 1);
  maxVertex.y = std::min<uint32_t>(maxVertex.y, height - 1);
}

/////////////////////////////////////////////////
std::vector<BoundingBox> Ogre2BoundingBoxCamera::BoundingBoxData() const
{
    return this->dataPtr->output_boxes;
}

/////////////////////////////////////////////////
ignition::common::ConnectionPtr
  Ogre2BoundingBoxCamera::ConnectNewBoundingBoxes(
  std::function<void(const std::vector<BoundingBox> &)>  _subscriber)
{
  return this->dataPtr->newBoundingBoxes.Connect(_subscriber);
}

/////////////////////////////////////////////////
void Ogre2BoundingBoxCamera::CreateRenderTexture()
{
  RenderTexturePtr base = this->scene->CreateRenderTexture();
  this->dataPtr->dummyTexture =
    std::dynamic_pointer_cast<Ogre2RenderTexture>(base);
  this->dataPtr->dummyTexture->SetWidth(1);
  this->dataPtr->dummyTexture->SetHeight(1);
}

/////////////////////////////////////////////////
RenderTargetPtr Ogre2BoundingBoxCamera::RenderTarget() const
{
  return this->dataPtr->dummyTexture;
}

/////////////////////////////////////////////////
void Ogre2BoundingBoxCamera::SetBoundingBoxType(BoundingBoxType _type)
{
  this->dataPtr->type = _type;
}

