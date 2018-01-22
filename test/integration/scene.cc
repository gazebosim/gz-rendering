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

#include "ignition/rendering/Camera.hh"
#include "ignition/rendering/RenderEngine.hh"
#include "ignition/rendering/RenderingIface.hh"
#include "ignition/rendering/Scene.hh"

using namespace ignition;
using namespace rendering;

class SceneTest: public testing::Test,
                  public testing::WithParamInterface<const char *>
{
  // Test and verify camera tracking
  public: void VisualAt(const std::string &_renderEngine);
};

/////////////////////////////////////////////////
void SceneTest::VisualAt(const std::string &_renderEngine)
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

  // create box visual
  VisualPtr box = scene->CreateVisual("box");
  ASSERT_TRUE(box != nullptr);
  box->AddGeometry(scene->CreateBox());
  box->SetOrigin(0.0, 0.5, 0.0);
  box->SetLocalPosition(3, 0, 0);
  box->SetLocalRotation(M_PI / 4, 0, M_PI / 3);
  box->SetLocalScale(1, 2.5, 1);
  root->AddChild(box);

  // create sphere visual
  VisualPtr sphere = scene->CreateVisual("sphere");
  ASSERT_TRUE(sphere != nullptr);
  sphere->AddGeometry(scene->CreateSphere());
  sphere->SetOrigin(0.0, -0.5, 0.0);
  sphere->SetLocalPosition(3, 0, 0);
  sphere->SetLocalRotation(0, 0, 0);
  sphere->SetLocalScale(1, 2.5, 1);
  root->AddChild(sphere);

  // create camera
  CameraPtr camera = scene->CreateCamera("camera");
  ASSERT_TRUE(camera != nullptr);
  camera->SetLocalPosition(0.0, 0.0, 0.0);
  camera->SetLocalRotation(0.0, 0.0, 0.0);
  camera->SetImageWidth(800);
  camera->SetImageHeight(600);
  camera->SetAntiAliasing(2);
  camera->SetAspectRatio(1.333);
  camera->SetHFOV(M_PI / 2);
  root->AddChild(camera);

  // render a frame
  camera->Update();

  // test get sphere object
  double sphere_x = 220  *2.0 / static_cast<double>(camera->ImageWidth()) - 1.0;
  double sphere_y = 1.0 - 2.0 * 307 / static_cast<double>(
      camera->ImageHeight());
  ignition::math::Vector2d sphere_position(sphere_x, sphere_y);
  VisualPtr sphere_visual = scene->VisualAt(camera, sphere_position);
  ASSERT_TRUE(sphere_visual != nullptr);
  EXPECT_EQ("sphere", sphere_visual->Name());

  // test get box object
  double box_x = 452 *2.0 / static_cast<double>(camera->ImageWidth()) - 1.0;
  double box_y = 1.0 - 2.0 * 338 / static_cast<double>(camera->ImageHeight());
  ignition::math::Vector2d box_position(box_x, box_y);
  VisualPtr box_visual = scene->VisualAt(camera, box_position);
  ASSERT_TRUE(box_visual != nullptr);
  EXPECT_EQ("box", box_visual->Name());

  // test get no object
  double empty_x = 300 *2.0 / static_cast<double>(camera->ImageWidth()) - 1.0;
  double empty_y = 1.0 - 2.0 * 150 / static_cast<double>(camera->ImageHeight());
  ignition::math::Vector2d empty_position(empty_x, empty_y);
  VisualPtr empty_visual = scene->VisualAt(camera, empty_position);
  ASSERT_TRUE(empty_visual == nullptr);

  // Clean up
  engine->DestroyScene(scene);
}

/////////////////////////////////////////////////
TEST_P(SceneTest, VisualAt)
{
  VisualAt(GetParam());
}

// It doesn't suppot optix just yet
INSTANTIATE_TEST_CASE_P(Scene, SceneTest,
    ::testing::Values("ogre"));

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
