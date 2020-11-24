/*
 * Copyright (C) 2020 Open Source Robotics Foundation
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
#include <ignition/common/ImageHeightmap.hh>

#include "test_config.h"  // NOLINT(build/include)
#include "ignition/rendering/RenderEngine.hh"
#include "ignition/rendering/RenderingIface.hh"
#include "ignition/rendering/Heightmap.hh"
#include "ignition/rendering/Scene.hh"

using namespace ignition;
using namespace rendering;

class HeightmapTest : public testing::Test,
                 public testing::WithParamInterface<const char *>
{
  // Documentation inherited
  protected: void SetUp() override
  {
    common::Console::SetVerbosity(4);
  }

  /// \brief Test basic heightmap functionality.
  /// \param[in] _renderEngine Engine to use.
  public: void Heightmap(const std::string &_renderEngine);

  /// \brief Path to test media files.
  public: const std::string TEST_MEDIA_PATH{
        common::joinPaths(std::string(PROJECT_SOURCE_PATH),
        "test", "media")};
};

/////////////////////////////////////////////////
void HeightmapTest::Heightmap(const std::string &_renderEngine)
{
  if (_renderEngine != "ogre")
  {
    igndbg << "Heightmap not supported yet in rendering engine: "
            << _renderEngine << std::endl;
    return;
  }

  auto engine = rendering::engine(_renderEngine);
  if (!engine)
  {
    igndbg << "Engine '" << _renderEngine
           << "' is not supported" << std::endl;
    return;
  }

  auto scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);

  // Heightmap data
  auto heightImage = common::joinPaths(TEST_MEDIA_PATH, "heightmap_bowl.png");
  math::Vector3d size{17, 17, 10};
  math::Vector3d position{1, 2, 3};
  auto textureImage = common::joinPaths(TEST_MEDIA_PATH, "materials",
      "textures", "texture.png");
  auto normalImage = common::joinPaths(TEST_MEDIA_PATH, "materials",
      "textures", "flat_normal.png");

  auto data = std::make_shared<common::ImageHeightmap>();
  data->Load(heightImage);

  EXPECT_EQ(heightImage, data->Filename());

  HeightmapDescriptor desc;
  desc.data = data;
  desc.size = size;
  desc.position = position;
  desc.useTerrainPaging = true;
  desc.sampling = 4u;

  HeightmapDescriptor::Texture textureA;
  textureA.size = 0.5;
  textureA.diffuse = textureImage;
  textureA.normal = normalImage;
  desc.textures.push_back(textureA);

  HeightmapDescriptor::Blend blendA;
  blendA.minHeight = 2.0;
  blendA.fadeDistance = 5.0;
  desc.blends.push_back(blendA);

  HeightmapDescriptor::Texture textureB;
  textureB.size = 0.5;
  textureB.diffuse = textureImage;
  textureB.normal = normalImage;
  desc.textures.push_back(textureB);

  HeightmapDescriptor::Blend blendB;
  blendB.minHeight = 4.0;
  blendB.fadeDistance = 5.0;
  desc.blends.push_back(blendB);

  HeightmapDescriptor::Texture textureC;
  textureC.size = 0.5;
  textureC.diffuse = textureImage;
  textureC.normal = normalImage;
  desc.textures.push_back(textureC);

  auto heightmap = scene->CreateHeightmap(desc);
  ASSERT_NE(nullptr, heightmap);

  EXPECT_NE(nullptr, heightmap->Descriptor().data);
  EXPECT_EQ(size, heightmap->Descriptor().size);
  EXPECT_EQ(position, heightmap->Descriptor().position);
  EXPECT_TRUE(heightmap->Descriptor().useTerrainPaging);
  EXPECT_EQ(4u, heightmap->Descriptor().sampling);

  EXPECT_EQ(3u, heightmap->Descriptor().textures.size());
  for (auto texture : heightmap->Descriptor().textures)
  {
    EXPECT_EQ(textureImage, texture.diffuse);
    EXPECT_EQ(normalImage, texture.normal);
    EXPECT_DOUBLE_EQ(0.5, texture.size);
  }

  ASSERT_EQ(2u, heightmap->Descriptor().blends.size());
  EXPECT_DOUBLE_EQ(2.0, heightmap->Descriptor().blends[0].minHeight);
  EXPECT_DOUBLE_EQ(5.0, heightmap->Descriptor().blends[0].fadeDistance);
  EXPECT_DOUBLE_EQ(4.0, heightmap->Descriptor().blends[1].minHeight);
  EXPECT_DOUBLE_EQ(5.0, heightmap->Descriptor().blends[1].fadeDistance);

  // Clean up
  engine->DestroyScene(scene);
  rendering::unloadEngine(engine->Name());
}

/////////////////////////////////////////////////
TEST_P(HeightmapTest, Heightmap)
{
  this->Heightmap(this->GetParam());
}

// TODO(anyone) Running test with Ogre1. Update once Ogre2 is supported.
INSTANTIATE_TEST_CASE_P(Heightmap, HeightmapTest,
    ::testing::ValuesIn({"ogre"}),
    ignition::rendering::PrintToStringParam());

/////////////////////////////////////////////////
int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
