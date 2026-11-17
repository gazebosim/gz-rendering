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

// Accuracy tests for the bounding box camera.
//
// Expected 2D boxes are computed independently of the camera implementation:
// the 8 corners of each box are transformed into the camera optical frame,
// clipped against the near plane, projected with a pinhole / OpenGL-style
// projection matrix built here from the camera parameters, and clamped to the
// image. Expected 3D boxes are the box corners expressed in the camera optical
// frame (X right, Y up, Z backwards), which is the frame the camera currently
// reports 3D boxes in.
//
// 2D box convention assumed by these tests: a box covers the continuous pixel
// range [center - size/2, center + size/2], with pixel i spanning [i, i+1).
// So a fully visible object whose projection spans [u0, u1] has a box of
// width ~= u1 - u0, regardless of the box type.
//
// Tests prefixed with DISABLED_ fail on the current implementation because of
// known bugs; each one names the bug it covers. Remove the prefix once the bug
// is fixed.

#include "boundingbox_camera_test_utils.hh"

using namespace gz;
using namespace rendering;
using namespace bbox_test;

//////////////////////////////////////////////////
// A single unoccluded box: both 2D box types must match the projection of
// the box's corners.
TEST_F(BoundingBoxCameraAccuracyTest, SingleBoxMatchesProjection)
{
  CHECK_SUPPORTED_ENGINE("ogre2");
  ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);

  const CameraModel model;
  auto camera = this->CreateCamera(scene, model,
      BoundingBoxType::BBT_FULLBOX2D);
  ASSERT_NE(nullptr, camera);

  struct Case
  {
    std::string name;
    math::Pose3d pose;
    math::Vector3d size;
  };
  const std::vector<Case> cases = {
    {"centered", {4, 0, 0, 0, 0, 0}, {1, 1, 1}},
    {"offset rotated scaled", {5, 1, 0.5, 0.3, 0.2, 0.7}, {1, 0.5, 2}},
    {"small and far", {20, -2, 1, 0, 0, 0.4}, {0.5, 0.5, 0.5}},
    {"thin and tilted", {6, -1, -1, 0.0, 0.6, -0.3}, {0.1, 2, 0.3}},
  };

  for (const auto &c : cases)
  {
    SCOPED_TRACE(c.name);
    VisualPtr vis = AddBox(scene, scene->RootVisual(), c.pose, c.size, 7);
    auto expected = ExpectedRect(BoxCorners(c.pose, c.size), model);
    ASSERT_TRUE(expected.has_value());

    auto full = this->Render(camera, BoundingBoxType::BBT_FULLBOX2D);
    ASSERT_EQ(1u, full.size());
    EXPECT_EQ(7u, full[0].Label());
    ExpectRectNear(ToRect(full[0]), *expected, kFullTol, "full");

    auto visible = this->Render(camera, BoundingBoxType::BBT_VISIBLEBOX2D);
    ASSERT_EQ(1u, visible.size());
    EXPECT_EQ(7u, visible[0].Label());
    ExpectRectNear(ToRect(visible[0]), *expected, kVisibleTol, "visible");

    scene->DestroyVisual(vis, true);
  }

  engine->DestroyScene(scene);
}

//////////////////////////////////////////////////
// Same as above with a non-square image.
TEST_F(BoundingBoxCameraAccuracyTest, NonSquareAspectRatio)
{
  CHECK_SUPPORTED_ENGINE("ogre2");
  ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);

  CameraModel model;
  model.width = 400;
  model.height = 160;
  model.hfov = 1.2;
  auto camera = this->CreateCamera(scene, model,
      BoundingBoxType::BBT_FULLBOX2D);
  ASSERT_NE(nullptr, camera);

  const math::Pose3d pose(6, 1.2, -0.4, 0.1, 0.2, 0.3);
  const math::Vector3d size(1, 1.5, 0.8);
  AddBox(scene, scene->RootVisual(), pose, size, 1);
  auto expected = ExpectedRect(BoxCorners(pose, size), model);
  ASSERT_TRUE(expected.has_value());

  auto full = this->Render(camera, BoundingBoxType::BBT_FULLBOX2D);
  ASSERT_EQ(1u, full.size());
  ExpectRectNear(ToRect(full[0]), *expected, kFullTol, "full");

  auto visible = this->Render(camera, BoundingBoxType::BBT_VISIBLEBOX2D);
  ASSERT_EQ(1u, visible.size());
  ExpectRectNear(ToRect(visible[0]), *expected, kVisibleTol, "visible");

  engine->DestroyScene(scene);
}

//////////////////////////////////////////////////
// A custom projection matrix with an off-center principal point must be
// honored by both 2D box types.
TEST_F(BoundingBoxCameraAccuracyTest, CustomProjectionMatrix)
{
  CHECK_SUPPORTED_ENGINE("ogre2");
  ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);

  const CameraModel model;
  auto camera = this->CreateCamera(scene, model,
      BoundingBoxType::BBT_FULLBOX2D);
  ASSERT_NE(nullptr, camera);

  // Narrower FOV than the camera's HFOV and a principal point shifted right
  // by 0.2 in NDC (= 32 px at 320 px wide).
  math::Matrix4d proj = model.Projection();
  proj(0, 0) *= 1.3;
  proj(1, 1) *= 1.3;
  proj(0, 2) = -0.2;
  camera->SetProjectionMatrix(proj);

  const math::Pose3d pose(5, 0.5, 0.3, 0, 0, 0.5);
  const math::Vector3d size(1, 1, 1);
  AddBox(scene, scene->RootVisual(), pose, size, 1);
  auto expected = ExpectedRect(BoxCorners(pose, size), model, proj);
  ASSERT_TRUE(expected.has_value());

  auto full = this->Render(camera, BoundingBoxType::BBT_FULLBOX2D);
  ASSERT_EQ(1u, full.size());
  ExpectRectNear(ToRect(full[0]), *expected, kFullTol, "full");

  auto visible = this->Render(camera, BoundingBoxType::BBT_VISIBLEBOX2D);
  ASSERT_EQ(1u, visible.size());
  ExpectRectNear(ToRect(visible[0]), *expected, kVisibleTol, "visible");

  engine->DestroyScene(scene);
}

//////////////////////////////////////////////////
// Boxes that are partly outside the image. The full box is the projection
// clamped to the image. The visible box only covers the part inside the view
// frustum, so it must lie inside the full box and reach the image border on
// the clipped sides.
TEST_F(BoundingBoxCameraAccuracyTest, PartiallyOutsideImage)
{
  CHECK_SUPPORTED_ENGINE("ogre2");
  ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);

  const CameraModel model;
  auto camera = this->CreateCamera(scene, model,
      BoundingBoxType::BBT_FULLBOX2D);
  ASSERT_NE(nullptr, camera);

  struct Case
  {
    std::string name;
    math::Pose3d pose;
  };
  const std::vector<Case> cases = {
    {"left edge", {3, 3.2, 0, 0, 0, 0}},
    {"right edge", {3, -3.2, 0.3, 0, 0, 0.3}},
    {"top edge", {4, 0.5, 3.1, 0, 0, 0}},
    {"bottom right corner", {4, -4.0, -3.2, 0, 0, 0}},
  };
  const math::Vector3d size(1, 1, 1);
  const double w = model.width;
  const double h = model.height;

  for (const auto &c : cases)
  {
    SCOPED_TRACE(c.name);
    VisualPtr vis = AddBox(scene, scene->RootVisual(), c.pose, size, 1);
    auto expected = ExpectedRect(BoxCorners(c.pose, size), model);
    ASSERT_TRUE(expected.has_value());

    auto full = this->Render(camera, BoundingBoxType::BBT_FULLBOX2D);
    ASSERT_EQ(1u, full.size());
    ExpectRectNear(ToRect(full[0]), *expected, kFullTol, "full");

    auto visible = this->Render(camera, BoundingBoxType::BBT_VISIBLEBOX2D);
    ASSERT_EQ(1u, visible.size());
    const Rect v = ToRect(visible[0]);
    const double tol = kVisibleTol;
    EXPECT_GE(v.minX, expected->minX - tol) << v << " " << *expected;
    EXPECT_GE(v.minY, expected->minY - tol) << v << " " << *expected;
    EXPECT_LE(v.maxX, expected->maxX + tol) << v << " " << *expected;
    EXPECT_LE(v.maxY, expected->maxY + tol) << v << " " << *expected;
    if (expected->minX <= 0)
    {
      EXPECT_NEAR(0, v.minX, tol);
    }
    if (expected->minY <= 0)
    {
      EXPECT_NEAR(0, v.minY, tol);
    }
    if (expected->maxX >= w)
    {
      EXPECT_NEAR(w, v.maxX, tol);
    }
    if (expected->maxY >= h)
    {
      EXPECT_NEAR(h, v.maxY, tol);
    }

    scene->DestroyVisual(vis, true);
  }

  engine->DestroyScene(scene);
}

//////////////////////////////////////////////////
// Known bug C3: an object whose projection is larger than the image on both
// sides is dropped by the full 2D box type
// (Ogre2BoundingBoxCamera::FullBoundingBoxes rejects boxes with
// |min| > 1 && |max| > 1 in NDC, which is also true for a box that covers
// the whole view).
TEST_F(BoundingBoxCameraAccuracyTest, DISABLED_ObjectLargerThanView)
{
  CHECK_SUPPORTED_ENGINE("ogre2");
  ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);

  const CameraModel model;
  auto camera = this->CreateCamera(scene, model,
      BoundingBoxType::BBT_FULLBOX2D);
  ASSERT_NE(nullptr, camera);

  const math::Pose3d pose(3, 0, 0, 0, 0, 0);
  const math::Vector3d size(0.5, 30, 30);
  AddBox(scene, scene->RootVisual(), pose, size, 1);
  const Rect wholeImage{0, 0, static_cast<double>(model.width),
      static_cast<double>(model.height)};

  auto visible = this->Render(camera, BoundingBoxType::BBT_VISIBLEBOX2D);
  ASSERT_EQ(1u, visible.size());
  ExpectRectNear(ToRect(visible[0]), wholeImage, kVisibleTol, "visible");

  auto full = this->Render(camera, BoundingBoxType::BBT_FULLBOX2D);
  ASSERT_EQ(1u, full.size());
  ExpectRectNear(ToRect(full[0]), wholeImage, kFullTol, "full");

  engine->DestroyScene(scene);
}

//////////////////////////////////////////////////
// Known bug C4: an object that extends behind the camera gets a wrong full
// 2D box, because vertices behind the camera are divided by a negative w
// and land on the wrong side of the image.
// A long bar on the left of the camera, from behind it to 5 m in front of
// it, must only cover the left part of the image.
TEST_F(BoundingBoxCameraAccuracyTest, DISABLED_ObjectCrossingNearPlane)
{
  CHECK_SUPPORTED_ENGINE("ogre2");
  ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);

  const CameraModel model;
  auto camera = this->CreateCamera(scene, model,
      BoundingBoxType::BBT_FULLBOX2D);
  ASSERT_NE(nullptr, camera);

  const math::Pose3d pose(0, 1.5, 0, 0, 0, 0);
  const math::Vector3d size(10, 0.2, 0.2);
  AddBox(scene, scene->RootVisual(), pose, size, 1);
  auto expected = ExpectedRect(BoxCorners(pose, size), model);
  ASSERT_TRUE(expected.has_value());
  // sanity check on the expectation itself: left half of the image only
  ASSERT_LT(expected->maxX, model.width * 0.5);

  auto visible = this->Render(camera, BoundingBoxType::BBT_VISIBLEBOX2D);
  ASSERT_EQ(1u, visible.size());
  EXPECT_LT(ToRect(visible[0]).maxX, model.width * 0.5);

  auto full = this->Render(camera, BoundingBoxType::BBT_FULLBOX2D);
  ASSERT_EQ(1u, full.size());
  ExpectRectNear(ToRect(full[0]), *expected, kFullTol, "full");

  engine->DestroyScene(scene);
}

//////////////////////////////////////////////////
// Known bug C5: visible 2D boxes are measured between pixel indices, so they
// are one pixel narrower and half a pixel off compared to the continuous
// convention used by the full 2D boxes.
// One box is moved across many sub-pixel positions; on average the visible
// box must have no bias relative to the true projection. The full box must
// have no bias either.
TEST_F(BoundingBoxCameraAccuracyTest, DISABLED_NoSubPixelBias)
{
  CHECK_SUPPORTED_ENGINE("ogre2");
  ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);

  const CameraModel model;
  auto camera = this->CreateCamera(scene, model,
      BoundingBoxType::BBT_FULLBOX2D);
  ASSERT_NE(nullptr, camera);

  const math::Vector3d size(0.3, 0.3, 0.3);
  VisualPtr vis = AddBox(scene, scene->RootVisual(),
      math::Pose3d::Zero, size, 1);

  const int steps = 24;
  double visWidthErr = 0, visHeightErr = 0, visCxErr = 0, visCyErr = 0;
  double fullWidthErr = 0, fullCxErr = 0;
  for (int i = 0; i < steps; ++i)
  {
    // Moves the box by ~0.1 px per step in both image axes.
    const math::Pose3d pose(6, -0.3 + i * 0.004, -0.2 + i * 0.004,
        0, 0, 0);
    vis->SetLocalPose(pose);
    auto expected = ExpectedRect(BoxCorners(pose, size), model);
    ASSERT_TRUE(expected.has_value());

    auto visible = this->Render(camera, BoundingBoxType::BBT_VISIBLEBOX2D);
    ASSERT_EQ(1u, visible.size());
    const Rect v = ToRect(visible[0]);
    visWidthErr += v.Width() - expected->Width();
    visHeightErr += v.Height() - expected->Height();
    visCxErr += v.CenterX() - expected->CenterX();
    visCyErr += v.CenterY() - expected->CenterY();

    auto full = this->Render(camera, BoundingBoxType::BBT_FULLBOX2D);
    ASSERT_EQ(1u, full.size());
    const Rect f = ToRect(full[0]);
    fullWidthErr += f.Width() - expected->Width();
    fullCxErr += f.CenterX() - expected->CenterX();
  }

  EXPECT_NEAR(0.0, fullWidthErr / steps, 0.1) << "full box width bias";
  EXPECT_NEAR(0.0, fullCxErr / steps, 0.1) << "full box center bias";
  EXPECT_NEAR(0.0, visWidthErr / steps, 0.4) << "visible box width bias";
  EXPECT_NEAR(0.0, visHeightErr / steps, 0.4) << "visible box height bias";
  EXPECT_NEAR(0.0, visCxErr / steps, 0.25) << "visible box center x bias";
  EXPECT_NEAR(0.0, visCyErr / steps, 0.25) << "visible box center y bias";

  engine->DestroyScene(scene);
}

//////////////////////////////////////////////////
// Labels 0 and 254 are valid labels and must be reported.
TEST_F(BoundingBoxCameraAccuracyTest, LowAndHighLabels)
{
  CHECK_SUPPORTED_ENGINE("ogre2");
  ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);

  const CameraModel model;
  auto camera = this->CreateCamera(scene, model,
      BoundingBoxType::BBT_VISIBLEBOX2D);
  ASSERT_NE(nullptr, camera);

  AddBox(scene, scene->RootVisual(), {5, 1.5, 0, 0, 0, 0}, {1, 1, 1}, 0);
  AddBox(scene, scene->RootVisual(), {5, -1.5, 0, 0, 0, 0}, {1, 1, 1}, 254);

  for (auto type : {BoundingBoxType::BBT_VISIBLEBOX2D,
      BoundingBoxType::BBT_FULLBOX2D, BoundingBoxType::BBT_BOX3D})
  {
    SCOPED_TRACE(static_cast<int>(type));
    auto result = this->Render(camera, type);
    EXPECT_EQ(2u, result.size());
    EXPECT_EQ(1u, WithLabel(result, 0).size());
    EXPECT_EQ(1u, WithLabel(result, 254).size());
  }

  engine->DestroyScene(scene);
}

//////////////////////////////////////////////////
// Known bug C2: label 255 is the internal background value, so objects with
// that label are never reported. Pending decision on the supported label
// range (see the review plan): enable this if 255 becomes a valid label, or
// replace it with a test of the documented limit.
TEST_F(BoundingBoxCameraAccuracyTest, DISABLED_Label255)
{
  CHECK_SUPPORTED_ENGINE("ogre2");
  ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);

  const CameraModel model;
  auto camera = this->CreateCamera(scene, model,
      BoundingBoxType::BBT_VISIBLEBOX2D);
  ASSERT_NE(nullptr, camera);

  AddBox(scene, scene->RootVisual(), {5, 0, 0, 0, 0, 0}, {1, 1, 1}, 255);

  auto result = this->Render(camera);
  ASSERT_EQ(1u, result.size());
  EXPECT_EQ(255u, result[0].Label());

  engine->DestroyScene(scene);
}

//////////////////////////////////////////////////
// An object without a label occludes labeled objects but gets no box.
TEST_F(BoundingBoxCameraAccuracyTest, UnlabeledOccluder)
{
  CHECK_SUPPORTED_ENGINE("ogre2");
  ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);

  const CameraModel model;
  auto camera = this->CreateCamera(scene, model,
      BoundingBoxType::BBT_VISIBLEBOX2D);
  ASSERT_NE(nullptr, camera);

  const math::Pose3d targetPose(6, 0, 0, 0, 0, 0);
  const math::Vector3d targetSize(1, 2, 1);
  AddBox(scene, scene->RootVisual(), targetPose, targetSize, 1);
  // covers the left part (+Y) of the target
  AddBox(scene, scene->RootVisual(), {3, 0.6, 0, 0, 0, 0}, {0.2, 0.8, 2},
      std::nullopt);
  auto expected = ExpectedRect(BoxCorners(targetPose, targetSize), model);
  ASSERT_TRUE(expected.has_value());

  auto full = this->Render(camera, BoundingBoxType::BBT_FULLBOX2D);
  ASSERT_EQ(1u, full.size());
  EXPECT_EQ(1u, full[0].Label());
  ExpectRectNear(ToRect(full[0]), *expected, kFullTol, "full");

  auto visible = this->Render(camera, BoundingBoxType::BBT_VISIBLEBOX2D);
  ASSERT_EQ(1u, visible.size());
  EXPECT_EQ(1u, visible[0].Label());
  const Rect v = ToRect(visible[0]);
  // the left part is hidden, the right edge is not
  EXPECT_GT(v.minX, expected->minX + 5);
  EXPECT_NEAR(v.maxX, expected->maxX, kVisibleTol);
  EXPECT_NEAR(v.minY, expected->minY, kVisibleTol);
  EXPECT_NEAR(v.maxY, expected->maxY, kVisibleTol);

  auto box3d = this->Render(camera, BoundingBoxType::BBT_BOX3D);
  ASSERT_EQ(1u, box3d.size());
  EXPECT_EQ(1u, box3d[0].Label());

  engine->DestroyScene(scene);
}

//////////////////////////////////////////////////
// More than 256 objects, so the item ids need more than one byte. Each one
// must get its own box at the right place.
TEST_F(BoundingBoxCameraAccuracyTest, ManyObjects)
{
  CHECK_SUPPORTED_ENGINE("ogre2");
  ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);

  CameraModel model;
  model.width = 640;
  model.height = 480;
  auto camera = this->CreateCamera(scene, model,
      BoundingBoxType::BBT_VISIBLEBOX2D);
  ASSERT_NE(nullptr, camera);

  const int cols = 20;
  const int rows = 15;
  const math::Vector3d size(0.2, 0.2, 0.2);
  std::vector<Rect> expected;
  for (int r = 0; r < rows; ++r)
  {
    for (int c = 0; c < cols; ++c)
    {
      const math::Pose3d pose(10, (c - (cols - 1) * 0.5) * 0.8,
          (r - (rows - 1) * 0.5) * 0.8, 0, 0, 0);
      AddBox(scene, scene->RootVisual(), pose, size, 1 + (r * cols + c) % 200);
      auto e = ExpectedRect(BoxCorners(pose, size), model);
      ASSERT_TRUE(e.has_value());
      expected.push_back(*e);
    }
  }

  for (auto type : {BoundingBoxType::BBT_VISIBLEBOX2D,
      BoundingBoxType::BBT_FULLBOX2D})
  {
    SCOPED_TRACE(static_cast<int>(type));
    auto result = this->Render(camera, type);
    ASSERT_EQ(expected.size(), result.size());

    // match each expected rect with the closest reported box
    int matched = 0;
    for (size_t i = 0; i < expected.size(); ++i)
    {
      const auto &e = expected[i];
      double best = std::numeric_limits<double>::max();
      Rect bestRect;
      uint32_t bestLabel = 0;
      for (const auto &b : result)
      {
        const Rect r = ToRect(b);
        const double d = std::hypot(r.CenterX() - e.CenterX(),
            r.CenterY() - e.CenterY());
        if (d < best)
        {
          best = d;
          bestRect = r;
          bestLabel = b.Label();
        }
      }
      if (best < kVisibleTol &&
          std::abs(bestRect.Width() - e.Width()) < 2 * kVisibleTol &&
          bestLabel == 1 + i % 200)
      {
        ++matched;
      }
    }
    EXPECT_EQ(static_cast<int>(expected.size()), matched);
  }

  engine->DestroyScene(scene);
}

//////////////////////////////////////////////////
// The links of a model are merged into one 2D box: the union of the links.
TEST_F(BoundingBoxCameraAccuracyTest, MultiLinkModel2D)
{
  CHECK_SUPPORTED_ENGINE("ogre2");
  ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);

  const CameraModel model;
  auto camera = this->CreateCamera(scene, model,
      BoundingBoxType::BBT_VISIBLEBOX2D);
  ASSERT_NE(nullptr, camera);

  VisualPtr robot = scene->CreateVisual("robot");
  robot->SetLocalPose({5, 0, 0, 0, 0, 0});
  scene->RootVisual()->AddChild(robot);
  const math::Pose3d link1(0, 1.5, 0, 0, 0, 0.3);
  const math::Pose3d link2(0.5, -1.5, 0.5, 0.2, 0, 0);
  const math::Vector3d size1(1, 1, 1);
  const math::Vector3d size2(0.5, 0.5, 1.5);
  AddBox(scene, robot, link1, size1, 3);
  AddBox(scene, robot, link2, size2, 3);
  // an unrelated box, to check it is not merged in
  AddBox(scene, scene->RootVisual(), {8, 0, -1.5, 0, 0, 0}, {1, 1, 1}, 4);

  const math::Pose3d robotPose = robot->WorldPose();
  Rect expected = *ExpectedRect(BoxCorners(robotPose * link1, size1), model);
  expected.Merge(*ExpectedRect(BoxCorners(robotPose * link2, size2), model));

  for (auto type : {BoundingBoxType::BBT_VISIBLEBOX2D,
      BoundingBoxType::BBT_FULLBOX2D})
  {
    SCOPED_TRACE(static_cast<int>(type));
    auto result = this->Render(camera, type);
    ASSERT_EQ(2u, result.size());
    auto robotBoxes = WithLabel(result, 3);
    ASSERT_EQ(1u, robotBoxes.size());
    ExpectRectNear(ToRect(robotBoxes[0]), expected,
        type == BoundingBoxType::BBT_FULLBOX2D ? kFullTol : kVisibleTol,
        "robot");
  }

  engine->DestroyScene(scene);
}

//////////////////////////////////////////////////
// 3D box of a single rotated, scaled box: its 8 vertices must be the box's
// corners expressed in the camera optical frame.
TEST_F(BoundingBoxCameraAccuracyTest, Box3DMatchesPose)
{
  CHECK_SUPPORTED_ENGINE("ogre2");
  ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);

  const CameraModel model;
  auto camera = this->CreateCamera(scene, model,
      BoundingBoxType::BBT_BOX3D);
  ASSERT_NE(nullptr, camera);

  const math::Pose3d pose(4, 0.5, -0.3, 0.2, -0.3, 0.9);
  const math::Vector3d size(1, 2, 0.5);
  AddBox(scene, scene->RootVisual(), pose, size, 1);

  auto result = this->Render(camera);
  ASSERT_EQ(1u, result.size());
  EXPECT_EQ(1u, result[0].Label());
  EXPECT_NEAR(ToOptical(pose.Pos()).X(), result[0].Center().X(), 1e-3);
  EXPECT_NEAR(ToOptical(pose.Pos()).Y(), result[0].Center().Y(), 1e-3);
  EXPECT_NEAR(ToOptical(pose.Pos()).Z(), result[0].Center().Z(), 1e-3);

  const auto &vertices = result[0].Vertices3D();
  ASSERT_EQ(8u, vertices.size());
  for (const auto &corner : BoxCorners(pose, size))
  {
    const math::Vector3d expected = ToOptical(corner);
    double best = std::numeric_limits<double>::max();
    for (const auto &v : vertices)
      best = std::min(best, v.Distance(expected));
    EXPECT_LT(best, 1e-3) << "no vertex at " << expected;
  }

  engine->DestroyScene(scene);
}

//////////////////////////////////////////////////
// The 3D box of a multi-link model must contain every link.
TEST_F(BoundingBoxCameraAccuracyTest, MultiLinkModel3DContainsAllLinks)
{
  CHECK_SUPPORTED_ENGINE("ogre2");
  ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);

  const CameraModel model;
  auto camera = this->CreateCamera(scene, model,
      BoundingBoxType::BBT_BOX3D);
  ASSERT_NE(nullptr, camera);

  VisualPtr robot = scene->CreateVisual("robot");
  robot->SetLocalPose({5, 0, 0, 0, 0, 0.2});
  scene->RootVisual()->AddChild(robot);
  const math::Pose3d link1(0, 1.2, 0, 0, 0, 0.3);
  const math::Pose3d link2(0.5, -1.2, 0.5, 0.2, 0, 0);
  const math::Vector3d size1(1, 1, 1);
  const math::Vector3d size2(0.5, 0.5, 1.5);
  AddBox(scene, robot, link1, size1, 3);
  AddBox(scene, robot, link2, size2, 3);

  auto result = this->Render(camera);
  ASSERT_EQ(1u, result.size());
  const BoundingBox &box = result[0];
  EXPECT_EQ(3u, box.Label());

  const math::Pose3d robotPose = robot->WorldPose();
  std::vector<math::Vector3d> corners = BoxCorners(robotPose * link1, size1);
  for (const auto &c : BoxCorners(robotPose * link2, size2))
    corners.push_back(c);

  const double eps = 1e-2;
  for (const auto &corner : corners)
  {
    const math::Vector3d local =
        box.Orientation().RotateVectorReverse(ToOptical(corner) -
        box.Center());
    EXPECT_LE(std::abs(local.X()), box.Size().X() * 0.5 + eps) << corner;
    EXPECT_LE(std::abs(local.Y()), box.Size().Y() * 0.5 + eps) << corner;
    EXPECT_LE(std::abs(local.Z()), box.Size().Z() * 0.5 + eps) << corner;
  }

  engine->DestroyScene(scene);
}

//////////////////////////////////////////////////
// Bug C6 / pending decision: the 3D box of a multi-link model only merges
// the links that are visible in this frame, so hiding one link shrinks the
// box. Enable once the expected behavior is decided.
TEST_F(BoundingBoxCameraAccuracyTest,
    DISABLED_MultiLinkModel3DPartiallyOccluded)
{
  CHECK_SUPPORTED_ENGINE("ogre2");
  ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);

  const CameraModel model;
  auto camera = this->CreateCamera(scene, model,
      BoundingBoxType::BBT_BOX3D);
  ASSERT_NE(nullptr, camera);

  VisualPtr robot = scene->CreateVisual("robot");
  robot->SetLocalPose({5, 0, 0, 0, 0, 0});
  scene->RootVisual()->AddChild(robot);
  const math::Pose3d link1(0, 1.2, 0, 0, 0, 0);
  const math::Pose3d link2(0, -1.2, 0, 0, 0, 0);
  const math::Vector3d size(1, 1, 1);
  AddBox(scene, robot, link1, size, 3);
  AddBox(scene, robot, link2, size, 3);
  // unlabeled wall hiding link2 completely
  AddBox(scene, scene->RootVisual(), {2, -1, 0, 0, 0, 0}, {0.1, 2, 3},
      std::nullopt);

  auto result = this->Render(camera);
  ASSERT_EQ(1u, result.size());
  const BoundingBox &box = result[0];

  const math::Pose3d robotPose = robot->WorldPose();
  for (const auto &corner : BoxCorners(robotPose * link2, size))
  {
    const math::Vector3d local =
        box.Orientation().RotateVectorReverse(ToOptical(corner) -
        box.Center());
    EXPECT_LE(std::abs(local.X()), box.Size().X() * 0.5 + 1e-2) << corner;
    EXPECT_LE(std::abs(local.Y()), box.Size().Y() * 0.5 + 1e-2) << corner;
    EXPECT_LE(std::abs(local.Z()), box.Size().Z() * 0.5 + 1e-2) << corner;
  }

  engine->DestroyScene(scene);
}

//////////////////////////////////////////////////
// Cross-check against the segmentation camera: the visible 2D box of each
// object must span exactly the pixels the segmentation camera labels with
// that object's label. Also checks visible boxes lie inside full boxes.
TEST_F(BoundingBoxCameraAccuracyTest, VisibleBoxesMatchSegmentation)
{
  CHECK_SUPPORTED_ENGINE("ogre2");
  ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);

  const CameraModel model;
  auto camera = this->CreateCamera(scene, model,
      BoundingBoxType::BBT_VISIBLEBOX2D);
  ASSERT_NE(nullptr, camera);

  // three overlapping boxes with unique labels
  AddBox(scene, scene->RootVisual(), {6, 1, 0.3, 0, 0, 0.4}, {1, 1.5, 1}, 1);
  AddBox(scene, scene->RootVisual(), {4, 0, 0, 0.3, 0.2, 0}, {1, 1, 1}, 2);
  AddBox(scene, scene->RootVisual(), {8, -1.5, -0.5, 0, 0, 0}, {2, 2, 2}, 3);

  auto segCamera = scene->CreateSegmentationCamera("seg_camera");
  ASSERT_NE(nullptr, segCamera);
  segCamera->SetLocalPosition(0.0, 0.0, 0.0);
  segCamera->SetLocalRotation(0.0, 0.0, 0.0);
  segCamera->SetImageWidth(model.width);
  segCamera->SetImageHeight(model.height);
  segCamera->SetAspectRatio(model.Aspect());
  segCamera->SetHFOV(model.hfov);
  segCamera->SetNearClipPlane(model.nearClip);
  segCamera->SetFarClipPlane(model.farClip);
  const int background = 200;
  segCamera->SetBackgroundLabel(background);
  segCamera->SetSegmentationType(SegmentationType::ST_SEMANTIC);
  segCamera->EnableColoredMap(false);
  scene->RootVisual()->AddChild(segCamera);

  std::vector<uint8_t> segBuffer;
  auto segConnection = segCamera->ConnectNewSegmentationFrame(
      [&segBuffer](const uint8_t *_data, unsigned int _width,
          unsigned int _height, unsigned int _channels, const std::string &)
      {
        segBuffer.assign(_data, _data + _width * _height * _channels);
      });
  segCamera->Update();
  ASSERT_EQ(model.width * model.height * 3u, segBuffer.size());

  // pixel extents of each label in the segmentation image
  std::map<uint32_t, Rect> segRects;
  for (unsigned int y = 0; y < model.height; ++y)
  {
    for (unsigned int x = 0; x < model.width; ++x)
    {
      const uint8_t label = segBuffer[(y * model.width + x) * 3];
      if (label == background)
        continue;
      Rect &r = segRects[label];
      // pixel (x, y) covers [x, x+1) x [y, y+1)
      r.Merge({static_cast<double>(x), static_cast<double>(y),
          x + 1.0, y + 1.0});
    }
  }
  ASSERT_EQ(3u, segRects.size());

  auto visible = this->Render(camera, BoundingBoxType::BBT_VISIBLEBOX2D);
  ASSERT_EQ(3u, visible.size());
  auto full = this->Render(camera, BoundingBoxType::BBT_FULLBOX2D);
  ASSERT_EQ(3u, full.size());

  for (const auto &[label, segRect] : segRects)
  {
    SCOPED_TRACE("label " + std::to_string(label));
    auto v = WithLabel(visible, label);
    auto f = WithLabel(full, label);
    ASSERT_EQ(1u, v.size());
    ASSERT_EQ(1u, f.size());
    // Up to one pixel off on each edge, to tolerate the pixel convention
    // (see NoSubPixelBias for the precise check).
    ExpectRectNear(ToRect(v[0]), segRect, 1.0, "visible vs segmentation");

    const Rect vr = ToRect(v[0]);
    const Rect fr = ToRect(f[0]);
    EXPECT_GE(vr.minX, fr.minX - kVisibleTol) << "visible not inside full";
    EXPECT_GE(vr.minY, fr.minY - kVisibleTol) << "visible not inside full";
    EXPECT_LE(vr.maxX, fr.maxX + kVisibleTol) << "visible not inside full";
    EXPECT_LE(vr.maxY, fr.maxY + kVisibleTol) << "visible not inside full";
  }

  engine->DestroyScene(scene);
}

//////////////////////////////////////////////////
// Boxes follow a moving object and disappear when it is removed; no state
// leaks from one frame to the next.
TEST_F(BoundingBoxCameraAccuracyTest, MovingAndRemovedObject)
{
  CHECK_SUPPORTED_ENGINE("ogre2");
  ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);

  const CameraModel model;
  auto camera = this->CreateCamera(scene, model,
      BoundingBoxType::BBT_VISIBLEBOX2D);
  ASSERT_NE(nullptr, camera);

  const math::Vector3d size(1, 1, 1);
  VisualPtr vis = AddBox(scene, scene->RootVisual(), math::Pose3d::Zero,
      size, 1);

  for (auto type : {BoundingBoxType::BBT_VISIBLEBOX2D,
      BoundingBoxType::BBT_FULLBOX2D, BoundingBoxType::BBT_BOX3D})
  {
    SCOPED_TRACE(static_cast<int>(type));
    vis->SetVisible(true);
    for (const math::Pose3d &pose : {math::Pose3d(5, 1.5, 0, 0, 0, 0),
        math::Pose3d(5, -1.5, 0.5, 0, 0, 0.5), math::Pose3d(7, 0, -1, 0, 0, 0)})
    {
      vis->SetLocalPose(pose);
      auto result = this->Render(camera, type);
      ASSERT_EQ(1u, result.size());
      if (type == BoundingBoxType::BBT_BOX3D)
      {
        EXPECT_NEAR(ToOptical(pose.Pos()).X(), result[0].Center().X(), 1e-3);
        EXPECT_NEAR(ToOptical(pose.Pos()).Y(), result[0].Center().Y(), 1e-3);
        EXPECT_NEAR(ToOptical(pose.Pos()).Z(), result[0].Center().Z(), 1e-3);
      }
      else
      {
        ExpectRectNear(ToRect(result[0]),
            *ExpectedRect(BoxCorners(pose, size), model), kVisibleTol,
            "moving");
      }
    }

    // out of view: no box
    vis->SetLocalPose({-5, 0, 0, 0, 0, 0});
    EXPECT_TRUE(this->Render(camera, type).empty());

    // hidden: no box
    vis->SetLocalPose({5, 0, 0, 0, 0, 0});
    vis->SetVisible(false);
    EXPECT_TRUE(this->Render(camera, type).empty());
  }

  scene->DestroyVisual(vis, true);
  EXPECT_TRUE(this->Render(camera).empty());

  engine->DestroyScene(scene);
}

//////////////////////////////////////////////////
// Rendering the same scene repeatedly gives identical results, in the same
// order.
TEST_F(BoundingBoxCameraAccuracyTest, Deterministic)
{
  CHECK_SUPPORTED_ENGINE("ogre2");
  ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);

  const CameraModel model;
  auto camera = this->CreateCamera(scene, model,
      BoundingBoxType::BBT_VISIBLEBOX2D);
  ASSERT_NE(nullptr, camera);

  AddBox(scene, scene->RootVisual(), {6, 1, 0.3, 0, 0, 0.4}, {1, 1.5, 1}, 1);
  AddBox(scene, scene->RootVisual(), {4, 0, 0, 0.3, 0.2, 0}, {1, 1, 1}, 2);
  AddBox(scene, scene->RootVisual(), {8, -1.5, -0.5, 0, 0, 0}, {2, 2, 2}, 3);

  for (auto type : {BoundingBoxType::BBT_VISIBLEBOX2D,
      BoundingBoxType::BBT_FULLBOX2D, BoundingBoxType::BBT_BOX3D})
  {
    SCOPED_TRACE(static_cast<int>(type));
    auto first = this->Render(camera, type);
    ASSERT_EQ(3u, first.size());
    for (int i = 0; i < 3; ++i)
    {
      auto again = this->Render(camera, type);
      ASSERT_EQ(first.size(), again.size());
      for (size_t j = 0; j < first.size(); ++j)
      {
        EXPECT_EQ(first[j].Label(), again[j].Label());
        EXPECT_EQ(first[j].Center(), again[j].Center());
        EXPECT_EQ(first[j].Size(), again[j].Size());
        EXPECT_EQ(first[j].Orientation(), again[j].Orientation());
      }
    }
  }

  engine->DestroyScene(scene);
}

//////////////////////////////////////////////////
// The ogre2 camera finds the boxes' pixel extents on the GPU when it can,
// and on the CPU otherwise (or when GZ_RENDERING_OGRE2_BBOX_CPU_SCAN is set
// when the camera first renders). Both must give exactly the same boxes.
TEST_F(BoundingBoxCameraAccuracyTest, GpuAndCpuExtentsAgree)
{
  CHECK_SUPPORTED_ENGINE("ogre2");
  ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);

  CameraModel model;
  model.width = 400;
  model.height = 300;

  // many overlapping objects, some cut by the image border, and a
  // multi-link model
  for (int i = 0; i < 150; ++i)
  {
    const double x = 3.0 + (i % 7) * 1.3;
    const double y = -5.0 + (i * 37 % 100) * 0.1;
    const double z = -3.0 + (i * 53 % 60) * 0.1;
    AddBox(scene, scene->RootVisual(), {x, y, z, 0.1 * i, 0.2 * i, 0.3 * i},
        {0.5, 0.3, 0.4}, 1 + i % 50);
  }
  VisualPtr robot = scene->CreateVisual("robot");
  robot->SetLocalPose({4, 0, 0, 0, 0, 0.3});
  scene->RootVisual()->AddChild(robot);
  AddBox(scene, robot, {0, 1, 0, 0, 0, 0}, {0.5, 0.5, 0.5}, 60);
  AddBox(scene, robot, {0, -1, 0.5, 0, 0, 0}, {0.5, 0.5, 0.5}, 60);

  const char *envName = "GZ_RENDERING_OGRE2_BBOX_CPU_SCAN";
  auto gpuCamera = this->CreateCamera(scene, model,
      BoundingBoxType::BBT_VISIBLEBOX2D);
  ASSERT_NE(nullptr, gpuCamera);
  // first render creates the camera's texture and picks the method
  this->Render(gpuCamera);

  ASSERT_TRUE(gz::utils::setenv(envName, "1"));
  auto cpuCamera = this->CreateCamera(scene, model,
      BoundingBoxType::BBT_VISIBLEBOX2D);
  ASSERT_NE(nullptr, cpuCamera);
  this->Render(cpuCamera);
  ASSERT_TRUE(gz::utils::unsetenv(envName));

  for (auto type : {BoundingBoxType::BBT_VISIBLEBOX2D,
      BoundingBoxType::BBT_FULLBOX2D, BoundingBoxType::BBT_BOX3D})
  {
    SCOPED_TRACE(static_cast<int>(type));
    auto gpu = this->Render(gpuCamera, type);
    auto cpu = this->Render(cpuCamera, type);
    ASSERT_GT(gpu.size(), 20u);
    ASSERT_EQ(cpu.size(), gpu.size());
    for (size_t i = 0; i < gpu.size(); ++i)
    {
      EXPECT_EQ(cpu[i].Label(), gpu[i].Label()) << i;
      EXPECT_EQ(cpu[i].Center(), gpu[i].Center()) << i;
      EXPECT_EQ(cpu[i].Size(), gpu[i].Size()) << i;
      EXPECT_EQ(cpu[i].Orientation(), gpu[i].Orientation()) << i;
    }
  }

  engine->DestroyScene(scene);
}

//////////////////////////////////////////////////
// Objects all around the camera, most of them out of view. Only the ones in
// view are reported, with the right extents, including one that straddles
// the edge of the view. Turning the camera brings other objects into view.
// Covers the material switcher leaving out-of-view items alone.
TEST_F(BoundingBoxCameraAccuracyTest, ObjectsAllAroundCamera)
{
  CHECK_SUPPORTED_ENGINE("ogre2");
  ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);

  const CameraModel model;
  auto camera = this->CreateCamera(scene, model,
      BoundingBoxType::BBT_FULLBOX2D);
  ASSERT_NE(nullptr, camera);

  // A ring of boxes every 30 degrees, 6 m away, plus boxes far above and
  // below the view, and one straddling the right edge of the view (45 deg).
  const math::Vector3d size(1, 1, 1);
  std::vector<math::Pose3d> poses;
  for (int i = 0; i < 12; ++i)
  {
    const double heading = i * GZ_PI / 6.0;
    poses.emplace_back(6 * std::cos(heading), 6 * std::sin(heading), 0,
        0, 0, heading);
  }
  poses.emplace_back(6, 0, 20, 0, 0, 0);
  poses.emplace_back(6, 0, -20, 0, 0, 0);
  const size_t straddling = poses.size();
  poses.emplace_back(6 * std::cos(-GZ_PI / 4), 6 * std::sin(-GZ_PI / 4), 0,
      0, 0, 0);
  for (size_t i = 0; i < poses.size(); ++i)
  {
    AddBox(scene, scene->RootVisual(), poses[i], size,
        static_cast<int>(i + 1));
  }

  for (double yaw : {0.0, GZ_PI, GZ_PI / 2})
  {
    camera->SetLocalRotation(0, 0, yaw);
    const math::Pose3d cameraPose(0, 0, 0, 0, 0, yaw);
    for (auto type : {BoundingBoxType::BBT_FULLBOX2D,
        BoundingBoxType::BBT_VISIBLEBOX2D})
    {
      SCOPED_TRACE("yaw " + std::to_string(yaw) + " type " +
          std::to_string(static_cast<int>(type)));
      auto result = this->Render(camera, type);

      size_t expectedCount = 0;
      for (size_t i = 0; i < poses.size(); ++i)
      {
        // the box as seen from the camera, which sits at the origin
        const math::Pose3d inCamera = cameraPose.Inverse() * poses[i];
        auto expected = ExpectedRect(BoxCorners(inCamera, size), model);
        auto found = WithLabel(result, static_cast<uint32_t>(i + 1));
        if (!expected)
        {
          EXPECT_TRUE(found.empty()) << "box " << i << " is out of view";
          continue;
        }
        ++expectedCount;
        ASSERT_EQ(1u, found.size()) << "box " << i << " is in view";
        const Rect actual = ToRect(found[0]);
        if (type == BoundingBoxType::BBT_FULLBOX2D)
        {
          ExpectRectNear(actual, *expected, kFullTol, "full");
        }
        else if (i == straddling)
        {
          // the visible box of a cut object ends at the image border
          EXPECT_NEAR(actual.maxX, expected->maxX, kVisibleTol);
          EXPECT_NEAR(actual.CenterY(), expected->CenterY(), kVisibleTol);
          EXPECT_GT(actual.Width(), 0.0);
        }
        else
        {
          ExpectRectNear(actual, *expected, kVisibleTol, "visible");
        }
      }
      // nothing else, e.g. an item rendered without its material switched
      EXPECT_EQ(expectedCount, result.size());
      EXPECT_GE(expectedCount, 3u);
    }
  }

  engine->DestroyScene(scene);
}

//////////////////////////////////////////////////
// A mesh whose vertices change every frame (a marker): the full box follows
// the new vertices. Covers the vertex cache leaving dynamic meshes alone.
TEST_F(BoundingBoxCameraAccuracyTest, DynamicMeshFullBox)
{
  CHECK_SUPPORTED_ENGINE("ogre2");
  ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);

  const CameraModel model;
  auto camera = this->CreateCamera(scene, model,
      BoundingBoxType::BBT_FULLBOX2D);
  ASSERT_NE(nullptr, camera);

  MarkerPtr marker = scene->CreateMarker();
  ASSERT_NE(nullptr, marker);
  marker->SetType(MarkerType::MT_TRIANGLE_LIST);
  VisualPtr vis = scene->CreateVisual();
  vis->AddGeometry(marker);
  vis->SetUserData("label", 3);
  scene->RootVisual()->AddChild(vis);

  // A rectangle facing the camera, made of two triangles drawn with both
  // windings so it's visible whatever the culling mode.
  auto setRect = [&marker](double _y0, double _y1, double _z0, double _z1)
  {
    marker->ClearPoints();
    const math::Vector3d a(5, _y0, _z0), b(5, _y1, _z0), c(5, _y1, _z1),
        d(5, _y0, _z1);
    for (const auto &p : {a, b, c, a, c, d, a, c, b, a, d, c})
      marker->AddPoint(p, math::Color::White);
  };

  for (const auto &rect : {math::Vector4d(-1, 0.5, -0.5, 0.8),
      math::Vector4d(0.2, 1.5, -1.0, 0.0), math::Vector4d(-2, -1, 0.3, 1.2)})
  {
    setRect(rect[0], rect[1], rect[2], rect[3]);
    auto result = this->Render(camera);
    ASSERT_EQ(1u, result.size());
    EXPECT_EQ(3u, result[0].Label());
    const math::Pose3d pose(5, 0.5 * (rect[0] + rect[1]),
        0.5 * (rect[2] + rect[3]), 0, 0, 0);
    const math::Vector3d rectSize(0, rect[1] - rect[0], rect[3] - rect[2]);
    ExpectRectNear(ToRect(result[0]),
        *ExpectedRect(BoxCorners(pose, rectSize), model), kFullTol,
        "marker");
  }

  engine->DestroyScene(scene);
}

//////////////////////////////////////////////////
/// \brief Add an unlabeled wall in the YZ plane at _x that fills the view,
/// and _count labeled boxes right behind it: they are in the frustum (so
/// they get an index) but hidden. Anything that writes a small index into
/// the image by mistake makes one of them show up.
static void AddHiddenBoxes(ScenePtr _scene, double _x, int _count,
    int _label)
{
  VisualPtr root = _scene->RootVisual();
  VisualPtr wall = _scene->CreateVisual();
  wall->AddGeometry(_scene->CreateBox());
  wall->SetLocalPosition(_x, 0, 0);
  wall->SetLocalScale(0.1, 4 * _x, 4 * _x);
  root->AddChild(wall);
  for (int i = 0; i < _count; ++i)
  {
    VisualPtr vis = _scene->CreateVisual();
    vis->AddGeometry(_scene->CreateBox());
    vis->SetLocalPosition(_x + 1, (i % 20 - 10) * 0.3, (i / 20 - 7) * 0.3);
    vis->SetLocalScale(0.2, 0.2, 0.2);
    vis->SetUserData("label", _label);
    root->AddChild(vis);
  }
}

//////////////////////////////////////////////////
// Particles are not objects: they must not add boxes, or grow the boxes of
// other objects. Red to black particles used to be read as object indices
// 1 to 255.
TEST_F(BoundingBoxCameraAccuracyTest, ParticlesDontAddBoxes)
{
  CHECK_SUPPORTED_ENGINE("ogre2");
  ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);

  const CameraModel model;
  auto camera = this->CreateCamera(scene, model,
      BoundingBoxType::BBT_VISIBLEBOX2D);
  ASSERT_NE(nullptr, camera);

  AddHiddenBoxes(scene, 8, 300, 2);
  const math::Pose3d pose(4, 1, 0, 0, 0, 0);
  const math::Vector3d size(0.5, 0.5, 0.5);
  AddBox(scene, scene->RootVisual(), pose, size, 1);

  // a cloud of particles drifting sideways in front of the camera
  ParticleEmitterPtr emitter = scene->CreateParticleEmitter();
  ASSERT_NE(nullptr, emitter);
  emitter->SetLocalPosition(3, 0, 0);
  emitter->SetLocalRotation(0, 0, GZ_PI / 2);
  emitter->SetEmitterSize({0.5, 3, 2});
  emitter->SetParticleSize({0.3, 0.3, 0.3});
  emitter->SetRate(500);
  emitter->SetLifetime(2);
  emitter->SetVelocityRange(0.5, 1.0);
  emitter->SetScaleRate(0.0);
  // Dark red: its pixels decode to indices 1 to 3, all valid indices of
  // the hidden boxes. (SetColorRange is not supported by ogre2.)
  MaterialPtr darkRed = scene->CreateMaterial();
  darkRed->SetDiffuse(2.0 / 255, 0, 0);
  darkRed->SetEmissive(2.0 / 255, 0, 0);
  emitter->SetMaterial(darkRed);
  emitter->SetType(EmitterType::EM_BOX);
  emitter->SetEmitting(true);
  scene->RootVisual()->AddChild(emitter);

  for (auto type : {BoundingBoxType::BBT_VISIBLEBOX2D,
      BoundingBoxType::BBT_FULLBOX2D, BoundingBoxType::BBT_BOX3D})
  {
    SCOPED_TRACE(static_cast<int>(type));
    camera->SetBoundingBoxType(type);
    for (int i = 0; i < 30; ++i)
    {
      scene->SetTime(scene->Time() + std::chrono::milliseconds(33));
      auto result = this->Render(camera);
      EXPECT_TRUE(WithLabel(result, 2).empty())
          << "frame " << i << ": a hidden box was reported";
      ASSERT_EQ(1u, WithLabel(result, 1).size()) << "frame " << i;
      if (type == BoundingBoxType::BBT_FULLBOX2D)
      {
        ExpectRectNear(ToRect(WithLabel(result, 1)[0]),
            *ExpectedRect(BoxCorners(pose, size), model), kFullTol,
            "full");
      }
      else if (type == BoundingBoxType::BBT_VISIBLEBOX2D)
      {
        // particles may hide part of the box, but never grow it
        const Rect actual = ToRect(WithLabel(result, 1)[0]);
        const Rect expected = *ExpectedRect(BoxCorners(pose, size), model);
        EXPECT_GE(actual.minX, expected.minX - kVisibleTol);
        EXPECT_LE(actual.maxX, expected.maxX + kVisibleTol);
        EXPECT_GE(actual.minY, expected.minY - kVisibleTol);
        EXPECT_LE(actual.maxY, expected.maxY + kVisibleTol);
      }
    }
  }

  engine->DestroyScene(scene);
}

//////////////////////////////////////////////////
// A heightmap is an unlabeled occluder: it hides what's behind it and adds
// no box of its own.
TEST_F(BoundingBoxCameraAccuracyTest, HeightmapOccludesWithoutBoxes)
{
  CHECK_SUPPORTED_ENGINE("ogre2");
  engine->AddResourcePath(
      common::joinPaths(std::string(PROJECT_BUILD_PATH), "src"));
  ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);

  const CameraModel model;
  auto camera = this->CreateCamera(scene, model,
      BoundingBoxType::BBT_VISIBLEBOX2D);
  ASSERT_NE(nullptr, camera);

  const std::string media = common::joinPaths(
      std::string(PROJECT_SOURCE_PATH), "test", "media");
  auto data = std::make_shared<common::ImageHeightmap>();
  ASSERT_EQ(0, data->Load(common::joinPaths(media, "heightmap_bowl.png")));
  HeightmapDescriptor desc;
  desc.SetName("bbox_heightmap");
  desc.SetData(data);
  desc.SetSize({40, 40, 6});
  desc.SetPosition({20, 0, -9});
  desc.SetSampling(2u);
  desc.SetUseTerrainPaging(false);
  HeightmapTexture texture;
  texture.SetSize(1.0);
  texture.SetDiffuse(common::joinPaths(media, "materials", "textures",
      "texture.png"));
  texture.SetNormal(common::joinPaths(media, "materials", "textures",
      "flat_normal.png"));
  desc.AddTexture(texture);
  auto heightmap = scene->CreateHeightmap(desc);
  ASSERT_NE(nullptr, heightmap);
  VisualPtr terrain = scene->CreateVisual();
  terrain->AddGeometry(heightmap);
  scene->RootVisual()->AddChild(terrain);

  // The camera looks down on the terrain (heights -9 to -3). A visible box
  // above the terrain, and many boxes buried under it, in
  // the frustum but hidden.
  const math::Pose3d pose(6, 0, 1.5, 0, 0, 0);
  const math::Vector3d size(0.5, 0.5, 0.5);
  AddBox(scene, scene->RootVisual(), pose, size, 1);
  for (int i = 0; i < 300; ++i)
  {
    AddBox(scene, scene->RootVisual(),
        {17 + (i % 20) * 0.5, (i / 20 - 7) * 0.5, -12, 0, 0, 0},
        {0.3, 0.3, 0.3}, 2);
  }

  // sanity check: the buried boxes are in the view, so they get an index
  bool inView = false;
  for (int i = 0; i < 300 && !inView; ++i)
  {
    inView = ExpectedRect(BoxCorners(
        {17 + (i % 20) * 0.5, (i / 20 - 7) * 0.5, -12, 0, 0, 0},
        {0.3, 0.3, 0.3}), model).has_value();
  }
  ASSERT_TRUE(inView);

  for (auto type : {BoundingBoxType::BBT_VISIBLEBOX2D,
      BoundingBoxType::BBT_FULLBOX2D, BoundingBoxType::BBT_BOX3D})
  {
    SCOPED_TRACE(static_cast<int>(type));
    auto result = this->Render(camera, type);
    EXPECT_TRUE(WithLabel(result, 2).empty()) << "a buried box was reported";
    ASSERT_EQ(1u, WithLabel(result, 1).size());
    EXPECT_EQ(1u, result.size());
    if (type == BoundingBoxType::BBT_VISIBLEBOX2D)
    {
      ExpectRectNear(ToRect(result[0]),
          *ExpectedRect(BoxCorners(pose, size), model), kVisibleTol,
          "visible");
    }
  }

  engine->DestroyScene(scene);
}

//////////////////////////////////////////////////
// A transparent object gets the same box as an opaque one: its index color
// must not be blended with what's behind it.
TEST_F(BoundingBoxCameraAccuracyTest, TransparentObject)
{
  CHECK_SUPPORTED_ENGINE("ogre2");
  ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);

  const CameraModel model;
  auto camera = this->CreateCamera(scene, model,
      BoundingBoxType::BBT_VISIBLEBOX2D);
  ASSERT_NE(nullptr, camera);

  MaterialPtr glass = scene->CreateMaterial();
  glass->SetDiffuse(0.2, 0.8, 0.2);
  glass->SetTransparency(0.6);

  // a transparent box in front of an opaque one
  const math::Pose3d frontPose(4, 0.3, 0, 0, 0, 0.3);
  const math::Vector3d frontSize(0.8, 0.8, 0.8);
  VisualPtr front = AddBox(scene, scene->RootVisual(), frontPose, frontSize,
      7);
  front->SetMaterial(glass);
  AddBox(scene, scene->RootVisual(), {7, -0.5, 0, 0, 0, 0}, {2, 2, 2}, 9);

  auto result = this->Render(camera);
  auto found = WithLabel(result, 7);
  ASSERT_EQ(1u, found.size());
  ExpectRectNear(ToRect(found[0]),
      *ExpectedRect(BoxCorners(frontPose, frontSize), model), kVisibleTol,
      "transparent");
  // no box from blended colors
  for (const auto &box : result)
    EXPECT_TRUE(box.Label() == 7u || box.Label() == 9u) << box.Label();

  // the material is still transparent for other cameras
  EXPECT_DOUBLE_EQ(0.6, glass->Transparency());

  engine->DestroyScene(scene);
}

//////////////////////////////////////////////////
// A point cloud marker (low level material) gets a box.
TEST_F(BoundingBoxCameraAccuracyTest, PointsMarker)
{
  CHECK_SUPPORTED_ENGINE("ogre2");
  ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);

  const CameraModel model;
  auto camera = this->CreateCamera(scene, model,
      BoundingBoxType::BBT_VISIBLEBOX2D);
  ASSERT_NE(nullptr, camera);

  MarkerPtr marker = scene->CreateMarker();
  ASSERT_NE(nullptr, marker);
  marker->SetType(MarkerType::MT_POINTS);
  marker->SetSize(4.0);
  for (int y = 0; y <= 20; ++y)
  {
    for (int z = 0; z <= 20; ++z)
      marker->AddPoint({5, -1 + y * 0.1, -0.5 + z * 0.05}, math::Color::Red);
  }
  VisualPtr vis = scene->CreateVisual();
  vis->AddGeometry(marker);
  vis->SetUserData("label", 4);
  scene->RootVisual()->AddChild(vis);

  auto result = this->Render(camera);
  ASSERT_EQ(1u, result.size());
  EXPECT_EQ(4u, result[0].Label());
  // the points span this rect; drawn points may be a few pixels wide
  const Rect expected = *ExpectedRect(BoxCorners({5, 0, 0, 0, 0, 0},
      {0, 2, 1}), model);
  const Rect actual = ToRect(result[0]);
  EXPECT_NEAR(actual.CenterX(), expected.CenterX(), 3.0);
  EXPECT_NEAR(actual.CenterY(), expected.CenterY(), 3.0);
  EXPECT_NEAR(actual.Width(), expected.Width(), 6.0);
  EXPECT_NEAR(actual.Height(), expected.Height(), 6.0);

  engine->DestroyScene(scene);
}

//////////////////////////////////////////////////
// Rendering the bounding box camera leaves the scene as it was: a regular
// camera sees exactly the same image before and after.
TEST_F(BoundingBoxCameraAccuracyTest, OtherCamerasUnchanged)
{
  CHECK_SUPPORTED_ENGINE("ogre2");
  ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);
  scene->SetAmbientLight(0.4, 0.4, 0.4);
  DirectionalLightPtr light = scene->CreateDirectionalLight();
  light->SetDirection(0.5, 0.3, -1);
  scene->RootVisual()->AddChild(light);

  const CameraModel model;
  auto bboxCamera = this->CreateCamera(scene, model,
      BoundingBoxType::BBT_VISIBLEBOX2D);
  ASSERT_NE(nullptr, bboxCamera);

  MaterialPtr glass = scene->CreateMaterial();
  glass->SetDiffuse(0.2, 0.2, 0.9);
  glass->SetTransparency(0.5);
  MaterialPtr red = scene->CreateMaterial();
  red->SetDiffuse(0.9, 0.1, 0.1);
  VisualPtr a = AddBox(scene, scene->RootVisual(), {4, 0.5, 0, 0, 0, 0.4},
      {1, 1, 1}, 1);
  a->SetMaterial(glass);
  VisualPtr b = AddBox(scene, scene->RootVisual(), {6, -0.5, 0, 0, 0, 0},
      {1.5, 1.5, 1.5}, 2);
  b->SetMaterial(red);
  AddBox(scene, scene->RootVisual(), {5, -2, 0.5, 0, 0, 0}, {1, 1, 1},
      std::nullopt);

  CameraPtr rgb = scene->CreateCamera();
  ASSERT_NE(nullptr, rgb);
  rgb->SetImageWidth(model.width);
  rgb->SetImageHeight(model.height);
  rgb->SetAspectRatio(model.Aspect());
  rgb->SetHFOV(model.hfov);
  scene->RootVisual()->AddChild(rgb);

  Image before = rgb->CreateImage();
  rgb->Capture(before);
  rgb->Capture(before);

  for (int i = 0; i < 3; ++i)
    EXPECT_EQ(2u, this->Render(bboxCamera).size());

  Image after = rgb->CreateImage();
  rgb->Capture(after);
  const size_t bytes = before.MemorySize();
  ASSERT_EQ(bytes, after.MemorySize());
  EXPECT_EQ(0, std::memcmp(before.Data<unsigned char>(),
      after.Data<unsigned char>(), bytes))
      << "the regular camera's image changed";

  engine->DestroyScene(scene);
}

//////////////////////////////////////////////////
// Moving a link to another model, or changing a label, shows in the very
// next frame. Covers the caches of the material switcher.
TEST_F(BoundingBoxCameraAccuracyTest, ReparentAndRelabel)
{
  CHECK_SUPPORTED_ENGINE("ogre2");
  ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);

  const CameraModel model;
  auto camera = this->CreateCamera(scene, model,
      BoundingBoxType::BBT_VISIBLEBOX2D);
  ASSERT_NE(nullptr, camera);

  // model A on the left, model B on the right, two links each
  const math::Vector3d size(0.4, 0.4, 0.4);
  VisualPtr modelA = scene->CreateVisual("model_a");
  modelA->SetLocalPosition(6, 1.5, 0);
  scene->RootVisual()->AddChild(modelA);
  VisualPtr modelB = scene->CreateVisual("model_b");
  modelB->SetLocalPosition(6, -1.5, 0);
  scene->RootVisual()->AddChild(modelB);
  VisualPtr a1 = AddBox(scene, modelA, {0, 0.5, 0, 0, 0, 0}, size, 1);
  AddBox(scene, modelA, {0, -0.5, 0, 0, 0, 0}, size, 1);
  AddBox(scene, modelB, {0, 0.5, 0, 0, 0, 0}, size, 2);
  AddBox(scene, modelB, {0, -0.5, 0, 0, 0, 0}, size, 2);

  // the only box with _label in _boxes
  auto only = [](const std::vector<BoundingBox> &_boxes, uint32_t _label)
  {
    auto found = WithLabel(_boxes, _label);
    EXPECT_EQ(1u, found.size()) << "label " << _label;
    return found.empty() ? Rect() : ToRect(found[0]);
  };

  auto rectOf = [&](const std::vector<math::Vector3d> &_centers)
  {
    Rect r = *ExpectedRect(BoxCorners({_centers[0], {}}, size), model);
    for (size_t i = 1; i < _centers.size(); ++i)
      r.Merge(*ExpectedRect(BoxCorners({_centers[i], {}}, size), model));
    return r;
  };

  for (int frame = 0; frame < 2; ++frame)
  {
    auto result = this->Render(camera);
    ASSERT_EQ(2u, result.size());
    ExpectRectNear(only(result, 1),
        rectOf({{6, 2.0, 0}, {6, 1.0, 0}}), kVisibleTol, "model a");
    ExpectRectNear(only(result, 2),
        rectOf({{6, -1.0, 0}, {6, -2.0, 0}}), kVisibleTol, "model b");
  }

  // move link a1 to model B, keeping it where it is in the world
  modelA->RemoveChild(a1);
  modelB->AddChild(a1);
  a1->SetLocalPosition(0, 3.5, 0);
  a1->SetUserData("label", 2);
  {
    auto result = this->Render(camera);
    ASSERT_EQ(2u, result.size());
    ExpectRectNear(only(result, 1),
        rectOf({{6, 1.0, 0}}), kVisibleTol, "model a after the move");
    ExpectRectNear(only(result, 2),
        rectOf({{6, 2.0, 0}, {6, -1.0, 0}, {6, -2.0, 0}}), kVisibleTol,
        "model b after the move");
  }

  // relabel model A's remaining link
  for (unsigned int i = 0; i < modelA->ChildCount(); ++i)
    modelA->ChildByIndex(i)->SetUserData("label", 5);
  {
    auto result = this->Render(camera);
    ASSERT_EQ(2u, result.size());
    EXPECT_TRUE(WithLabel(result, 1).empty());
    ASSERT_EQ(1u, WithLabel(result, 5).size());
  }

  // move model B under model A: one model, the first link's label
  scene->RootVisual()->RemoveChild(modelB);
  modelA->AddChild(modelB);
  modelB->SetLocalPosition(0, -3, 0);
  {
    auto result = this->Render(camera);
    ASSERT_EQ(1u, result.size());
    ExpectRectNear(ToRect(result[0]),
        rectOf({{6, 2.0, 0}, {6, 1.0, 0}, {6, -1.0, 0}, {6, -2.0, 0}}),
        kVisibleTol, "merged");
  }

  engine->DestroyScene(scene);
}

//////////////////////////////////////////////////
// The full box of a static object follows the camera when it moves, and
// stays the same when it doesn't. Covers reusing full boxes between frames.
TEST_F(BoundingBoxCameraAccuracyTest, CameraMovesFullBox)
{
  CHECK_SUPPORTED_ENGINE("ogre2");
  ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);

  const CameraModel model;
  auto camera = this->CreateCamera(scene, model,
      BoundingBoxType::BBT_FULLBOX2D);
  ASSERT_NE(nullptr, camera);

  const math::Pose3d pose(6, 0.5, 0.2, 0.3, 0.2, 0.7);
  const math::Vector3d size(1, 0.6, 0.8);
  AddBox(scene, scene->RootVisual(), pose, size, 1);

  for (const math::Pose3d &cameraPose : {math::Pose3d::Zero,
      math::Pose3d::Zero, math::Pose3d(0, 0.5, 0.3, 0, 0, 0),
      math::Pose3d(0, 0.5, 0.3, 0, 0, 0), math::Pose3d(1, 0, 0, 0, 0, 0.2),
      math::Pose3d(1, 0, 0, 0, 0.1, 0.2)})
  {
    camera->SetLocalPose(cameraPose);
    auto result = this->Render(camera);
    ASSERT_EQ(1u, result.size());
    // the box as seen from the camera, which sits at the origin
    ExpectRectNear(ToRect(result[0]), *ExpectedRect(BoxCorners(
        cameraPose.Inverse() * pose, size), model), kFullTol, "full");
  }

  engine->DestroyScene(scene);
}

//////////////////////////////////////////////////
// The full box of a non convex mesh is the box of all its projected
// vertices. Covers leaving out the vertices inside the mesh's hull.
TEST_F(BoundingBoxCameraAccuracyTest, MeshFullBoxMatchesVertices)
{
  CHECK_SUPPORTED_ENGINE("ogre2");
  ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);

  const CameraModel model;
  auto camera = this->CreateCamera(scene, model,
      BoundingBoxType::BBT_FULLBOX2D);
  ASSERT_NE(nullptr, camera);

  // a character: most of its vertices are inside its convex hull
  const std::string path = common::joinPaths(
      std::string(PROJECT_SOURCE_PATH), "test", "media", "meshes",
      "walk.dae");
  const common::Mesh *mesh = common::MeshManager::Instance()->Load(path);
  ASSERT_NE(nullptr, mesh);
  std::vector<math::Vector3d> vertices;
  for (unsigned int i = 0; i < mesh->SubMeshCount(); ++i)
  {
    auto subMesh = mesh->SubMeshByIndex(i).lock();
    ASSERT_NE(nullptr, subMesh);
    for (unsigned int v = 0; v < subMesh->VertexCount(); ++v)
      vertices.push_back(subMesh->Vertex(v));
  }
  ASSERT_GT(vertices.size(), 1000u);

  VisualPtr vis = scene->CreateVisual();
  vis->AddGeometry(scene->CreateMesh(MeshDescriptor(path)));
  vis->SetUserData("label", 3);
  scene->RootVisual()->AddChild(vis);

  const math::Vector3d scale(0.8, 0.6, 1.2);
  vis->SetLocalScale(scale);
  const math::Matrix4d proj = model.Projection();
  for (const math::Pose3d &pose : {math::Pose3d(4, 0, 0, 0, 0, 0),
      math::Pose3d(5, 1, -0.5, 0.3, -0.4, 1.0),
      math::Pose3d(3, -0.5, 0.4, 1.2, 0.5, -2.0),
      math::Pose3d(6, 0.2, 0.1, -0.7, 1.1, 2.6)})
  {
    vis->SetLocalPose(pose);
    auto result = this->Render(camera);
    ASSERT_EQ(1u, result.size());

    // project every vertex
    Rect expected;
    for (const auto &v : vertices)
    {
      const math::Vector3d o = ToOptical(pose.Pos() + pose.Rot() * (v * scale));
      const double cw = proj(3, 2) * o.Z();
      const double u = (proj(0, 0) * o.X() / cw + 1.0) * 0.5 * model.width;
      const double w = (1.0 - proj(1, 1) * o.Y() / cw) * 0.5 * model.height;
      expected.minX = std::min(expected.minX, u);
      expected.maxX = std::max(expected.maxX, u);
      expected.minY = std::min(expected.minY, w);
      expected.maxY = std::max(expected.maxY, w);
    }
    ASSERT_GT(expected.minX, 0.0);
    ASSERT_LT(expected.maxX, model.width - 1.0);
    ASSERT_GT(expected.minY, 0.0);
    ASSERT_LT(expected.maxY, model.height - 1.0);
    ExpectRectNear(ToRect(result[0]), expected, 0.01, "mesh");
  }

  engine->DestroyScene(scene);
}
