/*
 * Copyright (C) 2026 Open Source Robotics Foundation
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

#include <cstdlib>
#include <map>
#include <string>

#include <gz/math/Vector3.hh>

#include "gz/rendering/RayQuery.hh"
#include "gz/rendering/RenderEngine.hh"
#include "gz/rendering/RenderingIface.hh"
#include "gz/rendering/Scene.hh"
#include "gz/rendering/Visual.hh"

using namespace gz;
using namespace rendering;

/////////////////////////////////////////////////
// ClosestPoint ignores _forceSceneUpdate: a pose change is in the query at
// once, with no render in between, whichever the flag
TEST(FilamentRayQueryTest, ClosestPointSeesPoseChange)
{
  std::map<std::string, std::string> params;
  if (const char *backend = std::getenv("GZ_ENGINE_BACKEND"))
    params["backend"] = backend;
  // through the plugin loader: the engine singleton of the library
  RenderEngine *engine = rendering::engine("filament", params);
  ASSERT_NE(nullptr, engine);
  {
    ScenePtr scene = engine->CreateScene("scene");
    ASSERT_NE(nullptr, scene);
    VisualPtr box = scene->CreateVisual();
    box->AddGeometry(scene->CreateBox());
    scene->RootVisual()->AddChild(box);

    RayQueryPtr rayQuery = scene->CreateRayQuery();
    ASSERT_NE(nullptr, rayQuery);
    rayQuery->SetPreferGpu(false);
    rayQuery->SetOrigin(math::Vector3d(-5.0, 0.0, 0.0));
    rayQuery->SetDirection(math::Vector3d::UnitX);

    // the unit box spans x in [-0.5, 0.5]
    RayQueryResult result = rayQuery->ClosestPoint(false);
    ASSERT_TRUE(result);
    EXPECT_NEAR(4.5, result.distance, 1e-6);

    box->SetLocalPosition(10.0, 0.0, 0.0);
    for (const bool force : {false, true})
    {
      result = rayQuery->ClosestPoint(force);
      ASSERT_TRUE(result) << force;
      EXPECT_NEAR(14.5, result.distance, 1e-6) << force;
    }
    engine->DestroyScene(scene);
  }
  rendering::unloadEngine(engine->Name());
}
