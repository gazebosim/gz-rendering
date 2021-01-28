/*
 * Copyright (C) 2019 Open Source Robotics Foundation
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
#include <ignition/common/Image.hh>

#include "test_config.h"  // NOLINT(build/include)

#include "ignition/rendering/Camera.hh"
#include "ignition/rendering/DepthCamera.hh"
#include "ignition/rendering/GaussianNoisePass.hh"
#include "ignition/rendering/Image.hh"
#include "ignition/rendering/PixelFormat.hh"
#include "ignition/rendering/RenderEngine.hh"
#include "ignition/rendering/RenderingIface.hh"
#include "ignition/rendering/RenderPassSystem.hh"
#include "ignition/rendering/Scene.hh"

#define DOUBLE_TOL 1e-6
unsigned int g_pointCloudCounter = 0;

void OnNewRgbPointCloud(float *_scanDest, const float *_scan,
                  unsigned int _width, unsigned int _height,
                  unsigned int _channels,
                  const std::string &/*_format*/)
{
  float f;
  int size =  _width * _height * _channels;
  memcpy(_scanDest, _scan, size * sizeof(f));
  g_pointCloudCounter++;
}
using namespace ignition;
using namespace rendering;

class RenderPassTest: public testing::Test,
                      public testing::WithParamInterface<const char *>
{
  // Test and verify Gaussian noise pass is applied to a camera
  public: void GaussianNoise(const std::string &_renderEngine);

  // Test and verify Gaussian noise pass is applied to a depth camera
  public: void DepthGaussianNoise(const std::string &_renderEngine);
};

/////////////////////////////////////////////////
void RenderPassTest::GaussianNoise(const std::string &_renderEngine)
{
  // create and populate scene
  RenderEngine *engine = rendering::engine(_renderEngine);
  if (!engine)
  {
    igndbg << "Engine '" << _renderEngine
              << "' is not supported" << std::endl;
    return;
  }

  // add resources in build dir
  engine->AddResourcePath(
      common::joinPaths(std::string(PROJECT_BUILD_PATH), "src"));

  ScenePtr scene = engine->CreateScene("scene");
  ASSERT_TRUE(scene != nullptr);
  scene->SetAmbientLight(0.3, 0.3, 0.3);

  VisualPtr root = scene->RootVisual();

  // create  camera
  CameraPtr camera = scene->CreateCamera();
  ASSERT_TRUE(camera != nullptr);
  camera->SetImageWidth(100);
  camera->SetImageHeight(100);
  root->AddChild(camera);

  // create directional light
  DirectionalLightPtr light = scene->CreateDirectionalLight();
  light->SetDirection(0.0, 0.0, -1);
  light->SetDiffuseColor(0.5, 0.5, 0.5);
  light->SetSpecularColor(0.5, 0.5, 0.5);
  root->AddChild(light);

  // create green material
  MaterialPtr green = scene->CreateMaterial();
  green->SetDiffuse(0.0, 0.7, 0.0);
  green->SetSpecular(0.5, 0.5, 0.5);

  // create box
  VisualPtr box = scene->CreateVisual();
  box->AddGeometry(scene->CreateBox());
  box->SetLocalPosition(1.0, 0.0, 0.5);
  box->SetMaterial(green);
  root->AddChild(box);

  // capture original image with box (no noise)
  Image image = camera->CreateImage();
  camera->Capture(image);

  // add Gaussian noise render pass to camera
  double noiseMean = 0.1;
  double noiseStdDev = 0.01;
  RenderPassSystemPtr rpSystem = engine->RenderPassSystem();
  if (rpSystem)
  {
    // add gaussian noise pass
    RenderPassPtr pass = rpSystem->Create<GaussianNoisePass>();
    GaussianNoisePassPtr noisePass =
        std::dynamic_pointer_cast<GaussianNoisePass>(pass);
    noisePass->SetMean(noiseMean);
    noisePass->SetStdDev(noiseStdDev);
    camera->AddRenderPass(noisePass);
  }
  else
  {
    ignwarn << "Engine '" << _renderEngine << "' does not support "
            << "render pass  system" << std::endl;
    return;
  }

  // capture image with noise
  Image imageNoise = camera->CreateImage();
  camera->Capture(imageNoise);

  // Compare image pixels
  unsigned int diffMax = 0;
  unsigned int diffSum = 0;
  unsigned char *data = image.Data<unsigned char>();
  unsigned char *dataNoise = imageNoise.Data<unsigned char>();
  unsigned int height = camera->ImageHeight();
  unsigned int width = camera->ImageWidth();
  unsigned int channelCount = PixelUtil::ChannelCount(camera->ImageFormat());
  unsigned int step = width * channelCount;
  for (unsigned int i = 0; i < height; ++i)
  {
    for (unsigned int j = 0; j < step; j += channelCount)
    {
      unsigned int idx = i * step + j;
      for (unsigned int k = 0; k < channelCount; ++k)
      {
        int p = data[idx + k];
        int p2 = dataNoise[idx + k];

        unsigned int absDiff = std::abs(p - p2);
        if (absDiff > diffMax)
          diffMax = absDiff;
        diffSum += absDiff;
      }
    }
  }
  unsigned int diffAvg = diffSum / (width * height * channelCount);

  // We expect that there will be some non-zero difference between the two
  // images.
  EXPECT_NE(diffSum, 0u);
  // We expect that the average difference will be well within 3-sigma.
  EXPECT_NEAR(diffAvg/255., noiseMean, 3*noiseStdDev);

  // Clean up
  engine->DestroyScene(scene);
  rendering::unloadEngine(engine->Name());
}

/////////////////////////////////////////////////
void RenderPassTest::DepthGaussianNoise(const std::string &_renderEngine)
{
  int imgWidth = 10;
  int imgHeight = 10;

  double aspectRatio_ = imgWidth/imgHeight;

  double unitBoxSize = 1.0;
  ignition::math::Vector3d boxPosition(1.8, 0.0, 0.0);

  // Optix is not supported
  if (_renderEngine != "ogre2")
  {
    igndbg << "Engine '" << _renderEngine
           << "' doesn't support render pass for depth cameras " << std::endl;
    return;
  }

  // Setup ign-rendering with an empty scene
  auto *engine = ignition::rendering::engine(_renderEngine);
  if (!engine)
  {
    igndbg << "Engine '" << _renderEngine
              << "' is not supported" << std::endl;
    return;
  }

  ignition::rendering::ScenePtr scene = engine->CreateScene("scene");

  // red background
  scene->SetBackgroundColor(1.0, 0.0, 0.0);

  // Create an scene with a box in it
  scene->SetAmbientLight(1.0, 1.0, 1.0);
  ignition::rendering::VisualPtr root = scene->RootVisual();

  // create blue material
  ignition::rendering::MaterialPtr blue = scene->CreateMaterial();
  blue->SetAmbient(0.0, 0.0, 1.0);
  blue->SetDiffuse(0.0, 0.0, 1.0);
  blue->SetSpecular(0.0, 0.0, 1.0);

  // create box visual
  ignition::rendering::VisualPtr box = scene->CreateVisual();
  box->AddGeometry(scene->CreateBox());
  box->SetOrigin(0.0, 0.0, 0.0);
  box->SetLocalPosition(boxPosition);
  box->SetLocalRotation(0, 0, 0);
  box->SetLocalScale(unitBoxSize, unitBoxSize, unitBoxSize);
  box->SetMaterial(blue);
  root->AddChild(box);
  {
    double farDist = 10.0;
    double nearDist = 0.15;
    double hfov_ = 1.05;
    // Create depth camera
    auto depthCamera = scene->CreateDepthCamera("DepthCamera");
    ASSERT_NE(depthCamera, nullptr);

    ignition::math::Pose3d testPose(ignition::math::Vector3d(0, 0, 0),
        ignition::math::Quaterniond::Identity);
    depthCamera->SetLocalPose(testPose);

    // Configure depth camera
    depthCamera->SetImageWidth(imgWidth);
    EXPECT_EQ(depthCamera->ImageWidth(),
      static_cast<unsigned int>(imgWidth));
    depthCamera->SetImageHeight(imgHeight);
    EXPECT_EQ(depthCamera->ImageHeight(),
      static_cast<unsigned int>(imgHeight));
    depthCamera->SetFarClipPlane(farDist);
    EXPECT_NEAR(depthCamera->FarClipPlane(), farDist, DOUBLE_TOL);
    depthCamera->SetNearClipPlane(nearDist);
    EXPECT_NEAR(depthCamera->NearClipPlane(), nearDist, DOUBLE_TOL);
    depthCamera->SetAspectRatio(aspectRatio_);
    EXPECT_NEAR(depthCamera->AspectRatio(), aspectRatio_, DOUBLE_TOL);
    depthCamera->SetHFOV(hfov_);
    EXPECT_NEAR(depthCamera->HFOV().Radian(), hfov_, DOUBLE_TOL);

    depthCamera->CreateDepthTexture();
    scene->RootVisual()->AddChild(depthCamera);

    // Add Gaussian noise
    double noiseMean = 0.1;
    double noiseStdDev = 0.01;
    RenderPassSystemPtr rpSystem = engine->RenderPassSystem();
    if (rpSystem)
    {
      // add gaussian noise pass
      RenderPassPtr pass = rpSystem->Create<GaussianNoisePass>();
      GaussianNoisePassPtr noisePass =
          std::dynamic_pointer_cast<GaussianNoisePass>(pass);
      noisePass->SetMean(noiseMean);
      noisePass->SetStdDev(noiseStdDev);
      depthCamera->AddRenderPass(noisePass);
    }
    else
    {
      ignwarn << "Engine '" << _renderEngine << "' does not support "
              << "render pass  system" << std::endl;
      return;
    }

    // rgb point cloud data callback
    unsigned int pointCloudChannelCount = 4u;
    float *pointCloudData = new float[
        imgHeight * imgWidth * pointCloudChannelCount];
    ignition::common::ConnectionPtr connection =
      depthCamera->ConnectNewRgbPointCloud(
          std::bind(&::OnNewRgbPointCloud, pointCloudData,
            std::placeholders::_1, std::placeholders::_2, std::placeholders::_3,
            std::placeholders::_4, std::placeholders::_5));

    // update and verify we get new data
    g_pointCloudCounter = 0u;
    depthCamera->Update();
    EXPECT_EQ(1u, g_pointCloudCounter);

    // compute mid, left, and right indices to be used later for retrieving data
    // from point cloud image

    // point cloud image indices
    int midWidth = static_cast<int>(depthCamera->ImageWidth() * 0.5);
    int midHeight = static_cast<int>(depthCamera->ImageHeight() * 0.5);
    double expectedRangeAtMidPoint = boxPosition.X() - unitBoxSize * 0.5;

    int pcMid = midHeight * depthCamera->ImageWidth() * pointCloudChannelCount
        + (midWidth-1) * pointCloudChannelCount;
    int pcLeft = midHeight * depthCamera->ImageWidth() * pointCloudChannelCount;
    int pcRight = (midHeight+1)
        * (depthCamera->ImageWidth() * pointCloudChannelCount)
        - pointCloudChannelCount;

    float maxVal = ignition::math::INF_D;

    // values should be well within 4-sigma
    float noiseTol = 4.0*noiseStdDev;
    unsigned int colorNoiseTol = static_cast<unsigned int>(255.0*noiseTol);

    // Verify Point Cloud XYZ values
    {
      // check mid point
      float mx = pointCloudData[pcMid];
      EXPECT_NEAR(expectedRangeAtMidPoint + noiseMean, mx, noiseTol);

      // check left and right points
      float lx = pointCloudData[pcLeft];
      float ly = pointCloudData[pcLeft + 1];
      float lz = pointCloudData[pcLeft + 2];
      EXPECT_FLOAT_EQ(maxVal, lx);
      EXPECT_FLOAT_EQ(maxVal, ly);
      EXPECT_FLOAT_EQ(maxVal, lz);

      float rx = pointCloudData[pcRight];
      float ry = pointCloudData[pcRight + 1];
      float rz = pointCloudData[pcRight + 2];
      EXPECT_FLOAT_EQ(maxVal, rx);
      EXPECT_FLOAT_EQ(maxVal, ry);
      EXPECT_FLOAT_EQ(maxVal, rz);

      // all points on the box should have similar z position
      float mz = pointCloudData[pcMid + 2];
      float midLeftZ = pointCloudData[pcMid + 2 - pointCloudChannelCount];
      float midRightZ = pointCloudData[pcMid + 2 + pointCloudChannelCount];
      EXPECT_NEAR(mz, midLeftZ, noiseTol);
      EXPECT_NEAR(mz, midRightZ, noiseTol);

      // Verify Point Cloud RGB values
      // The mid point should be blue
      float mc = pointCloudData[pcMid + 3];
      uint32_t *mrgba = reinterpret_cast<uint32_t *>(&mc);
      unsigned int mr = *mrgba >> 24 & 0xFF;
      unsigned int mg = *mrgba >> 16 & 0xFF;
      unsigned int mb = *mrgba >> 8 & 0xFF;
      unsigned int ma = *mrgba >> 0 & 0xFF;
      EXPECT_NEAR(0u, mr, colorNoiseTol);
      EXPECT_NEAR(0u, mg, colorNoiseTol);
      EXPECT_GT(mb, 0u);
      EXPECT_EQ(255u, ma);

      // Far left and right points should be red (background color)
      float lc = pointCloudData[pcLeft + 3];
      uint32_t *lrgba = reinterpret_cast<uint32_t *>(&lc);
      unsigned int lr = *lrgba >> 24 & 0xFF;
      unsigned int lg = *lrgba >> 16 & 0xFF;
      unsigned int lb = *lrgba >> 8 & 0xFF;
      unsigned int la = *lrgba >> 0 & 0xFF;

      EXPECT_NEAR(255u, lr, colorNoiseTol);
      EXPECT_NEAR(0u, lg, colorNoiseTol);
      EXPECT_NEAR(0u, lb, colorNoiseTol);
      EXPECT_EQ(255u, la);

      float rc = pointCloudData[pcRight + 3];
      uint32_t *rrgba = reinterpret_cast<uint32_t *>(&rc);
      unsigned int rr = *rrgba >> 24 & 0xFF;
      unsigned int rg = *rrgba >> 16 & 0xFF;
      unsigned int rb = *rrgba >> 8 & 0xFF;
      unsigned int ra = *rrgba >> 0 & 0xFF;

      EXPECT_NEAR(255u, rr, colorNoiseTol);
      EXPECT_NEAR(0u, rg, colorNoiseTol);
      EXPECT_NEAR(0u, rb, colorNoiseTol);
      EXPECT_EQ(255u, ra);
    }

    // Clean up
    connection.reset();
    if (pointCloudData)
      delete [] pointCloudData;
  }

  engine->DestroyScene(scene);
  ignition::rendering::unloadEngine(engine->Name());
}

/////////////////////////////////////////////////
TEST_P(RenderPassTest, GaussianNoise)
{
  GaussianNoise(GetParam());
}

/////////////////////////////////////////////////
#ifdef __APPLE__
TEST_P(RenderPassTest, DISABLED_DepthGaussianNoise)
#else
TEST_P(RenderPassTest, DepthGaussianNoise)
#endif
{
  DepthGaussianNoise(GetParam());
}

INSTANTIATE_TEST_CASE_P(GaussianNoise, RenderPassTest,
    RENDER_ENGINE_VALUES,
    ignition::rendering::PrintToStringParam());

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
