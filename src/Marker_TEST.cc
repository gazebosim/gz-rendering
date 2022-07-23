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
#include "ignition/rendering/Marker.hh"
#include "ignition/rendering/Material.hh"
#include "ignition/rendering/RenderEngine.hh"
#include "ignition/rendering/RenderingIface.hh"
#include "ignition/rendering/Scene.hh"

using namespace ignition;
using namespace rendering;
using namespace std::chrono_literals;

class MarkerTest : public testing::Test,
                   public testing::WithParamInterface<const char *>
{
  public: void Marker(const std::string &_renderEngine);

  /// \brief Test setting material
  public: void Material(const std::string &_renderEngine);
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
  marker->SetType(MarkerType::MT_CAPSULE);
  EXPECT_EQ(MarkerType::MT_CAPSULE, marker->Type());

  // test attaching marker to visual
  VisualPtr visual = scene->CreateVisual();
  ASSERT_NE(nullptr, visual);
  visual->AddGeometry(marker);

  marker->SetType(MarkerType::MT_CYLINDER);
  EXPECT_EQ(MarkerType::MT_CYLINDER, marker->Type());

  marker->SetType(MarkerType::MT_BOX);
  EXPECT_EQ(MarkerType::MT_BOX, marker->Type());

  marker->SetType(MarkerType::MT_SPHERE);
  EXPECT_EQ(MarkerType::MT_SPHERE, marker->Type());

  marker->SetType(MarkerType::MT_NONE);
  EXPECT_EQ(MarkerType::MT_NONE, marker->Type());

  marker->SetType(static_cast<MarkerType>(99));
  EXPECT_EQ(static_cast<MarkerType>(99), marker->Type());

  marker->SetType(MarkerType::MT_POINTS);
  EXPECT_EQ(MarkerType::MT_POINTS, marker->Type());

  marker->SetType(MarkerType::MT_LINE_STRIP);
  EXPECT_EQ(MarkerType::MT_LINE_STRIP, marker->Type());

  marker->SetType(MarkerType::MT_LINE_LIST);
  EXPECT_EQ(MarkerType::MT_LINE_LIST, marker->Type());

  marker->SetType(MarkerType::MT_TRIANGLE_STRIP);
  EXPECT_EQ(MarkerType::MT_TRIANGLE_STRIP, marker->Type());

  marker->SetType(MarkerType::MT_TRIANGLE_LIST);
  EXPECT_EQ(MarkerType::MT_TRIANGLE_LIST, marker->Type());

  marker->SetType(MarkerType::MT_TRIANGLE_FAN);
  EXPECT_EQ(MarkerType::MT_TRIANGLE_FAN, marker->Type());

  // set same type again
  marker->SetType(MarkerType::MT_TRIANGLE_FAN);
  EXPECT_EQ(MarkerType::MT_TRIANGLE_FAN, marker->Type());

  // attach to visual again
  EXPECT_NO_THROW(visual->AddGeometry(marker));

  // exercise point api
  EXPECT_NO_THROW(marker->AddPoint(math::Vector3d(0, 1, 2),
      math::Color::White));
  EXPECT_NO_THROW(marker->SetPoint(0, math::Vector3d(3, 1, 2)));
  EXPECT_NO_THROW(marker->ClearPoints());

  EXPECT_DOUBLE_EQ(1.0, marker->Size());
  marker->SetSize(3.0);
  EXPECT_DOUBLE_EQ(3.0, marker->Size());

  // Clean up
  engine->DestroyScene(scene);
  rendering::unloadEngine(engine->Name());
}

/////////////////////////////////////////////////
void MarkerTest::Material(const std::string &_renderEngine)
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

  EXPECT_NO_THROW(marker->SetMaterial(MaterialPtr(), false));
  EXPECT_EQ(nullptr, marker->Material());

  MaterialPtr material = scene->CreateMaterial();
  ASSERT_NE(nullptr, material);
  material->SetDiffuse(0.1, 0.2, 0.3);

  marker->SetType(MarkerType::MT_NONE);
  EXPECT_EQ(MarkerType::MT_NONE, marker->Type());
  marker->SetMaterial(material, false);
  EXPECT_EQ(material, marker->Material());

  marker->SetType(MarkerType::MT_BOX);
  EXPECT_EQ(MarkerType::MT_BOX, marker->Type());
  marker->SetMaterial(material, false);
  EXPECT_EQ(material, marker->Material());

  marker->SetType(MarkerType::MT_LINE_STRIP);
  EXPECT_EQ(MarkerType::MT_LINE_STRIP, marker->Type());
  marker->SetMaterial(material, true);
  EXPECT_NE(material, marker->Material());
  EXPECT_EQ(math::Color(0.1f, 0.2f, 0.3f), marker->Material()->Diffuse());
}

/////////////////////////////////////////////////
TEST_P(MarkerTest, Marker)
{
  Marker(GetParam());
}

/////////////////////////////////////////////////
TEST_P(MarkerTest, Material)
{
  Material(GetParam());
}

INSTANTIATE_TEST_CASE_P(Marker, MarkerTest,
    RENDER_ENGINE_VALUES,
    ignition::rendering::PrintToStringParam());

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
