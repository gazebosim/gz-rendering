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
#include "ignition/rendering/GaussianNoisePass.hh"
#include "ignition/rendering/Image.hh"
#include "ignition/rendering/PixelFormat.hh"
#include "ignition/rendering/RenderEngine.hh"
#include "ignition/rendering/RenderingIface.hh"
#include "ignition/rendering/RenderPassSystem.hh"
#include "ignition/rendering/Scene.hh"

using namespace ignition;
using namespace rendering;

class RenderPassTest: public testing::Test,
                      public testing::WithParamInterface<const char *>
{
  // Test and verify Gaussian noise pass is applied to a camera
  public: void GaussianNoise(const std::string &_renderEngine);
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
TEST_P(RenderPassTest, GaussianNoise)
{
  GaussianNoise(GetParam());
}

INSTANTIATE_TEST_CASE_P(GaussianNoise, RenderPassTest,
    RENDER_ENGINE_VALUES,
    ignition::rendering::PrintToStringParam());

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
