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
#include "ignition/rendering/RenderTarget.hh"
#include "ignition/rendering/Scene.hh"

using namespace ignition;
using namespace rendering;

class RenderTargetTest : public testing::Test,
                         public testing::WithParamInterface<const char*>
{
  /// \brief test RenderTexture properties
  public: void RenderTexture(const std::string &_renderEngine);

  /// \brief test RenderWindow properties
  public: void RenderWindow(const std::string &_renderEngine);
};

/////////////////////////////////////////////////
void RenderTargetTest::RenderTexture(const std::string &_renderEngine)
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

  RenderTexturePtr renderTexture = scene->CreateRenderTexture();

  // default properties
  EXPECT_EQ(scene->BackgroundColor(), renderTexture->BackgroundColor());
  EXPECT_EQ(0u, renderTexture->GLId());

  // test basic properties
  renderTexture->SetFormat(PF_R8G8B8);
  renderTexture->SetWidth(800u);
  renderTexture->SetHeight(600u);

  EXPECT_EQ(PF_R8G8B8, renderTexture->Format());
  EXPECT_EQ(800u, renderTexture->Width());
  EXPECT_EQ(600u, renderTexture->Height());

  // Clean up
  engine->DestroyScene(scene);
  rendering::unloadEngine(engine->Name());
}

/////////////////////////////////////////////////
void RenderTargetTest::RenderWindow(const std::string &_renderEngine)
{
  if (_renderEngine != "ogre")
  {
    igndbg << "RenderWindow not supported yet in rendering engine: "
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
  CameraPtr camera =  scene->CreateCamera("camera");

  RenderWindowPtr renderWindow = camera->CreateRenderWindow();

  renderWindow->SetWidth(320);
  renderWindow->SetHeight(240);
  renderWindow->SetHandle("windowId");
  renderWindow->SetDevicePixelRatio(1.0);

  EXPECT_EQ(320u, renderWindow->Width());
  EXPECT_EQ(240u, renderWindow->Height());
  EXPECT_EQ("windowId", renderWindow->Handle());
  EXPECT_DOUBLE_EQ(1.0, renderWindow->DevicePixelRatio());
  EXPECT_EQ(math::Color::Black, renderWindow->BackgroundColor());

  // change render window properties and verify
  renderWindow->SetWidth(640);
  renderWindow->SetHeight(480);
  scene->SetBackgroundColor(math::Color::Red);

  EXPECT_EQ(640u, renderWindow->Width());
  EXPECT_EQ(480u, renderWindow->Height());
  EXPECT_EQ(math::Color::Red, renderWindow->BackgroundColor());

  // Clean up
  engine->DestroyScene(scene);
  rendering::unloadEngine(engine->Name());
}

/////////////////////////////////////////////////
TEST_P(RenderTargetTest, RenderTexture)
{
  RenderTexture(GetParam());
}

/////////////////////////////////////////////////
TEST_P(RenderTargetTest, RenderWindow)
{
  RenderWindow(GetParam());
}

INSTANTIATE_TEST_CASE_P(RenderTarget, RenderTargetTest,
    RENDER_ENGINE_VALUES,
    ignition::rendering::PrintToStringParam());

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
