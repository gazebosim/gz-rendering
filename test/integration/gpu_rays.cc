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

class GpuRaysTest: public testing::Test,
                  public testing::WithParamInterface<const char *>
{
  // Test and verify gpu rays properties setters and getters
  public: void Configure(const std::string &_renderEngine);
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

  VisualPtr visual = scene->CreateVisual();
  visual->AddGeometry(scene->CreateBox());
  visual->SetWorldPosition(0.0, 0.0, 0.0);
  visual->SetWorldRotation(0.0, 0.0, 3.14);
  root->AddChild(visual);

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
TEST_P(GpuRaysTest, Configure)
{
  Configure(GetParam());
}

INSTANTIATE_TEST_CASE_P(GpuRays, GpuRaysTest,
    ::testing::Values("ogre"),
    ignition::rendering::PrintToStringParam());

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
