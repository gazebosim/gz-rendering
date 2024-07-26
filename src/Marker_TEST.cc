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

#include <gz/common/Console.hh>

#include "test_config.h"  // NOLINT(build/include)
#include "gz/rendering/RenderEngine.hh"
#include "gz/rendering/RenderingIface.hh"
#include "gz/rendering/Marker.hh"
#include "gz/rendering/Scene.hh"

using namespace gz;
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
#ifdef __APPLE__                                                                              
  std::cerr << "Skipping test for apple, see issue #847." << std::endl;                       
  return;
#endif
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

  marker->SetType(MarkerType::MT_BOX);
  EXPECT_EQ(MarkerType::MT_BOX, marker->Type());

  marker->SetType(MarkerType::MT_SPHERE);
  EXPECT_EQ(MarkerType::MT_SPHERE, marker->Type());

  marker->SetType(static_cast<MarkerType>(11));
  EXPECT_EQ(static_cast<MarkerType>(11), marker->Type());

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

  // Repeat again
  marker->SetType(MarkerType::MT_TRIANGLE_FAN);
  EXPECT_EQ(MarkerType::MT_TRIANGLE_FAN, marker->Type());

  // exercise point api
  EXPECT_NO_THROW(marker->AddPoint(math::Vector3d(0, 1, 2),
      math::Color::White));
  EXPECT_NO_THROW(marker->AddPoint(-2, -1, 0, math::Color::White));

  EXPECT_NO_THROW(marker->SetPoint(0, math::Vector3d(3, 1, 2)));
  EXPECT_NO_THROW(marker->ClearPoints());

  marker->PreRender();

  // create material
  MaterialPtr mat = scene->CreateMaterial();
  mat->SetAmbient(0.6, 0.7, 0.8);
  mat->SetDiffuse(0.3, 0.8, 0.2);
  mat->SetSpecular(0.4, 0.9, 1.0);

  MaterialPtr markerMat;
  marker->SetType(MarkerType::MT_NONE);
  marker->SetMaterial(mat);
  markerMat = marker->Material();
  ASSERT_NE(nullptr, markerMat);

  marker->SetType(static_cast<MarkerType>(11));
  marker->SetMaterial(mat);
  markerMat = marker->Material();
  ASSERT_NE(nullptr, markerMat);

  marker->SetType(MarkerType::MT_BOX);
  marker->SetMaterial(mat);
  markerMat = marker->Material();
  ASSERT_NE(nullptr, markerMat);

  marker->SetType(MarkerType::MT_POINTS);
  marker->SetMaterial(mat);
  markerMat = marker->Material();
  ASSERT_NE(nullptr, markerMat);

  EXPECT_EQ(math::Color(0.6f, 0.7f, 0.8f), markerMat->Ambient());
  EXPECT_EQ(math::Color(0.3f, 0.8f, 0.2f), markerMat->Diffuse());
  EXPECT_EQ(math::Color(0.4f, 0.9f, 1.0f), markerMat->Specular());


  MaterialPtr matNull;
  marker->SetMaterial(matNull, false);
  ASSERT_NE(nullptr, markerMat);

  // Clean up
  engine->DestroyScene(scene);
  unloadEngine(engine->Name());
}

/////////////////////////////////////////////////
TEST_P(MarkerTest, Marker)
{
  Marker(GetParam());
}

INSTANTIATE_TEST_CASE_P(Marker, MarkerTest,
    RENDER_ENGINE_VALUES,
    PrintToStringParam());

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
