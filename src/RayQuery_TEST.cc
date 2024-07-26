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

#include <gz/common/Console.hh>

#include "test_config.h"  // NOLINT(build/include)

#include "gz/rendering/Camera.hh"
#include "gz/rendering/RayQuery.hh"
#include "gz/rendering/RenderEngine.hh"
#include "gz/rendering/RenderingIface.hh"
#include "gz/rendering/Scene.hh"

using namespace gz;
using namespace rendering;

class RayQueryTest : public testing::Test,
                     public testing::WithParamInterface<const char *>
{
  /// \brief Test ray query basic API
  public: void RayQuery(const std::string &_renderEngine);
};

/////////////////////////////////////////////////
void RayQueryTest::RayQuery(const std::string &_renderEngine)
{
#ifdef __APPLE__                                                                              
  std::cerr << "Skipping test for apple, see issue #847." << std::endl;                       
  return;
#endif
  if (_renderEngine == "optix")
  {
    igndbg << "RayQuery not supported yet in rendering engine: "
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

  RayQueryPtr rayQuery = scene->CreateRayQuery();
  EXPECT_TRUE(rayQuery != nullptr);

  // check basic api for setting ray origin and direction
  math::Vector3d o0 = math::Vector3d::Zero;
  math::Vector3d d0 = math::Vector3d::UnitZ;

  rayQuery->SetOrigin(o0);
  EXPECT_EQ(o0, rayQuery->Origin());
  rayQuery->SetDirection(d0);
  EXPECT_EQ(d0, rayQuery->Direction());

  math::Vector3d o1 = -math::Vector3d::UnitX;
  math::Vector3d d1 = -math::Vector3d::UnitY;

  rayQuery->SetOrigin(o1);
  EXPECT_EQ(o1, rayQuery->Origin());
  rayQuery->SetDirection(d1);
  EXPECT_EQ(d1, rayQuery->Direction());

  // non unit vectors
  math::Vector3d o2(1.0, 3.5, -6.4);
  math::Vector3d d2(3.0, 450.9, 20.0);

  rayQuery->SetOrigin(o2);
  EXPECT_EQ(o2, rayQuery->Origin());
  rayQuery->SetDirection(d2);
  EXPECT_EQ(d2, rayQuery->Direction());

  // set from camera
  CameraPtr camera =  scene->CreateCamera("camera");
  math::Vector2d pos(0.0, 0.0);
  rayQuery->SetFromCamera(camera, pos);

  EXPECT_GT(rayQuery->Origin().X(), 0.0);
  EXPECT_EQ(math::Vector3d::UnitX, rayQuery->Direction().Normalize());

  RayQueryResult result = rayQuery->ClosestPoint();
  EXPECT_EQ(math::Vector3d::Zero, result.point);
  EXPECT_LT(result.distance, 0.0);
  EXPECT_EQ(0u, result.objectId);
  EXPECT_FALSE((result));

  // Clean up
  engine->DestroyScene(scene);
  unloadEngine(engine->Name());
}

/////////////////////////////////////////////////
TEST_P(RayQueryTest, RayQuery)
{
  RayQuery(GetParam());
}

INSTANTIATE_TEST_CASE_P(RayQuery, RayQueryTest,
    RENDER_ENGINE_VALUES,
    PrintToStringParam());

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
