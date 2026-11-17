/* * Copyright (C) 2021 Open Source Robotics Foundation
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

#include "CommonRenderingTest.hh"

#include <array>
#include <cstring>
#include <vector>

#include <gz/common/geospatial/ImageHeightmap.hh>
#include <gz/utils/ExtraTestMacros.hh>

#include "gz/rendering/Camera.hh"
#include "gz/rendering/Heightmap.hh"
#include "gz/rendering/Image.hh"
#include "gz/rendering/PixelBuffer.hh"
#include "gz/rendering/PixelFormat.hh"
#include "gz/rendering/RayQuery.hh"
#include "gz/rendering/Scene.hh"
#include "gz/rendering/Utils.hh"
#include "gz/rendering/Visual.hh"

using namespace gz;
using namespace rendering;

class UtilTest : public CommonRenderingTest
{
  /// \brief Path to test media files.
  public: const std::string TEST_MEDIA_PATH{
        common::joinPaths(std::string(PROJECT_SOURCE_PATH),
        "test", "media")};
};

/////////////////////////////////////////////////
TEST_F(UtilTest, GZ_UTILS_TEST_ENABLED_ONLY_ON_LINUX(ClickToScene))
{
  ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);

  CameraPtr camera(scene->CreateCamera());
  ASSERT_NE(nullptr, camera);

  camera->SetLocalPosition(0.0, 0.0, 15);
  camera->SetLocalRotation(0.0, GZ_PI / 2, 0.0);

  unsigned int width = 640u;
  unsigned int height = 480u;
  camera->SetImageWidth(width);
  camera->SetImageHeight(height);

  const int halfWidth  = static_cast<int>(width / 2);
  const int halfHeight = static_cast<int>(height / 2);
  math::Vector2i centerClick(halfWidth, halfHeight);

  RayQueryPtr rayQuery = scene->CreateRayQuery();
  EXPECT_TRUE(rayQuery != nullptr);

  // screenToPlane
  math::Vector3d result = screenToPlane(centerClick, camera, rayQuery);

  EXPECT_NEAR(0.0, result.Z(), 1e-10);
  EXPECT_NEAR(0.0, result.X(), 2e-6);
  EXPECT_NEAR(0.0, result.Y(), 2e-6);

  // call with non-zero plane offset
  result = screenToPlane(centerClick, camera, rayQuery, 5.0);

  EXPECT_NEAR(5.0, result.Z(), 1e-10);
  EXPECT_NEAR(0.0, result.X(), 2e-6);
  EXPECT_NEAR(0.0, result.Y(), 2e-6);

  // screenToScene
  // API without RayQueryResult and default max distance
  result = screenToScene(centerClick, camera, rayQuery);

  // No objects currently in the scene, so return a point max distance in
  // front of camera
  // The default max distance is 10 meters away
  EXPECT_NEAR(5.0 - camera->NearClipPlane(), result.Z(), 4e-6);
  EXPECT_NEAR(0.0, result.X(), 2e-6);
  EXPECT_NEAR(0.0, result.Y(), 2e-6);

  // Try with different max distance
  RayQueryResult rayResult;
  result = screenToScene(centerClick, camera, rayQuery, rayResult, 20.0);

  EXPECT_NEAR(-5.0 - camera->NearClipPlane(), result.Z(), 4e-6);
  EXPECT_NEAR(0.0, result.X(), 4e-6);
  EXPECT_NEAR(0.0, result.Y(), 4e-6);
  EXPECT_FALSE(rayResult);
  EXPECT_EQ(0u, rayResult.objectId);

  VisualPtr root = scene->RootVisual();

  // create box visual to collide with the ray
  VisualPtr box = scene->CreateVisual();
  box->AddGeometry(scene->CreateBox());
  box->SetOrigin(0.0, 0.0, 0.0);
  box->SetLocalPosition(0.0, 0.0, 0.0);
  box->SetLocalRotation(0.0, 0.0, 0.0);
  box->SetLocalScale(1.0, 1.0, 1.0);
  root->AddChild(box);

  // add camera and render one frame
  root->AddChild(camera);
  camera->Update();

  // \todo(anyone)
  // the centerClick var above is set to a screen pos of (width/2, height/2).
  // This is off-by-1. The actual center pos should be at
  // (width/2 - 1, height/2 - 1) so the result.X() and result.Y() is a bit off
  // from the expected position. However, fixing the centerClick above caused
  // the screenToPlane tests to fail so only modifying the pos here, and the
  // cause of test failure need to be investigated.
  if (this->engineToTest == "ogre2")
    centerClick = gz::math::Vector2i(halfWidth-1, halfHeight-1);

  // API without RayQueryResult and default max distance
  result = screenToScene(centerClick, camera, rayQuery, rayResult);

  // high tol is used for z due to depth buffer precision.
  // Do not merge the tol changes forward to gz-rendering6.
  EXPECT_NEAR(0.5, result.Z(), 1e-3);
  EXPECT_NEAR(0.0, result.X(), 2e-6);
  EXPECT_NEAR(0.0, result.Y(), 2e-6);
  EXPECT_TRUE(rayResult);
  EXPECT_NEAR(14.5 - camera->NearClipPlane(), rayResult.distance, 1e-3);
  EXPECT_EQ(box->Id(), rayResult.objectId);

  result = screenToScene(centerClick, camera, rayQuery, rayResult, 20.0);

  EXPECT_NEAR(0.5, result.Z(), 1e-3);
  EXPECT_NEAR(0.0, result.X(), 2e-6);
  EXPECT_NEAR(0.0, result.Y(), 2e-6);
  EXPECT_TRUE(rayResult);
  EXPECT_NEAR(14.5 - camera->NearClipPlane(), rayResult.distance, 1e-3);
  EXPECT_EQ(box->Id(), rayResult.objectId);

  // Move camera closer to box
  camera->SetLocalPosition(0.0, 0.0, 7.0);
  camera->SetLocalRotation(0.0, GZ_PI / 2, 0.0);

  result = screenToScene(centerClick, camera, rayQuery, rayResult);

  EXPECT_NEAR(0.5, result.Z(), 1e-3);
  EXPECT_NEAR(0.0, result.X(), 2e-6);
  EXPECT_NEAR(0.0, result.Y(), 2e-6);
  EXPECT_TRUE(rayResult);
  EXPECT_NEAR(6.5 - camera->NearClipPlane(), rayResult.distance, 1e-4);
  EXPECT_EQ(box->Id(), rayResult.objectId);

  // Clean up
  engine->DestroyScene(scene);
}

/////////////////////////////////////////////////
TEST_F(UtilTest, GZ_UTILS_TEST_ENABLED_ONLY_ON_LINUX(ClickToSceneHeightmap))
{
  CHECK_SUPPORTED_ENGINE("ogre2");

  ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);

  CameraPtr camera(scene->CreateCamera());
  EXPECT_TRUE(camera != nullptr);

  math::Pose3d cameraPose(math::Vector3d(0.0, 0.0, 20.0),
      math::Quaterniond(0.0, GZ_PI / 2.0, 0.0));
  camera->SetLocalPosition(cameraPose.Pos());
  camera->SetLocalRotation(cameraPose.Rot());

  unsigned int width = 640u;
  unsigned int height = 480u;
  camera->SetImageWidth(width);
  camera->SetImageHeight(height);

  // Heightmap data
  auto heightImage = common::joinPaths(TEST_MEDIA_PATH, "heightmap_bowl.png");
  math::Vector3d size{100, 100, 10};
  math::Vector3d position{0, 0, 0};
  auto textureImage = common::joinPaths(TEST_MEDIA_PATH, "materials",
      "textures", "texture.png");
  auto normalImage = common::joinPaths(TEST_MEDIA_PATH, "materials",
      "textures", "flat_normal.png");

  auto data = std::make_shared<common::ImageHeightmap>();
  data->Load(heightImage);

  EXPECT_EQ(heightImage, data->Filename());

  HeightmapDescriptor desc;
  desc.SetData(data);
  desc.SetSize(size);
  desc.SetPosition(position);
  desc.SetUseTerrainPaging(true);
  desc.SetSampling(4u);

  HeightmapTexture textureA;
  textureA.SetSize(0.5);
  textureA.SetDiffuse(textureImage);
  textureA.SetNormal(normalImage);
  desc.AddTexture(textureA);

  HeightmapBlend blendA;
  blendA.SetMinHeight(2.0);
  blendA.SetFadeDistance(5.0);
  desc.AddBlend(blendA);

  HeightmapTexture textureB;
  textureB.SetSize(0.5);
  textureB.SetDiffuse(textureImage);
  textureB.SetNormal(normalImage);
  desc.AddTexture(textureB);

  HeightmapBlend blendB;
  blendB.SetMinHeight(4.0);
  blendB.SetFadeDistance(5.0);
  desc.AddBlend(blendB);

  HeightmapTexture textureC;
  textureC.SetSize(0.5);
  textureC.SetDiffuse(textureImage);
  textureC.SetNormal(normalImage);
  desc.AddTexture(textureC);

  auto heightmap = scene->CreateHeightmap(desc);
  ASSERT_NE(nullptr, heightmap);

  // Add to a visual
  auto vis = scene->CreateVisual();
  vis->AddGeometry(heightmap);
  EXPECT_EQ(1u, vis->GeometryCount());
  EXPECT_TRUE(vis->HasGeometry(heightmap));
  EXPECT_EQ(heightmap, vis->GeometryByIndex(0));
  scene->RootVisual()->AddChild(vis);

  // add camera and render one frame
  scene->RootVisual()->AddChild(camera);
  camera->Update();

  const int halfWidth  = static_cast<int>(width / 2);
  const int halfHeight = static_cast<int>(height / 2);
  math::Vector2i centerClick(halfWidth, halfHeight);

  RayQueryPtr rayQuery = scene->CreateRayQuery();
  EXPECT_TRUE(rayQuery != nullptr);

  // screenToScene
  RayQueryResult rayResult;
  math::Vector3d result =
      screenToScene(centerClick, camera, rayQuery, rayResult);
  math::Vector3d expectedPoint(-0.0271169, -0.0271008, 5.00273);

  // Camera should see heightmap point
  EXPECT_NEAR(expectedPoint.Z(), result.Z(), 4e-6);
  EXPECT_NEAR(expectedPoint.X(), result.X(), 2e-6);
  EXPECT_NEAR(expectedPoint.Y(), result.Y(), 2e-6);
  EXPECT_TRUE(rayResult);
  EXPECT_NEAR(cameraPose.Pos().Z() - result.Z() - camera->NearClipPlane(),
      rayResult.distance, 1e-4);
  EXPECT_EQ(expectedPoint, rayResult.point);

  // Clean up
  engine->DestroyScene(scene);
}

/// \brief Build a 2x2 RGB image where pixel i holds (10i, 10i+1, 10i+2), so
/// every byte identifies both its pixel and its channel.
Image MakeRgb2x2()
{
  Image image(2, 2, PF_R8G8B8);
  unsigned char *data = image.Data<unsigned char>();
  for (unsigned int i = 0; i < 4; ++i)
  {
    data[i * 3 + 0] = static_cast<unsigned char>(10 * i);
    data[i * 3 + 1] = static_cast<unsigned char>(10 * i + 1);
    data[i * 3 + 2] = static_cast<unsigned char>(10 * i + 2);
  }
  return image;
}

/////////////////////////////////////////////////
TEST(UtilsTest, ConvertRGBToBayerPatterns)
{
  // Expected 2x2 Bayer output per format, laid out row-major: each entry is
  // the byte from MakeRgb2x2 that the pattern samples at that position.
  const struct
  {
    PixelFormat format;
    std::array<unsigned char, 4> expected;
  } cases[] = {
    {PF_BAYER_RGGB8, {0, 11, 21, 32}},
    {PF_BAYER_BGGR8, {2, 11, 21, 30}},
    {PF_BAYER_GBRG8, {1, 12, 20, 31}},
    {PF_BAYER_GRBG8, {1, 10, 22, 31}},
  };

  const Image rgb = MakeRgb2x2();
  for (const auto &c : cases)
  {
    // Returning overload
    Image bayer = convertRGBToBayer(rgb, c.format);
    EXPECT_EQ(c.format, bayer.Format());
    EXPECT_EQ(4u, bayer.MemorySize());
    for (unsigned int i = 0; i < 4; ++i)
      EXPECT_EQ(c.expected[i], bayer.Data<unsigned char>()[i]) << i;

    // In-place overload
    Image inPlace(2, 2, c.format);
    EXPECT_TRUE(convertRGBToBayer(rgb, PixelBuffer(inPlace)));
    for (unsigned int i = 0; i < 4; ++i)
      EXPECT_EQ(c.expected[i], inPlace.Data<unsigned char>()[i]) << i;
  }
}

/////////////////////////////////////////////////
TEST(UtilsTest, ConvertRGBToBayerExternalBuffer)
{
  // The in-place overload must write into a caller owned buffer.
  const Image rgb = MakeRgb2x2();
  std::vector<unsigned char> buffer(4, 0xFF);
  EXPECT_TRUE(convertRGBToBayer(rgb,
      PixelBuffer(2, 2, PF_BAYER_RGGB8, buffer.data(), buffer.size())));
  EXPECT_EQ(0, buffer[0]);
  EXPECT_EQ(11, buffer[1]);
  EXPECT_EQ(21, buffer[2]);
  EXPECT_EQ(32, buffer[3]);
}

/////////////////////////////////////////////////
TEST(UtilsTest, ConvertRGBToBayerRejectsMismatch)
{
  const Image rgb = MakeRgb2x2();

  // Destination is not a Bayer format
  {
    Image dst(2, 2, PF_L8);
    std::memset(dst.Data(), 0xAB, dst.MemorySize());
    EXPECT_FALSE(convertRGBToBayer(rgb, PixelBuffer(dst)));
    EXPECT_EQ(0xAB, dst.Data<unsigned char>()[0]);
  }

  // Destination dimensions differ
  {
    Image dst(3, 2, PF_BAYER_RGGB8);
    std::memset(dst.Data(), 0xAB, dst.MemorySize());
    EXPECT_FALSE(convertRGBToBayer(rgb, PixelBuffer(dst)));
    EXPECT_EQ(0xAB, dst.Data<unsigned char>()[0]);
  }

  // Destination buffer is too small
  {
    std::vector<unsigned char> buffer(4, 0xAB);
    EXPECT_FALSE(convertRGBToBayer(rgb,
        PixelBuffer(2, 2, PF_BAYER_RGGB8, buffer.data(), 3)));
    EXPECT_EQ(0xAB, buffer[0]);
  }

  // Source is not RGB
  {
    Image src(2, 2, PF_L8);
    Image dst(2, 2, PF_BAYER_RGGB8);
    std::memset(dst.Data(), 0xAB, dst.MemorySize());
    EXPECT_FALSE(convertRGBToBayer(src, PixelBuffer(dst)));
    EXPECT_EQ(0xAB, dst.Data<unsigned char>()[0]);
  }
}
