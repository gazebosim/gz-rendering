/*
 * Copyright (C) 2022 Open Source Robotics Foundation
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
#include <string>

#include <ignition/common/Console.hh>
#include <ignition/common/HeightmapData.hh>
#include <ignition/common/ImageHeightmap.hh>

#include "test_config.h"  // NOLINT(build/include)

#include "ignition/rendering/Heightmap.hh"
#include "ignition/rendering/RenderEngine.hh"
#include "ignition/rendering/RenderingIface.hh"
#include "ignition/rendering/Scene.hh"
#include "ignition/rendering/Visual.hh"

using namespace ignition;
using namespace rendering;

class HeightmapTest : public testing::Test,
                      public testing::WithParamInterface<const char *>
{
  /// \brief Test loading heightmap
  public: void Heightmap(const std::string &_renderEngine);

  public: const std::string TEST_MEDIA_PATH =
        common::joinPaths(std::string(PROJECT_SOURCE_PATH),
        "test", "media");
};

/////////////////////////////////////////////////
void HeightmapTest::Heightmap(const std::string &_renderEngine)
{
  RenderEngine *engine = rendering::engine(_renderEngine);
  if (!engine)
  {
    igndbg << "Engine '" << _renderEngine
              << "' is not supported" << std::endl;
    return;
  }

  ScenePtr scene = engine->CreateScene("scene");

  auto data = std::make_shared<common::ImageHeightmap>();
  data->Load(common::joinPaths(TEST_MEDIA_PATH, "heightmap_bowl.png"));

  HeightmapDescriptor desc;
  desc.SetName("example_bowl");
  desc.SetData(data);
  desc.SetSize({17, 17, 10});
  desc.SetSampling(2u);
  desc.SetUseTerrainPaging(false);

  // heightmap texture
   HeightmapTexture textureA;
   textureA.SetSize(1.0);
   EXPECT_DOUBLE_EQ(1.0, textureA.Size());
   std::string diffuseTexture =
       common::joinPaths(TEST_MEDIA_PATH, "materials",
       "textures", "dirt_diffusespecular.png");
   textureA.SetDiffuse(diffuseTexture);
   std::string normalTexture =
       common::joinPaths(TEST_MEDIA_PATH, "materials",
       "textures", "flat_normal.png");
   // \todo(iche033) having a normal map causes a segfault when unloading
   // texture on exit
   // textureA.SetNormal(normalTexture);
   desc.AddTexture(textureA);

  auto heightmapGeom = scene->CreateHeightmap(desc);
  auto desc2 = heightmapGeom->Descriptor();
  EXPECT_EQ(desc.Name(), desc2.Name());
  EXPECT_EQ(desc.Size(), desc2.Size());
  EXPECT_EQ(desc.Sampling(), desc2.Sampling());
  EXPECT_EQ(desc.UseTerrainPaging(), desc2.UseTerrainPaging());
  EXPECT_EQ(desc.Position(), desc2.Position());
  EXPECT_EQ(desc.TextureCount(), desc2.TextureCount());
  EXPECT_EQ(desc.BlendCount(),  desc2.BlendCount());

  auto vis = scene->CreateVisual();
  vis->AddGeometry(heightmapGeom);
  VisualPtr root = scene->RootVisual();
  root->AddChild(vis);

  vis->Destroy();

  // Clean up
  engine->DestroyScene(scene);
  rendering::unloadEngine(engine->Name());
}

/////////////////////////////////////////////////
TEST_P(HeightmapTest, Heightmap)
{
  Heightmap(GetParam());
}

INSTANTIATE_TEST_CASE_P(Heightmap, HeightmapTest,
    RENDER_ENGINE_VALUES,
    ignition::rendering::PrintToStringParam());

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
