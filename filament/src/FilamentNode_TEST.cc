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
#include <filament/TransformManager.h>
#include <math/mat4.h>
#include <math/vec3.h>

#include "gz/rendering/RenderEngine.hh"
#include "gz/rendering/RenderingIface.hh"
#include "gz/rendering/Scene.hh"
#include "gz/rendering/filament/FilamentRenderEngine.hh"
#include "gz/rendering/filament/FilamentVisual.hh"

using namespace gz;
using namespace rendering;

namespace
{
/// \brief The plugin's engine (FilamentRenderEngine::Instance() inlined in
/// the test would be a singleton of its own)
FilamentRenderEngine *gEngine = nullptr;

/// \brief World transform filament draws _node with
filament::math::mat4f World(const VisualPtr &_node)
{
  auto &tm = gEngine->FilamentEngine()->getTransformManager();
  auto node = std::dynamic_pointer_cast<FilamentNode>(_node);
  return tm.getWorldTransform(tm.getInstance(node->Entity()));
}

/// \brief Expect the world transform of _node to have axes of lengths
/// _scale and the translation _pos
void ExpectWorld(const VisualPtr &_node, const math::Vector3d &_scale,
    const math::Vector3d &_pos)
{
  const filament::math::mat4f m = World(_node);
  for (int c = 0; c < 3; ++c)
    EXPECT_NEAR(_scale[c], length(m[c].xyz), 1e-5) << _node->Name() << c;
  for (int r = 0; r < 3; ++r)
    EXPECT_NEAR(_pos[r], m[3][r], 1e-5) << _node->Name() << r;
}
}

/////////////////////////////////////////////////
// SetInheritScale(false) drops the parent's scale from what filament
// draws, like ogre2: the position stays in the parent's scaled frame
TEST(FilamentNodeTest, InheritScale)
{
  std::map<std::string, std::string> params;
  if (const char *backend = std::getenv("GZ_ENGINE_BACKEND"))
    params["backend"] = backend;
  // through the plugin loader: the engine singleton of the library
  RenderEngine *engine = rendering::engine("filament", params);
  ASSERT_NE(nullptr, engine);
  gEngine = dynamic_cast<FilamentRenderEngine *>(engine);
  ASSERT_NE(nullptr, gEngine);
  {
    ScenePtr scene = engine->CreateScene("scene");
    ASSERT_NE(nullptr, scene);
    VisualPtr parent = scene->CreateVisual("parent");
    VisualPtr child = scene->CreateVisual("child");
    VisualPtr grandChild = scene->CreateVisual("grandChild");
    scene->RootVisual()->AddChild(parent);
    parent->SetLocalScale(2, 3, 4);
    parent->SetLocalPosition(1, 1, 1);
    child->SetLocalPosition(1, 1, 1);
    child->SetInheritScale(false);
    parent->AddChild(child);
    grandChild->SetLocalScale(5, 5, 5);
    child->AddChild(grandChild);

    ExpectWorld(parent, {2, 3, 4}, {1, 1, 1});
    ExpectWorld(child, {1, 1, 1}, {3, 4, 5});
    ExpectWorld(grandChild, {5, 5, 5}, {3, 4, 5});

    // the parent's scale changes later
    parent->SetLocalScale(0.5, 0.5, 0.5);
    ExpectWorld(child, {1, 1, 1}, {1.5, 1.5, 1.5});
    ExpectWorld(grandChild, {5, 5, 5}, {1.5, 1.5, 1.5});

    // back to inheriting
    child->SetInheritScale(true);
    ExpectWorld(child, {0.5, 0.5, 0.5}, {1.5, 1.5, 1.5});
    ExpectWorld(grandChild, {2.5, 2.5, 2.5}, {1.5, 1.5, 1.5});
    engine->DestroyScene(scene);
  }
  rendering::unloadEngine(engine->Name());
}
