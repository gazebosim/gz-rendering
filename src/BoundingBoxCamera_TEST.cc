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

#include <ignition/common/Console.hh>

#include "test_config.h"  // NOLINT(build/include)
#include "ignition/rendering/BoundingBoxCamera.hh"
#include "ignition/rendering/RenderEngine.hh"
#include "ignition/rendering/RenderingIface.hh"
#include "ignition/rendering/Scene.hh"

using namespace ignition;
using namespace rendering;

class BoundingBoxCameraTest : public testing::Test,
                          public testing::WithParamInterface<const char *>
{
  /// \brief Test basic api
  public: void BoundingBoxCamera(const std::string &_renderEngine);
};

/////////////////////////////////////////////////
void BoundingBoxCameraTest::BoundingBoxCamera(const std::string &_renderEngine)
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
  ASSERT_NE(nullptr, scene);

  BoundingBoxCameraPtr camera(scene->CreateBoundingBoxCamera());
  EXPECT_TRUE(camera != nullptr);

  // Test Draw Bounding Box
  int width = 200;
  int height = 200;
  int channels = 3;

  unsigned char *data = new unsigned char[width * height * channels];

  BoundingBox box;
  box.minX = 100;
  box.minY = 100;
  box.maxX = 150;
  box.maxY = 150;

  camera->SetImageWidth(width);
  camera->SetImageHeight(height);
  camera->DrawBoundingBox(data, box);

  // test the boundaries (min & max box's points) color
  int minIndex = (box.minY * width + box.minX) * channels;
  int maxIndex = (box.maxY * width + (box.maxX-1)) * channels;

  // color is green (middle value = 255)
  EXPECT_EQ(data[minIndex + 1], 255);
  EXPECT_EQ(data[maxIndex + 1], 255);

  // Test Bounding Box Type
  EXPECT_EQ(camera->Type(), BoundingBoxType::VisibleBox);
  camera->SetBoundingBoxType(BoundingBoxType::FullBox);
  EXPECT_EQ(camera->Type(), BoundingBoxType::FullBox);

  // Clean up
  // engine->DestroyScene(scene);
  // rendering::unloadEngine(engine->Name());
}

/////////////////////////////////////////////////
TEST_P(BoundingBoxCameraTest, BoundingBoxCamera)
{
  BoundingBoxCamera(GetParam());
}

INSTANTIATE_TEST_CASE_P(BoundingBoxCamera, BoundingBoxCameraTest,
    RENDER_ENGINE_VALUES,
    ignition::rendering::PrintToStringParam());

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}