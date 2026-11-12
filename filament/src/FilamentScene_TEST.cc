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
#include <memory>
#include <string>

#include <filament/Engine.h>
#include <utils/EntityManager.h>

#include "gz/rendering/RenderEngine.hh"
#include "gz/rendering/RenderingIface.hh"
#include "gz/rendering/Scene.hh"
#include "gz/rendering/filament/FilamentRenderEngine.hh"

using namespace gz;
using namespace rendering;

/// \brief The filament engine through the plugin loader: the engine
/// singleton of the library
static RenderEngine *LoadEngine()
{
  std::map<std::string, std::string> params;
  if (const char *backend = std::getenv("GZ_ENGINE_BACKEND"))
    params["backend"] = backend;
  return rendering::engine("filament", params);
}

/////////////////////////////////////////////////
// DestroyScene releases the filament scene, its lighting, its root visual
// and the scene itself
TEST(FilamentSceneTest, DestroyScene)
{
  RenderEngine *engine = LoadEngine();
  ASSERT_NE(nullptr, engine);
  auto *filamentEngine = dynamic_cast<FilamentRenderEngine *>(engine);
  ASSERT_NE(nullptr, filamentEngine);
  filament::Engine *native = filamentEngine->FilamentEngine();
  ASSERT_NE(nullptr, native);
  const size_t scenes = native->getSceneCount();
  const size_t lights = native->getIndirectLightCount();
  const size_t entities = ::utils::EntityManager::get().getEntityCount();
  for (int i = 0; i < 10; ++i)
  {
    ScenePtr scene = engine->CreateScene("scene" + std::to_string(i));
    ASSERT_NE(nullptr, scene);
    scene->SetAmbientLight(0.3, 0.3, 0.3);
    std::weak_ptr<Scene> weak = scene;
    engine->DestroyScene(scene);
    scene.reset();
    EXPECT_TRUE(weak.expired());
  }
  EXPECT_EQ(scenes, native->getSceneCount());
  EXPECT_EQ(lights, native->getIndirectLightCount());
  EXPECT_EQ(entities, ::utils::EntityManager::get().getEntityCount());
  rendering::unloadEngine(engine->Name());
}
