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

#include <filament/Scene.h>

#include "gz/rendering/RenderEngine.hh"
#include "gz/rendering/RenderingIface.hh"
#include "gz/rendering/filament/FilamentMesh.hh"
#include "gz/rendering/filament/FilamentRenderEngine.hh"
#include "gz/rendering/filament/FilamentScene.hh"
#include "gz/rendering/filament/FilamentVisual.hh"

using namespace gz;
using namespace rendering;

/////////////////////////////////////////////////
// A wireframe mesh draws a depth-only copy of its triangles (its hidden
// surface): it leaves and joins the scene with the mesh
TEST(FilamentVisualTest, DetachWireframeGeometry)
{
  std::map<std::string, std::string> params;
  if (const char *backend = std::getenv("GZ_ENGINE_BACKEND"))
    params["backend"] = backend;
  // through the plugin loader: the engine singleton of the library
  RenderEngine *engine = rendering::engine("filament", params);
  ASSERT_NE(nullptr, engine);
  {
    auto scene =
        std::dynamic_pointer_cast<FilamentScene>(engine->CreateScene("s"));
    ASSERT_NE(nullptr, scene);
    filament::Scene *native = scene->Native();
    VisualPtr visual = scene->CreateVisual();
    scene->RootVisual()->AddChild(visual);
    GeometryPtr box = scene->CreateBox();
    visual->AddGeometry(box);
    visual->SetWireframe(true);
    auto mesh = std::dynamic_pointer_cast<FilamentMesh>(box);
    ASSERT_NE(nullptr, mesh);
    const ::utils::Entity hidden = mesh->HiddenSurface();
    ASSERT_FALSE(hidden.isNull());
    EXPECT_TRUE(native->hasEntity(mesh->Entity()));
    EXPECT_TRUE(native->hasEntity(hidden));

    visual->RemoveGeometry(box);
    EXPECT_FALSE(native->hasEntity(mesh->Entity()));
    EXPECT_FALSE(native->hasEntity(hidden));

    visual->AddGeometry(box);
    EXPECT_TRUE(native->hasEntity(mesh->Entity()));
    EXPECT_TRUE(native->hasEntity(hidden));
    engine->DestroyScene(scene);
  }
  rendering::unloadEngine(engine->Name());
}
