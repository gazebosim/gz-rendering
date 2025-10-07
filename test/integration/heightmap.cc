/*
 * Copyright (C) 2023 Open Source Robotics Foundation
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

#include "test_config.h"  // NOLINT(build/include)

#include <gz/common/Console.hh>
#include <gz/common/Image.hh>
#include <gz/common/ImageHeightmap.hh>

#include "ignition/rendering/Camera.hh"
#include "ignition/rendering/Heightmap.hh"
#include "ignition/rendering/Image.hh"
#include "ignition/rendering/RenderEngine.hh"
#include "ignition/rendering/RenderingIface.hh"
#include "ignition/rendering/Scene.hh"

using namespace ignition;
using namespace rendering;

class HeightmapTest: public testing::Test,
                     public testing::WithParamInterface<const char *>
{
  // Test and verify semi transparent visuals over heightmap
  public: void TransparencyOverHeightmap(const std::string &_renderEngine);

  /// \brief Path to test media files.
  public: const std::string TEST_MEDIA_PATH{
        common::joinPaths(std::string(PROJECT_SOURCE_PATH),
        "test", "media")};
};

/////////////////////////////////////////////////
void HeightmapTest::TransparencyOverHeightmap(const std::string &_renderEngine)
{
  if (_renderEngine != "ogre2")
  {
    igndbg << "Heightmap test does not work in rendering engine: "
            << _renderEngine << std::endl;
    return;
  }

  // \todo(anyone) test fails on github action (Bionic) but pass on other
  // builds. Need to investigate further.
  // Github action sets the MESA_GL_VERSION_OVERRIDE variable
  // so check for this variable and disable test if it is set.
#ifdef __linux__
    std::string value;
    bool result = common::env("MESA_GL_VERSION_OVERRIDE", value, true);
    if (result && value == "3.3")
    {
      igndbg << "Test is run on machine with software rendering or mesa driver "
             << "Skipping test. " << std::endl;
      return;
    }
#endif

  // Test: the scene consists of red background, semi-transparent green box
  // over blue heightmap
  // check the rgb value of the image at position of the box
  // it should be blending of green box with blue heightmap and no
  // red background

  // create and populate scene
  RenderEngine *engine = rendering::engine(_renderEngine);
  if (!engine)
  {
    igndbg << "Engine '" << _renderEngine
           << "' is not supported" << std::endl;
    return;
  }

  ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);
  scene->SetAmbientLight(1.0, 1.0, 1.0);
  scene->SetBackgroundColor(1.0, 0.0, 0.0);

  VisualPtr root = scene->RootVisual();
  ASSERT_NE(nullptr, root);

  // create  camera
  CameraPtr camera = scene->CreateCamera();
  ASSERT_NE(nullptr, camera);
  camera->SetImageWidth(100);
  camera->SetImageHeight(100);
  camera->SetHFOV(camera->HFOV());
  root->AddChild(camera);

  // create ImageHeightmap
  auto data = std::make_shared<common::ImageHeightmap>();
  data->Load(common::joinPaths(TEST_MEDIA_PATH, "heightmap_bowl.png"));

  HeightmapDescriptor desc;
  desc.SetName("example_bowl");
  desc.SetData(data);
  desc.SetSize({17, 17, 7.0});
  desc.SetSampling(2u);
  desc.SetUseTerrainPaging(false);

  const auto textureImage =
    common::joinPaths(TEST_MEDIA_PATH, "materials", "textures",
    "blue_texture.png");
  const auto normalImage = common::joinPaths(TEST_MEDIA_PATH, "materials",
                                             "textures", "flat_normal.png");
  HeightmapTexture textureA;
  textureA.SetSize(1.0);
  textureA.SetDiffuse(textureImage);
  textureA.SetNormal(normalImage);
  desc.AddTexture(textureA);

  HeightmapBlend blendA;
  blendA.SetMinHeight(2.0);
  blendA.SetFadeDistance(5.0);
  desc.AddBlend(blendA);

  HeightmapTexture textureB;
  textureB.SetSize(1.0);
  textureB.SetDiffuse(textureImage);
  textureB.SetNormal(normalImage);
  desc.AddTexture(textureB);

  HeightmapBlend blendB;
  blendB.SetMinHeight(4.0);
  blendB.SetFadeDistance(5.0);
  desc.AddBlend(blendB);

  HeightmapTexture textureC;
  textureC.SetSize(1.0);
  textureC.SetDiffuse(textureImage);
  textureC.SetNormal(normalImage);
  desc.AddTexture(textureC);

  auto heightmapGeom = scene->CreateHeightmap(desc);

  auto vis = scene->CreateVisual();
  vis->AddGeometry(heightmapGeom);
  root->AddChild(vis);

  // create green material
  MaterialPtr green = scene->CreateMaterial();
  green->SetDiffuse(0.0, 0.7, 0.0);
  green->SetSpecular(0.5, 0.5, 0.5);
  green->SetTransparency(0.5);

  // create box
  VisualPtr box = scene->CreateVisual();
  box->AddGeometry(scene->CreateBox());
  box->SetLocalPosition(0.5, 0.5, 5.5);
  box->Scale(1.0);
  box->SetMaterial(green);
  root->AddChild(box);

  camera->SetLocalPosition(-0.802621, 5.84365, 9.67877);
  camera->SetLocalRotation(0.0, 0.588, -1.125);

  // capture original image with semi-transparent box
  Image image = camera->CreateImage();
  camera->Capture(image);

  auto box2dPos = camera->Project(box->WorldPosition());

  const unsigned char *imgData =
      static_cast<unsigned char *>(image.Data());
  const unsigned int width = camera->ImageWidth();
  const unsigned int channelCount = 3;
  const unsigned int step = width * channelCount;
  const unsigned int idx = box2dPos.Y() * step + box2dPos.X() * channelCount;
  uint8_t r = imgData[idx];
  uint8_t g = imgData[idx+1];
  uint8_t b = imgData[idx+2];

<<<<<<< HEAD
  // rgb value at the image pos of box should be a blend of green (box) and
  // blue (heightmap) and no red (background)
  EXPECT_EQ(0u, r);
  EXPECT_LT(0u, g);
  EXPECT_LT(0u, b);
=======
  g_pointCloudCounter = 0u;
  depthCamera->Update();
  EXPECT_EQ(1u, g_pointCloudCounter);

  {
    int numErrors = 0;
    int numLargeErrors = 0;
    int accumError = 0;

    const unsigned char *normalData =
      static_cast<unsigned char *>(normalCamImage.Data());
    const float *depthData = pointCloudData;
    const unsigned int height = camera->ImageHeight();
    const unsigned int width = camera->ImageWidth();
    const unsigned int channelCount = 4;
    const unsigned int step = width * channelCount;
    const unsigned int normalChannelCount = 3;
    const unsigned int normalStep = width * normalChannelCount;
    for (unsigned int i = 0; i < height; ++i)
    {
      for (unsigned int j = 0; j < width; ++j)
      {
        const unsigned int idx = i * step + j * channelCount;
        const unsigned int normalIdx = i * normalStep + j * normalChannelCount;

        const uint32_t *depthrgba =
          reinterpret_cast<const uint32_t *>(&depthData[idx + 3]);

        const uint8_t depthr = *depthrgba >> 24 & 0xFF;
        const uint8_t depthg = *depthrgba >> 16 & 0xFF;
        const uint8_t depthb = *depthrgba >> 8 & 0xFF;
        // const uint8_t deptha = *depthrgba >> 0 & 0xFF;

        const uint8_t largeError = 5u;

        if (abs(depthr - normalData[normalIdx + 0]) > largeError ||
            abs(depthg - normalData[normalIdx + 1]) > largeError ||
            abs(depthb - normalData[normalIdx + 2]) > largeError)
        {
          const uint8_t error = 10u;
          EXPECT_NEAR(depthr, normalData[normalIdx + 0], error);
          EXPECT_NEAR(depthg, normalData[normalIdx + 1], error);
          EXPECT_NEAR(depthb, normalData[normalIdx + 2], error);
          ++numLargeErrors;
        }
        else
        {
          const uint8_t error = 4u;
          EXPECT_NEAR(depthr, normalData[normalIdx + 0], error);
          EXPECT_NEAR(depthg, normalData[normalIdx + 1], error);
          EXPECT_NEAR(depthb, normalData[normalIdx + 2], error);
          // EXPECT_EQ(deptha, normalData[normalIdx + 3]);
        }

        if (depthr != normalData[normalIdx + 0] ||
            depthg != normalData[normalIdx + 1] ||
            depthb != normalData[normalIdx + 2])
        {
          accumError += abs(depthr - normalData[normalIdx + 0]);
          accumError += abs(depthg - normalData[normalIdx + 1]);
          accumError += abs(depthb - normalData[normalIdx + 2]);
          ++numErrors;
        }

        /// Background is red
        const bool isBackgroundNormal = normalData[normalIdx + 0] == 255u &&
                                        normalData[normalIdx + 1] == 0u &&
                                        normalData[normalIdx + 2] == 0u;
        const bool isBackgroundDepth =
          depthr == 255u && depthg == 0u && depthb == 0u;

        EXPECT_EQ(isBackgroundNormal, isBackgroundDepth);

        if (isBackgroundDepth)
        {
          EXPECT_FALSE(std::isnan(depthData[idx + 0]));
          EXPECT_FALSE(std::isnan(depthData[idx + 1]));
          EXPECT_FALSE(std::isnan(depthData[idx + 2]));

          EXPECT_TRUE(std::isinf(depthData[idx + 0]));
          EXPECT_TRUE(std::isinf(depthData[idx + 1]));
          EXPECT_TRUE(std::isinf(depthData[idx + 2]));

          // The sky should only be visible in the top part of the picture
          EXPECT_TRUE(i < height / 4);
        }
        else
        {
          EXPECT_FALSE(std::isnan(depthData[idx + 0]));
          EXPECT_FALSE(std::isnan(depthData[idx + 1]));
          EXPECT_FALSE(std::isnan(depthData[idx + 2]));

          EXPECT_FALSE(std::isinf(depthData[idx + 0]));
          EXPECT_FALSE(std::isinf(depthData[idx + 1]));
          EXPECT_FALSE(std::isinf(depthData[idx + 2]));
        }
      }
    }

    // Expect less than 15 pixels in 10k to be different due to GPU &
    // floating point differences when optimizing shaders
    EXPECT_LE(numErrors, width * height * 15 / 10000);
    // Expect less than an accumulated deviation of 25 per channel (RGB)
    EXPECT_LE(accumError, 25 * 3);
    // Expect very few "large" errors.
    EXPECT_LE(numLargeErrors, width * height * 5 / 10000);

    if (this->HasFailure())
    {
      std::string base64Encoded;

      {
        // Output reference
        base64Encoded.clear();
        const uint32_t header[3] = {
          width, height, static_cast<uint32_t>(normalCamImage.Format())
        };

        Base64Encode(header, sizeof(header), base64Encoded);
        Base64Encode(normalData, width * height * normalChannelCount,
                     base64Encoded);
        std::cout << "Reference Camera Output:" << std::endl;
        std::cout << base64Encoded << std::endl;
      }

      {
        // Output value
        base64Encoded.clear();
        const uint32_t header[3] = { width, height,
                                     static_cast<uint32_t>(PF_FLOAT32_RGBA) };

        std::cout << "Depth Camera Output:" << std::endl;
        Base64Encode(header, sizeof(header), base64Encoded);
        Base64Encode(pointCloudData, width * height * sizeof(float) * 4u,
                     base64Encoded);

        std::cout << base64Encoded << std::endl;
      }
    }
  }

  // cleanup
  connection.reset();
>>>>>>> 7900fc1b (Increase tol for heightmap test (#1179))

  // Clean up
  engine->DestroyScene(scene);
}

/////////////////////////////////////////////////
TEST_P(HeightmapTest, TransparencyOverHeightmap)
{
  TransparencyOverHeightmap(GetParam());
}

INSTANTIATE_TEST_CASE_P(Heightmap, HeightmapTest,
    RENDER_ENGINE_VALUES,
    PrintToStringParam());

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
