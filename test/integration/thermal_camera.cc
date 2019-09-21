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
#include <ignition/common/Filesystem.hh>
#include <ignition/common/Event.hh>

#include "test_config.h"  // NOLINT(build/include)

#include "ignition/rendering/ThermalCamera.hh"
#include "ignition/rendering/RenderEngine.hh"
#include "ignition/rendering/RenderingIface.hh"
#include "ignition/rendering/Scene.hh"

#define DEPTH_TOL 1e-4
#define DOUBLE_TOL 1e-6

unsigned int g_thermalCounter = 0;

void OnNewThermalFrame(uint16_t *_scanDest, const uint16_t *_scan,
                  unsigned int _width, unsigned int _height,
                  unsigned int _channels,
                  const std::string &/*_format*/)
{
  uint16_t u;
  int size =  _width * _height * _channels;
  memcpy(_scanDest, _scan, size * sizeof(u));
}



class ThermalCameraTest: public testing::Test,
  public testing::WithParamInterface<const char *>
{
  // Create a Camera sensor from a SDF and gets a image message
  public: void ThermalCameraBoxes(const std::string &_renderEngine);
};

void ThermalCameraTest::ThermalCameraBoxes(
    const std::string &_renderEngine)
{
  int imgWidth_ = 10;
  int imgHeight_ = 10;
  double aspectRatio_ = imgWidth_/imgHeight_;

  double unitBoxSize = 1.0;
  ignition::math::Vector3d boxPosition(1.8, 0.0, 0.0);

  // Optix is not supported
  if (_renderEngine.compare("optix") == 0 || _renderEngine.compare("ogre") == 0)
  {
    igndbg << "Engine '" << _renderEngine
              << "' doesn't support thermal cameras" << std::endl;
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

  // create box visual
  ignition::rendering::VisualPtr box = scene->CreateVisual();
  box->AddGeometry(scene->CreateBox());
  box->SetOrigin(0.0, 0.0, 0.0);
  box->SetLocalPosition(boxPosition);
  box->SetLocalRotation(0, 0, 0);
  box->SetLocalScale(unitBoxSize, unitBoxSize, unitBoxSize);
  box->SetUserData("temperature", 310.0);

  root->AddChild(box);
  {
    double farDist = 10.0;
    double nearDist = 0.15;
    double hfov_ = 1.05;
    // Create thermal camera
    auto thermalCamera = scene->CreateThermalCamera("ThermalCamera");
    ASSERT_NE(thermalCamera, nullptr);

    ignition::math::Pose3d testPose(ignition::math::Vector3d(0, 0, 0),
        ignition::math::Quaterniond::Identity);
    thermalCamera->SetLocalPose(testPose);

    // Configure thermal camera
    thermalCamera->SetImageWidth(imgWidth_);
    EXPECT_EQ(thermalCamera->ImageWidth(),
      static_cast<unsigned int>(imgWidth_));
    thermalCamera->SetImageHeight(imgHeight_);
    EXPECT_EQ(thermalCamera->ImageHeight(),
      static_cast<unsigned int>(imgHeight_));
    thermalCamera->SetFarClipPlane(farDist);
    EXPECT_NEAR(thermalCamera->FarClipPlane(), farDist, DOUBLE_TOL);
    thermalCamera->SetNearClipPlane(nearDist);
    EXPECT_NEAR(thermalCamera->NearClipPlane(), nearDist, DOUBLE_TOL);
    thermalCamera->SetAspectRatio(aspectRatio_);
    EXPECT_NEAR(thermalCamera->AspectRatio(), aspectRatio_, DOUBLE_TOL);
    thermalCamera->SetHFOV(hfov_);
    EXPECT_NEAR(thermalCamera->HFOV().Radian(), hfov_, DOUBLE_TOL);

    // thermal-specific params
    // set room temperature: 294 ~ 298 Kelvin
    thermalCamera->SetAmbientTemperature(296.0);
    EXPECT_FLOAT_EQ(296.0, thermalCamera->AmbientTemperature());
    thermalCamera->SetAmbientTemperatureRange(4.0);
    EXPECT_FLOAT_EQ(4.0, thermalCamera->AmbientTemperatureRange());
    thermalCamera->SetLinearResolution(0.01);
    EXPECT_FLOAT_EQ(0.01, thermalCamera->LinearResolution());

//    thermalCamera->CreateThermalTexture();
    scene->RootVisual()->AddChild(thermalCamera);

    // Set a callback on the  camera sensor to get a thermal camera frame
    uint16_t *thermalData = new uint16_t[imgHeight_ * imgWidth_];
    ignition::common::ConnectionPtr connection =
      thermalCamera->ConnectNewThermalFrame(
          std::bind(&::OnNewThermalFrame, thermalData,
            std::placeholders::_1, std::placeholders::_2, std::placeholders::_3,
            std::placeholders::_4, std::placeholders::_5));

    // Update once to create image
    std::cerr << "============== update " << std::endl;
    thermalCamera->Update();
    std::cerr << "============== update done" << std::endl;

   // Clean up
    connection.reset();
    delete [] thermalData;
  }

  engine->DestroyScene(scene);
  ignition::rendering::unloadEngine(engine->Name());
}

TEST_P(ThermalCameraTest, ThermalCameraBoxes)
{
  ThermalCameraBoxes(GetParam());
}

INSTANTIATE_TEST_CASE_P(ThermalCamera, ThermalCameraTest,
    RENDER_ENGINE_VALUES, ignition::rendering::PrintToStringParam());

//////////////////////////////////////////////////
int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
