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

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:5033)
#endif
#include <OgreBitwise.h>
#include <OgreDescriptorSetTexture.h>
#include <OgreDescriptorSetUav.h>
#include <OgreHlmsSamplerblock.h>
#include <OgreHlmsCompute.h>
#include <OgreHlmsComputeJob.h>
#include <OgreHlmsManager.h>
#include <OgreRenderSystem.h>
#include <OgreResourceTransition.h>
#include <OgreShaderParams.h>
#include <Vao/OgreAsyncTicket.h>
#include <Vao/OgreUavBufferPacked.h>
#include <Vao/OgreVaoManager.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <gz/common/Console.hh>
#include <gz/common/Profiler.hh>

#include <gz/math/Color.hh>
#include <gz/math/Vector3.hh>
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
#include "gz/rendering/ogre2/Ogre2ParticleEmitter.hh"
#include "gz/rendering/ogre2/Ogre2RenderEngine.hh"
#include "gz/rendering/ogre2/Ogre2RenderTarget.hh"
#include "gz/rendering/ogre2/Ogre2RenderTypes.hh"
#include "gz/rendering/ogre2/Ogre2Scene.hh"
#include "gz/rendering/ogre2/Ogre2Visual.hh"

#include "Ogre2BoundingBoxMaterialSwitcher.hh"
#include "Ogre2GpuReadbackTicket.hh"

using namespace gz;
using namespace rendering;

class gz::rendering::Ogre2BoundingBoxCameraPrivate
{
  /// \brief Merge 2D boxes. Used in multi-links model.
  /// \param[in] _boxes 2D boxes
  /// \return Merged bounding box
  public: BoundingBox MergeBoxes2D(
    const std::vector<const BoundingBox *> &_boxes) const;

  /// \brief Get the 3d vertices (in camera coord.) of items that
  /// belong to the same model (only used in multi-links models)
  /// \param[in] _indices Indices (see Ogre2BoundingBoxMaterialSwitcher) of
  /// the items
  /// \param[out] _vertices vector of 3d vertices of the items
  public: void MeshVertices(const std::vector<uint32_t> &_indices,
              std::vector<math::Vector3d> &_vertices);

  /// \brief Find the extent, in pixels, of each index in an image rendered
  /// by the material switcher. Fills pixelBounds and visibleIndices.
  /// \param[in] _data First pixel of the image
  /// \param[in] _bytesPerRow Bytes between the start of two rows
  /// \param[in] _bytesPerPixel Bytes per pixel (3 or 4); the first 3 bytes
  /// of a pixel hold the index.
  /// \param[in] _width Image width
  /// \param[in] _height Image height
  public: void ScanIndexImage(const uint8_t *_data, size_t _bytesPerRow,
              unsigned int _bytesPerPixel, unsigned int _width,
              unsigned int _height);

  /// \brief Group indexBoxes by model, in output order: models sorted by
  /// name, in descending order, and items of a model by ogre id. Fills
  /// groupedPositions and groupRanges.
  public: void GroupByModel();

  /// \brief Positions in indexBoxes, grouped by model (see GroupByModel).
  public: std::vector<uint32_t> groupedPositions;

  /// \brief [begin, end) of each model's group in groupedPositions, in
  /// output order.
  public: std::vector<std::pair<uint32_t, uint32_t>> groupRanges;

  /// \brief Scratch buffers of GroupByModel, kept to avoid allocating
  /// every frame.
  public: struct GroupKey
  {
    unsigned int modelId;
    Ogre::IdType itemId;
    uint32_t position;
  };
  public: std::vector<GroupKey> groupKeys;
  public: std::vector<std::pair<uint32_t, uint32_t>> unsortedRanges;
  public: std::vector<std::string> groupNames;
  public: std::vector<uint32_t> groupOrder;

  /// \brief Vertex positions of a mesh, in the mesh's frame. Read back
  /// from the GPU the first time the mesh is used and reused afterwards,
  /// unless the mesh's vertices can change (dynamic buffers).
  /// \param[in] _mesh The mesh
  /// \return The positions of all vertices of the first LOD of all
  /// submeshes. Valid until the next call.
  public: struct MeshPoints;
  public: const MeshPoints &LocalMeshPoints(Ogre::Mesh *_mesh);

  /// \brief Drop cached meshes that were not used in the last frame,
  /// once there are many of them.
  public: void PruneMeshCache();

  /// \brief Cached vertex positions of a mesh, see LocalMeshPoints.
  public: struct MeshPoints
  {
    /// \brief Handle of the mesh. Handles are never reused, so this tells
    /// a mesh apart from a later one allocated at the same address.
    Ogre::ResourceHandle handle{0};

    /// \brief Position buffer of each submesh when the points were read,
    /// to notice if the mesh's buffers are replaced.
    std::vector<const Ogre::VertexBufferPacked *> buffers;

    /// \brief Vertex positions, in the mesh's frame.
    std::vector<Ogre::Vector3> points;

    /// \brief The points that can be the extreme of a projection: points,
    /// without the ones strictly inside their convex hull (see
    /// ExtremePoints). Used for full 2D boxes.
    std::vector<Ogre::Vector3> extremePoints;

    /// \brief Last frame the mesh was used in.
    uint64_t lastUsed{0};

    /// \brief Changes every time the points are read, and is unique across
    /// meshes: equal generations mean equal points.
    uint64_t generation{0};
  };

  /// \brief Remove points that can't be the extreme of any projection:
  /// those strictly inside the convex hull of the others. Exact, but not
  /// minimal: the hull used is the hull of the extreme points along 26
  /// directions, which is inside the true hull.
  /// \param[in] _points The points
  /// \param[out] _out The points that are kept
  public: static void ExtremePoints(const std::vector<Ogre::Vector3> &_points,
              std::vector<Ogre::Vector3> &_out);

  /// \brief Source of MeshPoints::generation.
  public: uint64_t nextGeneration{1};

  /// \brief Extent of points projected with a matrix.
  /// \param[in] _toClip Points to clip space
  /// \param[in] _points The points
  /// \param[out] _min Minimum x/w, y/w and z
  /// \param[out] _max Maximum x/w, y/w and z
  /// \return False if a point is not in front of the camera (w <= 0)
  public: static bool ProjectedExtent(const Ogre::Matrix4 &_toClip,
              const std::vector<Ogre::Vector3> &_points,
              Ogre::Vector3 &_min, Ogre::Vector3 &_max);

  /// \brief Full 2D extent of an item in the last frame it was seen, to
  /// reuse while neither the item nor the camera move.
  public: struct FullExtent
  {
    /// \brief Item to clip space matrix the extent was computed with.
    Ogre::Matrix4 toClip;

    /// \brief MeshPoints::generation of the points it was computed from.
    uint64_t generation{0};

    /// \brief The extent, see ProjectedExtent.
    Ogre::Vector3 min;
    Ogre::Vector3 max;

    /// \brief Last frame the item was seen in.
    uint64_t lastUsed{0};
  };

  /// \brief Full extents, keyed by Ogre item id (never reused).
  public: std::unordered_map<Ogre::IdType, FullExtent> fullExtents;

  /// \brief Mesh cache, see LocalMeshPoints.
  public: std::unordered_map<const Ogre::Mesh *, MeshPoints> meshCache;

  /// \brief Scratch buffer of LocalMeshPoints.
  public: std::vector<const Ogre::VertexBufferPacked *> meshBuffers;

  /// \brief Number of frames processed so far.
  public: uint64_t frame{0};

  /// \brief Add a line to the viewport. If the line's endpoints are not inside
  /// the viewport, the added line will be a clipped line that fits in the
  /// viewport. If the line to be added doesn't intersect the viewport at all,
  /// the line won't be saved to _lines
  /// \param[in] _bounds The bounds of the viewport. Order of the vector should
  /// be: xmin, ymin, xmax, ymax
  /// \param[in] _p0 The line's first endpoint
  /// \param[in] _p1 The line's second endpoint
  /// \param[out] _lines The list of lines in the viewport. The line
  /// computed from _p0 and _p1 will be added to this list
  public: void AddToViewportLines(const math::Vector4d &_bounds,
              const math::Vector2d &_p0, const math::Vector2d &_p1,
              std::vector<math::Vector2d> &_lines) const;

  /// \brief Clip a line to be within the bounds of a viewport. Using:
  /// https://en.wikipedia.org/wiki/Cohen%E2%80%93Sutherland_algorithm
  /// \param[in] _bounds The bounds of the viewport. Order of the vector should
  /// be: xmin, ymin, xmax, ymax
  /// \param[in] _p0 The line's first endpoint
  /// \param[in] _p1 The line's second endpoint
  /// \return The new endpoints for the clipped line in the viewport. The first
  /// point in the pair is the clipped point for _p0, and the second
  /// point in the pair is the clipped point for _p1
  private: std::pair<math::Vector2d, math::Vector2d> ClipToViewPort(
              const math::Vector4d &_bounds, const math::Vector2d &_p0,
              const math::Vector2d &_p1) const;

  /// \brief Determine where a point is relative to the viewport
  /// \param[in] _bounds The bounds of the viewport. Order of the vector should
  /// be: xmin, ymin, xmax, ymax
  /// \param[in] _x The x coordinate of the point
  /// \param[in] _y The y coordinate of the point
  /// \return The location, which is a bitwise combination of the following:
  ///   INSIDE = 0 (0000)
  ///   LEFT   = 1 (0001)
  ///   RIGHT  = 2 (0010)
  ///   BOTTOM = 4 (0100)
  ///   TOP    = 8 (1000)
  private: int LocationRelativeToViewPort(const math::Vector4d &_bounds,
               double _x, double _y) const;

  /// \brief Material Switcher to switch item's material with ogre Ids
  /// For bounding boxes visibility checking & finding boundaires
  public: std::unique_ptr<Ogre2BoundingBoxMaterialSwitcher> materialSwitcher;

  /// \brief Workspace to interface with render texture
  public: Ogre::CompositorWorkspace *ogreCompositorWorkspace {nullptr};

  /// \brief Workspace Definition
  public: std::string workspaceDefinition;

  /// \brief Pointer to the ogre camera
  public: Ogre::Camera *ogreCamera{nullptr};

  /// \brief Definition of the scene pass, to set its visibility mask.
  public: Ogre::CompositorPassSceneDef *scenePassDef{nullptr};

  /// \brief Set what the scene pass renders: what the camera's visibility
  /// mask allows, except particles.
  /// \param[in] _mask The camera's visibility mask
  public: void UpdateVisibilityMask(uint32_t _mask);

  /// \brief Texture to create the render texture from.
  public: Ogre::TextureGpu *ogreRenderTexture {nullptr};

  /// \brief Persistent GPU->CPU readback of the render texture.
  public: Ogre2GpuReadbackTicket readback;

  /// \brief The render texture, read back and mapped during PostRender.
  /// Null outside PostRender.
  public: Ogre::TextureBox mapped;

  /// \brief Whether pixelBounds and visibleIndices are up to date for this
  /// frame (computed on the GPU, or already scanned).
  public: bool boundsValid{false};

  /// \brief Find the extent of each index on the GPU (see
  /// BoundingBoxReduce_cs.glsl) instead of reading the image back and
  /// scanning it. Decided when the render texture is created.
  public: bool gpuReduce{false};

  /// \brief Whether the GPU reduction was dispatched this frame.
  public: bool gpuReduced{false};

  /// \brief Compute job of the GPU reduction, shared by all cameras.
  public: Ogre::HlmsComputeJob *reduceJob{nullptr};

  /// \brief Extents written by the GPU reduction, 4 uint32 per index.
  public: Ogre::UavBufferPacked *reduceBuffer{nullptr};

  /// \brief Number of indices reduceBuffer can hold.
  public: size_t reduceCapacity{0};

  /// \brief Zeros, to clear reduceBuffer every frame.
  public: std::vector<uint32_t> reduceZeros;

  /// \brief Number of indices dispatched this frame.
  public: size_t reduceCount{0};

  /// \brief Whether the GPU reduction can be used. Sets up the compute job.
  /// \return False to use the CPU scan.
  public: bool InitGpuReduce();

  /// \brief Dispatch the GPU reduction on the rendered index image.
  /// \param[in] _width Image width
  /// \param[in] _height Image height
  public: void DispatchGpuReduce(uint32_t _width, uint32_t _height);

  /// \brief Read the GPU reduction back into pixelBounds and
  /// visibleIndices.
  public: void ReadGpuReduce();

  /// \brief Release the GPU reduction buffer.
  public: void DestroyGpuReduce();

  /// \brief Dummy render texture to set image dims
  public: Ogre2RenderTexturePtr dummyTexture {nullptr};

  /// \brief New BoundingBox Frame Event to notify listeners with new data
  public: common::EventT<void(const std::vector<BoundingBox> &)>
        newBoundingBoxes;

  /// \brief Image / Render Texture Format
  public: Ogre::PixelFormatGpu format = Ogre::PFG_RGBA8_UNORM;

  /// \brief Extent, in pixels, of an index in the rendered image.
  public: struct PixelBounds
  {
    uint32_t minX;
    uint32_t minY;
    uint32_t maxX;
    uint32_t maxY;
  };

  /// \brief Extent of each index in the last image, by index. Only valid
  /// for the indices in visibleIndices.
  public: std::vector<PixelBounds> pixelBounds;

  /// \brief Indices visible in the last image, in order of appearance.
  public: std::vector<uint32_t> visibleIndices;

  /// \brief Box of each visible item, before merging the items of a
  /// model. First: the item's index; second: its box.
  public: std::vector<std::pair<uint32_t, BoundingBox>> indexBoxes;

  /// \brief Output bounding boxes to notify listeners
  public: std::vector<BoundingBox> outputBoxes;

  /// \brief Bounding Box type
  public: BoundingBoxType type {BoundingBoxType::BBT_VISIBLEBOX2D};

  /// \brief Alias variable that's used in the ClipToViewPort and
  /// LocationRelativeToViewPort methods.
  /// Binary representation of 0000
  private: const int kInside = 0;

  /// \brief Alias variable that's used in the ClipToViewPort and
  /// LocationRelativeToViewPort methods.
  /// Binary representation of 0001
  private: const int kLeft = 1;

  /// \brief Alias variable that's used in the ClipToViewPort and
  /// LocationRelativeToViewPort methods.
  /// Binary representation of 0010
  private: const int kRight = 2;

  /// \brief Alias variable that's used in the ClipToViewPort and
  /// LocationRelativeToViewPort methods.
  /// Binary representation of 0100
  private: const int kBottom = 4;

  /// \brief Alias variable that's used in the ClipToViewPort and
  /// LocationRelativeToViewPort methods.
  /// Binary representation of 1000
  private: const int kTop = 8;
};

/////////////////////////////////////////////////
void Ogre2BoundingBoxCameraPrivate::AddToViewportLines(
    const math::Vector4d &_bounds, const math::Vector2d &_p0,
    const math::Vector2d &_p1, std::vector<math::Vector2d> &_lines) const
{
  auto endpoints = this->ClipToViewPort(_bounds, _p0, _p1);
  if (endpoints.first.IsFinite() && endpoints.second.IsFinite())
  {
    _lines.push_back(endpoints.first);
    _lines.push_back(endpoints.second);
  }
}

/////////////////////////////////////////////////
std::pair<math::Vector2d, math::Vector2d>
Ogre2BoundingBoxCameraPrivate::ClipToViewPort(const math::Vector4d &_bounds,
    const math::Vector2d &_p0, const math::Vector2d &_p1) const
{
  GZ_PROFILE("Ogre2BoundingBoxCameraPrivate::ClipToViewPort");
  const auto xmin = _bounds[0];
  const auto ymin = _bounds[1];
  const auto xmax = _bounds[2];
  const auto ymax = _bounds[3];

  auto x0 = _p0.X();
  auto y0 = _p0.Y();
  auto x1 = _p1.X();
  auto y1 = _p1.Y();

  auto location0 = this->LocationRelativeToViewPort(_bounds, x0, y0);
  auto location1 = this->LocationRelativeToViewPort(_bounds, x1, y1);
  bool accept = false;

  while (true)
  {
    if (!(location0 | location1))
    {
      // bitwise OR is 0, which means both endpoints are in the bounds
      accept = true;
      break;
    }
    else if (location0 & location1)
    {
      // bitwise AND is not 0, which means that both points share a zone
      // outside (left, right, top, bottom) of the window. This means that the
      // line formed by these points does not appear in the window formed by the
      // bounds. Reject this line
      break;
    }
    else
    {
      // calculate the line segment to clip from an outside point to an
      // intersection with clip edge
      double x = 0.0;
      double y = 0.0;

      // at least one endpoint is outside the clip rectangle; pick it
      int outerLocation = location1 > location0 ? location1 : location0;

      // Find the intersection point. Use formulas:
      //    slope = (y1 - y0) / (x1 - x0)
      //    x = x0 + (1 / slope) * (ym - y0), where ym is ymin or ymax
      //    y = y0 + slope * (xm - x0), where xm is xmin or xmax
      // Divide by zero wont happen because in each case, outerLocation bit
      // being tested guarantees the denominator is non-zero
      if (outerLocation & this->kTop)
      {
        // point is above clip window
        x = x0 + (x1 - x0) * (ymax - y0) / (y1 - y0);
        y = ymax;
      }
      else if (outerLocation & this->kBottom)
      {
        // point is below clip window
        x = x0 + (x1 - x0) * (ymin - y0) / (y1 - y0);
        y = ymin;
      }
      else if (outerLocation & this->kRight)
      {
        // point is to the right of clip window
        y = y0 + (y1 - y0) * (xmax - x0) / (x1 - x0);
        x = xmax;
      }
      else if (outerLocation & this->kLeft)
      {
        // point is to the left of clip window
        y = y0 + (y1 - y0) * (xmin - x0) / (x1 - x0);
        x = xmin;
      }
      else
      {
        gzerr << "Internal error: no point was found outside of the clip "
               << "window\n";
        break;
      }

      // update the outside point to the intersection point
      if (outerLocation == location0)
      {
        x0 = x;
        y0 = y;
        location0 = this->LocationRelativeToViewPort(_bounds, x0, y0);
      }
      else
      {
        x1 = x;
        y1 = y;
        location1 = this->LocationRelativeToViewPort(_bounds, x1, y1);
      }
    }
  }

  if (accept)
    return {math::Vector2d(x0, y0), math::Vector2d(x1, y1)};

  return {math::Vector2d::NaN, math::Vector2d::NaN};
}

/////////////////////////////////////////////////
int Ogre2BoundingBoxCameraPrivate::LocationRelativeToViewPort(
    const math::Vector4d &_bounds, double _x, double _y) const
{
  // Relative location is a bitwise combination of:
  //   inside = 0 (0000)
  //   left   = 1 (0001)
  //   right  = 2 (0010)
  //   bottom = 4 (0100)
  //   top    = 8 (1000)

  const auto xmin = _bounds[0];
  const auto ymin = _bounds[1];
  const auto xmax = _bounds[2];
  const auto ymax = _bounds[3];

  // initialize the point as being inside the bounds
  int relativeLocation = this->kInside;

  // to the left
  if (_x < xmin)
    relativeLocation |= this->kLeft;
  // to the right
  else if (_x > xmax)
    relativeLocation |= this->kRight;

  // below
  if (_y < ymin)
    relativeLocation |= this->kBottom;
  // above
  else if (_y > ymax)
    relativeLocation |= this->kTop;

  return relativeLocation;
}

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

  this->dataPtr->readback.Destroy();
  this->dataPtr->DestroyGpuReduce();

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
    this->dataPtr->ogreCompositorWorkspace = nullptr;
  }

  if (!this->dataPtr->workspaceDefinition.empty())
  {
    ogreCompMgr->removeWorkspaceDefinition(
        this->dataPtr->workspaceDefinition);
    this->dataPtr->workspaceDefinition.clear();
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

  // Detaches the camera from its parent. Without this, a destroyed camera
  // stays in the scene graph and is pre-rendered (and crashes) next frame.
  BaseBoundingBoxCamera::Destroy();
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
  this->dataPtr->gpuReduce = this->dataPtr->InitGpuReduce();
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

  // index 0 is the background
  auto backgroundColor = Ogre::ColourValue(0.0f, 0.0f, 0.0f, 1.0f);

  // basic workspace consist of clear pass with the givin color &
  // a render scene pass to the givin render texture
  ogreCompositorManager->createBasicWorkspaceDef(
    this->dataPtr->workspaceDefinition,
    backgroundColor
    );

  // Find the scene pass, to set what it renders every frame
  this->dataPtr->scenePassDef = nullptr;
  Ogre::CompositorWorkspaceDef *workspaceDef =
      ogreCompositorManager->getWorkspaceDefinitionNoThrow(
      this->dataPtr->workspaceDefinition);
  if (workspaceDef)
  {
    for (const auto &[alias, nodeName] : workspaceDef->getNodeAliasMap())
    {
      Ogre::CompositorNodeDef *nodeDef =
          ogreCompositorManager->getNodeDefinitionNonConst(nodeName);
      for (size_t t = 0; t < nodeDef->getNumTargetPasses(); ++t)
      {
        for (Ogre::CompositorPassDef *pass :
             nodeDef->getTargetPass(t)->getCompositorPassesNonConst())
        {
          if (pass->getType() == Ogre::PASS_SCENE)
          {
            this->dataPtr->scenePassDef =
                static_cast<Ogre::CompositorPassSceneDef *>(pass);
          }
        }
      }
    }
  }
  if (!this->dataPtr->scenePassDef)
  {
    gzerr << "Bounding box camera: scene pass not found" << std::endl;
    return;
  }
  // Objects are drawn in flat colors: no lights needed. Setting up lights
  // (Forward+ light culling) was most of the cost of the pass in small
  // scenes.
  this->dataPtr->scenePassDef->mEnableForwardPlus = false;
  this->dataPtr->scenePassDef->setLightVisibilityMask(0u);
  this->dataPtr->UpdateVisibilityMask(this->VisibilityMask());

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
void Ogre2BoundingBoxCameraPrivate::UpdateVisibilityMask(uint32_t _mask)
{
  // Particles are not objects, and their materials can't draw an object
  // index: leave them out.
  if (this->scenePassDef)
  {
    this->scenePassDef->setVisibilityMask(
        _mask & ~Ogre2ParticleEmitter::kParticleVisibilityFlags);
  }
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

  this->dataPtr->UpdateVisibilityMask(this->VisibilityMask());

  // update the compositors; heightmaps are updated for this camera's view,
  // otherwise they aren't drawn
  this->scene->StartRendering(this->dataPtr->ogreCamera);

  this->dataPtr->ogreCompositorWorkspace->_validateFinalTarget();
  this->dataPtr->ogreCompositorWorkspace->_beginUpdate(false);
  this->dataPtr->ogreCompositorWorkspace->_update();
  this->dataPtr->ogreCompositorWorkspace->_endUpdate(false);

  Ogre::vector<Ogre::TextureGpu *>::type swappedTargets;
  swappedTargets.reserve(2u);
  this->dataPtr->ogreCompositorWorkspace->_swapFinalTarget(swappedTargets);

  // find the boxes' extents on the GPU, if someone is listening
  this->dataPtr->gpuReduced = false;
  if (this->dataPtr->gpuReduce &&
      this->dataPtr->newBoundingBoxes.ConnectionCount() > 0)
  {
    this->dataPtr->DispatchGpuReduce(this->ImageWidth(), this->ImageHeight());
  }

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

  this->dataPtr->boundsValid = false;
  ++this->dataPtr->frame;
  Ogre::Image2 legacyImage;
  const bool legacy = Ogre2UseLegacyReadback();
  if (this->dataPtr->gpuReduced)
  {
    // the extents were computed on the GPU; only read those back
    this->dataPtr->ReadGpuReduce();
  }
  // Otherwise map the rendered index image; the box functions scan it
  // through MarkVisibleBoxes().
  else if (legacy)
  {
    // Legacy A/B control path: allocates a staging buffer every frame.
    legacyImage.convertFromTexture(this->dataPtr->ogreRenderTexture, 0u, 0u);
    this->dataPtr->mapped = legacyImage.getData(0);
  }
  else
  {
    this->dataPtr->mapped = this->dataPtr->readback.DownloadAndMap(
        this->dataPtr->ogreRenderTexture);
  }
  if (!this->dataPtr->boundsValid && !this->dataPtr->mapped.data)
  {
    gzerr << "Ogre2BoundingBoxCamera: GPU readback failed; dropping frame"
          << std::endl;
    return;
  }

  if (this->dataPtr->type == BoundingBoxType::BBT_VISIBLEBOX2D)
    this->VisibleBoundingBoxes();
  else if (this->dataPtr->type == BoundingBoxType::BBT_FULLBOX2D)
    this->FullBoundingBoxes();
  else if (this->dataPtr->type == BoundingBoxType::BBT_BOX3D)
    this->BoundingBoxes3D();

  if (this->dataPtr->mapped.data && !legacy)
    this->dataPtr->readback.Unmap();
  this->dataPtr->mapped = Ogre::TextureBox();
  this->dataPtr->boundsValid = false;
  this->dataPtr->indexBoxes.clear();
  this->dataPtr->PruneMeshCache();

  this->dataPtr->newBoundingBoxes(this->dataPtr->outputBoxes);
}

/////////////////////////////////////////////////
void Ogre2BoundingBoxCamera::MarkVisibleBoxes()
{
  GZ_PROFILE("Ogre2BoundingBoxCamera::MarkVisibleBoxes");
  if (this->dataPtr->boundsValid)
    return;

  const Ogre::TextureBox &box = this->dataPtr->mapped;
  if (!box.data)
  {
    gzerr << "No rendered image to find the visible boxes in" << std::endl;
    return;
  }

  this->dataPtr->ScanIndexImage(static_cast<const uint8_t *>(box.data),
      box.bytesPerRow, box.bytesPerPixel,
      std::min<uint32_t>(this->ImageWidth(), box.width),
      std::min<uint32_t>(this->ImageHeight(), box.height));
  this->dataPtr->boundsValid = true;
}

/////////////////////////////////////////////////
void Ogre2BoundingBoxCameraPrivate::ScanIndexImage(const uint8_t *_data,
    size_t _bytesPerRow, unsigned int _bytesPerPixel, unsigned int _width,
    unsigned int _height)
{
  GZ_PROFILE("Ogre2BoundingBoxCameraPrivate::ScanIndexImage");
  constexpr uint32_t kUnseen = std::numeric_limits<uint32_t>::max();
  const size_t count = this->materialSwitcher->Entries().size();
  this->pixelBounds.assign(count, {kUnseen, kUnseen, 0, 0});
  this->visibleIndices.clear();

  for (uint32_t y = 0; y < _height; ++y)
  {
    const uint8_t *row = _data + y * _bytesPerRow;
    uint32_t x = 0;
    while (x < _width)
    {
      const uint8_t *px = row + x * _bytesPerPixel;

      // the run of pixels with the same index
      uint32_t last = x;
      if (_bytesPerPixel == 4u)
      {
        // compare whole pixels; the 4th byte (alpha) is the same for all
        uint32_t current;
        std::memcpy(&current, px, 4);
        while (last + 1 < _width)
        {
          uint32_t next;
          std::memcpy(&next, row + (last + 1) * 4u, 4);
          if (next != current)
            break;
          ++last;
        }
      }
      else
      {
        while (last + 1 < _width)
        {
          const uint8_t *next = row + (last + 1) * _bytesPerPixel;
          if (next[0] != px[0] || next[1] != px[1] || next[2] != px[2])
            break;
          ++last;
        }
      }

      const uint32_t index =
          Ogre2BoundingBoxMaterialSwitcher::DecodeIndex(px[0], px[1], px[2]);
      if (index != 0 && index < count)
      {
        PixelBounds &b = this->pixelBounds[index];
        if (b.minX == kUnseen)
        {
          this->visibleIndices.push_back(index);
          b = {x, y, last, y};
        }
        else
        {
          b.minX = std::min(b.minX, x);
          b.maxX = std::max(b.maxX, last);
          b.maxY = y;
        }
      }
      x = last + 1;
    }
  }
}

/////////////////////////////////////////////////
bool Ogre2BoundingBoxCameraPrivate::InitGpuReduce()
{
  // A/B control and fallback
  const char *cpuScan = std::getenv("GZ_RENDERING_OGRE2_BBOX_CPU_SCAN");
  if ((cpuScan && std::string(cpuScan) != "0") || Ogre2UseLegacyReadback())
    return false;

  // Only the GLSL shader exists for now
  auto *engine = Ogre2RenderEngine::Instance();
  if (engine->GraphicsAPI() != GraphicsAPI::OPENGL)
    return false;
  Ogre::Root *root = engine->OgreRoot();
  if (!root->getRenderSystem()->getCapabilities()->hasCapability(
        Ogre::RSC_COMPUTE_PROGRAM))
  {
    return false;
  }

  Ogre::HlmsCompute *hlmsCompute = root->getHlmsManager()->getComputeHlms();
  const std::string jobName = "gz-rendering/BoundingBoxReduce";
  this->reduceJob = hlmsCompute->findComputeJobNoThrow(jobName);
  if (!this->reduceJob)
  {
    try
    {
      this->reduceJob = hlmsCompute->createComputeJob(jobName, jobName,
          "BoundingBoxReduce_cs", Ogre::StringVector());
    }
    catch (Ogre::Exception &e)
    {
      gzwarn << "Bounding box camera: can't create the GPU reduction ("
             << e.getDescription() << "); using the CPU." << std::endl;
      return false;
    }
    this->reduceJob->setThreadsPerGroup(8u, 8u, 1u);
    this->reduceJob->setNumTexUnits(1u);
    this->reduceJob->setNumUavUnits(1u);
    Ogre::ShaderParams &shaderParams =
        this->reduceJob->getShaderParams("default");
    Ogre::ShaderParams::Param param;
    param.name = "params";
    const uint32_t zeros[4] = {0u, 0u, 0u, 0u};
    param.setManualValue(zeros, 4u);
    shaderParams.mParams.push_back(param);
    shaderParams.setDirty();
  }
  return true;
}

/////////////////////////////////////////////////
void Ogre2BoundingBoxCameraPrivate::DispatchGpuReduce(uint32_t _width,
    uint32_t _height)
{
  GZ_PROFILE("Ogre2BoundingBoxCameraPrivate::DispatchGpuReduce");
  const size_t count = this->materialSwitcher->Entries().size();
  this->reduceCount = count;
  if (count <= 1u)
  {
    // nothing labeled: nothing to reduce
    this->gpuReduced = true;
    return;
  }

  auto *engine = Ogre2RenderEngine::Instance();
  Ogre::Root *root = engine->OgreRoot();
  Ogre::RenderSystem *renderSystem = root->getRenderSystem();

  try
  {
    // (re)allocate the extents buffer, and clear it
    if (count > this->reduceCapacity)
    {
      this->DestroyGpuReduce();
      this->reduceCapacity = std::max<size_t>(count, 256u);
      this->reduceCapacity = std::max(this->reduceCapacity,
          this->reduceCapacity * 3u / 2u);
      this->reduceBuffer = renderSystem->getVaoManager()->createUavBuffer(
          this->reduceCapacity * 4u, sizeof(uint32_t), 0u, nullptr, false);
      this->reduceZeros.assign(this->reduceCapacity * 4u, 0u);
    }
    this->reduceBuffer->upload(this->reduceZeros.data(), 0u, count * 4u);

    renderSystem->endRenderPassDescriptor();

    Ogre::BarrierSolver &solver = renderSystem->getBarrierSolver();
    Ogre::ResourceTransitionArray &before =
        solver.getNewResourceTransitionsArrayTmp();
    solver.resolveTransition(before, this->ogreRenderTexture,
        Ogre::ResourceLayout::Texture, Ogre::ResourceAccess::Read,
        1u << Ogre::GPT_COMPUTE_PROGRAM);
    solver.resolveTransition(before, this->reduceBuffer,
        Ogre::ResourceAccess::ReadWrite, 1u << Ogre::GPT_COMPUTE_PROGRAM);
    renderSystem->executeResourceTransition(before);

    Ogre::DescriptorSetTexture2::TextureSlot texSlot(
        Ogre::DescriptorSetTexture2::TextureSlot::makeEmpty());
    texSlot.texture = this->ogreRenderTexture;
    Ogre::HlmsSamplerblock pointSampler;
    pointSampler.setFiltering(Ogre::TFO_NONE);
    this->reduceJob->setTexture(0u, texSlot, &pointSampler);

    Ogre::DescriptorSetUav::BufferSlot bufSlot(
        Ogre::DescriptorSetUav::BufferSlot::makeEmpty());
    bufSlot.buffer = this->reduceBuffer;
    bufSlot.sizeBytes = count * 4u * sizeof(uint32_t);
    bufSlot.access = Ogre::ResourceAccess::ReadWrite;
    this->reduceJob->_setUavBuffer(0u, bufSlot);

    constexpr uint32_t kStrip = 32u;
    const uint32_t params[4] = {_width, _height,
        static_cast<uint32_t>(count), kStrip};
    Ogre::ShaderParams &shaderParams =
        this->reduceJob->getShaderParams("default");
    shaderParams.findParameter("params")->setManualValue(params, 4u);
    shaderParams.setDirty();

    const uint32_t strips = (_width + kStrip - 1u) / kStrip;
    this->reduceJob->setNumThreadGroups((strips + 7u) / 8u,
        (_height + 7u) / 8u, 1u);
    root->getHlmsManager()->getComputeHlms()->dispatch(
        this->reduceJob, nullptr, nullptr);

    // make the results visible to the readback copy
    Ogre::ResourceTransitionArray &after =
        solver.getNewResourceTransitionsArrayTmp();
    solver.resolveTransition(after, this->reduceBuffer,
        Ogre::ResourceAccess::Read, 0u);
    renderSystem->executeResourceTransition(after);

    // The job is shared by all cameras: don't leave it pointing at our
    // resources, which may be destroyed before it is used again.
    this->reduceJob->setTexture(0u,
        Ogre::DescriptorSetTexture2::TextureSlot::makeEmpty(), nullptr,
        false);
    this->reduceJob->_setUavBuffer(0u,
        Ogre::DescriptorSetUav::BufferSlot::makeEmpty());

    this->gpuReduced = true;
  }
  catch (Ogre::Exception &e)
  {
    gzwarn << "Bounding box camera: GPU reduction failed ("
           << e.getDescription() << "); using the CPU from now on."
           << std::endl;
    this->gpuReduce = false;
  }
}

/////////////////////////////////////////////////
void Ogre2BoundingBoxCameraPrivate::ReadGpuReduce()
{
  GZ_PROFILE("Ogre2BoundingBoxCameraPrivate::ReadGpuReduce");
  const size_t count = this->reduceCount;
  this->pixelBounds.resize(count);
  this->visibleIndices.clear();
  this->boundsValid = true;
  if (count <= 1u)
    return;

  Ogre::AsyncTicketPtr ticket =
      this->reduceBuffer->readRequest(0u, count * 4u);
  const uint32_t *data = static_cast<const uint32_t *>(ticket->map());
  for (uint32_t index = 1u; index < count; ++index)
  {
    const uint32_t *d = data + index * 4u;
    // 0 means not seen; minimums are stored inverted
    if (d[0] == 0u)
      continue;
    this->pixelBounds[index] = {0xFFFFFFFFu - d[0], 0xFFFFFFFFu - d[1],
        d[2], d[3]};
    this->visibleIndices.push_back(index);
  }
  ticket->unmap();
}

/////////////////////////////////////////////////
void Ogre2BoundingBoxCameraPrivate::DestroyGpuReduce()
{
  if (this->reduceBuffer)
  {
    Ogre2RenderEngine::Instance()->OgreRoot()->getRenderSystem()->
        getVaoManager()->destroyUavBuffer(this->reduceBuffer);
    this->reduceBuffer = nullptr;
  }
  this->reduceCapacity = 0u;
}

/////////////////////////////////////////////////
const Ogre2BoundingBoxCameraPrivate::MeshPoints &
    Ogre2BoundingBoxCameraPrivate::LocalMeshPoints(Ogre::Mesh *_mesh)
{
  GZ_PROFILE("Ogre2BoundingBoxCameraPrivate::LocalMeshPoints");
  // Find the position buffer of the first LOD of each submesh. Only
  // immutable buffers are cached: the others can be rewritten at any time.
  this->meshBuffers.clear();
  bool cacheable = true;
  for (const Ogre::SubMesh *subMesh : _mesh->getSubMeshes())
  {
    const Ogre::VertexArrayObjectArray &vaos = subMesh->mVao[0];
    const Ogre::VertexBufferPacked *positions = nullptr;
    if (!vaos.empty())
    {
      for (const Ogre::VertexBufferPacked *buffer :
           vaos[0]->getVertexBuffers())
      {
        for (const auto &element : buffer->getVertexElements())
        {
          if (element.mSemantic == Ogre::VES_POSITION)
            positions = buffer;
        }
        if (positions)
          break;
      }
    }
    if (positions && positions->getBufferType() != Ogre::BT_IMMUTABLE)
      cacheable = false;
    this->meshBuffers.push_back(positions);
  }

  MeshPoints &cached = this->meshCache[_mesh];
  cached.lastUsed = this->frame;
  if (cacheable && cached.handle == _mesh->getHandle() &&
      cached.buffers == this->meshBuffers)
  {
    return cached;
  }

  cached.handle = cacheable ? _mesh->getHandle() : 0;
  cached.buffers = this->meshBuffers;
  cached.generation = this->nextGeneration++;
  cached.points.clear();
  for (const Ogre::SubMesh *subMesh : _mesh->getSubMeshes())
  {
    const Ogre::VertexArrayObjectArray &vaos = subMesh->mVao[0];
    if (vaos.empty())
      continue;

    // Get the first LOD level
    Ogre::VertexArrayObject *vao = vaos[0];

    // request async read from buffer
    Ogre::VertexArrayObject::ReadRequestsArray requests;
    requests.push_back(Ogre::VertexArrayObject::ReadRequests(
      Ogre::VES_POSITION));
    vao->readRequests(requests);
    vao->mapAsyncTickets(requests);

    const size_t count = requests[0].vertexBuffer->getNumElements();
    const size_t stride = requests[0].vertexBuffer->getBytesPerElement();
    cached.points.reserve(cached.points.size() + count);
    for (size_t i = 0; i < count; ++i)
    {
      const char *data = requests[0].data + i * stride;
      Ogre::Vector3 vec;
      if (requests[0].type == Ogre::VET_HALF4)
      {
        const Ogre::uint16 *vertex =
            reinterpret_cast<const Ogre::uint16 *>(data);
        vec.x = Ogre::Bitwise::halfToFloat(vertex[0]);
        vec.y = Ogre::Bitwise::halfToFloat(vertex[1]);
        vec.z = Ogre::Bitwise::halfToFloat(vertex[2]);
      }
      else if (requests[0].type == Ogre::VET_FLOAT3)
      {
        const float *vertex = reinterpret_cast<const float *>(data);
        vec.x = vertex[0];
        vec.y = vertex[1];
        vec.z = vertex[2];
      }
      else
      {
        gzerr << "Vertex Buffer type error" << std::endl;
        continue;
      }
      cached.points.push_back(vec);
    }
    vao->unmapAsyncTickets(requests);
  }
  // Dynamic meshes are read every frame: not worth reducing them.
  if (cacheable)
    ExtremePoints(cached.points, cached.extremePoints);
  else
    cached.extremePoints = cached.points;
  return cached;
}

/////////////////////////////////////////////////
void Ogre2BoundingBoxCameraPrivate::PruneMeshCache()
{
  // full extents of items not seen in this frame
  if (this->fullExtents.size() > 2 * this->visibleIndices.size() + 256)
  {
    for (auto it = this->fullExtents.begin(); it != this->fullExtents.end();)
    {
      if (it->second.lastUsed != this->frame)
        it = this->fullExtents.erase(it);
      else
        ++it;
    }
  }

  if (this->meshCache.size() < 256)
    return;
  size_t used = 0;
  for (const auto &cached : this->meshCache)
    used += cached.second.lastUsed == this->frame ? 1u : 0u;
  if (this->meshCache.size() < 2 * used + 256)
    return;
  for (auto it = this->meshCache.begin(); it != this->meshCache.end();)
  {
    if (it->second.lastUsed != this->frame)
      it = this->meshCache.erase(it);
    else
      ++it;
  }
}

/////////////////////////////////////////////////
void Ogre2BoundingBoxCameraPrivate::MeshVertices(
    const std::vector<uint32_t> &_indices,
    std::vector<math::Vector3d> &_vertices)
{
  GZ_PROFILE("Ogre2BoundingBoxCameraPrivate::MeshVertices");
  const Ogre::Matrix4 viewMatrix = this->ogreCamera->getViewMatrix();
  const auto &entries = this->materialSwitcher->Entries();

  for (uint32_t index : _indices)
  {
    Ogre::Item *item = entries[index].item;
    Ogre::Node *node = item->getParentNode();

    // item to camera view coordinates
    Ogre::Matrix4 world;
    world.makeTransform(node->_getDerivedPosition(),
        node->_getDerivedScale(), node->_getDerivedOrientation());
    const Ogre::Matrix4 toView = viewMatrix * world;

    const auto &points =
        this->LocalMeshPoints(item->getMesh().get()).points;
    _vertices.reserve(_vertices.size() + points.size());
    for (const Ogre::Vector3 &p : points)
      _vertices.push_back(Ogre2Conversions::Convert(toView * p));
  }
}

/////////////////////////////////////////////////
void Ogre2BoundingBoxCameraPrivate::GroupByModel()
{
  const auto &entries = this->materialSwitcher->Entries();

  // Group: sort the items by model, then by item id
  this->groupKeys.clear();
  for (size_t i = 0; i < this->indexBoxes.size(); ++i)
  {
    const auto &entry = entries[this->indexBoxes[i].first];
    this->groupKeys.push_back({entry.modelId, entry.item->getId(),
        static_cast<uint32_t>(i)});
  }
  std::sort(this->groupKeys.begin(), this->groupKeys.end(),
      [](const GroupKey &_a, const GroupKey &_b)
      {
        return _a.modelId != _b.modelId ? _a.modelId < _b.modelId :
            _a.itemId < _b.itemId;
      });

  this->unsortedRanges.clear();
  size_t names = 0;
  for (size_t i = 0; i < this->groupKeys.size();)
  {
    size_t end = i + 1;
    while (end < this->groupKeys.size() &&
           this->groupKeys[end].modelId == this->groupKeys[i].modelId)
    {
      ++end;
    }
    this->unsortedRanges.emplace_back(static_cast<uint32_t>(i),
        static_cast<uint32_t>(end));
    // reuse the strings' storage across frames
    if (names == this->groupNames.size())
      this->groupNames.emplace_back();
    const auto &entry =
        entries[this->indexBoxes[this->groupKeys[i].position].first];
    this->groupNames[names++] = entry.model->Name();
    i = end;
  }

  // Models in descending name order, the order boxes have always been
  // reported in.
  this->groupOrder.resize(this->unsortedRanges.size());
  for (size_t g = 0; g < this->groupOrder.size(); ++g)
    this->groupOrder[g] = static_cast<uint32_t>(g);
  std::sort(this->groupOrder.begin(), this->groupOrder.end(),
      [this](uint32_t _a, uint32_t _b)
      {
        return this->groupNames[_a] > this->groupNames[_b];
      });

  this->groupedPositions.clear();
  this->groupRanges.clear();
  for (uint32_t g : this->groupOrder)
  {
    const auto range = this->unsortedRanges[g];
    const uint32_t begin = static_cast<uint32_t>(this->groupedPositions.size());
    for (uint32_t k = range.first; k < range.second; ++k)
      this->groupedPositions.push_back(this->groupKeys[k].position);
    this->groupRanges.emplace_back(begin,
        static_cast<uint32_t>(this->groupedPositions.size()));
  }
}

/////////////////////////////////////////////////
void Ogre2BoundingBoxCamera::MergeMultiLinksModels3D()
{
  GZ_PROFILE("Ogre2BoundingBoxCamera::MergeMultiLinksModels3D");
  const auto &indexBoxes = this->dataPtr->indexBoxes;
  const auto &positions = this->dataPtr->groupedPositions;
  this->dataPtr->GroupByModel();
  std::vector<uint32_t> indices;
  std::vector<math::Vector3d> vertices;
  for (const auto &[begin, end] : this->dataPtr->groupRanges)
  {
    // If not a multi-link model, add the 3d box from the OGRE API
    if (end - begin == 1)
    {
      this->dataPtr->outputBoxes.push_back(
          indexBoxes[positions[begin]].second);
      continue;
    }

    // Get all the 3D vertices of the sub-items(total mesh)
    indices.clear();
    for (uint32_t k = begin; k < end; ++k)
      indices.push_back(indexBoxes[positions[k]].first);
    vertices.clear();
    this->dataPtr->MeshVertices(indices, vertices);

    // Get the oriented bounding box from the mesh using PCA
    math::OrientedBoxd mergedBox = math::eigen3::verticesToOrientedBox(
      vertices);

    // convert to rendering::BoundingBox format
    BoundingBox box;
    auto pose = mergedBox.Pose();
    box.SetCenter(pose.Pos());
    box.SetOrientation(pose.Rot());
    box.SetSize(mergedBox.Size());
    box.SetLabel(indexBoxes[positions[begin]].second.Label());

    this->dataPtr->outputBoxes.push_back(box);
  }
}

/////////////////////////////////////////////////
void Ogre2BoundingBoxCamera::MergeMultiLinksModels2D()
{
  GZ_PROFILE("Ogre2BoundingBoxCamera::MergeMultiLinksModels2D");
  const auto &indexBoxes = this->dataPtr->indexBoxes;
  const auto &positions = this->dataPtr->groupedPositions;
  this->dataPtr->GroupByModel();
  std::vector<const BoundingBox *> boxes;
  for (const auto &[begin, end] : this->dataPtr->groupRanges)
  {
    boxes.clear();
    for (uint32_t k = begin; k < end; ++k)
      boxes.push_back(&indexBoxes[positions[k]].second);
    this->dataPtr->outputBoxes.push_back(this->dataPtr->MergeBoxes2D(boxes));
  }
}

/////////////////////////////////////////////////
BoundingBox Ogre2BoundingBoxCameraPrivate::MergeBoxes2D(
        const std::vector<const BoundingBox *> &_boxes) const
{
  if (_boxes.size() == 1)
    return *_boxes[0];

  BoundingBox mergedBox;
  double minX = std::numeric_limits<double>::max();
  double maxX = 0.0;
  double minY = std::numeric_limits<double>::max();
  double maxY = 0.0;

  for (const auto &box : _boxes)
  {
    double boxMinX = box->Center().X() - box->Size().X() * 0.5;
    double boxMaxX = box->Center().X() + box->Size().X() * 0.5;
    double boxMinY = box->Center().Y() - box->Size().Y() * 0.5;
    double boxMaxY = box->Center().Y() + box->Size().Y() * 0.5;

    minX = std::min(minX, boxMinX);
    maxX = std::max(maxX, boxMaxX);
    minY = std::min(minY, boxMinY);
    maxY = std::max(maxY, boxMaxY);
  }

  auto width = maxX - minX;
  auto height = maxY - minY;
  mergedBox.SetSize({width, height, 0});
  mergedBox.SetCenter({minX + width * 0.5, minY + height * 0.5, 0});
  mergedBox.SetLabel(_boxes[0]->Label());

  return mergedBox;
}

/////////////////////////////////////////////////
void Ogre2BoundingBoxCamera::BoundingBoxes3D()
{
  GZ_PROFILE("Ogre2BoundingBoxCamera::BoundingBoxes3D");
  auto viewMatrix = this->dataPtr->ogreCamera->getViewMatrix();

  // used to filter the hidden boxes
  this->MarkVisibleBoxes();

  // Compute the rotation of the boxes from their world rotation & the view
  // matrix
  const auto worldCameraRotation = Ogre2Conversions::Convert(
    viewMatrix.extractQuaternion());

  const auto &entries = this->dataPtr->materialSwitcher->Entries();
  for (uint32_t index : this->dataPtr->visibleIndices)
  {
    Ogre::Item *item = entries[index].item;

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
      continue;

    BoundingBox box;

    // Position in world coord
    Ogre::Vector3 position = worldAabb.getCenter();

    // Position in camera coord
    Ogre::Vector3 viewPosition = viewMatrix * position;

    // Convert to gz::math
    box.SetCenter(Ogre2Conversions::Convert(viewPosition));
    box.SetSize(Ogre2Conversions::Convert(size));

    // Body to camera rotation = body_world * world_camera
    auto bodyWorldRotation = Ogre2Conversions::Convert(orientation);
    box.SetOrientation(worldCameraRotation * bodyWorldRotation);
    box.SetLabel(entries[index].label);

    this->dataPtr->indexBoxes.emplace_back(index, std::move(box));
  }

  // Combine boxes of multi-links model if exists
  this->MergeMultiLinksModels3D();
}

/////////////////////////////////////////////////
void Ogre2BoundingBoxCamera::VisibleBoundingBoxes()
{
  GZ_PROFILE("Ogre2BoundingBoxCamera::VisibleBoundingBoxes");
  this->MarkVisibleBoxes();

  const auto &entries = this->dataPtr->materialSwitcher->Entries();
  for (uint32_t index : this->dataPtr->visibleIndices)
  {
    const auto &boundary = this->dataPtr->pixelBounds[index];
    auto boxWidth = boundary.maxX - boundary.minX;
    auto boxHeight = boundary.maxY - boundary.minY;

    BoundingBox box;
    box.SetLabel(entries[index].label);
    box.SetCenter({boundary.minX + boxWidth * 0.5,
        boundary.minY + boxHeight * 0.5, 0});
    box.SetSize(
        {static_cast<double>(boxWidth), static_cast<double>(boxHeight), 0.0});
    this->dataPtr->indexBoxes.emplace_back(index, std::move(box));
  }

  // Combine boxes of multi-links model if exists
  this->MergeMultiLinksModels2D();
}

/////////////////////////////////////////////////
void Ogre2BoundingBoxCamera::FullBoundingBoxes()
{
  GZ_PROFILE("Ogre2BoundingBoxCamera::FullBoundingBoxes");
  // used to filter the hidden boxes
  this->MarkVisibleBoxes();

  const Ogre::Matrix4 viewProj =
      this->dataPtr->ogreCamera->getProjectionMatrix() *
      this->dataPtr->ogreCamera->getViewMatrix();

  const auto &entries = this->dataPtr->materialSwitcher->Entries();
  for (uint32_t index : this->dataPtr->visibleIndices)
  {
    Ogre::Item *item = entries[index].item;
    Ogre::MeshPtr mesh = item->getMesh();

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
      continue;

    // Extent of the projected vertices. Reused from the last frame if
    // neither the item, its mesh nor the camera changed: same inputs, same
    // result.
    Ogre::Matrix4 world;
    world.makeTransform(position, scale, oreintation);
    const Ogre::Matrix4 toClip = viewProj * world;
    const auto &meshPoints = this->dataPtr->LocalMeshPoints(mesh.get());
    auto &extent = this->dataPtr->fullExtents[item->getId()];
    if (extent.lastUsed == 0 ||
        extent.generation != meshPoints.generation ||
        std::memcmp(&extent.toClip, &toClip, sizeof(toClip)) != 0)
    {
      // The extreme points give the same extent as all points, as long as
      // all of them are in front of the camera. Otherwise keep the result
      // of all points (vertices behind the camera are not handled yet).
      if (!Ogre2BoundingBoxCameraPrivate::ProjectedExtent(toClip,
          meshPoints.extremePoints, extent.min, extent.max))
      {
        Ogre2BoundingBoxCameraPrivate::ProjectedExtent(toClip,
            meshPoints.points, extent.min, extent.max);
      }
      extent.toClip = toClip;
      extent.generation = meshPoints.generation;
    }
    extent.lastUsed = this->dataPtr->frame;
    Ogre::Vector3 minVertex = extent.min;
    Ogre::Vector3 maxVertex = extent.max;

    if ((abs(minVertex.x) > 1 && abs(maxVertex.x) > 1) ||
        (abs(minVertex.y) > 1 && abs(maxVertex.y) > 1))
    {
      continue;
    }

    this->ConvertToScreenCoord(minVertex, maxVertex);

    BoundingBox box;
    auto boxWidth = maxVertex.x - minVertex.x;
    auto boxHeight = minVertex.y - maxVertex.y;
    box.SetCenter(
        {minVertex.x + boxWidth / 2, maxVertex.y + boxHeight / 2, 0});
    box.SetSize({boxWidth, boxHeight, 0});
    box.SetLabel(entries[index].label);

    this->dataPtr->indexBoxes.emplace_back(index, std::move(box));
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
  // item to clip space, so each vertex costs one matrix-vector product
  Ogre::Matrix4 world;
  world.makeTransform(_position, _scale, _orientation);
  Ogre2BoundingBoxCameraPrivate::ProjectedExtent(
      _projMatrix * _viewMatrix * world,
      this->dataPtr->LocalMeshPoints(_mesh.get()).points,
      _minVertex, _maxVertex);
}

/////////////////////////////////////////////////
bool Ogre2BoundingBoxCameraPrivate::ProjectedExtent(
    const Ogre::Matrix4 &_toClip, const std::vector<Ogre::Vector3> &_points,
    Ogre::Vector3 &_min, Ogre::Vector3 &_max)
{
  bool inFront = true;
  _min.x = std::numeric_limits<float>::max();
  _min.y = std::numeric_limits<float>::max();
  _min.z = std::numeric_limits<float>::max();
  _max.x = -std::numeric_limits<float>::max();
  _max.y = -std::numeric_limits<float>::max();
  _max.z = -std::numeric_limits<float>::max();

  for (const Ogre::Vector3 &p : _points)
  {
    const Ogre::Vector4 clip = _toClip * Ogre::Vector4(p.x, p.y, p.z, 1.0f);
    inFront = inFront && clip.w > 0.0f;

    // homogeneous
    const float x = clip.x / clip.w;
    const float y = clip.y / clip.w;

    _min.x = std::min(_min.x, x);
    _min.y = std::min(_min.y, y);
    _min.z = std::min(_min.z, clip.z);

    _max.x = std::max(_max.x, x);
    _max.y = std::max(_max.y, y);
    _max.z = std::max(_max.z, clip.z);
  }
  return inFront;
}

/////////////////////////////////////////////////
void Ogre2BoundingBoxCameraPrivate::ExtremePoints(
    const std::vector<Ogre::Vector3> &_points,
    std::vector<Ogre::Vector3> &_out)
{
  _out = _points;
  // not worth it for small meshes
  if (_points.size() < 64u)
    return;

  // Work in double, around the center
  using V = math::Vector3d;
  V center;
  for (const auto &p : _points)
    center += V(p.x, p.y, p.z);
  center /= static_cast<double>(_points.size());
  std::vector<V> pts;
  pts.reserve(_points.size());
  double extent = 0;
  for (const auto &p : _points)
  {
    pts.push_back(V(p.x, p.y, p.z) - center);
    extent = std::max(extent, pts.back().Abs().Max());
  }
  if (!(extent > 0))
    return;

  // S: the extreme points along 26 directions (cube faces, edges, corners)
  std::vector<size_t> s;
  for (int i = -1; i <= 1; ++i)
  {
    for (int j = -1; j <= 1; ++j)
    {
      for (int k = -1; k <= 1; ++k)
      {
        if (i == 0 && j == 0 && k == 0)
          continue;
        const V dir(i, j, k);
        size_t best = 0;
        for (size_t n = 1; n < pts.size(); ++n)
        {
          if (pts[n].Dot(dir) > pts[best].Dot(dir))
            best = n;
        }
        s.push_back(best);
      }
    }
  }
  std::sort(s.begin(), s.end());
  s.erase(std::unique(s.begin(), s.end()), s.end());
  if (s.size() < 4u)
    return;

  // All the facet planes of the hull of S, by brute force (S is small):
  // planes through 3 points of S with all of S on their inner side. Every
  // facet of the hull is found, so the planes enclose exactly that hull.
  struct Plane
  {
    V normal;
    double d;
  };
  std::vector<Plane> planes;
  const double tol = 1e-9 * extent;
  for (size_t a = 0; a < s.size(); ++a)
  {
    for (size_t b = a + 1; b < s.size(); ++b)
    {
      for (size_t c = b + 1; c < s.size(); ++c)
      {
        const V &pa = pts[s[a]];
        V normal = (pts[s[b]] - pa).Cross(pts[s[c]] - pa);
        if (normal.Length() < 1e-12 * extent * extent)
          continue;
        normal.Normalize();
        const double d = -normal.Dot(pa);
        bool allBelow = true;
        bool allAbove = true;
        for (size_t i : s)
        {
          const double side = normal.Dot(pts[i]) + d;
          allBelow = allBelow && side <= tol;
          allAbove = allAbove && side >= -tol;
        }
        if (allBelow && allAbove)
        {
          // S is flat: no inside, keep everything
          return;
        }
        if (allBelow)
          planes.push_back({normal, d});
        else if (allAbove)
          planes.push_back({-normal, -d});
      }
    }
  }
  if (planes.size() < 4u)
    return;

  // Drop the points strictly inside all planes, with a margin far larger
  // than float rounding: they are inside the hull of S, which is inside the
  // hull of the kept points, so they are never the extreme of a projection.
  const double margin = 1e-4 * extent;
  _out.clear();
  for (size_t n = 0; n < pts.size(); ++n)
  {
    bool inside = true;
    for (const Plane &plane : planes)
    {
      if (plane.normal.Dot(pts[n]) + plane.d >= -margin)
      {
        inside = false;
        break;
      }
    }
    if (!inside)
      _out.push_back(_points[n]);
  }
}

/////////////////////////////////////////////////
void Ogre2BoundingBoxCamera::DrawLine(unsigned char *_data,
  const math::Vector2i &_point1, const math::Vector2i &_point2,
  const math::Color &_color) const
{
  GZ_PROFILE("Ogre2BoundingBoxCamera::DrawLine");
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
      _data[index] = static_cast<unsigned char>(255 * _color.R());
      _data[index + 1] = static_cast<unsigned char>(255 * _color.G());
      _data[index + 2] = static_cast<unsigned char>(255 * _color.B());

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
      _data[index] = static_cast<unsigned char>(255 * _color.R());
      _data[index + 1] = static_cast<unsigned char>(255 * _color.G());
      _data[index + 2] = static_cast<unsigned char>(255 * _color.B());

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

    // clip the values outside the frustum range [-1, 1]
    /*

        1 -------- 0
       /|         /|
      2 -------- 3 .
      | |        | |
      . 5 -------- 4
      |/         |/
      6 -------- 7

    */

    std::vector<math::Vector2d> clippedVertices;
    const math::Vector4d box(-1.0, -1.0, 1.0, 1.0);
    // top
    this->dataPtr->AddToViewportLines(box, vertices2d[0], vertices2d[1],
        clippedVertices);
    this->dataPtr->AddToViewportLines(box, vertices2d[1], vertices2d[2],
        clippedVertices);
    this->dataPtr->AddToViewportLines(box, vertices2d[2], vertices2d[3],
        clippedVertices);
    this->dataPtr->AddToViewportLines(box, vertices2d[3], vertices2d[0],
        clippedVertices);
    // bottom
    this->dataPtr->AddToViewportLines(box, vertices2d[4], vertices2d[5],
        clippedVertices);
    this->dataPtr->AddToViewportLines(box, vertices2d[5], vertices2d[6],
        clippedVertices);
    this->dataPtr->AddToViewportLines(box, vertices2d[6], vertices2d[7],
        clippedVertices);
    this->dataPtr->AddToViewportLines(box, vertices2d[7], vertices2d[4],
        clippedVertices);
    // pillars
    this->dataPtr->AddToViewportLines(box, vertices2d[0], vertices2d[4],
        clippedVertices);
    this->dataPtr->AddToViewportLines(box, vertices2d[1], vertices2d[5],
        clippedVertices);
    this->dataPtr->AddToViewportLines(box, vertices2d[2], vertices2d[6],
        clippedVertices);
    this->dataPtr->AddToViewportLines(box, vertices2d[3], vertices2d[7],
        clippedVertices);

    // Convert To screen coord.
    uint32_t width = this->ImageWidth();
    uint32_t height = this->ImageHeight();

    std::vector<math::Vector2i> projVertices;
    projVertices.reserve(clippedVertices.size());
    for (auto &vertex : clippedVertices)
    {
      // convert from [-1, 1] range to [0, 1] range & to the screen range
      vertex.X() = uint32_t((vertex.X() + 1.0) / 2 * width );
      vertex.Y() = uint32_t((1.0 - vertex.Y()) / 2 * height);

      vertex.X() = std::max<double>(0.0, vertex.X());
      vertex.Y() = std::max<double>(0.0, vertex.Y());
      vertex.X() = std::min<double>(vertex.X(), width - 1.0);
      vertex.Y() = std::min<double>(vertex.Y(), height - 1.0);

      projVertices.push_back(math::Vector2i(static_cast<int>(vertex.X()),
          static_cast<int>(vertex.Y())));
    }

    for (size_t endPt = 0; endPt < projVertices.size(); endPt += 2)
    {
      this->DrawLine(_data, projVertices[endPt], projVertices[endPt + 1],
          _color);
    }

    return;
  }

  // 2D box
  math::Vector2i minVertex(
    static_cast<int>(_box.Center().X() - _box.Size().X() / 2),
    static_cast<int>(_box.Center().Y() - _box.Size().Y() / 2));

  math::Vector2i maxVertex(
    static_cast<int>(_box.Center().X() + _box.Size().X() / 2),
    static_cast<int>(_box.Center().Y() + _box.Size().Y() / 2));

  uint32_t width = this->ImageWidth();

  std::vector<uint32_t> xValues =
    {uint32_t(minVertex.X()),
        std::min(this->ImageWidth() - 1, uint32_t(maxVertex.X()))};
  std::vector<uint32_t> yValues =
    {uint32_t(minVertex.Y()),
        std::min(this->ImageHeight() - 1, uint32_t(maxVertex.Y()))};

  for (auto i = minVertex.Y(); i < maxVertex.Y(); i++)
  {
    for (auto j : xValues)
    {
      auto index = (i * width + j) * 3;
      _data[index] = static_cast<unsigned char>(255 * _color.R());
      _data[index + 1] = static_cast<unsigned char>(255 * _color.G());
      _data[index + 2] = static_cast<unsigned char>(255 * _color.B());
    }
  }
  for (auto i : yValues)
  {
    for (auto j = minVertex.X(); j < maxVertex.X(); j++)
    {
      auto index = (i * width + j) * 3;
      _data[index] = static_cast<unsigned char>(255 * _color.R());
      _data[index + 1] = static_cast<unsigned char>(255 * _color.G());
      _data[index + 2] = static_cast<unsigned char>(255 * _color.B());
    }
  }
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
