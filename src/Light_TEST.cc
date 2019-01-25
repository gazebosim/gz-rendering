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
#include "ignition/rendering/Light.hh"
#include "ignition/rendering/RenderEngine.hh"
#include "ignition/rendering/RenderingIface.hh"
#include "ignition/rendering/Scene.hh"

using namespace ignition;
using namespace rendering;

class LightTest : public testing::Test,
                  public testing::WithParamInterface<const char*>
{
  /// \brief Test light APIs
  public: void Light(const std::string &_renderEngine);
};


/////////////////////////////////////////////////
void LightTest::Light(const std::string &_renderEngine)
{
  // create and populate scene
  ignerr << "Before rendering::engine" << std::endl;
  RenderEngine *engine = rendering::engine(_renderEngine);
  if (!engine)
  {
    igndbg << "Engine '" << _renderEngine
           << "' is not supported" << std::endl;
    return;
  }
  
  ignerr << "Before CreateScene" << std::endl;
  ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);

  // Point light
  ignerr << "Before CreatePointLight" << std::endl;
  LightPtr light = scene->CreatePointLight();
  EXPECT_TRUE(light != nullptr);
  EXPECT_EQ(1u, scene->LightCount());
  EXPECT_TRUE(scene->HasLight(light));
  // Clean up
  engine->DestroyScene(scene);
  ignerr << "After DestroyScene" << std::endl;
  rendering::unloadEngine(engine->Name());
  ignerr << "After unloadEngine" << std::endl;
}

/////////////////////////////////////////////////
TEST_P(LightTest, Light)
{
  Light(GetParam());
}

INSTANTIATE_TEST_CASE_P(Light, LightTest,
    RENDER_ENGINE_VALUES,
    ignition::rendering::PrintToStringParam());

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
