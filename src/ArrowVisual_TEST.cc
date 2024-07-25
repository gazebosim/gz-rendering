/*
 * Copyright (C) 2022 Open Source Robotics Foundation
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
#include <string>

#include <ignition/common/Console.hh>

#include "test_config.h"  // NOLINT(build/include)

#include "gz/rendering/ArrowVisual.hh"
#include "gz/rendering/RenderEngine.hh"
#include "gz/rendering/RenderingIface.hh"
#include "gz/rendering/Scene.hh"

using namespace ignition;
using namespace rendering;

class ArrowVisualTest : public testing::Test,
                        public testing::WithParamInterface<const char *>
{
  /// \brief Test basic API
  public: void ArrowVisual(const std::string &_renderEngine);

  /// \brief Test gizmo material
  public: void Material(const std::string &_renderEngine);
};

/////////////////////////////////////////////////
void ArrowVisualTest::ArrowVisual(const std::string &_renderEngine)
{
#ifdef __APPLE__                                                                                                                                                                              
  std::cerr << "Skipping test for apple, see issue #35." << std::endl;
  return;
#endif

  RenderEngine *engine = rendering::engine(_renderEngine);
  if (!engine)
  {
    igndbg << "Engine '" << _renderEngine
           << "' is not supported" << std::endl;
    return;
  }

  ScenePtr scene = engine->CreateScene("scene");

  // create visual
  ArrowVisualPtr arrow = scene->CreateArrowVisual();
  EXPECT_NE(nullptr, arrow);
  EXPECT_EQ(2u, arrow->ChildCount());

  EXPECT_NE(nullptr, arrow->Head());
  EXPECT_NE(nullptr, arrow->Shaft());

  EXPECT_NE(arrow->Head(), arrow->Shaft());

  ArrowVisualPtr arrow_name = scene->CreateArrowVisual("arrow_name");
  EXPECT_NE(nullptr, arrow_name);
  EXPECT_EQ(2u, arrow_name->ChildCount());

  ArrowVisualPtr arrow_id = scene->CreateArrowVisual(98);
  EXPECT_NE(nullptr, arrow_id);
  EXPECT_EQ(2u, arrow_id->ChildCount());

  // Clean up
  engine->DestroyScene(scene);
  rendering::unloadEngine(engine->Name());
}

/////////////////////////////////////////////////
TEST_P(ArrowVisualTest, ArrowVisual)
{
  ArrowVisual(GetParam());
}

INSTANTIATE_TEST_CASE_P(Visual, ArrowVisualTest,
    RENDER_ENGINE_VALUES,
    ignition::rendering::PrintToStringParam());

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
