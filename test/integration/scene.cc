/*
 * Copyright (C) 2018 Open Source Robotics Foundation
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
  if (_renderEngine == "optix")
  {
    igndbg << "RayQuery not supported yet in rendering engine: "
            << _renderEngine << std::endl;
    return;
  }

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
  box->SetLocalRotation(IGN_PI / 4, 0, IGN_PI / 3);
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
  camera->SetHFOV(IGN_PI / 2);
  root->AddChild(camera);

  // render a frame
  camera->Update();

  // test get sphere object
  ignition::math::Vector2i spherePosition(220, 307);
  VisualPtr sphere_visual = scene->VisualAt(camera, spherePosition);
  ASSERT_TRUE(sphere_visual != nullptr);
  EXPECT_EQ("sphere", sphere_visual->Name());

  // test get box object
  ignition::math::Vector2i boxPosition(452, 338);
  VisualPtr box_visual = scene->VisualAt(camera, boxPosition);
  ASSERT_TRUE(box_visual != nullptr);
  EXPECT_EQ("box", box_visual->Name());

  // test get no object
  ignition::math::Vector2i emptyPosition(300, 150);
  VisualPtr empty_visual = scene->VisualAt(camera, emptyPosition);
  ASSERT_TRUE(empty_visual == nullptr);

  // Clean up
  engine->DestroyScene(scene);
  rendering::unloadEngine(engine->Name());
}

/////////////////////////////////////////////////
TEST_P(SceneTest, VisualAt)
{
  VisualAt(GetParam());
}

// It doesn't suppot optix just yet
INSTANTIATE_TEST_CASE_P(Scene, SceneTest,
    RENDER_ENGINE_VALUES,
    ignition::rendering::PrintToStringParam());

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
