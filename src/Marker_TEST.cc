/* * Copyright (C) 2019 Open Source Robotics Foundation
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
#include "ignition/rendering/RenderingIface.hh"
#include "ignition/rendering/Marker.hh"
#include "ignition/rendering/Scene.hh"

using namespace ignition;
using namespace rendering;
using namespace std::chrono_literals;

class MarkerTest : public testing::Test,
                   public testing::WithParamInterface<const char *>
{
  public: void Marker(const std::string &_renderEngine);
};

/////////////////////////////////////////////////
void MarkerTest::Marker(const std::string &_renderEngine)
{
  if (_renderEngine == "optix")
  {
    igndbg << "Marker not supported yet in rendering engine: "
            << _renderEngine << std::endl;
    return;
  }


  RenderEngine *engine = rendering::engine(_renderEngine);
  if (!engine)
  {
    igndbg << "Engine '" << _renderEngine
           << "' is not supported" << std::endl;
    return;
  }

  ScenePtr scene = engine->CreateScene("scene");

  MarkerPtr marker = scene->CreateMarker();
  ASSERT_NE(nullptr, marker);

  // initial values
  EXPECT_EQ(std::chrono::steady_clock::duration::zero().count(),
      marker->Lifetime().count());
  EXPECT_EQ(0, marker->Layer());
  EXPECT_EQ(MarkerType::MT_NONE, marker->Type());

  // lifetime
  std::chrono::steady_clock::duration duration{2ms};
  marker->SetLifetime(duration);
  EXPECT_EQ(duration.count(), marker->Lifetime().count());

  // layer
  marker->SetLayer(1);
  EXPECT_EQ(1, marker->Layer());
  marker->SetLayer(-2);
  EXPECT_EQ(-2, marker->Layer());

  // type
  marker->SetType(MarkerType::MT_CYLINDER);
  EXPECT_EQ(MarkerType::MT_CYLINDER, marker->Type());
  marker->SetType(MarkerType::MT_NONE);
  EXPECT_EQ(MarkerType::MT_NONE, marker->Type());

  // Clean up
  engine->DestroyScene(scene);
  rendering::unloadEngine(engine->Name());
}

/////////////////////////////////////////////////
TEST_P(MarkerTest, Marker)
{
  Marker(GetParam());
}

INSTANTIATE_TEST_CASE_P(Marker, MarkerTest,
    RENDER_ENGINE_VALUES,
    ignition::rendering::PrintToStringParam());

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
