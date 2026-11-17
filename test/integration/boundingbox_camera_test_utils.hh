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

// Helpers shared by the bounding box camera accuracy tests. See
// boundingbox_camera_accuracy.cc for the conventions they assume.

#ifndef GZ_RENDERING_TEST_INTEGRATION_BOUNDINGBOX_CAMERA_TEST_UTILS_HH_
#define GZ_RENDERING_TEST_INTEGRATION_BOUNDINGBOX_CAMERA_TEST_UTILS_HH_

#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <limits>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include "CommonRenderingTest.hh"

#include <gz/common/Event.hh>
#include <gz/common/Filesystem.hh>
#include <gz/common/Mesh.hh>
#include <gz/common/MeshManager.hh>
#include <gz/common/SubMesh.hh>
#include <gz/common/geospatial/ImageHeightmap.hh>
#include <gz/math/Matrix4.hh>
#include <gz/math/Pose3.hh>
#include <gz/math/Vector3.hh>
#include <gz/math/Vector4.hh>
#include <gz/utils/Environment.hh>

#include "gz/rendering/BoundingBoxCamera.hh"
#include "gz/rendering/Heightmap.hh"
#include "gz/rendering/Marker.hh"
#include "gz/rendering/Mesh.hh"
#include "gz/rendering/MeshDescriptor.hh"
#include "gz/rendering/ParticleEmitter.hh"
#include "gz/rendering/Scene.hh"
#include "gz/rendering/SegmentationCamera.hh"
#include "gz/rendering/Visual.hh"

namespace bbox_test
{
using namespace gz;
using namespace gz::rendering;

/// \brief Axis-aligned rectangle in continuous pixel coordinates.
struct Rect
{
  double minX{std::numeric_limits<double>::max()};
  double minY{std::numeric_limits<double>::max()};
  double maxX{-std::numeric_limits<double>::max()};
  double maxY{-std::numeric_limits<double>::max()};

  double Width() const { return this->maxX - this->minX; }
  double Height() const { return this->maxY - this->minY; }
  double CenterX() const { return 0.5 * (this->minX + this->maxX); }
  double CenterY() const { return 0.5 * (this->minY + this->maxY); }

  void Merge(const Rect &_o)
  {
    this->minX = std::min(this->minX, _o.minX);
    this->minY = std::min(this->minY, _o.minY);
    this->maxX = std::max(this->maxX, _o.maxX);
    this->maxY = std::max(this->maxY, _o.maxY);
  }
};

inline std::ostream &operator<<(std::ostream &_out, const Rect &_r)
{
  _out << "[x " << _r.minX << " .. " << _r.maxX << ", y " << _r.minY
       << " .. " << _r.maxY << "]";
  return _out;
}

/// \brief Rect of a 2D bounding box reported by the camera.
inline Rect ToRect(const BoundingBox &_box)
{
  Rect r;
  r.minX = _box.Center().X() - _box.Size().X() * 0.5;
  r.maxX = _box.Center().X() + _box.Size().X() * 0.5;
  r.minY = _box.Center().Y() - _box.Size().Y() * 0.5;
  r.maxY = _box.Center().Y() + _box.Size().Y() * 0.5;
  return r;
}

/// \brief Camera parameters used by the tests. The camera always sits at the
/// world origin with identity rotation, looking along +X (Gazebo convention).
struct CameraModel
{
  unsigned int width{320};
  unsigned int height{240};
  double hfov{GZ_PI / 2.0};
  double nearClip{0.1};
  double farClip{100.0};

  double Aspect() const
  {
    return static_cast<double>(this->width) / this->height;
  }

  /// \brief OpenGL-style projection matrix (optical frame -> clip space),
  /// built from the camera parameters the same way a pinhole camera is.
  math::Matrix4d Projection() const
  {
    const double t = std::tan(this->hfov * 0.5);
    const double n = this->nearClip;
    const double f = this->farClip;
    return math::Matrix4d(
        1.0 / t, 0, 0, 0,
        0, this->Aspect() / t, 0, 0,
        0, 0, -(f + n) / (f - n), -2.0 * f * n / (f - n),
        0, 0, -1, 0);
  }
};

/// \brief Gazebo camera frame (X forward, Y left, Z up) to the optical frame
/// the camera reports 3D boxes in (X right, Y up, Z backwards).
inline math::Vector3d ToOptical(const math::Vector3d &_p)
{
  return {-_p.Y(), _p.Z(), -_p.X()};
}

/// \brief World-frame corners of a box with the given pose and size.
/// Corner i has its x/y/z at +size/2 when bit 0/1/2 of i is set.
inline std::vector<math::Vector3d> BoxCorners(const math::Pose3d &_pose,
    const math::Vector3d &_size)
{
  std::vector<math::Vector3d> corners;
  for (int i = 0; i < 8; ++i)
  {
    math::Vector3d local(
        (i & 1 ? 0.5 : -0.5) * _size.X(),
        (i & 2 ? 0.5 : -0.5) * _size.Y(),
        (i & 4 ? 0.5 : -0.5) * _size.Z());
    corners.push_back(_pose.Pos() + _pose.Rot() * local);
  }
  return corners;
}

/// \brief Expected 2D box of a world-frame box (given by BoxCorners) as seen
/// by a camera at the origin with the given projection. The box is clipped
/// against the near plane, projected and clamped to the image.
/// \return std::nullopt if no part of the box projects inside the image.
inline std::optional<Rect> ExpectedRect(
    const std::vector<math::Vector3d> &_corners,
    const CameraModel &_cam, const math::Matrix4d &_proj)
{
  // Clip against the near plane (x >= near in the Gazebo camera frame): keep
  // the corners in front of it plus the intersections of the 12 box edges
  // with it.
  std::vector<math::Vector3d> pts;
  const double n = _cam.nearClip;
  for (int i = 0; i < 8; ++i)
  {
    if (_corners[i].X() >= n)
      pts.push_back(_corners[i]);
    for (int bit : {1, 2, 4})
    {
      if (i & bit)
        continue;
      const auto &a = _corners[i];
      const auto &b = _corners[i | bit];
      if ((a.X() - n) * (b.X() - n) < 0)
      {
        const double t = (n - a.X()) / (b.X() - a.X());
        pts.push_back(a + (b - a) * t);
      }
    }
  }
  if (pts.empty())
    return std::nullopt;

  Rect r;
  for (const auto &p : pts)
  {
    const math::Vector3d o = ToOptical(p);
    const double cx = _proj(0, 0) * o.X() + _proj(0, 1) * o.Y() +
        _proj(0, 2) * o.Z() + _proj(0, 3);
    const double cy = _proj(1, 0) * o.X() + _proj(1, 1) * o.Y() +
        _proj(1, 2) * o.Z() + _proj(1, 3);
    const double cw = _proj(3, 0) * o.X() + _proj(3, 1) * o.Y() +
        _proj(3, 2) * o.Z() + _proj(3, 3);
    const double u = (cx / cw + 1.0) * 0.5 * _cam.width;
    const double v = (1.0 - cy / cw) * 0.5 * _cam.height;
    r.minX = std::min(r.minX, u);
    r.maxX = std::max(r.maxX, u);
    r.minY = std::min(r.minY, v);
    r.maxY = std::max(r.maxY, v);
  }

  const double w = _cam.width;
  const double h = _cam.height;
  if (r.maxX <= 0 || r.minX >= w || r.maxY <= 0 || r.minY >= h)
    return std::nullopt;

  r.minX = std::clamp(r.minX, 0.0, w);
  r.maxX = std::clamp(r.maxX, 0.0, w);
  r.minY = std::clamp(r.minY, 0.0, h);
  r.maxY = std::clamp(r.maxY, 0.0, h);
  return r;
}

inline std::optional<Rect> ExpectedRect(
    const std::vector<math::Vector3d> &_corners, const CameraModel &_cam)
{
  return ExpectedRect(_corners, _cam, _cam.Projection());
}

/// \brief Expect two rects to match within _tol pixels on every edge.
inline void ExpectRectNear(const Rect &_actual, const Rect &_expected,
    double _tol, const std::string &_what)
{
  SCOPED_TRACE(_what);
  // the camera computes in float
  _tol += 1e-3;
  EXPECT_NEAR(_actual.minX, _expected.minX, _tol)
      << "actual " << _actual << " expected " << _expected;
  EXPECT_NEAR(_actual.maxX, _expected.maxX, _tol)
      << "actual " << _actual << " expected " << _expected;
  EXPECT_NEAR(_actual.minY, _expected.minY, _tol)
      << "actual " << _actual << " expected " << _expected;
  EXPECT_NEAR(_actual.maxY, _expected.maxY, _tol)
      << "actual " << _actual << " expected " << _expected;
}

/// \brief Tolerance, in pixels, for full 2D boxes: they come from exact
/// vertex projection. One pixel covers clamping to the last pixel index
/// (width - 1) instead of the image border (width) at the image edges.
constexpr double kFullTol = 1.0;

/// \brief Tolerance, in pixels, for visible 2D boxes: they come from the
/// rasterized image, so each edge can be off by the sampling of one pixel
/// plus the half-pixel ambiguity of the pixel convention.
constexpr double kVisibleTol = 1.5;
}  // namespace bbox_test

/// \brief Fixture: creates cameras at the origin looking along +X and
/// collects the boxes of each frame.
using namespace gz;
using namespace gz::rendering;

class BoundingBoxCameraAccuracyTest : public CommonRenderingTest
{
  protected: using CameraModel = bbox_test::CameraModel;

  /// \brief Create a camera at the origin, looking along +X, that stores
  /// each frame's boxes in this->boxes.
  protected: BoundingBoxCameraPtr CreateCamera(ScenePtr _scene,
      const CameraModel &_model, BoundingBoxType _type)
  {
    auto camera = _scene->CreateBoundingBoxCamera(
        "bbox_camera_" + std::to_string(this->cameraCount++));
    if (!camera)
      return camera;
    camera->SetLocalPosition(0.0, 0.0, 0.0);
    camera->SetLocalRotation(0.0, 0.0, 0.0);
    camera->SetImageWidth(_model.width);
    camera->SetImageHeight(_model.height);
    camera->SetAspectRatio(_model.Aspect());
    camera->SetHFOV(_model.hfov);
    camera->SetNearClipPlane(_model.nearClip);
    camera->SetFarClipPlane(_model.farClip);
    camera->SetBoundingBoxType(_type);
    _scene->RootVisual()->AddChild(camera);
    this->connections.push_back(camera->ConnectNewBoundingBoxes(
        [this](const std::vector<BoundingBox> &_boxes)
        {
          this->boxes = _boxes;
          ++this->frames;
        }));
    return camera;
  }

  /// \brief Render one frame and return the boxes it produced.
  protected: std::vector<BoundingBox> Render(BoundingBoxCameraPtr _camera)
  {
    this->boxes.clear();
    const unsigned int before = this->frames;
    _camera->Update();
    EXPECT_EQ(before + 1, this->frames) << "no bounding box frame received";
    return this->boxes;
  }

  /// \brief Render one frame with the given box type.
  protected: std::vector<BoundingBox> Render(BoundingBoxCameraPtr _camera,
      BoundingBoxType _type)
  {
    _camera->SetBoundingBoxType(_type);
    return this->Render(_camera);
  }

  /// \brief Add a box visual under _parent.
  protected: static VisualPtr AddBox(ScenePtr _scene, VisualPtr _parent,
      const math::Pose3d &_pose, const math::Vector3d &_size,
      std::optional<int> _label)
  {
    VisualPtr vis = _scene->CreateVisual();
    vis->AddGeometry(_scene->CreateBox());
    vis->SetLocalPose(_pose);
    vis->SetLocalScale(_size);
    if (_label)
      vis->SetUserData("label", *_label);
    _parent->AddChild(vis);
    return vis;
  }

  /// \brief Boxes in _boxes with the given label.
  protected: static std::vector<BoundingBox> WithLabel(
      const std::vector<BoundingBox> &_boxes, uint32_t _label)
  {
    std::vector<BoundingBox> out;
    for (const auto &b : _boxes)
    {
      if (b.Label() == _label)
        out.push_back(b);
    }
    return out;
  }

  /// \brief Boxes from the last frame.
  protected: std::vector<BoundingBox> boxes;

  /// \brief Number of frames received.
  protected: unsigned int frames{0};

  /// \brief Keep the frame connections alive.
  protected: std::vector<common::ConnectionPtr> connections;

  /// \brief Used to give cameras unique names.
  protected: unsigned int cameraCount{0};
};

#endif
