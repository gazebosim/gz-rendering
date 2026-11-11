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
#include "FilamentUtils.hh"

using namespace gz;
using namespace rendering;

namespace
{
  /// \brief Same value and channel as ogre2's material switcher
  const uint32_t kBackgroundLabel = 255u;

  using FilamentConversions::Transform;

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
      const math::Vector4d c = Transform(_pv, corner);
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

}

//////////////////////////////////////////////////
void FilamentBoundingBoxCamera::Init()
{
  FilamentCamera::Init();
  this->view->setShadowingEnabled(false);
  // ids and labels must be written as is, not sRGB encoded
  this->renderTexture->SetSrgb(false);
}

//////////////////////////////////////////////////
void FilamentBoundingBoxCamera::Destroy()
{
  filament_utils::DestroyFlatInstances(this->flatInstances);
  this->items.clear();
  FilamentCamera::Destroy();
}

//////////////////////////////////////////////////
RenderTargetPtr FilamentBoundingBoxCamera::RenderTarget() const
{
  return this->renderTexture;
}

//////////////////////////////////////////////////
void FilamentBoundingBoxCamera::Render()
{
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
    const math::Color color((i % 256u) / 255.0f, (i / 256u) / 255.0f,
        label / 255.0f);
    if (filament::MaterialInstance *mi =
        filament_utils::FlatInstance(this->flatInstances, color))
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
  if (this->image.Width() != width || this->image.Height() != height)
    this->image = Image(width, height, PF_R8G8B8);
  this->renderTexture->Copy(this->image);

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
std::string FilamentBoundingBoxCamera::ParentName(uint32_t _itemId) const
{
  // ids past the items come from meshes drawn without the flat material
  return _itemId < this->items.size() ?
      this->items[_itemId].parentName : std::string();
}

//////////////////////////////////////////////////
void FilamentBoundingBoxCamera::VisibleBoundingBoxes()
{
  this->MergeMultiLinksModels2D(
      this->VisibleBoxes2D(this->image.Data<uint8_t>(), kBackgroundLabel),
      [this](uint32_t _itemId) { return this->ParentName(_itemId); },
      this->boundingBoxes);
}

//////////////////////////////////////////////////
void FilamentBoundingBoxCamera::FullBoundingBoxes()
{
  const math::Matrix4d pv = this->ProjectionMatrix() * this->ViewMatrix();
  const double width = this->ImageWidth();
  const double height = this->ImageHeight();

  std::map<uint32_t, BoundingBox> boxes;
  for (const auto &[itemId, label] :
      this->VisibleBoxesLabel(this->image.Data<uint8_t>(), kBackgroundLabel))
  {
    if (itemId >= this->items.size())
      continue;
    const Item &item = this->items[itemId];
    math::AxisAlignedBox local, world;
    if (!ItemBoxes(item.mesh, item.visual, local, world) ||
        !InFrustum(world, pv))
      continue;

    // projected extents of every vertex
    math::Vector2d minV(std::numeric_limits<double>::max(),
        std::numeric_limits<double>::max());
    math::Vector2d maxV(-std::numeric_limits<double>::max(),
        -std::numeric_limits<double>::max());
    ForEachWorldVertex(item.mesh, item.visual,
        [&](const math::Vector3d &_p)
        {
          const math::Vector4d c = Transform(pv, _p);
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

    // clip to the frustum, convert to pixels and clip to the image
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

    BoundingBox &box = boxes[itemId];
    const double boxWidth = maxV.X() - minV.X();
    const double boxHeight = minV.Y() - maxV.Y();
    box.SetCenter({minV.X() + boxWidth / 2, maxV.Y() + boxHeight / 2, 0});
    box.SetSize({boxWidth, boxHeight, 0});
    box.SetLabel(label);
  }
  this->MergeMultiLinksModels2D(boxes,
      [this](uint32_t _itemId) { return this->ParentName(_itemId); },
      this->boundingBoxes);
}

//////////////////////////////////////////////////
void FilamentBoundingBoxCamera::BoundingBoxes3D()
{
  const math::Matrix4d viewMatrix = this->ViewMatrix();
  const math::Matrix4d pv = this->ProjectionMatrix() * viewMatrix;
  const math::Quaterniond worldCameraRotation = viewMatrix.Rotation();

  std::map<std::string, std::vector<uint32_t>> parentToIds;
  std::map<uint32_t, BoundingBox> boxes;
  for (const auto &[itemId, label] :
      this->VisibleBoxesLabel(this->image.Data<uint8_t>(), kBackgroundLabel))
  {
    if (itemId >= this->items.size())
      continue;
    const Item &item = this->items[itemId];
    math::AxisAlignedBox local, world;
    if (!ItemBoxes(item.mesh, item.visual, local, world) ||
        !InFrustum(world, pv))
      continue;

    BoundingBox box;
    const math::Vector4d c = Transform(viewMatrix, world.Center());
    box.SetCenter({c[0], c[1], c[2]});
    box.SetSize(local.Size());
    box.SetOrientation(worldCameraRotation * item.visual->WorldPose().Rot());
    box.SetLabel(label);
    boxes[itemId] = box;
    parentToIds[item.parentName].push_back(itemId);
  }

  // merge multi-link models into the oriented box of all their vertices
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
            const math::Vector4d v = Transform(viewMatrix, _p);
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
  if (this->Type() != BoundingBoxType::BBT_BOX3D)
  {
    this->DrawBoundingBox2D(_data, _color, _box);
    return;
  }

  const math::Matrix4d proj = this->ProjectionMatrix();
  std::vector<math::Vector2d> vertices2d;
  for (const auto &vertex : _box.Vertices3D())
  {
    // skip boxes with any vertex behind the camera
    if (vertex.Z() > 0)
      return;
    const math::Vector4d p = Transform(proj, vertex);
    vertices2d.push_back({p[0] / p[3], p[1] / p[3]});
  }
  this->DrawBoundingBox3D(_data, _color, vertices2d);
}
