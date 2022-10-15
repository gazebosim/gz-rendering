/*
 * Copyright (C) 2021 Open Source Robotics Foundation
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

#include <gz/common/Filesystem.hh>
#include <gz/common/Event.hh>

#include <gz/math/Color.hh>

#include "gz/rendering/Scene.hh"
#include "gz/rendering/WideAngleCamera.hh"

#include <gz/utils/ExtraTestMacros.hh>

using namespace gz;
using namespace rendering;

//////////////////////////////////////////////////
class WideAngleCameraTest: public CommonRenderingTest
{
};

/// \brief mutex for thread safety
std::mutex g_mutex;

/// \brief WideAngle buffer
unsigned char *g_buffer = nullptr;

/// \brief counter of received wideAngle msgs
int g_counter = 0;

//////////////////////////////////////////////////
/// \brief callback to get the wide angle camera image data
void OnNewWideAngleFrame(const unsigned char *_data,
                    unsigned int _width, unsigned int _height,
                    unsigned int _channels,
                    const std::string &/*_format*/)
{
  g_mutex.lock();
  auto bufferSize = _width * _height * _channels;

  if (!g_buffer)
    g_buffer = new unsigned char[bufferSize];

  memcpy(g_buffer, _data, bufferSize);

  g_counter++;
  g_mutex.unlock();
}

//////////////////////////////////////////////////
TEST_F(WideAngleCameraTest, GZ_UTILS_TEST_DISABLED_ON_WIN32(WideAngleCamera))
{
  CHECK_UNSUPPORTED_ENGINE("optix");

  gz::rendering::ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);
  scene->SetAmbientLight(1.0, 1.0, 1.0);
  scene->SetBackgroundColor(0.2, 0.2, 0.2);

  rendering::VisualPtr root = scene->RootVisual();

  unsigned int width = 320u;
  unsigned int height = 240u;

  // Create Wide Angle camera
  auto camera = scene->CreateWideAngleCamera("WideAngleCamera");
  ASSERT_NE(camera, nullptr);

  CameraLens lens;
  lens.SetCustomMappingFunction(1.05, 4.0, AFT_TAN, 1.0, 0.0);
  lens.SetType(MFT_CUSTOM);
  lens.SetCutOffAngle(GZ_PI);

  camera->SetLens(lens);
  camera->SetHFOV(2.6);
  camera->SetImageWidth(width);
  camera->SetImageHeight(height);
  camera->SetAspectRatio(1.333);
  camera->SetLocalPosition(0.0, 0.0, 0.0);
  camera->SetLocalRotation(0.0, 0.0, 0.0);
  scene->RootVisual()->AddChild(camera);

  // create regular camera for comparison
  CameraPtr cameraRegular = scene->CreateCamera();
  ASSERT_NE(nullptr, camera);
  cameraRegular->SetImageWidth(width);
  cameraRegular->SetImageHeight(height);
  cameraRegular->SetAspectRatio(1.333);
  cameraRegular->SetHFOV(2.6);
  cameraRegular->SetLocalPosition(0.0, 0.0, 0.0);
  cameraRegular->SetLocalRotation(0.0, 0.0, 0.0);
  root->AddChild(cameraRegular);

  // create blue material
  MaterialPtr blue = scene->CreateMaterial();
  blue->SetAmbient(0.0, 0.0, 0.3);
  blue->SetDiffuse(0.0, 0.0, 0.8);
  blue->SetSpecular(0.5, 0.5, 0.5);

  // create box visual in front of cameras
  VisualPtr box = scene->CreateVisual();
  box->AddGeometry(scene->CreateBox());
  box->SetOrigin(0.0, 0.0, 0.0);
  box->SetLocalPosition(2, 0, 0);
  box->SetLocalScale(1, 1, 1);
  box->SetMaterial(blue);
  root->AddChild(box);

  // capture original image
  Image imageRegular = cameraRegular->CreateImage();
  cameraRegular->Capture(imageRegular);
  unsigned char *dataRegular = imageRegular.Data<unsigned char>();

  // Set a callback on the  camera sensor to get a wide angle camera frame
  gz::common::ConnectionPtr connection =
      camera->ConnectNewWideAngleFrame(
          std::bind(OnNewWideAngleFrame,
          std::placeholders::_1, std::placeholders::_2, std::placeholders::_3,
          std::placeholders::_4, std::placeholders::_5));
  ASSERT_NE(nullptr, connection);

  // Update once to create image
  camera->Update();
  EXPECT_EQ(1, g_counter);

  // Compare image pixels
  unsigned int channelCount = PixelUtil::ChannelCount(camera->ImageFormat());
  unsigned int step = width * channelCount;

  // verify both cameras can see the blue box in the middle
  unsigned int mid = static_cast<unsigned int>(
     height / 2.0 * step + step / 2.0);
  unsigned int r = g_buffer[mid];
  unsigned int g = g_buffer[mid + 1];
  unsigned int b = g_buffer[mid + 2];
  EXPECT_GT(b, g);
  EXPECT_GT(b, r);

  r = dataRegular[mid];
  g = dataRegular[mid + 1];
  b = dataRegular[mid + 2];
  EXPECT_GT(b, g);
  EXPECT_GT(b, r);

  // get sum of pixel colors in both wide angle camera and regular camera images
  unsigned int rSum = 0u;
  unsigned int gSum = 0u;
  unsigned int bSum = 0u;
  unsigned int rRegularSum = 0u;
  unsigned int gRegularSum = 0u;
  unsigned int bRegularSum = 0u;

  for (unsigned int i = 0; i < height; ++i)
  {
    for (unsigned int j = 0; j < step; j += channelCount)
    {
      unsigned int idx = i * step + j;
      rSum += g_buffer[idx];
      gSum += g_buffer[idx + 1];
      bSum += g_buffer[idx + 2];

      rRegularSum += dataRegular[idx];
      gRegularSum += dataRegular[idx + 1];
      bRegularSum += dataRegular[idx + 2];
    }
  }

  // wide angle camera image should not be black
  EXPECT_GT(rSum, 0u);
  EXPECT_GT(gSum, 0u);
  EXPECT_GT(bSum, 0u);

  // sum of regular camera image should be brighter than wide angle camera image
  // as there are more background visible
  EXPECT_GT(rRegularSum, rSum);
  EXPECT_GT(gRegularSum, rSum);
  EXPECT_GT(bRegularSum, rSum);

  // The wide angle camera should have more blue pixels than regular camera
  // because the box is larger in the image due to distortion
  EXPECT_GT(bSum, rRegularSum);

  // Clean up
  engine->DestroyScene(scene);
}

//////////////////////////////////////////////////
TEST_F(WideAngleCameraTest, GZ_UTILS_TEST_DISABLED_ON_WIN32(Projection))
{
  CHECK_UNSUPPORTED_ENGINE("optix");

  gz::rendering::ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);
  scene->SetAmbientLight(1.0, 1.0, 1.0);
  scene->SetBackgroundColor(0.2, 0.2, 0.2);

  rendering::VisualPtr root = scene->RootVisual();

  unsigned int width = 320u;
  unsigned int height = 240u;
  // Create Wide Angle camera
  auto camera = scene->CreateWideAngleCamera("WideAngleCamera");
  ASSERT_NE(camera, nullptr);

  CameraLens lens;
  lens.SetCustomMappingFunction(1.05, 4.0, AFT_TAN, 1.0, 0.0);
  lens.SetType(MFT_CUSTOM);
  lens.SetCutOffAngle(GZ_PI);

  camera->SetLens(lens);
  camera->SetHFOV(2.6);
  camera->SetImageWidth(width);
  camera->SetImageHeight(height);
  camera->SetAspectRatio(1.333);
  camera->SetLocalPosition(0.0, 0.0, 0.0);
  camera->SetLocalRotation(0.0, 0.0, 0.0);
  scene->RootVisual()->AddChild(camera);

  camera->Update();

  // point directly in front of camera
  auto worldPoint = gz::math::Vector3d::UnitX;
  auto screenPt = camera->Project3d(worldPoint);
  EXPECT_FLOAT_EQ(camera->ImageWidth() * 0.5, screenPt.X());
  EXPECT_FLOAT_EQ(camera->ImageHeight() * 0.5, screenPt.Y());
  EXPECT_GT(screenPt.Z(), 0.0);
  EXPECT_LT(screenPt.Z(), 1.0);

  // point behind camera
  worldPoint = -gz::math::Vector3d::UnitX;
  screenPt = camera->Project3d(worldPoint);
  // z is distance of point from image center
  // in this case it'll be outside of image so greater than 1.0
  EXPECT_GT(screenPt.Z(), 1.0);

  // point at right side of camera image
  worldPoint = gz::math::Vector3d(1, -0.5, 0.0);
  screenPt = camera->Project3d(worldPoint);
  EXPECT_GT(screenPt.X(), camera->ImageWidth() * 0.5);
  EXPECT_FLOAT_EQ(camera->ImageHeight() * 0.5, screenPt.Y());
  EXPECT_GT(screenPt.Z(), 0.0);
  EXPECT_LT(screenPt.Z(), 1.0);

  // point at left side of camera image
  worldPoint = gz::math::Vector3d(1, 0.5, 0.0);
  screenPt = camera->Project3d(worldPoint);
  EXPECT_LT(screenPt.X(), camera->ImageWidth() * 0.5);
  EXPECT_FLOAT_EQ(camera->ImageHeight() * 0.5, screenPt.Y());
  EXPECT_GT(screenPt.Z(), 0.0);
  EXPECT_LT(screenPt.Z(), 1.0);

  // point at top half of camera image
  worldPoint = gz::math::Vector3d(1, 0.0, 0.5);
  screenPt = camera->Project3d(worldPoint);
  EXPECT_FLOAT_EQ(camera->ImageWidth() * 0.5, screenPt.X());
  EXPECT_LT(screenPt.Y(), camera->ImageHeight() * 0.5);
  EXPECT_GT(screenPt.Z(), 0.0);
  EXPECT_LT(screenPt.Z(), 1.0);

  // point at bottom half of camera image
  worldPoint = gz::math::Vector3d(1, 0.0, -0.5);
  screenPt = camera->Project3d(worldPoint);
  EXPECT_FLOAT_EQ(camera->ImageWidth() * 0.5, screenPt.X());
  EXPECT_GT(screenPt.Y(), camera->ImageHeight() * 0.5);
  EXPECT_GT(screenPt.Z(), 0.0);
  EXPECT_LT(screenPt.Z(), 1.0);

  // point at top left quadrant of camera image
  worldPoint = gz::math::Vector3d(1, 0.5, 0.5);
  screenPt = camera->Project3d(worldPoint);
  EXPECT_LT(screenPt.X(), camera->ImageWidth() * 0.5);
  EXPECT_LT(screenPt.Y(), camera->ImageHeight() * 0.5);
  EXPECT_GT(screenPt.Z(), 0.0);
  EXPECT_LT(screenPt.Z(), 1.0);

  // point at top right quadrant of camera image
  worldPoint = gz::math::Vector3d(1, -0.5, 0.5);
  screenPt = camera->Project3d(worldPoint);
  EXPECT_GT(screenPt.X(), camera->ImageWidth() * 0.5);
  EXPECT_LT(screenPt.Y(), camera->ImageHeight() * 0.5);
  EXPECT_GT(screenPt.Z(), 0.0);
  EXPECT_LT(screenPt.Z(), 1.0);

  // point at bottom left quadrant of camera image
  worldPoint = gz::math::Vector3d(1, 0.5, -0.5);
  screenPt = camera->Project3d(worldPoint);
  EXPECT_LT(screenPt.X(), camera->ImageWidth() * 0.5);
  EXPECT_GT(screenPt.Y(), camera->ImageHeight() * 0.5);
  EXPECT_GT(screenPt.Z(), 0.0);
  EXPECT_LT(screenPt.Z(), 1.0);

  // point at bottom right quadrant of camera image
  worldPoint = gz::math::Vector3d(1, -0.5, -0.5);
  screenPt = camera->Project3d(worldPoint);
  EXPECT_GT(screenPt.X(), camera->ImageWidth() * 0.5);
  EXPECT_GT(screenPt.Y(), camera->ImageHeight() * 0.5);
  EXPECT_GT(screenPt.Z(), 0.0);
  EXPECT_LT(screenPt.Z(), 1.0);

  // Clean up
  engine->DestroyScene(scene);

  ASSERT_EQ(1u, camera.use_count());
}
