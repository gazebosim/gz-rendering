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

#include <gz/common/Filesystem.hh>
#include <gz/common/Util.hh>

#include "gz/rendering/Camera.hh"
#include "gz/rendering/Marker.hh"
#include "gz/rendering/RenderEngine.hh"
#include "gz/rendering/RenderingIface.hh"
#include "gz/rendering/Scene.hh"

using namespace gz;
using namespace rendering;

/////////////////////////////////////////////////
// Without its compiled materials (e.g. a wrong resource path) the engine
// logs errors and renders what it can, without crashing
TEST(FilamentMissingMaterialsTest, Render)
{
  const std::string empty =
      common::joinPaths(::testing::TempDir(), "filament_no_media");
  ASSERT_TRUE(common::createDirectories(empty));
  ASSERT_TRUE(common::setenv("GZ_RENDERING_FILAMENT_RESOURCE_PATH", empty));

  std::map<std::string, std::string> params;
  if (const char *backend = std::getenv("GZ_ENGINE_BACKEND"))
    params["backend"] = backend;
  RenderEngine *engine = rendering::engine("filament", params);
  ASSERT_NE(nullptr, engine);
  {
    ScenePtr scene = engine->CreateScene("scene");
    ASSERT_NE(nullptr, scene);
    VisualPtr root = scene->RootVisual();

    // a marker is a dynamic mesh, drawn with gz_unlit without a material
    MarkerPtr marker = scene->CreateMarker();
    marker->SetType(MT_LINE_STRIP);
    marker->AddPoint(0, 0, 0, math::Color::Red);
    marker->AddPoint(1, 0, 0, math::Color::Red);
    VisualPtr visual = scene->CreateVisual();
    visual->AddGeometry(marker);
    visual->SetLocalPosition(2, 0, 0);
    root->AddChild(visual);

    CameraPtr camera = scene->CreateCamera();
    camera->SetImageWidth(32);
    camera->SetImageHeight(32);
    root->AddChild(camera);
    Image image = camera->CreateImage();
    camera->Capture(image);
    engine->DestroyScene(scene);
  }
  rendering::unloadEngine(engine->Name());
}
