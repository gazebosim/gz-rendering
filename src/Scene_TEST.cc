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
#include "ignition/rendering/RenderEngine.hh"
#include "ignition/rendering/RenderTarget.hh"
#include "ignition/rendering/RenderingIface.hh"
#include "ignition/rendering/Scene.hh"

using namespace ignition;
using namespace rendering;

class SceneTest : public testing::Test,
                  public testing::WithParamInterface<const char *>
{
  public: void Scene(const std::string &_renderEngine);
  public: void Nodes(const std::string &_renderEngine);
};

/////////////////////////////////////////////////
void SceneTest::Scene(const std::string &_renderEngine)
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

  // TODO(anyone) background color and render window only supported
  // by ogre
  if (_renderEngine == "ogre")
  {
    EXPECT_FALSE(scene->IsGradientBackgroundColor());

    // Check background color
    EXPECT_EQ(math::Color::Black, scene->BackgroundColor());
    scene->SetBackgroundColor(0, 1, 0, 1);
    EXPECT_EQ(math::Color(0, 1, 0, 1), scene->BackgroundColor());
    math::Color red(1, 0, 0, 1);
    scene->SetBackgroundColor(red);
    EXPECT_EQ(red, scene->BackgroundColor());
    EXPECT_FALSE(scene->IsGradientBackgroundColor());

    // Check gradient background color
    std::array<math::Color, 4> gradientBackgroundColor =
        scene->GradientBackgroundColor();
    for (auto i = 0u; i < 4; ++i)
      EXPECT_EQ(math::Color::Black, gradientBackgroundColor[i]);
    gradientBackgroundColor[0] = math::Color::Red;
    gradientBackgroundColor[1] = math::Color::Green;
    gradientBackgroundColor[2] = math::Color::Blue;
    gradientBackgroundColor[3] = math::Color::Black;
    scene->SetGradientBackgroundColor(gradientBackgroundColor);
    EXPECT_TRUE(scene->IsGradientBackgroundColor());
    auto currentGradientBackgroundColor = scene->GradientBackgroundColor();
    EXPECT_EQ(math::Color::Red, currentGradientBackgroundColor[0]);
    EXPECT_EQ(math::Color::Green, currentGradientBackgroundColor[1]);
    EXPECT_EQ(math::Color::Blue, currentGradientBackgroundColor[2]);
    EXPECT_EQ(math::Color::Black, currentGradientBackgroundColor[3]);
    gradientBackgroundColor[0] = math::Color::White;
    scene->SetGradientBackgroundColor(gradientBackgroundColor);
    currentGradientBackgroundColor = scene->GradientBackgroundColor();
    EXPECT_EQ(math::Color::White, currentGradientBackgroundColor[0]);
    EXPECT_EQ(math::Color::Green, currentGradientBackgroundColor[1]);
    EXPECT_EQ(math::Color::Blue, currentGradientBackgroundColor[2]);
    EXPECT_EQ(math::Color::Black, currentGradientBackgroundColor[3]);
    scene->RemoveGradientBackgroundColor();
    EXPECT_FALSE(scene->IsGradientBackgroundColor());

    // test creating render window from scene
    RenderWindowPtr renderWindow = scene->CreateRenderWindow();
    EXPECT_NE(nullptr, renderWindow->Scene());
    EXPECT_EQ(scene, renderWindow->Scene());
  }

  // Clean up
  engine->DestroyScene(scene);
  rendering::unloadEngine(engine->Name());
}

/////////////////////////////////////////////////
void SceneTest::Nodes(const std::string &_renderEngine)
{
  auto engine = rendering::engine(_renderEngine);
  if (!engine)
  {
    igndbg << "Engine '" << _renderEngine << "' is not supported" << std::endl;
    return;
  }

  auto scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);

  auto root = scene->RootVisual();
  ASSERT_NE(nullptr, root);

  // No nodes
  EXPECT_EQ(0u, scene->NodeCount());

  // Box visual
  auto box = scene->CreateVisual();
  ASSERT_NE(nullptr, box);

  box->AddGeometry(scene->CreateBox());
  root->AddChild(box);

  // Has node
  EXPECT_EQ(1u, scene->NodeCount());
  EXPECT_TRUE(scene->HasNode(box));
  EXPECT_TRUE(scene->HasNodeId(box->Id()));
  EXPECT_TRUE(scene->HasNodeName(box->Name()));

  // Get node
  EXPECT_EQ(box, scene->NodeByIndex(0));
  EXPECT_EQ(box, scene->NodeById(box->Id()));
  EXPECT_EQ(box, scene->NodeByName(box->Name()));

  // Clean up
  engine->DestroyScene(scene);
  rendering::unloadEngine(engine->Name());
}

/////////////////////////////////////////////////
TEST_P(SceneTest, Scene)
{
  Scene(GetParam());
}

/////////////////////////////////////////////////
TEST_P(SceneTest, Nodes)
{
  Nodes(GetParam());
}

INSTANTIATE_TEST_CASE_P(Scene, SceneTest,
    RENDER_ENGINE_VALUES,
    ignition::rendering::PrintToStringParam());

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
