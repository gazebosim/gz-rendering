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

using namespace ignition;
using namespace rendering;

const std::string TEST_MEDIA_PATH =
      common::joinPaths(std::string(PROJECT_SOURCE_PATH),
      "test", "media");

/////////////////////////////////////////////////
TEST(HeightmapDescriptorTest, HeightmapDescriptor)
{
  auto data = std::make_shared<common::ImageHeightmap>();
  data->Load(common::joinPaths(TEST_MEDIA_PATH, "heightmap_bowl.png"));

  HeightmapDescriptor desc;
  desc.SetName("example_bowl");
  EXPECT_EQ("example_bowl", desc.Name());
  desc.SetData(data);
  EXPECT_EQ(data, desc.Data());
  desc.SetSize({17, 17, 10});
  EXPECT_EQ(math::Vector3d(17, 17, 10), desc.Size());
  desc.SetSampling(2u);
  EXPECT_EQ(2u, desc.Sampling());
  desc.SetUseTerrainPaging(false);
  EXPECT_FALSE(desc.UseTerrainPaging());
  desc.SetPosition(math::Vector3d(0.2, 0.3, 0.4));
  EXPECT_EQ(math::Vector3d(0.2, 0.3, 0.4), desc.Position());
  EXPECT_EQ(0u, desc.TextureCount());
  EXPECT_EQ(nullptr, desc.TextureByIndex(0u));
  EXPECT_EQ(0u, desc.BlendCount());
  EXPECT_EQ(nullptr, desc.BlendByIndex(0u));

  // heightmap texture
  HeightmapTexture textureA;
  textureA.SetSize(1.0);
  EXPECT_DOUBLE_EQ(1.0, textureA.Size());
  std::string diffuseTexture =
      common::joinPaths(TEST_MEDIA_PATH, "materials",
      "textures", "dirt_diffusespecular.png");
  textureA.SetDiffuse(diffuseTexture);
  EXPECT_EQ(diffuseTexture, textureA.Diffuse());
  std::string normalTexture =
      common::joinPaths(TEST_MEDIA_PATH, "materials",
      "textures", "flat_normal.png");
  textureA.SetNormal(normalTexture);
  EXPECT_EQ(normalTexture, textureA.Normal());
  desc.AddTexture(textureA);
  EXPECT_EQ(1u, desc.TextureCount());
  const HeightmapTexture *tex = desc.TextureByIndex(0u);
  ASSERT_NE(nullptr, tex);
  EXPECT_EQ(diffuseTexture, tex->Diffuse());
  EXPECT_EQ(normalTexture, tex->Normal());

  // heightmap blend
  HeightmapBlend blendA;
  blendA.SetMinHeight(2.0);
  EXPECT_DOUBLE_EQ(2.0, blendA.MinHeight());
  blendA.SetFadeDistance(5.0);
  EXPECT_DOUBLE_EQ(5.0, blendA.FadeDistance());
  desc.AddBlend(blendA);
  EXPECT_EQ(1u, desc.BlendCount());
  const HeightmapBlend *blend = desc.BlendByIndex(0u);
  ASSERT_NE(nullptr, blend);
  EXPECT_DOUBLE_EQ(2.0, blend->MinHeight());
  EXPECT_DOUBLE_EQ(5.0, blend->FadeDistance());

  // test copy constructor
  auto desc2 = HeightmapDescriptor(desc);
  EXPECT_EQ("example_bowl", desc2.Name());
  EXPECT_EQ(math::Vector3d(17, 17, 10), desc2.Size());
  EXPECT_EQ(2u, desc2.Sampling());
  EXPECT_FALSE(desc2.UseTerrainPaging());
  EXPECT_EQ(math::Vector3d(0.2, 0.3, 0.4), desc2.Position());
  EXPECT_EQ(1u, desc2.TextureCount());
  EXPECT_NE(nullptr, desc2.TextureByIndex(0u));
  EXPECT_EQ(1u, desc2.BlendCount());
  EXPECT_NE(nullptr, desc2.BlendByIndex(0u));

  // test assignment operator
  auto desc3 = desc;
  EXPECT_EQ("example_bowl", desc3.Name());
  EXPECT_EQ(math::Vector3d(17, 17, 10), desc3.Size());
  EXPECT_EQ(2u, desc3.Sampling());
  EXPECT_FALSE(desc3.UseTerrainPaging());
  EXPECT_EQ(math::Vector3d(0.2, 0.3, 0.4), desc3.Position());
  EXPECT_EQ(1u, desc3.TextureCount());
  EXPECT_NE(nullptr, desc3.TextureByIndex(0u));
  EXPECT_EQ(1u, desc3.BlendCount());
  EXPECT_NE(nullptr, desc3.BlendByIndex(0u));
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
