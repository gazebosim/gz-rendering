/* * Copyright (C) 2021 Open Source Robotics Foundation
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

#include "ignition/rendering/Camera.hh"
#include "ignition/rendering/RayQuery.hh"
#include "ignition/rendering/RenderEngine.hh"
#include "ignition/rendering/RenderingIface.hh"
#include "ignition/rendering/Scene.hh"
#include "ignition/rendering/Utils.hh"
#include "ignition/rendering/Visual.hh"

#include "test_config.h"  // NOLINT(build/include)

using namespace ignition;
using namespace rendering;

class UtilTest : public testing::Test,
                 public testing::WithParamInterface<const char *>
{
  // Documentation inherited
  public: void SetUp() override
  {
    ignition::common::Console::SetVerbosity(4);
  }

  public: void ClickToScene(const std::string &_renderEngine);
};

void UtilTest::ClickToScene(const std::string &_renderEngine)
{
  RenderEngine *engine = rendering::engine(_renderEngine);
  if (!engine)
  {
    igndbg << "Engine '" << _renderEngine
           << "' is not supported" << std::endl;
    return;
  }
  ScenePtr scene = engine->CreateScene("scene");

  CameraPtr camera(scene->CreateCamera());
  EXPECT_TRUE(camera != nullptr);

  camera->SetLocalPosition(0.0, 0.0, 15);
  camera->SetLocalRotation(0.0, 1.57, 0.0);

  unsigned int width = 640u;
  unsigned int height = 480u;
  camera->SetImageWidth(width);
  camera->SetImageHeight(height);

  RayQueryPtr rayQuery = scene->CreateRayQuery();
  EXPECT_TRUE(rayQuery != nullptr);

  // ScreenToPlane
  math::Vector3d result = ScreenToPlane(
    math::Vector2i(static_cast<int>(width / 2), static_cast<int>(height / 2)),
    camera, rayQuery);

  EXPECT_NEAR(0.0, result.Z(), 0.01);
  EXPECT_NEAR(0.0, result.X(), 0.1);
  EXPECT_NEAR(0.0, result.Y(), 0.01);

  // ScreenToScene
  RayQueryResult rayResult;
  result = ScreenToScene(
    math::Vector2i(static_cast<int>(width / 2), static_cast<int>(height / 2)),
    camera, rayQuery, rayResult);

  VisualPtr root = scene->RootVisual();

  // create box visual to collide with the ray
  VisualPtr box = scene->CreateVisual();
  box->AddGeometry(scene->CreateBox());
  box->SetOrigin(0.0, 0.0, 0.0);
  box->SetLocalPosition(0.0, 0.0, 0.0);
  box->SetLocalRotation(0.0, 0.0, 0.0);
  box->SetLocalScale(1.0, 1.0, 1.0);
  root->AddChild(box);

  // The default limit of the function is 10 meters away
  EXPECT_NEAR(5.0, result.Z(), 0.01);
  EXPECT_NEAR(0.0, result.X(), 0.1);
  EXPECT_NEAR(0.0, result.Y(), 0.01);

  result = ScreenToScene(
    math::Vector2i(static_cast<int>(width / 2), static_cast<int>(height / 2)),
    camera, rayQuery, rayResult, 20.0);

  EXPECT_NEAR(0.5, result.Z(), 0.01);
  EXPECT_NEAR(0.0, result.X(), 0.1);
  EXPECT_NEAR(0.0, result.Y(), 0.01);

  camera->SetLocalPosition(0.0, 0.0, 7.0);
  camera->SetLocalRotation(0.0, 1.57, 0.0);

  // The default limit of the function is 10 meters away
  result = ScreenToScene(
    math::Vector2i(static_cast<int>(width / 2), static_cast<int>(height / 2)),
    camera, rayQuery, rayResult);

  EXPECT_NEAR(0.5, result.Z(), 0.01);
  EXPECT_NEAR(0.0, result.X(), 0.1);
  EXPECT_NEAR(0.0, result.Y(), 0.01);
}

/////////////////////////////////////////////////
TEST_P(UtilTest, ClickToScene)
{
  ClickToScene(GetParam());
}

INSTANTIATE_TEST_CASE_P(ClickToScene, UtilTest,
    RENDER_ENGINE_VALUES,
    ignition::rendering::PrintToStringParam());

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
