/*
 * Copyright (C) 2017 Open Source Robotics Foundation
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

#include <ignition/common/Console.hh>

#include "test_config.h"  // NOLINT(build/include)

#include "ignition/rendering/GpuRays.hh"
#include "ignition/rendering/RenderEngine.hh"
#include "ignition/rendering/RenderingIface.hh"
#include "ignition/rendering/Scene.hh"

using namespace ignition;
using namespace rendering;

void OnNewLaserFrame(int *_scanCounter, float *_scanDest,
                  const float *_scan,
                  unsigned int _width, unsigned int _height,
                  unsigned int _depth,
                  const std::string &/*_format*/)
{
 // memcpy(_scanDest, _scan, _width * _height * _depth);
  *_scanCounter += 1;
}


class GpuRaysTest: public testing::Test,
                  public testing::WithParamInterface<const char *>
{
  // Test and verify gpu rays properties setters and getters
  public: void Configure(const std::string &_renderEngine);

  // Test boxes detection
  public: void RaysUnitBox(const std::string &_renderEngine);
};

/////////////////////////////////////////////////
void GpuRaysTest::Configure(const std::string &_renderEngine)
{
  // create and populate scene
  RenderEngine *engine = rendering::engine(_renderEngine);
  if (!engine)
  {
    igndbg << "Engine '" << _renderEngine
              << "' is not supported" << std::endl;
    return;
  }

  ScenePtr scene = engine->CreateScene("scene");
  ASSERT_TRUE(scene != nullptr);

  VisualPtr root = scene->RootVisual();

  GpuRaysPtr gpuRays = scene->CreateGpuRays();
  ASSERT_TRUE(gpuRays != nullptr);
  root->AddChild(gpuRays);

  // set gpu rays caster initial pose
  math::Vector3d initPos(-2, 0.0, 5.0);
  math::Quaterniond initRot = math::Quaterniond::Identity;
  gpuRays->SetWorldPosition(initPos);
  EXPECT_EQ(initPos, gpuRays->WorldPosition());
  EXPECT_EQ(initRot, gpuRays->WorldRotation());

  // The following tests all the getters and setters
  {
    gpuRays->SetNearClipPlane(0.1);
    EXPECT_NEAR(gpuRays->NearClipPlane(), 0.1, 1e-6);

    gpuRays->SetFarClipPlane(100.0);
    EXPECT_NEAR(gpuRays->FarClipPlane(), 100, 1e-6);

    gpuRays->SetHorzHalfAngle(1.2);
    EXPECT_NEAR(gpuRays->HorzHalfAngle(), 1.2, 1e-6);

    gpuRays->SetVertHalfAngle(0.5);
    EXPECT_NEAR(gpuRays->VertHalfAngle(), 0.5, 1e-6);

    gpuRays->SetIsHorizontal(false);
    EXPECT_FALSE(gpuRays->IsHorizontal());

    gpuRays->SetHorzFOV(2.4);
    EXPECT_NEAR(gpuRays->HorzFOV(), 2.4, 1e-6);

    gpuRays->SetVertFOV(1.0);
    EXPECT_NEAR(gpuRays->VertFOV(), 1.0, 1e-6);

    gpuRays->SetCosHorzFOV(0.2);
    EXPECT_NEAR(gpuRays->CosHorzFOV(), 0.2, 1e-6);

    gpuRays->SetCosVertFOV(0.1);
    EXPECT_NEAR(gpuRays->CosVertFOV(), 0.1, 1e-6);

    gpuRays->SetRayCountRatio(0.344);
    EXPECT_NEAR(gpuRays->RayCountRatio(), 0.344, 1e-6);

    gpuRays->SetCameraCount(4u);
    EXPECT_EQ(gpuRays->CameraCount(), 4u);
  }

  // Clean up
  engine->DestroyScene(scene);
}


/////////////////////////////////////////////////
void GpuRaysTest::RaysUnitBox(const std::string &_renderEngine)
{
  const double hMinAngle = -M_PI/2.0;
  const double hMaxAngle = M_PI/2.0;
  const double minRange = 0.1;
  const double maxRange = 5.0;
  const int hRangeCount = 320;
  const int vRangeCount = 320;
  const double rangeResolution = 0.02;
  const unsigned int samples = 320;

  common::Time waitTime = common::Time(0.01);

  // create and populate scene
  RenderEngine *engine = rendering::engine(_renderEngine);
  if (!engine)
  {
    igndbg << "Engine '" << _renderEngine
              << "' is not supported" << std::endl;
    return;
  }

  ScenePtr scene = engine->CreateScene("scene");
  ASSERT_TRUE(scene != nullptr);

  VisualPtr root = scene->RootVisual();

  // Create first ray caster
  ignition::math::Pose3d testPose(ignition::math::Vector3d(0, 0, 0.1),
      ignition::math::Quaterniond::Identity);

  GpuRaysPtr gpuRays = scene->CreateGpuRays();
  root->AddChild(gpuRays);

  gpuRays->SetWorldPosition(testPose.Pos());
  gpuRays->SetWorldRotation(testPose.Rot());
  gpuRays->SetNearClipPlane(minRange);
  gpuRays->SetFarClipPlane(maxRange);
  gpuRays->SetRangeCount(hRangeCount, vRangeCount);
  gpuRays->SetHorzFOV(hMaxAngle - hMinAngle);

  // Create a second ray caster rotated
  ignition::math::Pose3d testPose2(ignition::math::Vector3d(0, 0, 0.1),
      ignition::math::Quaterniond(M_PI/2.0, 0, 0));

  GpuRaysPtr gpuRays2 = scene->CreateGpuRays();
  root->AddChild(gpuRays2);

  gpuRays2->SetWorldPosition(testPose2.Pos());
  gpuRays2->SetWorldRotation(testPose2.Rot());
  gpuRays2->SetNearClipPlane(minRange);
  gpuRays2->SetFarClipPlane(maxRange);
  gpuRays2->SetHorzFOV(hMaxAngle - hMinAngle);

  // Create testing boxes
  // box in the center
  ignition::math::Pose3d box01Pose(ignition::math::Vector3d(1, 0, 0.5),
                                   ignition::math::Quaterniond::Identity);
  VisualPtr visualBox1 = scene->CreateVisual();
  visualBox1->AddGeometry(scene->CreateBox());
  visualBox1->SetWorldPosition(box01Pose.Pos());
  visualBox1->SetWorldRotation(box01Pose.Rot());
  root->AddChild(visualBox1);

  // box on the right of ray sensor 1
  ignition::math::Pose3d box02Pose(ignition::math::Vector3d(0, -1, 0.5),
                                   ignition::math::Quaterniond::Identity);
  VisualPtr visualBox2 = scene->CreateVisual();
  visualBox2->AddGeometry(scene->CreateBox());
  visualBox2->SetWorldPosition(box02Pose.Pos());
  visualBox2->SetWorldRotation(box02Pose.Rot());
  root->AddChild(visualBox2);

  // box on the left of the ray sensor 1 but out of range
  ignition::math::Pose3d box03Pose(
      ignition::math::Vector3d(0, maxRange + 1, 0.5),
      ignition::math::Quaterniond::Identity);
  VisualPtr visualBox3 = scene->CreateVisual();
  visualBox3->AddGeometry(scene->CreateBox());
  visualBox3->SetWorldPosition(box03Pose.Pos());
  visualBox3->SetWorldRotation(box03Pose.Rot());
  root->AddChild(visualBox3);

  // Verify ray sensor 1 range readings
  // listen to new laser frames
  // float *scan = new float[gpuRays->RayCount()
  //     * gpuRays->VerticalRayCount() * 3];
  float *scan = new float[3];
  int scanCount = 0;
  common::ConnectionPtr c =
    gpuRays->ConnectNewLaserFrame(
        std::bind(&::OnNewLaserFrame, &scanCount, scan,
          std::placeholders::_1, std::placeholders::_2, std::placeholders::_3,
          std::placeholders::_4, std::placeholders::_5));

  // wait for a few laser scans
  int i = 0;
  while (scanCount < 10 && i < 300)
  {
    common::Time::Sleep(waitTime);
    gpuRays->Update();
    i++;
  }
  EXPECT_LT(i, 300);

  int mid = samples / 2;
  double unitBoxSize = 1.0;
  double expectedRangeAtMidPoint = box01Pose.Pos().X() - unitBoxSize/2;

  // // ray sensor 1 should see box01 and box02
  // EXPECT_NEAR(raySensor->Range(mid), expectedRangeAtMidPoint, LASER_TOL);
  // EXPECT_NEAR(raySensor->Range(0), expectedRangeAtMidPoint, LASER_TOL);
  // EXPECT_DOUBLE_EQ(raySensor->Range(samples-1), ignition::math::INF_D);

  // Verify ray sensor 2 range readings
  // listen to new laser frames
  // float *scan2 = new float[raySensor2->RayCount()
  //     * raySensor2->VerticalRayCount() * 3];
  float *scan2 = new float[3];
  int scanCount2 = 0;
  common::ConnectionPtr c2 =
    gpuRays2->ConnectNewLaserFrame(
        std::bind(&::OnNewLaserFrame, &scanCount2, scan2,
          std::placeholders::_1, std::placeholders::_2, std::placeholders::_3,
          std::placeholders::_4, std::placeholders::_5));

  // wait for a few laser scans
  i = 0;
  scanCount2 = 0;
  while (scanCount2 < 10 && i < 300)
  {
    common::Time::Sleep(waitTime);
    gpuRays2->Update();
    i++;
  }
  EXPECT_LT(i, 300);

  // // Only box01 should be visible to ray sensor 2
  // EXPECT_NEAR(raySensor2->Range(mid), expectedRangeAtMidPoint, LASER_TOL);
  // EXPECT_DOUBLE_EQ(raySensor2->Range(0), ignition::math::INF_D);
  // EXPECT_DOUBLE_EQ(raySensor->Range(samples-1), ignition::math::INF_D);

  // // Move all boxes out of range
  // world->ModelByName(box01)->SetWorldPose(ignition::math::Pose3d(
  //     ignition::math::Vector3d(maxRange + 1, 0, 0),
  //     ignition::math::Quaterniond::Identity));
  // world->ModelByName(box02)->SetWorldPose(ignition::math::Pose3d(
  //     ignition::math::Vector3d(0, -(maxRange + 1), 0),
  //     ignition::math::Quaterniond::Identity));

  // wait for a few more laser scans
  i = 0;
  scanCount = 0;
  scanCount2 = 0;
  while ((scanCount < 10 ||scanCount2 < 10) && i < 300)
  {
    common::Time::Sleep(waitTime);
    gpuRays->Update();
    gpuRays2->Update();
    i++;
  }
  EXPECT_LT(i, 300);

  // for (int i = 0; i < raySensor->RayCount(); ++i)
  //   EXPECT_DOUBLE_EQ(raySensor->Range(i), ignition::math::INF_D);
  //
  // for (int i = 0; i < raySensor->RayCount(); ++i)
  //   EXPECT_DOUBLE_EQ(raySensor2->Range(i), ignition::math::INF_D);

  c.reset();
  c2.reset();

  delete [] scan;
  delete [] scan2;

  // Clean up
  engine->DestroyScene(scene);
}

/////////////////////////////////////////////////
TEST_P(GpuRaysTest, Configure)
{
  Configure(GetParam());
}

/////////////////////////////////////////////////
TEST_P(GpuRaysTest, RaysUnitBox)
{
  RaysUnitBox(GetParam());
}


INSTANTIATE_TEST_CASE_P(GpuRays, GpuRaysTest,
    ::testing::Values("ogre"),
    ignition::rendering::PrintToStringParam());

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
