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

#include "ignition/rendering/Camera.hh"
#include "ignition/rendering/RenderEngine.hh"
#include "ignition/rendering/RenderingIface.hh"
#include "ignition/rendering/RenderTarget.hh"
#include "ignition/rendering/Scene.hh"

using namespace ignition;
using namespace rendering;

class RenderTargetTest : public testing::Test,
                         public testing::WithParamInterface<const char*>
{
  public: void RenderWindow(const std::string &_renderEngine);
};

/////////////////////////////////////////////////
void RenderTargetTest::RenderWindow(const std::string &_renderEngine)
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
  CameraPtr camera =  scene->CreateCamera("camera");

  RenderWindowPtr renderWindow = camera->CreateRenderWindow();

  renderWindow->SetWidth(320);
  renderWindow->SetHeight(240);
  renderWindow->SetHandle("windowId");
  renderWindow->SetDevicePixelRatio(1.0);

  EXPECT_EQ(320u, renderWindow->Width());
  EXPECT_EQ(240u, renderWindow->Height());
  EXPECT_EQ("windowId", renderWindow->Handle());
  EXPECT_EQ(1.0, renderWindow->DevicePixelRatio());
  EXPECT_EQ(math::Color::Black, renderWindow->BackgroundColor());

  // change render window properties and verify
  renderWindow->SetWidth(640);
  renderWindow->SetHeight(480);
  scene->SetBackgroundColor(math::Color::Red);

  EXPECT_EQ(640u, renderWindow->Width());
  EXPECT_EQ(480u, renderWindow->Height());
  EXPECT_EQ(math::Color::Red, renderWindow->BackgroundColor());
}

/////////////////////////////////////////////////
TEST_P(RenderTargetTest, RenderWindow)
{
  RenderWindow(GetParam());
}

INSTANTIATE_TEST_CASE_P(RenderTarget, RenderTargetTest,
    ::testing::Values("ogre", "optix"));

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
