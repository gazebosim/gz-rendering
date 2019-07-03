/*
 * Copyright (C) 2018 Open Source Robotics Foundation
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

#include "ignition/rendering/DepthCamera.hh"
#include "ignition/rendering/RenderEngine.hh"
#include "ignition/rendering/RenderingIface.hh"
#include "ignition/rendering/Scene.hh"

#define DEPTH_TOL 1e-4
#define DOUBLE_TOL 1e-6

unsigned int g_depthCounter = 0;
float *g_depthBuffer = nullptr;

void OnNewDepthFrame(float *_scanDest, const float *_scan,
                  unsigned int _width, unsigned int _height,
                  unsigned int _channels,
                  const std::string &/*_format*/)
{
  float f;
  int size =  _width * _height * _channels;
  memcpy(_scanDest, _scan, size * sizeof(f));
}

class DepthCameraTest: public testing::Test,
  public testing::WithParamInterface<const char *>
{
  // Create a Camera sensor from a SDF and gets a image message
  public: void DepthCameraBoxes(const std::string &_renderEngine);
};

void DepthCameraTest::DepthCameraBoxes(
    const std::string &_renderEngine)
{
  int imgWidth_ = 256;
  int imgHeight_ = 256;
  double aspectRatio_ = imgWidth_/imgHeight_;

  double unitBoxSize = 1.0;
  ignition::math::Vector3d boxPosition(1.5, 0.0, 0.0);

  // Optix is not supported
  if (_renderEngine.compare("optix") == 0)
  {
    igndbg << "Engine '" << _renderEngine
              << "' doesn't support depth cameras" << std::endl;
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

  // Create an scene with a box in it
  scene->SetAmbientLight(0.3, 0.3, 0.3);
  ignition::rendering::VisualPtr root = scene->RootVisual();

  // create blue material
  ignition::rendering::MaterialPtr blue = scene->CreateMaterial();
  blue->SetAmbient(0.0, 0.0, 0.3);
  blue->SetDiffuse(0.0, 0.0, 0.8);
  blue->SetSpecular(0.5, 0.5, 0.5);
  blue->SetShininess(50);
  blue->SetReflectivity(0);

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
    double far_ = 10.0;
    double near_ = 0.15;
    double hfov_ = 1.05;
    // Create depth camera
    auto depthCamera = scene->CreateDepthCamera("DepthCamera");
    ASSERT_NE(depthCamera, nullptr);

    ignition::math::Pose3d testPose(ignition::math::Vector3d(0, 0, 0),
        ignition::math::Quaterniond::Identity);
    depthCamera->SetLocalPose(testPose);

    // Configure depth camera
    depthCamera->SetImageWidth(imgWidth_);
    EXPECT_EQ(depthCamera->ImageWidth(),
      static_cast<unsigned int>(imgWidth_));
    depthCamera->SetImageHeight(imgHeight_);
    EXPECT_EQ(depthCamera->ImageHeight(),
      static_cast<unsigned int>(imgHeight_));
    depthCamera->SetFarClipPlane(far_);
    EXPECT_NEAR(depthCamera->FarClipPlane(), far_, DOUBLE_TOL);
    depthCamera->SetNearClipPlane(near_);
    EXPECT_NEAR(depthCamera->NearClipPlane(), near_, DOUBLE_TOL);
    depthCamera->SetAspectRatio(aspectRatio_);
    EXPECT_NEAR(depthCamera->AspectRatio(), aspectRatio_, DOUBLE_TOL);
    depthCamera->SetHFOV(hfov_);
    EXPECT_NEAR(depthCamera->HFOV().Radian(), hfov_, DOUBLE_TOL);

    depthCamera->SetImageFormat(ignition::rendering::PF_FLOAT32_R);
    depthCamera->SetAntiAliasing(2);
    depthCamera->CreateDepthTexture();
    scene->RootVisual()->AddChild(depthCamera);

    // Set a callback on the  camera sensor to get a depth camera frame
    float *scan = new float[imgHeight_ * imgWidth_];
    ignition::common::ConnectionPtr connection =
      depthCamera->ConnectNewDepthFrame(
          std::bind(&::OnNewDepthFrame, scan,
            std::placeholders::_1, std::placeholders::_2, std::placeholders::_3,
            std::placeholders::_4, std::placeholders::_5));

    // Update once to create image
    depthCamera->Update();

    int midWidth = depthCamera->ImageWidth() * 0.5;
    int midHeight = depthCamera->ImageHeight() * 0.5;
    int mid = midHeight * depthCamera->ImageWidth() + midWidth -1;
    double expectedRangeAtMidPoint = boxPosition.X() - unitBoxSize * 0.5;

    // Depth sensor should see box in the middle of the image
    EXPECT_NEAR(scan[mid], expectedRangeAtMidPoint, DEPTH_TOL);

    // The left and right side of the depth frame should be far value
    int left = midHeight * depthCamera->ImageWidth();
    EXPECT_DOUBLE_EQ(scan[left], far_);
    int right = (midHeight+1) * depthCamera->ImageWidth() - 1;
    EXPECT_DOUBLE_EQ(scan[right], far_);

    // Check that for a box really close it returns far (it is not seen)
    ignition::math::Vector3d boxPositionNear(
        unitBoxSize * 0.5 + near_ * 0.5, 0.0, 0.0);
    box->SetLocalPosition(boxPositionNear);
    depthCamera->Update();
    EXPECT_DOUBLE_EQ(scan[mid], far_);

    // Check that for a box really far it returns far
    ignition::math::Vector3d boxPositionFar(
        unitBoxSize * 0.5 + far_ * 1.5, 0.0, 0.0);
    box->SetLocalPosition(boxPositionFar);
    depthCamera->Update();
    EXPECT_DOUBLE_EQ(scan[mid], far_);

    // Clean up
    connection.reset();
  }
  engine->DestroyScene(scene);
  ignition::rendering::unloadEngine(engine->Name());
}

TEST_P(DepthCameraTest, DepthCameraBoxes)
{
  DepthCameraBoxes(GetParam());
}

INSTANTIATE_TEST_CASE_P(DepthCamera, DepthCameraTest,
    RENDER_ENGINE_VALUES, ignition::rendering::PrintToStringParam());

//////////////////////////////////////////////////
int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
