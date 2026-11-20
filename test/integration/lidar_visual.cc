/*
 * Copyright (C) 2020 Open Source Robotics Foundation
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

#include <gtest/gtest.h>

#include "CommonRenderingTest.hh"

#include <algorithm>
#include <cstring>
#include <limits>
#include <random>
#include <string>
#include <vector>

#include <gz/common/Image.hh>
#include <gz/common/Filesystem.hh>

#include "gz/rendering/Camera.hh"
#include "gz/rendering/GpuRays.hh"
#include "gz/rendering/Image.hh"
#include "gz/rendering/Light.hh"
#include "gz/rendering/LidarVisual.hh"
#include "gz/rendering/Scene.hh"

#include <gz/utils/ExtraTestMacros.hh>

constexpr double LASER_TOL = 2e-4;

// vertical range values seem to be less accurate
constexpr double VERTICAL_LASER_TOL = 1e-3;

using namespace gz;
using namespace rendering;

/////////////////////////////////////////////////
void OnNewGpuRaysFrame(float *_scanDest, const float *_scan,
                  unsigned int _width, unsigned int _height,
                  unsigned int _channels,
                  const std::string &/*_format*/)
{
  const auto size = _width * _height * _channels;
  memcpy(_scanDest, _scan, size * sizeof(float));
}

/////////////////////////////////////////////////
class LidarVisualTest: public CommonRenderingTest
{
};

/////////////////////////////////////////////////
/// \brief Test LidarVisual configuraions
TEST_F(LidarVisualTest, Configure)
{
  ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);

  VisualPtr root = scene->RootVisual();
  ASSERT_NE(nullptr, root);

  LidarVisualPtr lidarVis = scene->CreateLidarVisual();
  ASSERT_NE(nullptr, lidarVis);
  root->AddChild(lidarVis);

  // set initial pose for lidar
  math::Vector3d initPos(-2, 0.0, 5.0);
  math::Quaterniond initRot = math::Quaterniond::Identity;
  lidarVis->SetWorldPosition(initPos);
  EXPECT_EQ(initPos, lidarVis->WorldPosition());
  EXPECT_EQ(initRot, lidarVis->WorldRotation());

  // The following tests all the getters and setters
  {
    lidarVis->SetMinVerticalAngle(0.1);
    EXPECT_DOUBLE_EQ(lidarVis->MinVerticalAngle(), 0.1);

    lidarVis->SetMaxVerticalAngle(251.20);
    EXPECT_DOUBLE_EQ(lidarVis->MaxVerticalAngle(), 251.20);

    lidarVis->SetMinRange(0.05);
    EXPECT_DOUBLE_EQ(lidarVis->MinRange(), 0.05);

    lidarVis->SetMaxRange(105.20);
    EXPECT_DOUBLE_EQ(lidarVis->MaxRange(), 105.20);

    lidarVis->SetMinHorizontalAngle(-1.5136);
    EXPECT_DOUBLE_EQ(lidarVis->MinHorizontalAngle(), -1.5136);

    lidarVis->SetMaxHorizontalAngle(2.513);
    EXPECT_DOUBLE_EQ(lidarVis->MaxHorizontalAngle(), 2.513);

    lidarVis->SetHorizontalRayCount(360);
    EXPECT_EQ(lidarVis->HorizontalRayCount(), 360u);

    lidarVis->SetVerticalRayCount(30);
    EXPECT_EQ(lidarVis->VerticalRayCount(), 30u);

    gz::math::Pose3d offset(1.5, 3.6, 2.9, 1.1, -5.3, -2.9);
    lidarVis->SetOffset(offset);
    EXPECT_EQ(lidarVis->Offset(), offset);

    lidarVis->SetType(LVT_NONE);
    EXPECT_EQ(lidarVis->Type(), LVT_NONE);
    lidarVis->SetType(LVT_POINTS);
    EXPECT_EQ(lidarVis->Type(), LVT_POINTS);
    lidarVis->SetType(LVT_POINTS);
    EXPECT_EQ(lidarVis->Type(), LVT_POINTS);
    lidarVis->SetType(LVT_TRIANGLE_STRIPS);
    EXPECT_EQ(lidarVis->Type(), LVT_TRIANGLE_STRIPS);

    lidarVis->SetDisplayNonHitting(true);
    EXPECT_EQ(lidarVis->DisplayNonHitting(), true);
    lidarVis->SetDisplayNonHitting(false);
    EXPECT_EQ(lidarVis->DisplayNonHitting(), false);

    std::vector<double> pts = {2, 14, 15, 3, 5, 10, 3};
    lidarVis->SetPoints(pts);
    EXPECT_EQ(lidarVis->PointCount(), pts.size());
    lidarVis->ClearPoints();
    EXPECT_EQ(lidarVis->PointCount(), 0u);
  }

  // Clean up
  engine->DestroyScene(scene);
}

/////////////////////////////////////////////////
/// \brief Test detection of different boxes
TEST_F(LidarVisualTest, GZ_UTILS_TEST_DISABLED_ON_WIN32(RaysUnitBox))
{
  #ifdef __APPLE__
    GTEST_SKIP() << "Unsupported on apple, see issue #35.";
  #endif

  // Test lidar visual with 3 boxes in the world, using reading from GPU rays
  // First GPU rays at identity orientation, second at 90 degree roll
  // First place 2 of 3 boxes within range and verify range values from lidar.
  // then move all 3 boxes out of range and verify range values from lidar

  const double hMinAngle = -GZ_PI/2.0;
  const double hMaxAngle = GZ_PI/2.0;
  const double minRange = 0.1;
  const double maxRange = 10.0;
  const int hRayCount = 320;
  const int vRayCount = 1;

  ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);

  VisualPtr root = scene->RootVisual();
  ASSERT_NE(nullptr, root);

  // Create first ray caster
  gz::math::Pose3d testPose(gz::math::Vector3d(0, 0, 0.1),
      gz::math::Quaterniond::Identity);

  GpuRaysPtr gpuRays = scene->CreateGpuRays("gpu_rays_1");
  gpuRays->SetWorldPosition(testPose.Pos());
  gpuRays->SetWorldRotation(testPose.Rot());
  gpuRays->SetNearClipPlane(minRange);
  gpuRays->SetFarClipPlane(maxRange);
  gpuRays->SetAngleMin(hMinAngle);
  gpuRays->SetAngleMax(hMaxAngle);
  gpuRays->SetRayCount(hRayCount);
  gpuRays->SetVerticalRayCount(vRayCount);
  root->AddChild(gpuRays);

  // Create first lidar
  LidarVisualPtr lidarVis = scene->CreateLidarVisual();
  lidarVis->SetWorldPosition(testPose.Pos());
  lidarVis->SetWorldRotation(testPose.Rot());
  lidarVis->SetMinRange(minRange);
  lidarVis->SetMaxRange(maxRange);
  lidarVis->SetMinHorizontalAngle(hMinAngle);
  lidarVis->SetMaxHorizontalAngle(hMaxAngle);
  lidarVis->SetHorizontalRayCount(hRayCount);
  lidarVis->SetVerticalRayCount(vRayCount);
  lidarVis->SetType(LidarVisualType::LVT_TRIANGLE_STRIPS);
  lidarVis->SetDisplayNonHitting(true);
  root->AddChild(lidarVis);

  // Create a second ray caster rotated
  gz::math::Pose3d testPose2(gz::math::Vector3d(0, 0, 0.1),
      gz::math::Quaterniond(GZ_PI/2.0, 0, 0));

  GpuRaysPtr gpuRays2 = scene->CreateGpuRays("gpu_rays_2");
  gpuRays2->SetWorldPosition(testPose2.Pos());
  gpuRays2->SetWorldRotation(testPose2.Rot());
  gpuRays2->SetNearClipPlane(minRange);
  gpuRays2->SetFarClipPlane(maxRange);
  gpuRays2->SetClamp(true);
  gpuRays2->SetAngleMin(hMinAngle);
  gpuRays2->SetAngleMax(hMaxAngle);
  gpuRays2->SetRayCount(hRayCount);
  gpuRays2->SetVerticalRayCount(vRayCount);
  root->AddChild(gpuRays2);

  // Create a second lidar rotated
  LidarVisualPtr lidarVis2 = scene->CreateLidarVisual();
  lidarVis2->SetWorldPosition(testPose2.Pos());
  lidarVis2->SetWorldRotation(testPose2.Rot());
  lidarVis2->SetMinRange(minRange);
  lidarVis2->SetMaxRange(maxRange);
  lidarVis2->SetMinHorizontalAngle(hMinAngle);
  lidarVis2->SetMaxHorizontalAngle(hMaxAngle);
  lidarVis2->SetHorizontalRayCount(hRayCount);
  lidarVis2->SetVerticalRayCount(vRayCount);
  lidarVis2->SetType(LidarVisualType::LVT_TRIANGLE_STRIPS);
  lidarVis2->SetDisplayNonHitting(true);
  root->AddChild(lidarVis2);

  // Create testing boxes
  // box in the center
  gz::math::Pose3d box01Pose(gz::math::Vector3d(3, 0, 0.5),
                                   gz::math::Quaterniond::Identity);
  VisualPtr visualBox1 = scene->CreateVisual("UnitBox1");
  visualBox1->AddGeometry(scene->CreateBox());
  visualBox1->SetWorldPosition(box01Pose.Pos());
  visualBox1->SetWorldRotation(box01Pose.Rot());
  root->AddChild(visualBox1);

  // box on the right of the first gpu rays caster
  gz::math::Pose3d box02Pose(gz::math::Vector3d(0, -5, 0.5),
                                   gz::math::Quaterniond::Identity);
  VisualPtr visualBox2 = scene->CreateVisual("UnitBox2");
  visualBox2->AddGeometry(scene->CreateBox());
  visualBox2->SetWorldPosition(box02Pose.Pos());
  visualBox2->SetWorldRotation(box02Pose.Rot());
  root->AddChild(visualBox2);

  // box on the left of the rays caster 1 but out of range
  gz::math::Pose3d box03Pose(
      gz::math::Vector3d(0, maxRange + 1, 0.5),
      gz::math::Quaterniond::Identity);
  VisualPtr visualBox3 = scene->CreateVisual("UnitBox3");
  visualBox3->AddGeometry(scene->CreateBox());
  visualBox3->SetWorldPosition(box03Pose.Pos());
  visualBox3->SetWorldRotation(box03Pose.Rot());
  root->AddChild(visualBox3);

  // Verify rays caster 1 range readings
  // listen to new gpu rays frames
  unsigned int channels = gpuRays->Channels();
  float *scan = new float[hRayCount * vRayCount * channels];
  common::ConnectionPtr c =
    gpuRays->ConnectNewGpuRaysFrame(
        std::bind(&::OnNewGpuRaysFrame, scan,
          std::placeholders::_1, std::placeholders::_2, std::placeholders::_3,
          std::placeholders::_4, std::placeholders::_5));

  gpuRays->Update();

  std::vector<double> pts;
  for (int j = 0; j < vRayCount; j++)
  {
    for (int i = 0; i < gpuRays->RayCount(); ++i)
    {
      pts.push_back(scan[j*channels*gpuRays->RayCount() + i * channels]);
    }
  }
  lidarVis->SetPoints(pts);
  lidarVis->Update();

  // receive points from lidar to check values
  std::vector<double> pts_back = lidarVis->Points();

  int mid = static_cast<int>(hRayCount/2);
  int last = (hRayCount - 1);
  double unitBoxSize = 1.0;
  double expectedRangeAtMidPointBox1 = abs(box01Pose.Pos().X()) - unitBoxSize/2;
  double expectedRangeAtMidPointBox2 = abs(box02Pose.Pos().Y()) - unitBoxSize/2;

  // rays caster values received from lidar should see box01 and box02
  EXPECT_NEAR(pts_back[mid], expectedRangeAtMidPointBox1, LASER_TOL);
  EXPECT_NEAR(pts_back[0], expectedRangeAtMidPointBox2, LASER_TOL);
  EXPECT_FLOAT_EQ(pts_back[last], gz::math::INF_F);

  // Verify rays caster 2 range readings
  // listen to new gpu rays frames
  float *scan2 = new float[hRayCount * vRayCount * 3];

  gpuRays2->Update();
  // Test Copy method instead of using the callback for the second rays caster
  gpuRays2->Copy(scan2);

  // copy ray caster values to lidar data
  std::vector<double> pts2;
  for (int j = 0; j < vRayCount; j++)
  {
    for (int i = 0; i < gpuRays->RayCount(); ++i)
    {
      pts2.push_back(scan2[j*channels*gpuRays->RayCount() + i * channels]);
    }
  }
  lidarVis2->SetPoints(pts2);
  lidarVis2->Update();
  std::vector<double> pts_back2 = lidarVis2->Points();

  // Only box01 should be visible to rays caster 2
  EXPECT_FLOAT_EQ(pts_back2[0], maxRange);
  EXPECT_NEAR(pts_back2[mid], expectedRangeAtMidPointBox1, LASER_TOL);
  EXPECT_FLOAT_EQ(pts_back2[last], maxRange);

  // Move all boxes out of range
  visualBox1->SetWorldPosition(
      gz::math::Vector3d(maxRange + 1, 0, 0));
  visualBox1->SetWorldRotation(box01Pose.Rot());
  visualBox2->SetWorldPosition(
      gz::math::Vector3d(0, -(maxRange + 1), 0));
  visualBox2->SetWorldRotation(box02Pose.Rot());

  gpuRays->Update();
  gpuRays2->Update();
  gpuRays2->Copy(scan2);

  pts.clear();
  for (int j = 0; j < vRayCount; j++)
  {
    for (int i = 0; i < gpuRays->RayCount(); ++i)
    {
      pts.push_back(scan[j*channels*gpuRays->RayCount() + i * channels]);
    }
  }

  pts2.clear();
  for (int j = 0; j < vRayCount; j++)
  {
    for (int i = 0; i < gpuRays->RayCount(); ++i)
    {
      pts2.push_back(scan2[j*channels*gpuRays->RayCount() + i * channels]);
    }
  }

  for (unsigned int i = 0; i < lidarVis->HorizontalRayCount(); ++i)
    EXPECT_FLOAT_EQ(pts[i], gz::math::INF_F);

  for (unsigned int i = 0; i < lidarVis->HorizontalRayCount(); ++i)
    EXPECT_FLOAT_EQ(pts2[i], maxRange);

  c.reset();

  delete [] scan;
  delete [] scan2;

  scan = nullptr;
  scan2 = nullptr;

  // Clean up
  engine->DestroyScene(scene);
}

/////////////////////////////////////////////////
/// \brief Test GPU rays vertical component
TEST_F(LidarVisualTest, GZ_UTILS_TEST_DISABLED_ON_WIN32(LaserVertical))
{
  #ifdef __APPLE__
    GTEST_SKIP() << "Unsupported on apple, see issue #35.";
  #endif

  // Test a rays that has a vertical range component.
  // Place a box within range and verify range values,
  // then move the box out of range and verify range values

  double hMinAngle = -GZ_PI/2.0;
  double hMaxAngle = GZ_PI/2.0;
  double vMinAngle = -GZ_PI/4.0;
  double vMaxAngle = GZ_PI/4.0;
  double minRange = 0.1;
  double maxRange = 5.0;
  unsigned int hRayCount = 640;
  unsigned int vRayCount = 4;

  ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);

  VisualPtr root = scene->RootVisual();
  ASSERT_NE(nullptr, root);

  // Create first ray caster
  gz::math::Pose3d testPose(gz::math::Vector3d(0.25, 0, 0.5),
      gz::math::Quaterniond::Identity);

  GpuRaysPtr gpuRays = scene->CreateGpuRays("vertical_gpu_rays");
  gpuRays->SetWorldPosition(testPose.Pos());
  gpuRays->SetWorldRotation(testPose.Rot());
  gpuRays->SetNearClipPlane(minRange);
  gpuRays->SetFarClipPlane(maxRange);
  gpuRays->SetAngleMin(hMinAngle);
  gpuRays->SetAngleMax(hMaxAngle);
  gpuRays->SetVerticalAngleMin(vMinAngle);
  gpuRays->SetVerticalAngleMax(vMaxAngle);
  gpuRays->SetRayCount(hRayCount);
  gpuRays->SetVerticalRayCount(vRayCount);
  root->AddChild(gpuRays);

  LidarVisualPtr lidarVis = scene->CreateLidarVisual();
  lidarVis->SetWorldPosition(testPose.Pos());
  lidarVis->SetWorldRotation(testPose.Rot());
  lidarVis->SetMinRange(minRange);
  lidarVis->SetMaxRange(maxRange);
  lidarVis->SetMinHorizontalAngle(hMinAngle);
  lidarVis->SetMaxHorizontalAngle(hMaxAngle);
  lidarVis->SetHorizontalRayCount(hRayCount);
  lidarVis->SetVerticalRayCount(vRayCount);
  lidarVis->SetType(LidarVisualType::LVT_TRIANGLE_STRIPS);
  lidarVis->SetDisplayNonHitting(true);
  root->AddChild(lidarVis);

  // Create testing boxes
  // box in front of ray sensor
  gz::math::Pose3d box01Pose(gz::math::Vector3d(1, 0, 0.5),
      gz::math::Quaterniond::Identity);
  VisualPtr visualBox1 = scene->CreateVisual("VerticalTestBox1");
  visualBox1->AddGeometry(scene->CreateBox());
  visualBox1->SetWorldPosition(box01Pose.Pos());
  visualBox1->SetWorldRotation(box01Pose.Rot());
  root->AddChild(visualBox1);

  unsigned int channels = gpuRays->Channels();
  float *scan = new float[hRayCount * vRayCount * channels];
  common::ConnectionPtr c =
    gpuRays->ConnectNewGpuRaysFrame(
        std::bind(&::OnNewGpuRaysFrame, scan,
          std::placeholders::_1, std::placeholders::_2, std::placeholders::_3,
          std::placeholders::_4, std::placeholders::_5));

  gpuRays->Update();
  std::vector<double> pts;
  for (unsigned int j = 0; j < vRayCount; j++)
  {
    for (int i = 0; i < gpuRays->RayCount(); ++i)
    {
      pts.push_back(scan[j*channels*gpuRays->RayCount() + i * channels]);
    }
  }
  lidarVis->SetPoints(pts);
  lidarVis->Update();

  unsigned int mid = hRayCount / 2;
  double unitBoxSize = 1.0;
  double expectedRangeAtMidPoint = box01Pose.Pos().X() - unitBoxSize/2
      - testPose.Pos().X();

  double vAngleStep = (vMaxAngle - vMinAngle) / (vRayCount-1);
  double angleStep = vMinAngle;

  // all vertical laser planes should sense box
  for (unsigned int i = 0; i < vRayCount; ++i)
  {
    double expectedRange = expectedRangeAtMidPoint / cos(angleStep);

    EXPECT_NEAR(pts[i * hRayCount + mid],
        expectedRange, VERTICAL_LASER_TOL);

    angleStep += vAngleStep;

    // check that the values in the extremes are infinity
    EXPECT_FLOAT_EQ(pts[i * hRayCount ],
        gz::math::INF_F);
    EXPECT_FLOAT_EQ(pts[(i * hRayCount + (hRayCount - 1))],
        gz::math::INF_F);
  }

  // Move box out of range
  visualBox1->SetWorldPosition(
      gz::math::Vector3d(maxRange + 1, 0, 0));
  visualBox1->SetWorldRotation(
      gz::math::Quaterniond::Identity);

  // after a few more laser scans
  gpuRays->Update();
  pts.clear();
  for (unsigned int j = 0; j < vRayCount; j++)
  {
    for (int i = 0; i < gpuRays->RayCount(); ++i)
    {
      pts.push_back(scan[j*channels*gpuRays->RayCount() + i * channels]);
    }
  }
  lidarVis->SetPoints(pts);
  lidarVis->Update();


  for (unsigned int j = 0; j < lidarVis->VerticalRayCount(); ++j)
  {
    for (unsigned int i = 0; i < lidarVis->HorizontalRayCount(); ++i)
    {
      EXPECT_FLOAT_EQ(pts[j * lidarVis->HorizontalRayCount()+ i],
          gz::math::INF_F);
    }
  }

  c.reset();

  delete [] scan;
  scan = nullptr;

  // Clean up
  engine->DestroyScene(scene);
}

namespace
{
/// \brief A repeatable scan of _horizontal x _vertical rays: 30% hit
/// nothing (inf), the others hit something 1 to 9 m away.
std::vector<double> RandomScan(unsigned int _seed, unsigned int _horizontal,
    unsigned int _vertical)
{
  std::mt19937 rng(_seed);
  std::uniform_real_distribution<double> range(1.0, 9.0);
  std::uniform_real_distribution<double> unit(0.0, 1.0);
  std::vector<double> points(_horizontal * _vertical);
  for (auto &p : points)
  {
    const bool hit = unit(rng) >= 0.3;
    const double r = range(rng);
    p = hit ? r : std::numeric_limits<double>::infinity();
  }
  return points;
}

/// \brief A scene with a light and a camera looking down at a lidar visual
/// at the origin.
struct LidarScene
{
  ScenePtr scene;
  CameraPtr camera;
  LidarVisualPtr lidar;
};

/// \brief Create a LidarScene
LidarScene CreateLidarScene(RenderEngine *_engine, const std::string &_name,
    LidarVisualType _type, bool _displayNonHitting, unsigned int _horizontal,
    unsigned int _vertical)
{
  LidarScene result;
  result.scene = _engine->CreateScene(_name);
  ScenePtr scene = result.scene;
  scene->SetBackgroundColor(0, 0, 0);
  scene->SetAmbientLight(0.3, 0.3, 0.3);
  VisualPtr root = scene->RootVisual();

  DirectionalLightPtr light = scene->CreateDirectionalLight();
  light->SetDirection(0.2, 0.3, -1);
  light->SetDiffuseColor(0.8, 0.8, 0.8);
  root->AddChild(light);

  result.lidar = scene->CreateLidarVisual();
  result.lidar->SetMinRange(0.1);
  result.lidar->SetMaxRange(10.0);
  result.lidar->SetMinHorizontalAngle(-GZ_PI);
  result.lidar->SetMaxHorizontalAngle(GZ_PI);
  result.lidar->SetHorizontalRayCount(_horizontal);
  result.lidar->SetMinVerticalAngle(-0.26);
  result.lidar->SetMaxVerticalAngle(0.26);
  result.lidar->SetVerticalRayCount(_vertical);
  result.lidar->SetType(_type);
  result.lidar->SetDisplayNonHitting(_displayNonHitting);
  result.lidar->SetSize(2.0);
  root->AddChild(result.lidar);

  result.camera = scene->CreateCamera();
  result.camera->SetImageWidth(320);
  result.camera->SetImageHeight(240);
  result.camera->SetHFOV(GZ_PI / 2.0);
  result.camera->SetLocalPosition(0, 0, 12);
  result.camera->SetLocalRotation(0, GZ_PI / 2.0, 0);
  root->AddChild(result.camera);
  return result;
}

/// \brief Give the lidar visual a scan and render it.
/// \return The camera image
std::vector<unsigned char> UpdateAndCapture(LidarScene &_s,
    const std::vector<double> &_scan)
{
  _s.lidar->SetPoints(_scan);
  _s.lidar->Update();
  Image image = _s.camera->CreateImage();
  _s.camera->Capture(image);
  const unsigned char *data = image.Data<unsigned char>();
  return std::vector<unsigned char>(data, data + image.MemorySize());
}

/// \brief Number of bytes that differ between two images
size_t DiffBytes(const std::vector<unsigned char> &_a,
    const std::vector<unsigned char> &_b)
{
  if (_a.size() != _b.size())
    return std::max(_a.size(), _b.size());
  size_t diff = 0;
  for (size_t i = 0; i < _a.size(); ++i)
    diff += _a[i] != _b[i];
  return diff;
}

/// \brief Number of bytes that aren't background (black)
size_t NonBlackBytes(const std::vector<unsigned char> &_a)
{
  return static_cast<size_t>(std::count_if(_a.begin(), _a.end(),
      [](unsigned char _c) { return _c != 0; }));
}

const LidarVisualType kDrawnTypes[] = {LidarVisualType::LVT_RAY_LINES,
    LidarVisualType::LVT_TRIANGLE_STRIPS, LidarVisualType::LVT_POINTS};
}  // namespace

/////////////////////////////////////////////////
/// \brief A lidar visual updated with several scans must look the same as a
/// new one given only the last scan, whether non-hitting rays are shown or
/// not.
TEST_F(LidarVisualTest,
       GZ_UTILS_TEST_DISABLED_ON_WIN32(UpdatedVisualMatchesNewVisual))
{
  CHECK_SUPPORTED_ENGINE("ogre2");
#ifdef __APPLE__
  GTEST_SKIP() << "Unsupported on apple, see issue #35.";
#endif

  const unsigned int horizontal = 200;
  const unsigned int vertical = 8;
  const auto scan0 = RandomScan(1, horizontal, vertical);
  const auto scan1 = RandomScan(2, horizontal, vertical);

  int sceneCount = 0;
  for (auto type : kDrawnTypes)
  {
    for (bool nonHitting : {true, false})
    {
      SCOPED_TRACE("type " + std::to_string(static_cast<int>(type)) +
          (nonHitting ? ", non hitting shown" : ", non hitting hidden"));

      LidarScene updated = CreateLidarScene(engine,
          "updated_" + std::to_string(sceneCount), type, nonHitting,
          horizontal, vertical);
      for (int i = 0; i < 4; ++i)
        UpdateAndCapture(updated, i % 2 == 0 ? scan0 : scan1);
      const auto updatedImage = UpdateAndCapture(updated, scan1);

      LidarScene fresh = CreateLidarScene(engine,
          "fresh_" + std::to_string(sceneCount), type, nonHitting,
          horizontal, vertical);
      const auto freshImage = UpdateAndCapture(fresh, scan1);
      ++sceneCount;

      EXPECT_GT(NonBlackBytes(freshImage), 0u);
      EXPECT_EQ(0u, DiffBytes(updatedImage, freshImage));

      engine->DestroyScene(updated.scene);
      engine->DestroyScene(fresh.scene);
    }
  }
}

/////////////////////////////////////////////////
/// \brief Changing the number of vertical rays, or whether non-hitting rays
/// are shown, must give the same image as a new visual with that setting.
TEST_F(LidarVisualTest,
       GZ_UTILS_TEST_DISABLED_ON_WIN32(ChangedSettingsMatchNewVisual))
{
  CHECK_SUPPORTED_ENGINE("ogre2");
#ifdef __APPLE__
  GTEST_SKIP() << "Unsupported on apple, see issue #35.";
#endif

  const unsigned int horizontal = 200;
  const auto scan8 = RandomScan(3, horizontal, 8);
  const auto scan4 = RandomScan(4, horizontal, 4);

  int sceneCount = 0;
  for (auto type : kDrawnTypes)
  {
    SCOPED_TRACE("type " + std::to_string(static_cast<int>(type)));
    for (bool nonHitting : {true, false})
    {
      // fewer vertical rays
      LidarScene updated = CreateLidarScene(engine,
          "shrink_" + std::to_string(sceneCount), type, nonHitting,
          horizontal, 8);
      UpdateAndCapture(updated, scan8);
      updated.lidar->SetVerticalRayCount(4);
      const auto updatedImage = UpdateAndCapture(updated, scan4);

      LidarScene fresh = CreateLidarScene(engine,
          "shrink_fresh_" + std::to_string(sceneCount), type, nonHitting,
          horizontal, 4);
      const auto freshImage = UpdateAndCapture(fresh, scan4);
      ++sceneCount;
      EXPECT_EQ(0u, DiffBytes(updatedImage, freshImage))
          << (nonHitting ? "shown" : "hidden");
      engine->DestroyScene(updated.scene);
      engine->DestroyScene(fresh.scene);
    }

    // shown -> hidden -> shown
    LidarScene toggled = CreateLidarScene(engine,
        "toggle_" + std::to_string(sceneCount), type, true, horizontal, 8);
    LidarScene freshShown = CreateLidarScene(engine,
        "toggle_shown_" + std::to_string(sceneCount), type, true,
        horizontal, 8);
    LidarScene freshHidden = CreateLidarScene(engine,
        "toggle_hidden_" + std::to_string(sceneCount), type, false,
        horizontal, 8);
    ++sceneCount;
    const auto shownImage = UpdateAndCapture(freshShown, scan8);
    const auto hiddenImage = UpdateAndCapture(freshHidden, scan8);
    EXPECT_NE(0u, DiffBytes(shownImage, hiddenImage));

    EXPECT_EQ(0u, DiffBytes(UpdateAndCapture(toggled, scan8), shownImage));
    toggled.lidar->SetDisplayNonHitting(false);
    EXPECT_EQ(0u, DiffBytes(UpdateAndCapture(toggled, scan8), hiddenImage));
    toggled.lidar->SetDisplayNonHitting(true);
    EXPECT_EQ(0u, DiffBytes(UpdateAndCapture(toggled, scan8), shownImage));

    engine->DestroyScene(toggled.scene);
    engine->DestroyScene(freshShown.scene);
    engine->DestroyScene(freshHidden.scene);
  }
}
