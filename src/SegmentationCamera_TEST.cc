/*
 * Copyright (C) 2021 Open Source Robotics Foundation
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

#include <gz/common/Console.hh>

#include "test_config.hh"  // NOLINT(build/include)
#include "gz/rendering/SegmentationCamera.hh"
#include "gz/rendering/RenderEngine.hh"
#include "gz/rendering/RenderingIface.hh"
#include "gz/rendering/Scene.hh"

using namespace gz;
using namespace rendering;

class SegmentationCameraTest : public testing::Test,
                          public testing::WithParamInterface<const char *>
{
  /// \brief Test basic api
  public: void SegmentationCamera(const std::string &_renderEngine);
};

/////////////////////////////////////////////////
void SegmentationCameraTest::SegmentationCamera(
  const std::string &_renderEngine)
{
  // Currently, only ogre2 supports segmentation cameras
  if (_renderEngine.compare("ogre2") != 0)
  {
    gzerr << "Engine '" << _renderEngine
              << "' doesn't support segmentation cameras" << std::endl;
    return;
  }

  // create and populate scene
  RenderEngine *engine = rendering::engine(_renderEngine);
  if (!engine)
  {
    gzerr << "Engine '" << _renderEngine
              << "' was unable to be retrieved" << std::endl;
    return;
  }
  ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);

  SegmentationCameraPtr camera(scene->CreateSegmentationCamera());
  ASSERT_NE(nullptr, camera);

  math::Color backgroundColor(0.5, 0.5, 0.5);
  camera->SetBackgroundColor(backgroundColor);
  EXPECT_EQ(camera->BackgroundColor(), backgroundColor);

  int backgroundLabel = 23;
  camera->SetBackgroundLabel(backgroundLabel);
  EXPECT_EQ(camera->BackgroundLabel(), backgroundLabel);

  camera->SetSegmentationType(SegmentationType::ST_SEMANTIC);
  EXPECT_EQ(camera->Type(), SegmentationType::ST_SEMANTIC);

  camera->EnableColoredMap(true);
  EXPECT_TRUE(camera->IsColoredMap());

  // Clean up
  engine->DestroyScene(scene);
  rendering::unloadEngine(engine->Name());
}

/////////////////////////////////////////////////
TEST_P(SegmentationCameraTest, SegmentationCamera)
{
  SegmentationCamera(GetParam());
}

INSTANTIATE_TEST_SUITE_P(SegmentationCamera, SegmentationCameraTest,
    RENDER_ENGINE_VALUES,
    gz::rendering::PrintToStringParam());
