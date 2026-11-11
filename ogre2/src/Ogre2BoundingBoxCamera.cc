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

#include <limits>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:5033)
#endif
#include <OgreBitwise.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <gz/common/Console.hh>
#include <gz/common/Profiler.hh>

#include <gz/math/Color.hh>
#include <gz/math/Vector4.hh>
#include <gz/math/eigen3/Util.hh>
#include <gz/math/Matrix4.hh>
#include <gz/math/OrientedBox.hh>

#include "gz/rendering/RenderTypes.hh"
#include "gz/rendering/Utils.hh"
#include "gz/rendering/ogre2/Ogre2BoundingBoxCamera.hh"
#include "gz/rendering/ogre2/Ogre2Camera.hh"
#include "gz/rendering/ogre2/Ogre2Conversions.hh"
#include "gz/rendering/ogre2/Ogre2Includes.hh"
#include "gz/rendering/ogre2/Ogre2RenderEngine.hh"
#include "gz/rendering/ogre2/Ogre2RenderTarget.hh"
#include "gz/rendering/ogre2/Ogre2RenderTypes.hh"
#include "gz/rendering/ogre2/Ogre2Scene.hh"
#include "gz/rendering/ogre2/Ogre2Visual.hh"

#include "Ogre2BoundingBoxMaterialSwitcher.hh"

using namespace gz;
using namespace rendering;

class gz::rendering::Ogre2BoundingBoxCameraPrivate
{
  /// \brief Get the 3d vertices (in world coord.) of the item's that
  /// belongs to the same parent (only used in multi-links models)
  /// \param[in] _ogreIds vector of ogre ids that belongs to the same model
  /// \param[out] _vertices vector of 3d vertices of the item
  public: void MeshVertices(const std::vector<uint32_t> &_ogreIds,
              std::vector<math::Vector3d> &_vertices);

  /// \brief Material Switcher to switch item's material with ogre Ids
  /// For bounding boxes visibility checking & finding boundaires
  public: std::unique_ptr<Ogre2BoundingBoxMaterialSwitcher> materialSwitcher;

  /// \brief Workspace to interface with render texture
  public: Ogre::CompositorWorkspace *ogreCompositorWorkspace {nullptr};

  /// \brief Workspace Definition
  public: std::string workspaceDefinition;

  /// \brief Pointer to the ogre camera
  public: Ogre::Camera *ogreCamera{nullptr};

  /// \brief Texture to create the render texture from.
  public: Ogre::TextureGpu *ogreRenderTexture {nullptr};

  /// \brief Buffer to store render texture data & to be sent to listeners
  public: uint8_t *buffer = nullptr;

  /// \brief Dummy render texture to set image dims
  public: Ogre2RenderTexturePtr dummyTexture {nullptr};

  /// \brief New BoundingBox Frame Event to notify listeners with new data
  public: common::EventT<void(const std::vector<BoundingBox> &)>
        newBoundingBoxes;

  /// \brief Image / Render Texture Format
  public: Ogre::PixelFormatGpu format = Ogre::PFG_RGBA8_UNORM;

  /// \brief map ogreId id to bounding box
  /// Key: ogreId, value: bounding box contains max & min boundaries
  public: std::map<uint32_t, BoundingBox> boundingboxes;

  /// \brief Keep track of the visible bounding boxes (used in filtering)
  /// Key: ogreId, value: label id
  public: std::map<uint32_t, uint32_t> visibleBoxesLabel;

  /// \brief Keep track of the visible bounding boxes (used in filtering)
  /// Key: parent name, value: vector of ogre ids of it's children
  public: std::map<std::string, std::vector<uint32_t>> parentNameToOgreIds;

  /// \brief The ogre item's 3d vertices from the vao(used in multi-link models)
  /// Key: ogre id, value: vector of it's 3d vertices(pointcloud or mesh points)
  public: std::map<uint32_t, std::vector<math::Vector3d>> itemVertices;

  /// \brief Map ogre id to Ogre::Item (used in multi-link models)
  /// Key: ogre id, value: ogre item pointer
  public: std::map<uint32_t, Ogre::Item *> ogreIdToItem;

  /// \brief Output bounding boxes to notify listeners
  public: std::vector<BoundingBox> outputBoxes;

  /// \brief Bounding Box type
  public: BoundingBoxType type {BoundingBoxType::BBT_VISIBLEBOX2D};
};

/////////////////////////////////////////////////
Ogre2BoundingBoxCamera::Ogre2BoundingBoxCamera() :
  dataPtr(std::make_unique<Ogre2BoundingBoxCameraPrivate>())
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
    gzerr << "Scene manager cannot be obtained" << std::endl;
    return;
  }

  this->dataPtr->ogreCamera = ogreScene->createCamera(this->Name());
  if (this->dataPtr->ogreCamera == nullptr)
  {
    gzerr << "Ogre camera cannot be created" << std::endl;
    return;
  }

  this->dataPtr->ogreCamera->detachFromParent();
  this->ogreNode->attachObject(this->dataPtr->ogreCamera);

  // rotate to Gazebo coord.
  this->dataPtr->ogreCamera->yaw(Ogre::Degree(-90));
  this->dataPtr->ogreCamera->roll(Ogre::Degree(-90));
  this->dataPtr->ogreCamera->setFixedYawAxis(false);

  this->dataPtr->ogreCamera->setRenderingDistance(100);
  this->dataPtr->ogreCamera->setProjectionType(
      Ogre::ProjectionType::PT_PERSPECTIVE);
  this->dataPtr->ogreCamera->setCustomProjectionMatrix(false);
}

/////////////////////////////////////////////////
void Ogre2BoundingBoxCamera::Destroy()
{
  this->RemoveAllRenderPasses();

  if (this->dataPtr->buffer)
  {
    delete [] this->dataPtr->buffer;
    this->dataPtr->buffer = nullptr;
  }

  if (!this->dataPtr->ogreCamera)
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
    gzerr << "Scene manager cannot be obtained" << std::endl;
  }
  else
  {
    if (ogreSceneManager->findCameraNoThrow(this->name) != nullptr)
    {
      ogreSceneManager->destroyCamera(this->dataPtr->ogreCamera);
      this->dataPtr->ogreCamera = nullptr;
    }
  }

  if (this->dataPtr->materialSwitcher)
    this->dataPtr->materialSwitcher.reset();
}

/////////////////////////////////////////////////
void Ogre2BoundingBoxCamera::PreRender()
{
  GZ_PROFILE("Ogre2BoundingBoxCamera::PreRender");
  if (!this->dataPtr->ogreRenderTexture)
    this->CreateBoundingBoxTexture();

  this->dataPtr->outputBoxes.clear();
}

/////////////////////////////////////////////////
void Ogre2BoundingBoxCamera::CreateBoundingBoxTexture()
{
  GZ_PROFILE("Ogre2BoundingBoxCamera::CreateBoundingBoxTexture");
  // Camera Parameters
  this->dataPtr->ogreCamera->setNearClipDistance(this->NearClipPlane());
  this->dataPtr->ogreCamera->setFarClipDistance(this->FarClipPlane());
  const double aspectRatio = this->AspectRatio();
  const double angle = this->HFOV().Radian();
  const double vfov = 2.0 * atan(tan(angle / 2.0) / aspectRatio);
  this->dataPtr->ogreCamera->setFOVy(Ogre::Radian((Ogre::Real)vfov));
  this->dataPtr->ogreCamera->setAspectRatio((Ogre::Real)aspectRatio);

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

  if (!this->dataPtr->ogreRenderTexture)
  {
    gzerr << "Null render texture" << std::endl;
    return;
  }

  this->dataPtr->ogreRenderTexture->setResolution(
      this->ImageWidth(), this->ImageHeight());
  this->dataPtr->ogreRenderTexture->setNumMipmaps(1u);
  this->dataPtr->ogreRenderTexture->setPixelFormat(this->dataPtr->format);

  this->dataPtr->ogreRenderTexture->scheduleTransitionTo(
      Ogre::GpuResidency::Resident);

  // Switch material to OGRE Ids map to use it to get the visible bboxes
  // or to check visibility in full bboxes
  this->dataPtr->ogreCamera->addListener(this->dataPtr->materialSwitcher.get());

  // workspace
  auto ogreCompositorManager = ogreRoot->getCompositorManager2();
  if (ogreCompositorManager == nullptr)
  {
    gzerr << "Null Ogre compositor manager." << std::endl;
    return;
  }

  this->dataPtr->workspaceDefinition = "BoundingBoxCameraWorkspace_" +
    this->Name();

  float background = static_cast<float>(
      this->dataPtr->materialSwitcher->backgroundLabel);
  auto backgroundColor = Ogre::ColourValue(background, background, background);

  // basic workspace consist of clear pass with the givin color &
  // a render scene pass to the givin render texture
  ogreCompositorManager->createBasicWorkspaceDef(
    this->dataPtr->workspaceDefinition,
    backgroundColor
    );

  // connect the compositor with the render texture to render the final output
  this->dataPtr->ogreCompositorWorkspace =
    ogreCompositorManager->addWorkspace(
      this->scene->OgreSceneManager(),
      this->dataPtr->ogreRenderTexture,
      this->dataPtr->ogreCamera,
      this->dataPtr->workspaceDefinition,
      false
    );
}

/////////////////////////////////////////////////
void Ogre2BoundingBoxCamera::Render()
{
  GZ_PROFILE("Ogre2BoundingBoxCamera::Render");
  if (!this->scene)
  {
    gzerr << "Null scene." << std::endl;
    return;
  }
  if (!this->dataPtr->ogreCompositorWorkspace)
  {
    gzerr << "Null Ogre compositor workspace." << std::endl;
    return;
  }

  // update the compositors
  this->scene->StartRendering(nullptr);

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
  GZ_PROFILE("Ogre2BoundingBoxCamera::PostRender");
  // return if no one is listening to the new frame
  if (this->dataPtr->newBoundingBoxes.ConnectionCount() == 0)
    return;

  if (!this->dataPtr->ogreRenderTexture)
  {
    gzerr << "Null render texture" << std::endl;
    return;
  }

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

  this->dataPtr->boundingboxes.clear();
  this->dataPtr->visibleBoxesLabel.clear();
  this->dataPtr->parentNameToOgreIds.clear();
  this->dataPtr->itemVertices.clear();
  this->dataPtr->ogreIdToItem.clear();
  this->dataPtr->materialSwitcher->ogreIdName.clear();

  this->dataPtr->newBoundingBoxes(this->dataPtr->outputBoxes);
}

/////////////////////////////////////////////////
void Ogre2BoundingBoxCamera::MarkVisibleBoxes()
{
  GZ_PROFILE("Ogre2BoundingBoxCamera::MarkVisibleBoxes");
  if (!this->dataPtr->buffer)
  {
    gzerr << "Null buffer" << std::endl;
    return;
  }

  // Filter bounding boxes by looping over all pixels in ogre ids map
  this->dataPtr->visibleBoxesLabel = this->VisibleBoxesLabel(
      this->dataPtr->buffer, this->dataPtr->materialSwitcher->backgroundLabel);
}

/////////////////////////////////////////////////
void Ogre2BoundingBoxCameraPrivate::MeshVertices(
    const std::vector<uint32_t> &_ogreIds,
    std::vector<math::Vector3d> &_vertices)
{
  GZ_PROFILE("Ogre2BoundingBoxCameraPrivate::MeshVertices");
  auto viewMatrix = this->ogreCamera->getViewMatrix();

  for (const auto &ogreId : _ogreIds)
  {
    Ogre::Item *item = this->ogreIdToItem[ogreId];
    Ogre::MeshPtr mesh = item->getMesh();
    Ogre::Node *node = item->getParentNode();

    Ogre::Vector3 position = node->_getDerivedPosition();
    Ogre::Quaternion oreintation = node->_getDerivedOrientation();
    Ogre::Vector3 scale = node->_getDerivedScale();

    auto subMeshes = mesh->getSubMeshes();

    for (const auto &subMesh : subMeshes)
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
            gzerr << "Vertex Buffer type error" << std::endl;

          // Convert to world coordinates
          vec = (oreintation * (vec * scale)) + position;

          // Convert to camera view coordinates
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
  GZ_PROFILE("Ogre2BoundingBoxCamera::MergeMultiLinksModels3D");
  // Combine the boxes with the same parent name together to merge them
  for (const auto &box : this->dataPtr->boundingboxes)
  {
    auto ogreId = box.first;
    auto parentName = this->dataPtr->materialSwitcher->ogreIdName[ogreId];
    this->dataPtr->parentNameToOgreIds[parentName].push_back(ogreId);
  }

  // Merge the boxes that is related to the same parent
  for (const auto &nameToOgreIds : this->dataPtr->parentNameToOgreIds)
  {
    auto ogreIds = nameToOgreIds.second;

    // If not a multi-link model, add the 3d box from the OGRE API
    if (ogreIds.size() == 1)
    {
      this->dataPtr->outputBoxes.push_back(
          this->dataPtr->boundingboxes[ogreIds[0]]);
    }
    else
    {
      std::vector<math::Vector3d> vertices;

      // Get all the 3D vertices of the sub-items(total mesh)
      this->dataPtr->MeshVertices(ogreIds, vertices);

      // Get the oriented bounding box from the mesh using PCA
      math::OrientedBoxd mergedBox = math::eigen3::verticesToOrientedBox(
        vertices);

      // convert to rendering::BoundingBox format
      BoundingBox box;
      auto pose = mergedBox.Pose();
      box.SetCenter(pose.Pos());
      box.SetOrientation(pose.Rot());
      box.SetSize(mergedBox.Size());
      box.SetLabel(this->dataPtr->visibleBoxesLabel[ogreIds[0]]);

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
  GZ_PROFILE("Ogre2BoundingBoxCamera::MergeMultiLinksModels2D");
  BaseBoundingBoxCamera::MergeMultiLinksModels2D(this->dataPtr->boundingboxes,
      [this](uint32_t _ogreId)
      {
        return this->dataPtr->materialSwitcher->ogreIdName[_ogreId];
      },
      this->dataPtr->outputBoxes);
}

/////////////////////////////////////////////////
void Ogre2BoundingBoxCamera::BoundingBoxes3D()
{
  GZ_PROFILE("Ogre2BoundingBoxCamera::BoundingBoxes3D");
  auto viewMatrix = this->dataPtr->ogreCamera->getViewMatrix();

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
    if (!this->dataPtr->ogreCamera->isVisible(worldAabb))
    {
      itor.moveNext();
      continue;
    }

    // Keep track of mesh, useful in multi-links models
    this->dataPtr->ogreIdToItem[ogreId] = item;

    BoundingBox box;

    // Position in world coord
    Ogre::Vector3 position = worldAabb.getCenter();

    // Position in camera coord
    Ogre::Vector3 viewPosition = viewMatrix * position;

    // Convert to gz::math
    box.SetCenter(Ogre2Conversions::Convert(viewPosition));
    box.SetSize(Ogre2Conversions::Convert(size));

    // Compute the rotation of the box from its world rotation & view matrix
    auto worldCameraRotation = Ogre2Conversions::Convert(
      viewMatrix.extractQuaternion());
    auto bodyWorldRotation = Ogre2Conversions::Convert(orientation);

    // Body to camera rotation = body_world * world_camera
    auto bodyCameraRotation = worldCameraRotation * bodyWorldRotation;
    box.SetOrientation(bodyCameraRotation);

    this->dataPtr->boundingboxes[ogreId] = box;
    itor.moveNext();
  }

  // Set boxes labels
  for (auto &box : this->dataPtr->boundingboxes)
  {
    uint32_t ogreId = box.first;
    uint32_t label = this->dataPtr->visibleBoxesLabel[ogreId];

    box.second.SetLabel(label);
  }

  // Combine boxes of multi-links model if exists
  this->MergeMultiLinksModels3D();
}

/////////////////////////////////////////////////
void Ogre2BoundingBoxCamera::VisibleBoundingBoxes()
{
  GZ_PROFILE("Ogre2BoundingBoxCamera::VisibleBoundingBoxes");
  if (!this->dataPtr->buffer)
  {
    gzerr << "Null buffer" << std::endl;
    return;
  }

  // find item's boundaries from panoptic BoundingBox
  this->dataPtr->boundingboxes = this->VisibleBoxes2D(
      this->dataPtr->buffer, this->dataPtr->materialSwitcher->backgroundLabel);

  // Combine boxes of multi-links model if exists
  this->MergeMultiLinksModels2D();
}

/////////////////////////////////////////////////
void Ogre2BoundingBoxCamera::FullBoundingBoxes()
{
  GZ_PROFILE("Ogre2BoundingBoxCamera::FullBoundingBoxes");
  // used to filter the hidden boxes
  this->MarkVisibleBoxes();

  Ogre::Matrix4 viewMatrix = this->dataPtr->ogreCamera->getViewMatrix();
  Ogre::Matrix4 projMatrix = this->dataPtr->ogreCamera->getProjectionMatrix();

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
    if (!this->dataPtr->ogreCamera->isVisible(worldAabb))
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

    BoundingBox box;
    auto boxWidth = maxVertex.x - minVertex.x;
    auto boxHeight = minVertex.y - maxVertex.y;
    box.SetCenter(
        {minVertex.x + boxWidth / 2, maxVertex.y + boxHeight / 2, 0});
    box.SetSize({boxWidth, boxHeight, 0});

    this->dataPtr->boundingboxes[ogreId] = box;

    itor.moveNext();
  }

  // Set boxes label
  for (auto &box : this->dataPtr->boundingboxes)
  {
    uint32_t ogreId = box.first;
    uint32_t label = this->dataPtr->visibleBoxesLabel[ogreId];

    box.second.SetLabel(label);
  }

  // Combine boxes of multi-links model if exists
  this->MergeMultiLinksModels2D();
}

/////////////////////////////////////////////////
void Ogre2BoundingBoxCamera::MeshMinimalBox(
  const Ogre::MeshPtr _mesh,
  const Ogre::Matrix4 &_viewMatrix,
  const Ogre::Matrix4 &_projMatrix,
  Ogre::Vector3 &_minVertex,
  Ogre::Vector3 &_maxVertex,
  const Ogre::Vector3 &_position,
  const Ogre::Quaternion &_orientation,
  const Ogre::Vector3 &_scale
  )
{
  GZ_PROFILE("Ogre2BoundingBoxCamera::MeshMinimalBox");
  _minVertex.x = std::numeric_limits<float>::max();
  _minVertex.y = std::numeric_limits<float>::max();
  _minVertex.z = std::numeric_limits<float>::max();
  _maxVertex.x = -std::numeric_limits<float>::max();
  _maxVertex.y = -std::numeric_limits<float>::max();
  _maxVertex.z = -std::numeric_limits<float>::max();

  auto subMeshes = _mesh->getSubMeshes();

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
          gzerr << "Vertex Buffer type error" << std::endl;

        vec = (_orientation * (vec * _scale)) + _position;

        Ogre::Vector4 vec4(vec.x, vec.y, vec.z, 1);
        vec4 =  _projMatrix * _viewMatrix * vec4;

        // homogeneous
        vec.x = vec4.x / vec4.w;
        vec.y = vec4.y / vec4.w;
        vec.z = vec4.z;

        _minVertex.x = std::min(_minVertex.x, vec.x);
        _minVertex.y = std::min(_minVertex.y, vec.y);
        _minVertex.z = std::min(_minVertex.z, vec.z);

        _maxVertex.x = std::max(_maxVertex.x, vec.x);
        _maxVertex.y = std::max(_maxVertex.y, vec.y);
        _maxVertex.z = std::max(_maxVertex.z, vec.z);

        // get the next element
        requests[0].data += requests[0].vertexBuffer->getBytesPerElement();
      }
      vao->unmapAsyncTickets(requests);
    }
  }
}

/////////////////////////////////////////////////
void Ogre2BoundingBoxCamera::DrawLine(unsigned char *_data,
  const math::Vector2i &_point1, const math::Vector2i &_point2,
  const math::Color &_color) const
{
  GZ_PROFILE("Ogre2BoundingBoxCamera::DrawLine");
  BaseBoundingBoxCamera::DrawLine(_data, _point1, _point2, _color);
}

/////////////////////////////////////////////////
void Ogre2BoundingBoxCamera::DrawBoundingBox(unsigned char *_data,
    const math::Color &_color, const BoundingBox &_box) const
{
  GZ_PROFILE("Ogre2BoundingBoxCamera::DrawBoundingBox");
  // 3D box
  if (this->Type() == BoundingBoxType::BBT_BOX3D)
  {
    // Get the 3D vertices of the box in 3D camera coord.
    auto vertices = _box.Vertices3D();

    // Project the 3D vertices in 3D camera coord to 2D vertices in clip coord
    auto projMatrix = this->dataPtr->ogreCamera->getProjectionMatrix();
    std::vector<math::Vector2d> vertices2d;
    for (auto &vertex : vertices)
    {
      // Skip boxes which has any vertex behind the camera (has positive z)
      if (vertex.Z() > 0)
        return;

      // Convert to homogeneous coord.
      auto homoVertex =
        Ogre::Vector4(vertex.X(), vertex.Y(), vertex.Z(), 1);

      auto projVertex = projMatrix * homoVertex;
      projVertex.x /= projVertex.w;
      projVertex.y /= projVertex.w;

      vertices2d.push_back({projVertex.x, projVertex.y});
    }

    this->DrawBoundingBox3D(_data, _color, vertices2d);
    return;
  }

  this->DrawBoundingBox2D(_data, _color, _box);
}

/////////////////////////////////////////////////
void Ogre2BoundingBoxCamera::ConvertToScreenCoord(
  Ogre::Vector3 &_minVertex, Ogre::Vector3 &_maxVertex) const
{
  uint32_t width = this->ImageWidth();
  uint32_t height = this->ImageHeight();

  // clip the values outside the frustum range
  _minVertex.x = std::clamp<float>(_minVertex.x, -1.0, 1.0);
  _minVertex.y = std::clamp<float>(_minVertex.y, -1.0, 1.0);
  _maxVertex.x = std::clamp<float>(_maxVertex.x, -1.0, 1.0);
  _maxVertex.y = std::clamp<float>(_maxVertex.y, -1.0, 1.0);

  // convert from [-1, 1] range to [0, 1] range & multiply by screen dims
  _minVertex.x = (_minVertex.x + 1.0) / 2 * width;
  _minVertex.y = (1.0 - _minVertex.y) / 2 * height;
  _maxVertex.x = (_maxVertex.x + 1.0) / 2 * width;
  _maxVertex.y = (1.0 - _maxVertex.y) / 2 * height;

  // clip outside screen boundaries
  _minVertex.x = std::max<float>(0.0, _minVertex.x);
  _minVertex.y = std::max<float>(0.0, _minVertex.y);
  _maxVertex.x = std::min<float>(_maxVertex.x, width - 1.0);
  _maxVertex.y = std::min<float>(_maxVertex.y, height - 1.0);
}

/////////////////////////////////////////////////
const std::vector<BoundingBox> &Ogre2BoundingBoxCamera::BoundingBoxData() const
{
  return this->dataPtr->outputBoxes;
}

/////////////////////////////////////////////////
common::ConnectionPtr
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

/////////////////////////////////////////////////
math::Matrix4d Ogre2BoundingBoxCamera::ProjectionMatrix() const
{
  return Ogre2Conversions::Convert(
      this->dataPtr->ogreCamera->getProjectionMatrix());
}

/////////////////////////////////////////////////
void Ogre2BoundingBoxCamera::SetProjectionMatrix(const math::Matrix4d &_matrix)
{
  BaseBoundingBoxCamera::SetProjectionMatrix(_matrix);
  this->dataPtr->ogreCamera->setCustomProjectionMatrix(
      true, Ogre2Conversions::Convert(this->projectionMatrix));
}
