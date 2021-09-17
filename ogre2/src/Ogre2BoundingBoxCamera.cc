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

#include <random>
#include <limits>

#include <ignition/common/Console.hh>

#include <ignition/math/Color.hh>
#include <ignition/math/eigen3/Util.hh>
#include <ignition/math/OrientedBox.hh>

#include "ignition/rendering/ogre2/Ogre2Camera.hh"
#include "ignition/rendering/ogre2/Ogre2Conversions.hh"
#include "ignition/rendering/ogre2/Ogre2Includes.hh"
#include "ignition/rendering/ogre2/Ogre2RenderTarget.hh"
#include "ignition/rendering/ogre2/Ogre2Scene.hh"
#include "ignition/rendering/RenderTypes.hh"
#include "ignition/rendering/ogre2/Ogre2RenderTypes.hh"
#include "ignition/rendering/ogre2/Ogre2RenderEngine.hh"
#include "ignition/rendering/ogre2/Ogre2BoundingBoxCamera.hh"
#include "ignition/rendering/ogre2/Ogre2Visual.hh"
#include "ignition/rendering/Utils.hh"

#include <OgreBitwise.h>

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
    // class BoundingBoxMaterialSwitcher : public Ogre::RenderTargetListener
    class Ogre2BoundingBoxMaterialSwitcher : public Ogre::Camera::Listener
    {
      /// \brief Constructor
      public: explicit Ogre2BoundingBoxMaterialSwitcher(Ogre2ScenePtr _scene);

      /// \brief Destructor
      public: ~Ogre2BoundingBoxMaterialSwitcher();

      /// \brief Get the top level model visual of a particular visual
      /// \param[in] _visual The visual who's top level model visual we are
      /// interested in
      /// \return The top level model visual of _visual
      private: VisualPtr TopLevelModelVisual(VisualPtr _visual) const;

      /// \brief Callback when a camara is about to be rendered
      /// \param[in] _cam Ogre camera pointer which is about to render
      private: virtual void cameraPreRenderScene(
        Ogre::Camera * _cam) override;

      /// \brief Callback when a camera is finisned being rendered
      /// \param[in] _cam Ogre camera pointer which has already render
      private: virtual void cameraPostRenderScene(
        Ogre::Camera * _cam) override;

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
      private: const std::string labelKey = "label";

      /// \brief Label for background pixels in the ogre Ids map
      private: uint32_t backgroundLabel {255};

      /// \brief Map ogre ID to the top parent name of the item.
      /// used in multi-link models, key: ogreId, value: parent name
      private: std::map<uint32_t, std::string> ogreIdName;

      /// \brief Ogre2 Scene
      private: Ogre2ScenePtr scene;

      friend class Ogre2BoundingBoxCamera;
    };
    }
  }
}

/////////////////////////////////////////////////
class ignition::rendering::Ogre2BoundingBoxCameraPrivate
{
  /// \brief Material Switcher to switch item's material with ogre Ids
  /// For bounding boxes visibility checking & finding boundaires
  public: std::unique_ptr<Ogre2BoundingBoxMaterialSwitcher> materialSwitcher;

  /// \brief Compositor Manager to create workspace
  public: Ogre::CompositorManager2 *ogreCompositorManager {nullptr};

  /// \brief Workspace to interface with render texture
  public: Ogre::CompositorWorkspace *ogreCompositorWorkspace {nullptr};

  /// \brief Workspace Definition
  public: std::string workspaceDefinition;

  /// \brief Texture to create the render texture from.
  public: Ogre::TextureGpu *ogreRenderTexture {nullptr};

  /// \brief Buffer to store render texture data & to be sent to listeners
  public: uint8_t *buffer = nullptr;

  /// \brief Dummy render texture to set image dims
  public: Ogre2RenderTexturePtr dummyTexture {nullptr};

  /// \brief New BoundingBox Frame Event to notify listeners with new data
  public: ignition::common::EventT<void(const std::vector<BoundingBox> &)>
        newBoundingBoxes;

  /// \brief Image / Render Texture Format
  public: Ogre::PixelFormatGpu format = Ogre::PFG_RGBA8_UNORM;

  /// \brief map ogreId id to bounding box
  /// Key: ogreId, value: bounding box contains max & min boundaries
  public: std::map<uint32_t, BoundingBox *> boundingboxes;

  /// \brief Keep track of the visible bounding boxes (used in filtering)
  /// Key: ogreId, value: label id
  public: std::map<uint32_t, uint32_t> visibleBoxesLabel;

  /// \brief Map parent name of the visual to the boxes in it to merge them.
  /// Used in multi-link models, as each parent contains many boxes in it.
  /// Key: parent name, value: vector of boxes that belongs to that model.
  public: std::map<std::string, std::vector<BoundingBox *>> parentNameToBoxes;

  /// \brief Keep track of the visible bounding boxes (used in filtering)
  /// Key: parent name, value: vector of ogre ids of it's childern
  public: std::map<std::string, std::vector<uint32_t>> parentNameToOgreIds;

  /// \brief The ogre item's 3d vertices from the vao(used in multi-link models)
  /// Key: ogre id, value: vector of it's 3d vertices(pointcloud or mesh points)
  public: std::map<uint32_t, std::vector<math::Vector3d>> itemVertices;

  /// \brief Map ogre id to Ogre::Item (used in multi-link models)
  /// Key: ogre id, value: ogre item pointer
  public: std::map<uint32_t, Ogre::Item *> ogreIdToItem;

  /// \brief Output bounding boxes to nofity listeners
  public: std::vector<BoundingBox> outputBoxes;

  /// \brief Bounding Box type
  public: BoundingBoxType type {BoundingBoxType::BBT_VISIBLEBOX2D};
};

/////////////////////////////////////////////////
Ogre2BoundingBoxMaterialSwitcher::Ogre2BoundingBoxMaterialSwitcher(
    Ogre2ScenePtr _scene)
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
    ignerr << "Problem creating bounding box camera overlay material"
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
Ogre2BoundingBoxMaterialSwitcher::~Ogre2BoundingBoxMaterialSwitcher()
{
}

////////////////////////////////////////////////
VisualPtr Ogre2BoundingBoxMaterialSwitcher::TopLevelModelVisual(
    VisualPtr _visual) const
{
  if (!_visual)
    return _visual;
  VisualPtr p = _visual;
  while (p->Parent() && p->Parent() != _visual->Scene()->RootVisual())
    p = std::dynamic_pointer_cast<Visual>(p->Parent());
  return p;
}

////////////////////////////////////////////////
void Ogre2BoundingBoxMaterialSwitcher::cameraPreRenderScene(
    Ogre::Camera * /*_cam*/)
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

      // for full bbox, each pixel contains 1 channel for label
      // and 2 channels stores ogreId
      uint32_t ogreId = item->getId();

      float labelColor = label / 255.0;
      float ogreId1 = (ogreId / 256) / 255.0;
      float ogreId2 = (ogreId % 256) / 255.0;

      // Material color
      auto customParameter = Ogre::Vector4(ogreId2, ogreId1, labelColor, 1.0);

      // Multi-links models handeling
      auto itemName = visual->Name();
      std::string parentName = this->TopLevelModelVisual(visual)->Name();

      this->ogreIdName[ogreId] = parentName;

      // Switch material for all sub items
      for (unsigned int i = 0; i < item->getNumSubItems(); i++)
      {
        // save subitems material
        Ogre::SubItem *subItem = item->getSubItem(i);
        Ogre::HlmsDatablock *datablock = subItem->getDatablock();
        this->datablockMap[subItem] = datablock;

        subItem->setCustomParameter(1, customParameter);

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
void Ogre2BoundingBoxMaterialSwitcher::cameraPostRenderScene(
    Ogre::Camera * /*_cam*/)
{
  // restore the original material
  for (auto it : this->datablockMap)
  {
    Ogre::SubItem *subItem = it.first;
    subItem->setDatablock(it.second);
  }
}

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

  this->dataPtr->materialSwitcher = std::make_unique<
    Ogre2BoundingBoxMaterialSwitcher>(this->scene);
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
  if (this->dataPtr->buffer)
  {
    delete [] this->dataPtr->buffer;
    this->dataPtr->buffer = nullptr;
  }

  if (!this->ogreCamera)
    return;

  auto engine = Ogre2RenderEngine::Instance();
  auto ogreRoot = engine->OgreRoot();
  Ogre::CompositorManager2 *ogreCompMgr =
      ogreRoot->getCompositorManager2();

  // remove render texture, material, compositor
  if (this->dataPtr->ogreRenderTexture)
  {
    ogreRoot->getRenderSystem()->getTextureGpuManager()->destroyTexture(
          this->dataPtr->ogreRenderTexture);
    this->dataPtr->ogreRenderTexture = nullptr;
  }

  if (this->dataPtr->ogreCompositorWorkspace)
  {
    ogreCompMgr->removeWorkspace(
        this->dataPtr->ogreCompositorWorkspace);
  }

  if (!this->dataPtr->workspaceDefinition.empty())
  {
    ogreCompMgr->removeWorkspaceDefinition(
        this->dataPtr->workspaceDefinition);
  }

  Ogre::SceneManager *ogreSceneManager;
  ogreSceneManager = this->scene->OgreSceneManager();
  if (ogreSceneManager == nullptr)
  {
    ignerr << "Scene manager cannot be obtained" << std::endl;
  }
  else
  {
    if (ogreSceneManager->findCameraNoThrow(this->name) != nullptr)
    {
      ogreSceneManager->destroyCamera(this->ogreCamera);
      this->ogreCamera = nullptr;
    }
  }

  if (this->dataPtr->materialSwitcher)
    this->dataPtr->materialSwitcher.reset();
}

/////////////////////////////////////////////////
void Ogre2BoundingBoxCamera::PreRender()
{
  if (!this->dataPtr->ogreRenderTexture)
    this->CreateBoundingBoxTexture();

  this->dataPtr->outputBoxes.clear();
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

  // render texture
  auto engine = Ogre2RenderEngine::Instance();
  auto ogreRoot = engine->OgreRoot();
  Ogre::TextureGpuManager *textureMgr =
      ogreRoot->getRenderSystem()->getTextureGpuManager();
  this->dataPtr->ogreRenderTexture =
      textureMgr->createOrRetrieveTexture(this->Name() + "_boundingbox_cam",
      Ogre::GpuPageOutStrategy::SaveToSystemRam,
      Ogre::TextureFlags::RenderToTexture,
      Ogre::TextureTypes::Type2D);

  this->dataPtr->ogreRenderTexture->setResolution(
      this->ImageWidth(), this->ImageHeight());
  this->dataPtr->ogreRenderTexture->setNumMipmaps(1u);
  this->dataPtr->ogreRenderTexture->setPixelFormat(this->dataPtr->format);

  this->dataPtr->ogreRenderTexture->scheduleTransitionTo(
      Ogre::GpuResidency::Resident);

  // Switch material to OGRE Ids map to use it to get the visible bboxes
  // or to check visiblity in full bboxes
  this->ogreCamera->addListener(this->dataPtr->materialSwitcher.get());

  // workspace
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
}

/////////////////////////////////////////////////
void Ogre2BoundingBoxCamera::Render()
{
  // update the compositors
  this->scene->StartRendering();

  this->dataPtr->ogreCompositorWorkspace->_validateFinalTarget();
  this->dataPtr->ogreCompositorWorkspace->_beginUpdate(false);
  this->dataPtr->ogreCompositorWorkspace->_update();
  this->dataPtr->ogreCompositorWorkspace->_endUpdate(false);

  Ogre::vector<Ogre::TextureGpu *>::type swappedTargets;
  swappedTargets.reserve(2u);
  this->dataPtr->ogreCompositorWorkspace->_swapFinalTarget(swappedTargets);

  this->scene->FlushGpuCommandsAndStartNewFrame(1u, false);
}

/////////////////////////////////////////////////
void Ogre2BoundingBoxCamera::PostRender()
{
  // return if no one is listening to the new frame
  if (this->dataPtr->newBoundingBoxes.ConnectionCount() == 0)
    return;

  unsigned int width = this->ImageWidth();
  unsigned int height = this->ImageHeight();

  PixelFormat format = PF_R8G8B8;
  unsigned int channelCount = PixelUtil::ChannelCount(format);
  unsigned int bytesPerChannel = PixelUtil::BytesPerChannel(format);
  // raw gpu texture format is RGBA8
  unsigned int rawChannelCount = 4u;

  Ogre::Image2 image;
  image.convertFromTexture(this->dataPtr->ogreRenderTexture, 0u, 0u);
  Ogre::TextureBox box = image.getData(0);
  uint8_t *imgBufferTmp = static_cast<uint8_t *>(box.data);
  if (!this->dataPtr->buffer)
  {
    auto bufferSize = PixelUtil::MemorySize(format, width, height);
    this->dataPtr->buffer = new uint8_t[bufferSize];
  }

  for (unsigned int row = 0; row < height; ++row)
  {
    // the texture box step size could be larger than our image buffer step
    // size
    unsigned int rawDataRowIdx = row * box.bytesPerRow / bytesPerChannel;
    for (unsigned int column = 0; column < width; ++column)
    {
      unsigned int idx = (row * width * channelCount) +
          column * channelCount;
      unsigned int rawIdx = rawDataRowIdx +
          column * rawChannelCount;

      this->dataPtr->buffer[idx] = imgBufferTmp[rawIdx];
      this->dataPtr->buffer[idx + 1] = imgBufferTmp[rawIdx + 1];
      this->dataPtr->buffer[idx + 2] = imgBufferTmp[rawIdx + 2];
    }
  }

  if (this->dataPtr->type == BoundingBoxType::BBT_VISIBLEBOX2D)
    this->VisibleBoundingBoxes();
  else if (this->dataPtr->type == BoundingBoxType::BBT_FULLBOX2D)
    this->FullBoundingBoxes();
  else if (this->dataPtr->type == BoundingBoxType::BBT_BOX3D)
    this->BoundingBoxes3D();

  for (auto bbox : this->dataPtr->boundingboxes)
    delete bbox.second;

  this->dataPtr->boundingboxes.clear();
  this->dataPtr->visibleBoxesLabel.clear();
  this->dataPtr->parentNameToBoxes.clear();
  this->dataPtr->parentNameToOgreIds.clear();
  this->dataPtr->itemVertices.clear();
  this->dataPtr->ogreIdToItem.clear();
  this->dataPtr->materialSwitcher->ogreIdName.clear();

  this->dataPtr->newBoundingBoxes(this->dataPtr->outputBoxes);
}

/////////////////////////////////////////////////
void Ogre2BoundingBoxCamera::MarkVisibleBoxes()
{
  uint32_t width = this->ImageWidth();
  uint32_t height = this->ImageHeight();
  uint32_t channelCount = 3;

  // Filter bounding boxes by looping over all pixels in ogre ids map
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
}

/////////////////////////////////////////////////
void Ogre2BoundingBoxCamera::MeshVertices(
  std::vector<uint32_t> _ogreIds, std::vector<math::Vector3d> &_vertices)
{
  auto viewMatrix = this->ogreCamera->getViewMatrix();

  for (auto ogreId : _ogreIds)
  {
    Ogre::Item *item = this->dataPtr->ogreIdToItem[ogreId];
    Ogre::MeshPtr mesh = item->getMesh();
    Ogre::Node *node = item->getParentNode();

    Ogre::Vector3 position = node->_getDerivedPosition();
    Ogre::Quaternion oreintation = node->_getDerivedOrientation();
    Ogre::Vector3 scale = node->_getDerivedScale();

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

          // Convert to world coordinates
          vec = (oreintation * (vec * scale)) + position;

          // Convert to camera view coordiantes
          Ogre::Vector4 vec4(vec.x, vec.y, vec.z, 1);
          vec4 = viewMatrix * vec4;

          vec.x = vec4.x;
          vec.y = vec4.y;
          vec.z = vec4.z;

          // Add the vertex to the vertices of all items that
          // belongs to the same parent
          _vertices.push_back(Ogre2Conversions::Convert(vec));

          // get the next element
          requests[0].data += requests[0].vertexBuffer->getBytesPerElement();
        }
        vao->unmapAsyncTickets(requests);
      }
    }
  }
}

/////////////////////////////////////////////////
void Ogre2BoundingBoxCamera::MergeMultiLinksModels3D()
{
  // Combine the boxes with the same parent name together to merge them
  for (auto box : this->dataPtr->boundingboxes)
  {
    auto ogreId = box.first;
    auto parentName = this->dataPtr->materialSwitcher->ogreIdName[ogreId];
    this->dataPtr->parentNameToOgreIds[parentName].push_back(ogreId);
  }

  // Merge the boxes that is related to the same parent
  for (auto nameToOgreIds : this->dataPtr->parentNameToOgreIds)
  {
    auto ogreIds = nameToOgreIds.second;

    // If not a multi-link model, add the 3d box from the OGRE API
    if (ogreIds.size() == 1)
    {
      auto box = this->dataPtr->boundingboxes[ogreIds[0]];
      this->dataPtr->outputBoxes.push_back(*box);
    }
    else
    {
      std::vector<math::Vector3d> vertices;

      // Get all the 3D vertices of the sub-items(total mesh)
      this->MeshVertices(ogreIds, vertices);

      // Get the oriented bounding box from the mesh using PCA
      math::OrientedBoxd mergedBox = math::eigen3::verticesToOrientedBox(
        vertices);

      // convert to rendering::BoundingBox format
      BoundingBox box(BoundingBoxType::BBT_BOX3D);
      auto pose = mergedBox.Pose();
      box.center = pose.Pos();
      box.orientation = pose.Rot();
      box.size = mergedBox.Size();
      box.label = this->dataPtr->visibleBoxesLabel[ogreIds[0]];

      this->dataPtr->outputBoxes.push_back(box);
    }
  }

  // reverse the order of the boxes (useful in testing)
  std::reverse(this->dataPtr->outputBoxes.begin(),
    this->dataPtr->outputBoxes.end());
}

/////////////////////////////////////////////////
void Ogre2BoundingBoxCamera::MergeMultiLinksModels2D()
{
  // Combine the boxes with the same parent name together to merge them
  for (auto box : this->dataPtr->boundingboxes)
  {
    auto ogreId = box.first;
    auto parentName = this->dataPtr->materialSwitcher->ogreIdName[ogreId];
    this->dataPtr->parentNameToBoxes[parentName].push_back(box.second);
  }

  // Merge the boxes that is related to the same parent
  for (auto nameToBoxes : this->dataPtr->parentNameToBoxes)
  {
    auto mergedBox = this->MergeBoxes2D(nameToBoxes.second);

    // Store boxes in the output vector
    this->dataPtr->outputBoxes.push_back(mergedBox);
  }

  // reverse the order of the boxes (usful in testing)
  std::reverse(this->dataPtr->outputBoxes.begin(),
    this->dataPtr->outputBoxes.end());
}

/////////////////////////////////////////////////
BoundingBox Ogre2BoundingBoxCamera::MergeBoxes2D(
        const std::vector<BoundingBox *> &_boxes)
{
  if (_boxes.size() == 1)
    return *_boxes[0];

  BoundingBox mergedBox(this->dataPtr->type);
  uint32_t minX = UINT32_MAX;
  uint32_t maxX = 0;
  uint32_t minY = UINT32_MAX;
  uint32_t maxY = 0;

  for (auto box : _boxes)
  {
    uint32_t boxMinX = box->center.X() - box->size.X() / 2;
    uint32_t boxMaxX = box->center.X() + box->size.X() / 2;
    uint32_t boxMinY = box->center.Y() - box->size.Y() / 2;
    uint32_t boxMaxY = box->center.Y() + box->size.Y() / 2;

    minX = std::min<uint32_t>(minX, boxMinX);
    maxX = std::max<uint32_t>(maxX, boxMaxX);
    minY = std::min<uint32_t>(minY, boxMinY);
    maxY = std::max<uint32_t>(maxY, boxMaxY);
  }

  uint32_t width = maxX - minX;
  uint32_t height = maxY - minY;
  mergedBox.size.Set(width, height);
  mergedBox.center.Set(minX + width / 2, minY + height / 2);
  mergedBox.label = _boxes[0]->label;

  return mergedBox;
}

/////////////////////////////////////////////////
void Ogre2BoundingBoxCamera::BoundingBoxes3D()
{
  auto viewMatrix = this->ogreCamera->getViewMatrix();

  // used to filter the hidden boxes
  this->MarkVisibleBoxes();

  auto itor = this->scene->OgreSceneManager()->getMovableObjectIterator(
      Ogre::ItemFactory::FACTORY_TYPE_NAME);
  while (itor.hasMoreElements())
  {
    Ogre::MovableObject *object = itor.peekNext();
    Ogre::Item *item = static_cast<Ogre::Item *>(object);
    Ogre::MeshPtr mesh = item->getMesh();

    uint32_t ogreId = item->getId();

    // Skip the items which is hidden from the ogreId map
    if (!this->dataPtr->visibleBoxesLabel.count(ogreId))
    {
      itor.moveNext();
      continue;
    }

    // get attached node
    Ogre::Node *node = item->getParentNode();

    Ogre::Quaternion orientation = node->_getDerivedOrientation();
    Ogre::Vector3 scale = node->_getDerivedScale();
    Ogre::Vector3 size = item->getLocalAabb().getSize();
    size *= scale;

    Ogre::Aabb aabb = item->getWorldAabb();

    // filter the boxes outside the camera frustum
    Ogre::AxisAlignedBox worldAabb;
    worldAabb.setExtents(aabb.getMinimum(), aabb.getMaximum());
    if (!this->ogreCamera->isVisible(worldAabb))
    {
      itor.moveNext();
      continue;
    }

    // Keep track of mesh, useful in multi-links models
    this->dataPtr->ogreIdToItem[ogreId] = item;

    BoundingBox *box = new BoundingBox(BoundingBoxType::BBT_BOX3D);

    // Position in world coord
    Ogre::Vector3 position = worldAabb.getCenter();

    // Position in camera coord
    Ogre::Vector3 viewPosition = viewMatrix * position;

    // Convert to ignition::math
    box->center = Ogre2Conversions::Convert(viewPosition);
    box->size = Ogre2Conversions::Convert(size);

    // Compute the rotation of the box from its world rotation & view matrix
    auto worldCameraRotation = Ogre2Conversions::Convert(
      viewMatrix.extractQuaternion());
    auto bodyWorldRotation = Ogre2Conversions::Convert(orientation);

    // Body to camera rotation = body_world * world_camera
    auto bodyCameraRotation = worldCameraRotation * bodyWorldRotation;
    box->orientation = bodyCameraRotation;

    this->dataPtr->boundingboxes[ogreId] = box;
    itor.moveNext();
  }

  // Set boxes labels
  for (auto box : this->dataPtr->boundingboxes)
  {
    uint32_t ogreId = box.first;
    uint32_t label = this->dataPtr->visibleBoxesLabel[ogreId];

    box.second->label = label;
  }

  // Combine boxes of multi-links model if exists
  this->MergeMultiLinksModels3D();
}

/////////////////////////////////////////////////
void Ogre2BoundingBoxCamera::VisibleBoundingBoxes()
{
  uint32_t width = this->ImageWidth();
  uint32_t height = this->ImageHeight();
  uint32_t channelCount = 3;

  struct BoxBoundary
  {
    uint32_t minX;
    uint32_t minY;
    uint32_t maxX;
    uint32_t maxY;
  };

  std::unordered_map<uint32_t, BoxBoundary*> boxesBoundary;

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
        BoxBoundary *boundary;

        // create new boxes when its first pixel appears
        if (!this->dataPtr->boundingboxes.count(ogreId))
        {
          box = new BoundingBox(BoundingBoxType::BBT_VISIBLEBOX2D);
          box->label = label;

          boundary = new BoxBoundary();
          boundary->minX = width;
          boundary->minY = height;
          boundary->maxX = 0;
          boundary->maxY = 0;

          boxesBoundary[ogreId] = boundary;
          this->dataPtr->boundingboxes[ogreId] = box;
        }
        else
        {
          boundary = boxesBoundary[ogreId];
        }

        boundary->minX = std::min<uint32_t>(boundary->minX, x);
        boundary->minY = std::min<uint32_t>(boundary->minY, y);
        boundary->maxX = std::max<uint32_t>(boundary->maxX, x);
        boundary->maxY = std::max<uint32_t>(boundary->maxY, y);
      }
    }
  }

  for (auto box : this->dataPtr->boundingboxes)
  {
    // Get the box's boundary
    auto ogreId = box.first;
    auto boundary = boxesBoundary[ogreId];
    auto boxWidth = boundary->maxX - boundary->minX;
    auto boxHeight = boundary->maxY - boundary->minY;

    box.second->center.X() = boundary->minX + boxWidth / 2;
    box.second->center.Y() = boundary->minY + boxHeight / 2;
    box.second->center.Z() = 0;

    box.second->size.X() = boxWidth;
    box.second->size.Y() = boxHeight;
    box.second->size.Z() = 0;
  }

  for (auto bb : boxesBoundary)
    delete bb.second;

  // Combine boxes of multi-links model if exists
  this->MergeMultiLinksModels2D();
}

/////////////////////////////////////////////////
void Ogre2BoundingBoxCamera::FullBoundingBoxes()
{
  // used to filter the hidden boxes
  this->MarkVisibleBoxes();

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

    // get the derived position
    Ogre::Vector3 position = node->_getDerivedPosition();
    Ogre::Quaternion oreintation = node->_getDerivedOrientation();
    Ogre::Vector3 scale = node->_getDerivedScale();

    Ogre::Aabb aabb = item->getWorldAabb();
    Ogre::AxisAlignedBox worldAabb;
    worldAabb.setExtents(aabb.getMinimum(), aabb.getMaximum());

    // filter the boxes outside the camera frustum
    if (!this->ogreCamera->isVisible(worldAabb))
    {
      itor.moveNext();
      continue;
    }

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

    BoundingBox *box = new BoundingBox(BoundingBoxType::BBT_FULLBOX2D);
    auto boxWidth = maxVertex.x - minVertex.x;
    auto boxHeight = minVertex.y - maxVertex.y;
    box->center.X() = minVertex.x + boxWidth / 2;
    box->center.Y() = maxVertex.y + boxHeight / 2;
    box->center.Z() = 0;
    box->size.X() = boxWidth;
    box->size.Y() = boxHeight;
    box->size.Z() = 0;

    this->dataPtr->boundingboxes[ogreId] = box;

    itor.moveNext();
  }

  // Set boxes label
  for (auto box : this->dataPtr->boundingboxes)
  {
    uint32_t ogreId = box.first;
    uint32_t label = this->dataPtr->visibleBoxesLabel[ogreId];

    box.second->label = label;
  }

  // Combine boxes of multi-links model if exists
  this->MergeMultiLinksModels2D();
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

        Ogre::Vector4 vec4(vec.x, vec.y, vec.z, 1);
        vec4 =  projMatrix * viewMatrix * vec4;

        // homogenous
        vec.x = vec4.x / vec4.w;
        vec.y = vec4.y / vec4.w;
        vec.z = vec4.z;

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
void Ogre2BoundingBoxCamera::DrawLine(unsigned char *_data,
  const math::Vector2i _point1, const math::Vector2i _point2)
{
  int x0, y0, x1, y1;

  // Check if the line is close to a vertical or horizontal line
  if (std::abs(_point2.Y() - _point1.Y())
    < std::abs(_point2.X() - _point1.X()))
  {
    if (_point1.X() < _point2.X())
    {
      x0 = _point1.X();
      y0 = _point1.Y();
      x1 = _point2.X();
      y1 = _point2.Y();
    }
    else
    {
      x0 = _point2.X();
      y0 = _point2.Y();
      x1 = _point1.X();
      y1 = _point1.Y();
    }
    auto dx = x1 - x0;
    auto dy = y1 - y0;
    auto yi = 1;
    if (dy < 0)
    {
      yi = -1;
      dy = -dy;
    }
    auto D = 2 * dy - dx;
    auto y = y0;

    for (int x = x0; x < x1; x++)
    {
      // Plot the point
      auto index = (y * this->ImageWidth() + x) * 3;
      _data[index] = 0;
      _data[index + 1] = 255;
      _data[index + 2] = 0;

      if (D > 0)
      {
        y += yi;
        D += (2 * (dy - dx));
      }
      else
      {
        D += (2 * dy);
      }
    }
  }
  else
  {
    if (_point1.Y() < _point2.Y())
    {
      x0 = _point1.X();
      y0 = _point1.Y();
      x1 = _point2.X();
      y1 = _point2.Y();
    }
    else
    {
      x0 = _point2.X();
      y0 = _point2.Y();
      x1 = _point1.X();
      y1 = _point1.Y();
    }
    auto dx = x1 - x0;
    auto dy = y1 - y0;
    auto xi = 1;
    if (dx < 0)
    {
      xi = -1;
      dx = -dx;
    }
    auto D = 2 * dx - dy;
    auto x = x0;

    for (int y = y0; y < y1; y++)
    {
      // Plot the point
      auto index = (y * this->ImageWidth() + x) * 3;
      _data[index] = 0;
      _data[index + 1] = 255;
      _data[index + 2] = 0;

      if (D > 0)
      {
        x += xi;
        D += (2 * (dx - dy));
      }
      else
      {
        D += (2 * dx);
      }
    }
  }
}

/////////////////////////////////////////////////
void Ogre2BoundingBoxCamera::DrawBoundingBox(
  unsigned char *_data, const BoundingBox &_box)
{
  // 3D box
  if (_box.type == BoundingBoxType::BBT_BOX3D)
  {
    // Get the 3D vertices of the box in 3D camera coord.
    auto vertices = _box.Vertices();

    // Project the 3D vertices in 3D camera coord
    // to 2D vertices in clip coord[-1,1]
    auto projMatrix = this->ogreCamera->getProjectionMatrix();
    for (auto &vertex : vertices)
    {
      // Convert to homogeneous coord.
      auto homoVertex =
        Ogre::Vector4(vertex.X(), vertex.Y(), vertex.Z(), 1);

      auto projVertex = projMatrix * homoVertex;
      projVertex.x /= projVertex.w;
      projVertex.y /= projVertex.w;

      vertex = math::Vector3d(projVertex.x, projVertex.y, projVertex.z);
    }

    std::vector<math::Vector2i> projVertices;

    // Convert To screen coord.
    uint32_t width = this->ImageWidth();
    uint32_t height = this->ImageHeight();
    for (auto &vertex : vertices)
    {
      // clip the values outside the frustum range [-1, 1]
      vertex.X() = std::clamp<double>(vertex.X(), -1.0, 1.0);
      vertex.Y() = std::clamp<double>(vertex.Y(), -1.0, 1.0);

      // convert from [-1, 1] range to [0, 1] range & to the screen range
      vertex.X() = uint32_t((vertex.X() + 1.0) / 2 * width );
      vertex.Y() = uint32_t((1.0 - vertex.Y()) / 2 * height);

      vertex.X() = std::max<uint32_t>(0, vertex.X());
      vertex.Y() = std::max<uint32_t>(0, vertex.Y());
      vertex.X() = std::min<uint32_t>(vertex.X(), width - 1);
      vertex.Y() = std::min<uint32_t>(vertex.Y(), height - 1);

      projVertices.push_back(math::Vector2i(vertex.X(), vertex.Y()));
    }

    // // Uncomment to debug the projected 2D points of the 3D box
    // for (auto &vertex : vertices)
    // {
    //   auto index = static_cast<uint32_t>(
    //     (vertex.Y() * width + vertex.X()) * 3);
    //   _data[index] = 0;
    //   _data[index + 1] = 255;
    //   _data[index + 2] = 0;
    // }
    // return;

    /* Draw every line in the 3D box according to that structure

        1 -------- 0
        /|         /|
      2 -------- 3 .
      | |        | |
      . 5 -------- 4
      |/         |/
      6 -------- 7
    */

    // Upper rectangle
    this->DrawLine(_data, projVertices[0], projVertices[1]);
    this->DrawLine(_data, projVertices[1], projVertices[2]);
    this->DrawLine(_data, projVertices[2], projVertices[3]);
    this->DrawLine(_data, projVertices[3], projVertices[0]);

    // Lower rectangle
    this->DrawLine(_data, projVertices[4], projVertices[5]);
    this->DrawLine(_data, projVertices[5], projVertices[6]);
    this->DrawLine(_data, projVertices[6], projVertices[7]);
    this->DrawLine(_data, projVertices[7], projVertices[4]);

    // Pillars
    this->DrawLine(_data, projVertices[0], projVertices[4]);
    this->DrawLine(_data, projVertices[1], projVertices[5]);
    this->DrawLine(_data, projVertices[2], projVertices[6]);
    this->DrawLine(_data, projVertices[3], projVertices[7]);

    return;
  }

  // 2D box
  math::Vector2 minVertex(_box.center.X() - _box.size.X() / 2,
    _box.center.Y() - _box.size.Y() / 2);

  math::Vector2 maxVertex(_box.center.X() + _box.size.X() / 2,
    _box.center.Y() + _box.size.Y() / 2);

  uint32_t width = this->ImageWidth();

  std::vector<uint32_t> x_values =
    {uint32_t(minVertex.X()), uint32_t(maxVertex.X())};
  std::vector<uint32_t> y_values =
    {uint32_t(minVertex.Y()), uint32_t(maxVertex.Y())};

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
    return this->dataPtr->outputBoxes;
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

/////////////////////////////////////////////////
BoundingBoxType Ogre2BoundingBoxCamera::Type() const
{
  return this->dataPtr->type;
}
