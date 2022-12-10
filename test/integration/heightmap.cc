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

#include "CommonRenderingTest.hh"

#include <gz/common/Image.hh>
#include <gz/common/geospatial/ImageHeightmap.hh>

#include "gz/rendering/Camera.hh"
#include "gz/rendering/DepthCamera.hh"
#include "gz/rendering/GpuRays.hh"
#include "gz/rendering/Heightmap.hh"
#include "gz/rendering/Image.hh"
#include "gz/rendering/PixelFormat.hh"
#include "gz/rendering/Scene.hh"

#include <gz/utils/ExtraTestMacros.hh>

#define DOUBLE_TOL 1e-6
unsigned int g_pointCloudCounter = 0;

/////////////////////////////////////////////////
static void OnNewRgbPointCloud(float *_scanDest, const float *_scan,
                               unsigned int _width, unsigned int _height,
                               unsigned int _channels,
                               const std::string & /*_format*/)
{
  float f;
  int size = _width * _height * _channels;
  memcpy(_scanDest, _scan, size * sizeof(f));
  g_pointCloudCounter++;
}

static void OnNewGpuRaysFrame(float *_scanDest, const float *_scan,
                              unsigned int _width, unsigned int _height,
                              unsigned int _channels,
                              const std::string & /*_format*/)
{
  float f;
  int size = _width * _height * _channels;
  memcpy(_scanDest, _scan, size * sizeof(f));
}

using namespace gz;
using namespace rendering;

/////////////////////////////////////////////////
class HeightmapTest : public CommonRenderingTest
{
  /// \brief Path to test media files.
public:
  const std::string TEST_MEDIA_PATH{ common::joinPaths(
    std::string(PROJECT_SOURCE_PATH), "test", "media") };
};

/////////////////////////////////////////////////
TEST_F(HeightmapTest, GZ_UTILS_TEST_DISABLED_ON_WIN32(Heightmap))
{
  // add resources in build dir
  engine->AddResourcePath(
    common::joinPaths(std::string(PROJECT_BUILD_PATH), "src"));

  ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);
  scene->SetAmbientLight(0.3, 0.3, 0.3);
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

  // create directional light
  DirectionalLightPtr light = scene->CreateDirectionalLight();
  light->SetDirection(-0.5, -0.5, -1);
  light->SetDiffuseColor(0.9, 0.9, 0.9);
  light->SetSpecularColor(0.9, 0.9, 0.9);
  root->AddChild(light);

  // create ImageHeightmap
  auto data = std::make_shared<common::ImageHeightmap>();
  data->Load(common::joinPaths(TEST_MEDIA_PATH, "heightmap_bowl.png"));

  HeightmapDescriptor desc;
  desc.SetName("example_bowl");
  desc.SetData(data);
  desc.SetSize({ 17, 17, 7.0 });
  desc.SetSampling(2u);
  desc.SetUseTerrainPaging(false);

  const auto textureImage =
    common::joinPaths(TEST_MEDIA_PATH, "materials", "textures", "texture.png");
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

  // create box
  VisualPtr box = scene->CreateVisual();
  box->AddGeometry(scene->CreateBox());
  box->SetLocalPosition(0.5, 0.5, 5.5);
  box->Scale(1.0);
  box->SetMaterial(green);
  root->AddChild(box);

  camera->SetLocalPosition(-0.802621, 5.84365, 9.67877);
  camera->SetLocalRotation(0.0, 0.588, -1.125);

  DepthCameraPtr depthCamera = scene->CreateDepthCamera();
  depthCamera->SetImageWidth(camera->ImageWidth());
  depthCamera->SetImageHeight(camera->ImageHeight());
  depthCamera->SetHFOV(camera->HFOV());
  depthCamera->SetNearClipPlane(camera->NearClipPlane());
  depthCamera->SetFarClipPlane(camera->FarClipPlane());
  depthCamera->CreateDepthTexture();
  depthCamera->SetLocalPosition(camera->LocalPosition());
  depthCamera->SetLocalRotation(camera->LocalRotation());
  root->AddChild(depthCamera);

  unsigned int pointCloudChannelCount = 4u;
  float *pointCloudData =
    new float[depthCamera->ImageHeight() * depthCamera->ImageWidth() *
              pointCloudChannelCount];
  common::ConnectionPtr connection = depthCamera->ConnectNewRgbPointCloud(
    std::bind(&::OnNewRgbPointCloud, pointCloudData, std::placeholders::_1,
              std::placeholders::_2, std::placeholders::_3,
              std::placeholders::_4, std::placeholders::_5));

  // capture original image with box (no noise)
  Image normalCamImage = camera->CreateImage();
  camera->Capture(normalCamImage);

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
          const uint8_t error = 9u;
          EXPECT_NEAR(depthr, normalData[normalIdx + 0], error);
          EXPECT_NEAR(depthg, normalData[normalIdx + 1], error);
          EXPECT_NEAR(depthb, normalData[normalIdx + 2], error);
          ++numLargeErrors;
        }
        else
        {
          const uint8_t error = 3u;
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
          EXPECT_TRUE(depthData[idx + 0] ==
                      std::numeric_limits<float>::infinity());
          EXPECT_TRUE(depthData[idx + 1] ==
                      std::numeric_limits<float>::infinity());
          EXPECT_TRUE(depthData[idx + 2] ==
                      std::numeric_limits<float>::infinity());

          // The sky should only be visible in the top part of the picture
          EXPECT_TRUE(i < height / 4);
        }
        else
        {
          EXPECT_FALSE(std::isnan(depthData[idx + 0]));
          EXPECT_FALSE(std::isnan(depthData[idx + 1]));
          EXPECT_FALSE(std::isnan(depthData[idx + 2]));
          EXPECT_FALSE(depthData[idx + 0] ==
                       std::numeric_limits<float>::infinity());
          EXPECT_FALSE(depthData[idx + 1] ==
                       std::numeric_limits<float>::infinity());
          EXPECT_FALSE(depthData[idx + 2] ==
                       std::numeric_limits<float>::infinity());
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
  }

  // cleanup
  connection.reset();

  // Clean up
  engine->DestroyScene(scene);
}

/////////////////////////////////////////////////
TEST_F(HeightmapTest, GZ_UTILS_TEST_DISABLED_ON_WIN32(HeightmapGpuRays))
{
  // Test GPU rays heightmap detection
  const double hMinAngle = -GZ_PI / 8.0;
  const double hMaxAngle = GZ_PI / 8.0;
  const double minRange = 1.0;
  const double maxRange = 100.0;
  const int hRayCount = 20;
  const int vRayCount = 1;

  ScenePtr scene = engine->CreateScene("scene");
  ASSERT_TRUE(scene != nullptr);

  VisualPtr root = scene->RootVisual();

  // Create ray caster oriented to look down at the heightmap
  math::Pose3d testPose(math::Vector3d(0, 0, 20),
                        math::Quaterniond(math::Vector3d(0, GZ_PI / 2, 0)));

  GpuRaysPtr gpuRays = scene->CreateGpuRays("gpu_rays_1");
  gpuRays->SetWorldPosition(testPose.Pos());
  gpuRays->SetWorldRotation(testPose.Rot());
  gpuRays->SetNearClipPlane(minRange);
  gpuRays->SetFarClipPlane(maxRange);
  gpuRays->SetAngleMin(hMinAngle);
  gpuRays->SetAngleMax(hMaxAngle);
  gpuRays->SetRayCount(hRayCount);
  // set visibility mask
  // note this is not the same as GZ_VISIBILITY_MASK
  // which is 0x0FFFFFFF
  gpuRays->SetVisibilityMask(0xFFFFFFFF);

  gpuRays->SetVerticalRayCount(vRayCount);
  root->AddChild(gpuRays);

  // create heightmap

  // Heightmap data
  auto heightImage = common::joinPaths(TEST_MEDIA_PATH, "heightmap_bowl.png");
  math::Vector3d size{ 100, 100, 10 };
  math::Vector3d position{ 0, 0, 0 };
  auto textureImage =
    common::joinPaths(TEST_MEDIA_PATH, "materials", "textures", "texture.png");
  auto normalImage = common::joinPaths(TEST_MEDIA_PATH, "materials", "textures",
                                       "flat_normal.png");

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

  // Verify rays caster range readings
  // listen to new gpu rays frames
  unsigned int channels = gpuRays->Channels();
  float *scan = new float[hRayCount * vRayCount * channels];
  common::ConnectionPtr connection = gpuRays->ConnectNewGpuRaysFrame(std::bind(
    &::OnNewGpuRaysFrame, scan, std::placeholders::_1, std::placeholders::_2,
    std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));

  scene->SetTime(scene->Time() + std::chrono::milliseconds(16));
  gpuRays->Update();

  for (unsigned int i = 0; i < hRayCount * channels; i += channels)
  {
    // range readings should not be inf and far lower than the max range
    // it should be between ~15m and 20m
    double range = scan[i];
    EXPECT_LT(14.9, range);
    EXPECT_GT(20.0, range);
  }

  // cleanup
  connection.reset();

  delete[] scan;
  scan = nullptr;

  // Clean up
  engine->DestroyScene(scene);
}