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

#include <filament/Scene.h>

#include "gz/rendering/RenderingIface.hh"
#include "gz/rendering/Visual.hh"
#include "gz/rendering/filament/FilamentRenderEngine.hh"
#include "gz/rendering/filament/FilamentScene.hh"

using namespace gz;
using namespace rendering;

/////////////////////////////////////////////////
// The GUI camera's reflection probe renders one cube face per frame. In a
// scene that changes every frame it must still complete a cube (and swap
// the scene IBL for it) after 6 frames. Counted in frames: the probe's IBL
// is in place between BeginReflections and EndReflections once a cube is
// complete.
TEST(FilamentReflectionProbeTest, CompletesWhileTheSceneMoves)
{
  std::map<std::string, std::string> params;
  if (const char *backend = std::getenv("GZ_ENGINE_BACKEND"))
    params["backend"] = backend;
  RenderEngine *engine = rendering::engine("filament", params);
  ASSERT_NE(nullptr, engine);
  // the probe's prefilter runs on filament's job system (the quality-mode
  // camera adopts its render thread the same way)
  dynamic_cast<FilamentRenderEngine *>(engine)->AdoptThread();
  {
    auto scene =
        std::dynamic_pointer_cast<FilamentScene>(engine->CreateScene("s"));
    ASSERT_NE(nullptr, scene);
    scene->SetAmbientLight(math::Color(0.3f, 0.3f, 0.3f));
    VisualPtr box = scene->CreateVisual();
    box->AddGeometry(scene->CreateBox());
    scene->RootVisual()->AddChild(box);
    filament::Scene *native = scene->Native();

    // the box moves every frame, from the first one
    int firstCube = -1;
    for (int frame = 0; frame < 12 && firstCube < 0; ++frame)
    {
      box->SetLocalPosition(2.0 + 0.1 * frame, 0, 0);
      scene->PreRender();
      filament::IndirectLight *base = native->getIndirectLight();
      scene->BeginReflections({0, 0, 0.5}, 0xffffffffu, 0.1, 100.0);
      if (native->getIndirectLight() != base)
        firstCube = frame;
      scene->EndReflections();
      EXPECT_EQ(base, native->getIndirectLight());
    }
    // 6 faces, one per frame: the first cube is complete at frame 5
    EXPECT_EQ(5, firstCube);
  }
  rendering::unloadEngine(engine->Name());
}
