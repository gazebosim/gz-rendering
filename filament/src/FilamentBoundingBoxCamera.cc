/*
 * Copyright (C) 2026 Open Source Robotics Foundation
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

// The box computations are a port of ogre2/src/Ogre2BoundingBoxCamera.cc,
// kept line-for-line where possible so both engines report the same boxes.
// ponytail: the CPU half is duplicated from ogre2; move it into
// BaseBoundingBoxCamera if the filament engine is ever merged.

#include <algorithm>
#include <cmath>
#include <limits>
#include <map>
#include <variant>

#include <filament/Material.h>
#include <filament/MaterialInstance.h>
#include <filament/RenderableManager.h>
#include <filament/Box.h>
#include <filament/View.h>

#include <gz/common/Console.hh>
#include <gz/common/Mesh.hh>
#include <gz/common/SubMesh.hh>
#include <gz/math/OrientedBox.hh>
#include <gz/math/Vector4.hh>
#include <gz/math/eigen3/Util.hh>

#include "gz/rendering/Image.hh"
#include "gz/rendering/Utils.hh"
#include "gz/rendering/filament/FilamentBoundingBoxCamera.hh"
#include "gz/rendering/filament/FilamentConversions.hh"
#include "gz/rendering/filament/FilamentDynamicMesh.hh"
#include "gz/rendering/filament/FilamentMesh.hh"
#include "gz/rendering/filament/FilamentMeshFactory.hh"
#include "gz/rendering/filament/FilamentRenderEngine.hh"
#include "gz/rendering/filament/FilamentRenderTarget.hh"
#include "gz/rendering/filament/FilamentScene.hh"

using namespace gz;
using namespace rendering;

namespace
{
  /// \brief Same value and channel as ogre2's material switcher
  const uint32_t kBackgroundLabel = 255u;

  /// \brief _m * (_v, 1)
  math::Vector4d Mul(const math::Matrix4d &_m, const math::Vector3d &_v)
  {
    math::Vector4d r;
    for (int i = 0; i < 4; ++i)
    {
      r[i] = _m(i, 0) * _v.X() + _m(i, 1) * _v.Y() + _m(i, 2) * _v.Z() +
          _m(i, 3);
    }
    return r;
  }

  /// \brief Frustum test of a world box (what Ogre::Camera::isVisible does):
  /// invisible when all 8 corners are outside the same clip plane.
  bool InFrustum(const math::AxisAlignedBox &_box, const math::Matrix4d &_pv)
  {
    if (_box.Min().X() > _box.Max().X())
      return false;
    int out[6] = {0, 0, 0, 0, 0, 0};
    for (int i = 0; i < 8; ++i)
    {
      const math::Vector3d corner(
          (i & 1) ? _box.Max().X() : _box.Min().X(),
          (i & 2) ? _box.Max().Y() : _box.Min().Y(),
          (i & 4) ? _box.Max().Z() : _box.Min().Z());
      const math::Vector4d c = Mul(_pv, corner);
      for (int axis = 0; axis < 3; ++axis)
      {
        if (c[axis] < -c[3])
          ++out[axis * 2];
        if (c[axis] > c[3])
          ++out[axis * 2 + 1];
      }
    }
    return std::none_of(std::begin(out), std::end(out),
        [](int _n) { return _n == 8; });
  }

  /// \brief The mesh's local box (scaled) and world box, like ogre2's
  /// item->getLocalAabb() * scale and item->getWorldAabb()
  bool ItemBoxes(const FilamentMesh *_mesh, const VisualPtr &_visual,
      math::AxisAlignedBox &_local, math::AxisAlignedBox &_world)
  {
    auto &rm = FilamentRenderEngine::Instance()->FilamentEngine()
        ->getRenderableManager();
    const ::utils::Entity e = _mesh->Entity();
    if (e.isNull() || !rm.hasComponent(e))
      return false;
    const filament::Box bb = rm.getAxisAlignedBoundingBox(rm.getInstance(e));
    const math::Vector3d scale = _visual->WorldScale();
    _local = math::AxisAlignedBox(
        scale * math::Vector3d(bb.getMin().x, bb.getMin().y, bb.getMin().z),
        scale * math::Vector3d(bb.getMax().x, bb.getMax().y, bb.getMax().z));
    _world = transformAxisAlignedBox(_local, _visual->WorldPose());
    return true;
  }

  /// \brief Calls _fn with every vertex of the mesh, in world coordinates
  template <typename F>
  void ForEachWorldVertex(const FilamentMesh *_mesh, const VisualPtr &_visual,
      F _fn)
  {
    MeshDescriptor desc = _mesh->Descriptor();
    desc.Load();
    if (!desc.mesh)
      return;
    const math::Pose3d pose = _visual->WorldPose();
    const math::Vector3d scale = _visual->WorldScale();
    for (unsigned int i = 0; i < desc.mesh->SubMeshCount(); ++i)
    {
      auto s = desc.mesh->SubMeshByIndex(i).lock();
      if (!s || (!desc.subMeshName.empty() && s->Name() != desc.subMeshName))
        continue;
      const math::Vector3d center =
          FilamentMeshFactory::SubMeshCenter(*s, desc);
      for (unsigned int v = 0; v < s->VertexCount(); ++v)
        _fn(pose.Rot() * ((s->Vertex(v) - center) * scale) + pose.Pos());
    }
  }

  // Cohen-Sutherland line clipping, as in ogre2
  const int kInside = 0;
  const int kLeft = 1;
  const int kRight = 2;
  const int kBottom = 4;
  const int kTop = 8;

  int LocationRelativeToViewPort(const math::Vector4d &_bounds, double _x,
      double _y)
  {
    int relativeLocation = kInside;
    if (_x < _bounds[0])
      relativeLocation |= kLeft;
    else if (_x > _bounds[2])
      relativeLocation |= kRight;
    if (_y < _bounds[1])
      relativeLocation |= kBottom;
    else if (_y > _bounds[3])
      relativeLocation |= kTop;
    return relativeLocation;
  }

  void AddToViewportLines(const math::Vector4d &_bounds,
      const math::Vector2d &_p0, const math::Vector2d &_p1,
      std::vector<math::Vector2d> &_lines)
  {
    const double xmin = _bounds[0];
    const double ymin = _bounds[1];
    const double xmax = _bounds[2];
    const double ymax = _bounds[3];
    double x0 = _p0.X();
    double y0 = _p0.Y();
    double x1 = _p1.X();
    double y1 = _p1.Y();
    int location0 = LocationRelativeToViewPort(_bounds, x0, y0);
    int location1 = LocationRelativeToViewPort(_bounds, x1, y1);
    while (true)
    {
      if (!(location0 | location1))
      {
        _lines.push_back(math::Vector2d(x0, y0));
        _lines.push_back(math::Vector2d(x1, y1));
        return;
      }
      if (location0 & location1)
        return;
      double x = 0.0;
      double y = 0.0;
      const int outerLocation = location1 > location0 ? location1 : location0;
      if (outerLocation & kTop)
      {
        x = x0 + (x1 - x0) * (ymax - y0) / (y1 - y0);
        y = ymax;
      }
      else if (outerLocation & kBottom)
      {
        x = x0 + (x1 - x0) * (ymin - y0) / (y1 - y0);
        y = ymin;
      }
      else if (outerLocation & kRight)
      {
        y = y0 + (y1 - y0) * (xmax - x0) / (x1 - x0);
        x = xmax;
      }
      else
      {
        y = y0 + (y1 - y0) * (xmin - x0) / (x1 - x0);
        x = xmin;
      }
      if (outerLocation == location0)
      {
        x0 = x;
        y0 = y;
        location0 = LocationRelativeToViewPort(_bounds, x0, y0);
      }
      else
      {
        x1 = x;
        y1 = y;
        location1 = LocationRelativeToViewPort(_bounds, x1, y1);
      }
    }
  }

  /// \brief ogre2's MergeBoxes2D, including its maxX/maxY start at 0
  BoundingBox MergeBoxes2D(const std::vector<BoundingBox> &_boxes)
  {
    if (_boxes.size() == 1)
      return _boxes[0];
    double minX = std::numeric_limits<double>::max();
    double maxX = 0.0;
    double minY = std::numeric_limits<double>::max();
    double maxY = 0.0;
    for (const auto &box : _boxes)
    {
      minX = std::min(minX, box.Center().X() - box.Size().X() * 0.5);
      maxX = std::max(maxX, box.Center().X() + box.Size().X() * 0.5);
      minY = std::min(minY, box.Center().Y() - box.Size().Y() * 0.5);
      maxY = std::max(maxY, box.Center().Y() + box.Size().Y() * 0.5);
    }
    BoundingBox merged;
    const double width = maxX - minX;
    const double height = maxY - minY;
    merged.SetSize({width, height, 0});
    merged.SetCenter({minX + width * 0.5, minY + height * 0.5, 0});
    merged.SetLabel(_boxes[0].Label());
    return merged;
  }
}

//////////////////////////////////////////////////
FilamentBoundingBoxCamera::FilamentBoundingBoxCamera()
{
}

//////////////////////////////////////////////////
FilamentBoundingBoxCamera::~FilamentBoundingBoxCamera()
{
}

//////////////////////////////////////////////////
void FilamentBoundingBoxCamera::Init()
{
  FilamentCamera::Init();
  this->view->setShadowingEnabled(false);
  this->view->setPostProcessingEnabled(false);
  // ids and labels must be written as is, not sRGB encoded
  this->renderTexture->SetSrgb(false);
}

//////////////////////////////////////////////////
void FilamentBoundingBoxCamera::Destroy()
{
  auto *engine = FilamentRenderEngine::Instance()->FilamentEngine();
  if (engine)
  {
    for (auto &kv : this->flatInstances)
      engine->destroy(kv.second);
  }
  this->flatInstances.clear();
  this->items.clear();
  FilamentCamera::Destroy();
}

//////////////////////////////////////////////////
RenderTargetPtr FilamentBoundingBoxCamera::RenderTarget() const
{
  return this->renderTexture;
}

//////////////////////////////////////////////////
filament::MaterialInstance *FilamentBoundingBoxCamera::FlatInstance(
    uint32_t _key)
{
  auto it = this->flatInstances.find(_key);
  if (it != this->flatInstances.end())
    return it->second;
  filament::Material *mat =
      FilamentRenderEngine::Instance()->Material("gz_flat");
  if (!mat)
  {
    gzerr << "gz_flat material not available" << std::endl;
    return nullptr;
  }
  filament::MaterialInstance *mi = mat->createInstance();
  mi->setParameter("color", filament::math::float3{
      ((_key >> 16) & 0xff) / 255.0f, ((_key >> 8) & 0xff) / 255.0f,
      (_key & 0xff) / 255.0f});
  this->flatInstances[_key] = mi;
  return mi;
}

//////////////////////////////////////////////////
void FilamentBoundingBoxCamera::Render()
{
  if (this->renderTexture->Width() != this->ImageWidth() ||
      this->renderTexture->Height() != this->ImageHeight())
  {
    this->renderTexture->SetWidth(this->ImageWidth());
    this->renderTexture->SetHeight(this->ImageHeight());
  }
  this->renderTexture->PreRender();
  this->UpdateProjection();
  const float bg = kBackgroundLabel / 255.0f;
  this->renderTexture->SetClearColor({bg, bg, bg, 1.0f});
  this->view->setVisibleLayers(0xff,
      FilamentConversions::ToSensorLayers(this->VisibilityMask()));

  // Items in creation order, like ogre ids. Each one is drawn with
  // R = index % 256, G = index / 256, B = label.
  this->items.clear();
  for (FilamentMesh *m : this->scene->Meshes())
  {
    VisualPtr v = m->Parent();
    if (!v)
      continue;
    VisualPtr top = v;
    while (top->Parent() && top->Parent() != this->scene->RootVisual())
      top = std::dynamic_pointer_cast<Visual>(top->Parent());
    this->items.push_back({m, v, top->Name()});
  }
  std::sort(this->items.begin(), this->items.end(),
      [](const Item &_a, const Item &_b)
      { return _a.mesh->Id() < _b.mesh->Id(); });
  if (this->items.size() > 65536u)
  {
    gzwarn << "Bounding box camera supports 65536 meshes, ignoring the rest"
           << std::endl;
    this->items.resize(65536u);
  }

  for (size_t i = 0; i < this->items.size(); ++i)
  {
    // terrain occludes but never gets a box: ogre2's Terra is not an Item
    uint32_t label = kBackgroundLabel;
    Variant labelAny = this->items[i].visual->UserData("label");
    const int *labelPtr = std::get_if<int>(&labelAny);
    if (labelPtr && !this->items[i].mesh->terrain)
      label = static_cast<uint32_t>(*labelPtr) & 0xffu;
    const uint32_t key = ((i % 256u) << 16) | ((i / 256u) << 8) | label;
    if (filament::MaterialInstance *mi = this->FlatInstance(key))
    {
      // ogre2's plain_color material has no alpha test
      this->items[i].mesh->OverrideMaterial(mi, false);
      // ogre2's bounding box camera sees no marker points (measured with
      // filament_parity PARITY_MARKERS=1), unlike the other sensors
      if (auto *dyn = dynamic_cast<FilamentDynamicMesh *>(
          this->items[i].mesh))
        dyn->UseDots(false);
    }
  }

  this->renderTexture->Render(this->view);
  for (Item &item : this->items)
    item.mesh->RestoreMaterials();
}

//////////////////////////////////////////////////
void FilamentBoundingBoxCamera::PostRender()
{
  if (this->newBoundingBoxes.ConnectionCount() == 0u)
    return;

  const unsigned int width = this->ImageWidth();
  const unsigned int height = this->ImageHeight();
  Image image(width, height, PF_R8G8B8);
  this->renderTexture->Copy(image);
  const uint8_t *src = image.Data<uint8_t>();
  this->buffer.assign(src, src + static_cast<size_t>(width) * height * 3u);

  this->boundingBoxes.clear();
  if (this->type == BoundingBoxType::BBT_VISIBLEBOX2D)
    this->VisibleBoundingBoxes();
  else if (this->type == BoundingBoxType::BBT_FULLBOX2D)
    this->FullBoundingBoxes();
  else if (this->type == BoundingBoxType::BBT_BOX3D)
    this->BoundingBoxes3D();

  this->newBoundingBoxes(this->boundingBoxes);
}

//////////////////////////////////////////////////
std::map<uint32_t, uint32_t> FilamentBoundingBoxCamera::MarkVisibleBoxes()
    const
{
  std::map<uint32_t, uint32_t> visible;
  for (size_t index = 0; index + 2 < this->buffer.size(); index += 3)
  {
    const uint32_t label = this->buffer[index + 2];
    if (label == kBackgroundLabel)
      continue;
    const uint32_t itemId = this->buffer[index + 1] * 256u + this->buffer[index];
    visible.emplace(itemId, label);
  }
  return visible;
}

//////////////////////////////////////////////////
void FilamentBoundingBoxCamera::VisibleBoundingBoxes()
{
  const uint32_t width = this->ImageWidth();
  const uint32_t height = this->ImageHeight();
  struct Boundary
  {
    uint32_t minX, minY, maxX, maxY, label;
  };
  std::map<uint32_t, Boundary> bounds;
  for (uint32_t y = 0; y < height; ++y)
  {
    for (uint32_t x = 0; x < width; ++x)
    {
      const size_t index = (static_cast<size_t>(y) * width + x) * 3u;
      const uint32_t label = this->buffer[index + 2];
      if (label == kBackgroundLabel)
        continue;
      const uint32_t itemId =
          this->buffer[index + 1] * 256u + this->buffer[index];
      auto it = bounds.try_emplace(itemId, Boundary{width, height, 0, 0, label})
          .first;
      Boundary &b = it->second;
      b.minX = std::min(b.minX, x);
      b.minY = std::min(b.minY, y);
      b.maxX = std::max(b.maxX, x);
      b.maxY = std::max(b.maxY, y);
    }
  }

  // merge multi-link models (grouped by top level visual name)
  std::map<std::string, std::vector<BoundingBox>> parentToBoxes;
  for (const auto &[itemId, b] : bounds)
  {
    BoundingBox box;
    box.SetLabel(b.label);
    const uint32_t boxWidth = b.maxX - b.minX;
    const uint32_t boxHeight = b.maxY - b.minY;
    box.SetCenter({b.minX + boxWidth * 0.5, b.minY + boxHeight * 0.5, 0});
    box.SetSize({static_cast<double>(boxWidth),
        static_cast<double>(boxHeight), 0.0});
    const std::string parentName = itemId < this->items.size() ?
        this->items[itemId].parentName : std::string();
    parentToBoxes[parentName].push_back(box);
  }
  for (const auto &kv : parentToBoxes)
    this->boundingBoxes.push_back(MergeBoxes2D(kv.second));
  std::reverse(this->boundingBoxes.begin(), this->boundingBoxes.end());
}

//////////////////////////////////////////////////
void FilamentBoundingBoxCamera::FullBoundingBoxes()
{
  const std::map<uint32_t, uint32_t> visible = this->MarkVisibleBoxes();
  const math::Matrix4d pv = this->ProjectionMatrix() * this->ViewMatrix();
  const double width = this->ImageWidth();
  const double height = this->ImageHeight();

  std::map<std::string, std::vector<BoundingBox>> parentToBoxes;
  for (const auto &[itemId, label] : visible)
  {
    if (itemId >= this->items.size())
      continue;
    const Item &item = this->items[itemId];
    math::AxisAlignedBox local, world;
    if (!ItemBoxes(item.mesh, item.visual, local, world) ||
        !InFrustum(world, pv))
      continue;

    // MeshMinimalBox: projected extents of every vertex
    math::Vector2d minV(std::numeric_limits<double>::max(),
        std::numeric_limits<double>::max());
    math::Vector2d maxV(-std::numeric_limits<double>::max(),
        -std::numeric_limits<double>::max());
    ForEachWorldVertex(item.mesh, item.visual,
        [&](const math::Vector3d &_p)
        {
          const math::Vector4d c = Mul(pv, _p);
          const double x = c[0] / c[3];
          const double y = c[1] / c[3];
          minV.X(std::min(minV.X(), x));
          minV.Y(std::min(minV.Y(), y));
          maxV.X(std::max(maxV.X(), x));
          maxV.Y(std::max(maxV.Y(), y));
        });

    if ((std::abs(minV.X()) > 1 && std::abs(maxV.X()) > 1) ||
        (std::abs(minV.Y()) > 1 && std::abs(maxV.Y()) > 1))
      continue;

    // ConvertToScreenCoord
    minV.X(std::clamp(minV.X(), -1.0, 1.0));
    minV.Y(std::clamp(minV.Y(), -1.0, 1.0));
    maxV.X(std::clamp(maxV.X(), -1.0, 1.0));
    maxV.Y(std::clamp(maxV.Y(), -1.0, 1.0));
    minV.X((minV.X() + 1.0) / 2 * width);
    minV.Y((1.0 - minV.Y()) / 2 * height);
    maxV.X((maxV.X() + 1.0) / 2 * width);
    maxV.Y((1.0 - maxV.Y()) / 2 * height);
    minV.X(std::max(0.0, minV.X()));
    minV.Y(std::max(0.0, minV.Y()));
    maxV.X(std::min(maxV.X(), width - 1.0));
    maxV.Y(std::min(maxV.Y(), height - 1.0));

    BoundingBox box;
    const double boxWidth = maxV.X() - minV.X();
    const double boxHeight = minV.Y() - maxV.Y();
    box.SetCenter({minV.X() + boxWidth / 2, maxV.Y() + boxHeight / 2, 0});
    box.SetSize({boxWidth, boxHeight, 0});
    box.SetLabel(label);
    parentToBoxes[item.parentName].push_back(box);
  }
  for (const auto &kv : parentToBoxes)
    this->boundingBoxes.push_back(MergeBoxes2D(kv.second));
  std::reverse(this->boundingBoxes.begin(), this->boundingBoxes.end());
}

//////////////////////////////////////////////////
void FilamentBoundingBoxCamera::BoundingBoxes3D()
{
  const std::map<uint32_t, uint32_t> visible = this->MarkVisibleBoxes();
  const math::Matrix4d viewMatrix = this->ViewMatrix();
  const math::Matrix4d pv = this->ProjectionMatrix() * viewMatrix;
  const math::Quaterniond worldCameraRotation = viewMatrix.Rotation();

  std::map<std::string, std::vector<uint32_t>> parentToIds;
  std::map<uint32_t, BoundingBox> boxes;
  for (const auto &[itemId, label] : visible)
  {
    if (itemId >= this->items.size())
      continue;
    const Item &item = this->items[itemId];
    math::AxisAlignedBox local, world;
    if (!ItemBoxes(item.mesh, item.visual, local, world) ||
        !InFrustum(world, pv))
      continue;

    BoundingBox box;
    const math::Vector4d c = Mul(viewMatrix, world.Center());
    box.SetCenter({c[0], c[1], c[2]});
    box.SetSize(local.Size());
    box.SetOrientation(worldCameraRotation * item.visual->WorldPose().Rot());
    box.SetLabel(label);
    boxes[itemId] = box;
    parentToIds[item.parentName].push_back(itemId);
  }

  // MergeMultiLinksModels3D
  for (const auto &kv : parentToIds)
  {
    const std::vector<uint32_t> &ids = kv.second;
    if (ids.size() == 1)
    {
      this->boundingBoxes.push_back(boxes[ids[0]]);
      continue;
    }
    std::vector<math::Vector3d> vertices;
    for (uint32_t itemId : ids)
    {
      ForEachWorldVertex(this->items[itemId].mesh, this->items[itemId].visual,
          [&](const math::Vector3d &_p)
          {
            const math::Vector4d v = Mul(viewMatrix, _p);
            vertices.emplace_back(v[0], v[1], v[2]);
          });
    }
    const math::OrientedBoxd merged =
        math::eigen3::verticesToOrientedBox(vertices);
    BoundingBox box;
    box.SetCenter(merged.Pose().Pos());
    box.SetOrientation(merged.Pose().Rot());
    box.SetSize(merged.Size());
    box.SetLabel(boxes[ids[0]].Label());
    this->boundingBoxes.push_back(box);
  }
  std::reverse(this->boundingBoxes.begin(), this->boundingBoxes.end());
}

//////////////////////////////////////////////////
common::ConnectionPtr FilamentBoundingBoxCamera::ConnectNewBoundingBoxes(
    std::function<void(const std::vector<BoundingBox> &)> _subscriber)
{
  return this->newBoundingBoxes.Connect(_subscriber);
}

//////////////////////////////////////////////////
void FilamentBoundingBoxCamera::DrawBoundingBox(unsigned char *_data,
    const math::Color &_color, const BoundingBox &_box) const
{
  const uint32_t width = this->ImageWidth();
  const uint32_t height = this->ImageHeight();
  auto plot = [&](int _x, int _y)
  {
    const size_t index = (static_cast<size_t>(_y) * width + _x) * 3u;
    _data[index] = static_cast<unsigned char>(255 * _color.R());
    _data[index + 1] = static_cast<unsigned char>(255 * _color.G());
    _data[index + 2] = static_cast<unsigned char>(255 * _color.B());
  };

  if (this->Type() == BoundingBoxType::BBT_BOX3D)
  {
    const math::Matrix4d proj = this->ProjectionMatrix();
    std::vector<math::Vector2d> vertices2d;
    for (const auto &vertex : _box.Vertices3D())
    {
      // skip boxes with any vertex behind the camera
      if (vertex.Z() > 0)
        return;
      const math::Vector4d p = Mul(proj, vertex);
      vertices2d.push_back({p[0] / p[3], p[1] / p[3]});
    }

    // edges: top 0-1-2-3, bottom 4-5-6-7, pillars i-(i+4)
    static const int kEdges[12][2] = {{0, 1}, {1, 2}, {2, 3}, {3, 0},
        {4, 5}, {5, 6}, {6, 7}, {7, 4}, {0, 4}, {1, 5}, {2, 6}, {3, 7}};
    std::vector<math::Vector2d> clipped;
    const math::Vector4d bounds(-1.0, -1.0, 1.0, 1.0);
    for (const auto &edge : kEdges)
    {
      AddToViewportLines(bounds, vertices2d[edge[0]], vertices2d[edge[1]],
          clipped);
    }

    std::vector<math::Vector2i> pts;
    for (auto &v : clipped)
    {
      double x = uint32_t((v.X() + 1.0) / 2 * width);
      double y = uint32_t((1.0 - v.Y()) / 2 * height);
      x = std::min<double>(std::max(0.0, x), width - 1.0);
      y = std::min<double>(std::max(0.0, y), height - 1.0);
      pts.push_back(math::Vector2i(static_cast<int>(x), static_cast<int>(y)));
    }

    // Bresenham, as in ogre2 (the last pixel is not drawn)
    for (size_t k = 0; k + 1 < pts.size(); k += 2)
    {
      math::Vector2i p1 = pts[k];
      math::Vector2i p2 = pts[k + 1];
      const bool shallow =
          std::abs(p2.Y() - p1.Y()) < std::abs(p2.X() - p1.X());
      // walk along the major axis, from low to high
      const bool swap = shallow ? p1.X() > p2.X() : p1.Y() > p2.Y();
      if (swap)
        std::swap(p1, p2);
      int major0 = shallow ? p1.X() : p1.Y();
      int major1 = shallow ? p2.X() : p2.Y();
      int minor = shallow ? p1.Y() : p1.X();
      int dMajor = major1 - major0;
      int dMinor = shallow ? p2.Y() - p1.Y() : p2.X() - p1.X();
      int step = 1;
      if (dMinor < 0)
      {
        step = -1;
        dMinor = -dMinor;
      }
      int d = 2 * dMinor - dMajor;
      for (int m = major0; m < major1; ++m)
      {
        if (shallow)
          plot(m, minor);
        else
          plot(minor, m);
        if (d > 0)
        {
          minor += step;
          d += 2 * (dMinor - dMajor);
        }
        else
        {
          d += 2 * dMinor;
        }
      }
    }
    return;
  }

  // 2D box
  const math::Vector2i minV(
      static_cast<int>(_box.Center().X() - _box.Size().X() / 2),
      static_cast<int>(_box.Center().Y() - _box.Size().Y() / 2));
  const math::Vector2i maxV(
      static_cast<int>(_box.Center().X() + _box.Size().X() / 2),
      static_cast<int>(_box.Center().Y() + _box.Size().Y() / 2));
  const uint32_t xValues[2] = {uint32_t(minV.X()),
      std::min(width - 1, uint32_t(maxV.X()))};
  const uint32_t yValues[2] = {uint32_t(minV.Y()),
      std::min(height - 1, uint32_t(maxV.Y()))};
  for (int i = minV.Y(); i < maxV.Y(); ++i)
    for (uint32_t j : xValues)
      plot(static_cast<int>(j), i);
  for (uint32_t i : yValues)
    for (int j = minV.X(); j < maxV.X(); ++j)
      plot(j, static_cast<int>(i));
}
