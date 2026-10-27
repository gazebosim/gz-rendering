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

#include <algorithm>
#include <cmath>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <string>
#include <thread>
#include <vector>

#include "CommonRenderingTest.hh"

#include <gz/common/Filesystem.hh>
#include <gz/common/Event.hh>
#include <gz/math/Rand.hh>

#include "gz/rendering/DepthCamera.hh"
#include "gz/rendering/GaussianNoisePass.hh"
#include "gz/rendering/Material.hh"
#include "gz/rendering/ParticleEmitter.hh"
#include "gz/rendering/RenderPassSystem.hh"
#include "gz/rendering/Scene.hh"

#include <gz/utils/Environment.hh>
#include <gz/utils/ExtraTestMacros.hh>

constexpr double DEPTH_TOL = 1e-4;
constexpr double DOUBLE_TOL = 1e-6;

unsigned int g_depthCounter = 0;
unsigned int g_pointCloudCounter = 0;

/////////////////////////////////////////////////
void OnNewDepthFrame(float *_scanDest, const float *_scan,
                  unsigned int _width, unsigned int _height,
                  unsigned int _channels,
                  const std::string &/*_format*/)
{
  EXPECT_EQ(256u, _width);
  EXPECT_EQ(256u, _height);
  EXPECT_EQ(1u, _channels);

  const auto size = _width * _height * _channels;
  memcpy(_scanDest, _scan, size * sizeof(float));
  g_depthCounter++;
}

/////////////////////////////////////////////////
void OnNewRgbPointCloud(float *_scanDest, const float *_scan,
                  unsigned int _width, unsigned int _height,
                  unsigned int _channels,
                  const std::string &/*_format*/)
{
  EXPECT_EQ(256u, _width);
  EXPECT_EQ(256u, _height);
  EXPECT_EQ(4u, _channels);

  const auto size = _width * _height * _channels;
  memcpy(_scanDest, _scan, size * sizeof(float));
  g_pointCloudCounter++;
}


/////////////////////////////////////////////////
/// \brief A depth camera of the given size and clip planes, HFOV 1.05.
/// The caller sets its point cloud format and render passes, then calls
/// CreateDepthTexture() and adds it to the scene.
gz::rendering::DepthCameraPtr NewDepthCamera(
    const gz::rendering::ScenePtr &_scene, const std::string &_name,
    unsigned int _width, unsigned int _height, double _near, double _far)
{
  gz::rendering::DepthCameraPtr camera = _scene->CreateDepthCamera(_name);
  if (camera)
  {
    camera->SetImageWidth(_width);
    camera->SetImageHeight(_height);
    camera->SetNearClipPlane(_near);
    camera->SetFarClipPlane(_far);
    camera->SetAspectRatio(static_cast<double>(_width) / _height);
    camera->SetHFOV(1.05);
  }
  return camera;
}

/////////////////////////////////////////////////
/// \brief A depth frame or point cloud callback that copies each frame
/// into _dest, which must have the frame's size, and counts the frames in
/// _count.
std::function<void(const float *, unsigned int, unsigned int, unsigned int,
    const std::string &)> CaptureInto(std::vector<float> &_dest, int &_count)
{
  return [&_dest, &_count](const float *_data, unsigned int _w,
      unsigned int _h, unsigned int _c, const std::string &)
  {
    ASSERT_EQ(_dest.size(), static_cast<size_t>(_w) * _h * _c);
    memcpy(_dest.data(), _data, _dest.size() * sizeof(float));
    ++_count;
  };
}

/////////////////////////////////////////////////
/// \brief The x, y, z of an RGBA point cloud, without the colour.
std::vector<float> Xyz(const std::vector<float> &_rgba)
{
  std::vector<float> xyz;
  xyz.reserve(_rgba.size() / 4 * 3);
  for (size_t p = 0; p + 4 <= _rgba.size(); p += 4)
    xyz.insert(xyz.end(), &_rgba[p], &_rgba[p + 3]);
  return xyz;
}

/////////////////////////////////////////////////
/// \brief Adds a unit box for the caller to move, and a small box across
/// a near clip plane of 0.3 (its front face in front of it) at the left
/// edge of the image of a camera at the origin. Returns the unit box.
gz::rendering::VisualPtr AddBoxes(const gz::rendering::ScenePtr &_scene)
{
  gz::rendering::VisualPtr root = _scene->RootVisual();
  gz::rendering::VisualPtr box = _scene->CreateVisual();
  box->AddGeometry(_scene->CreateBox());
  root->AddChild(box);
  gz::rendering::VisualPtr nearBox = _scene->CreateVisual();
  nearBox->AddGeometry(_scene->CreateBox());
  nearBox->SetLocalScale(0.2, 0.2, 0.2);
  nearBox->SetLocalPosition(0.3, 0.12, 0.0);
  root->AddChild(nearBox);
  return box;
}

/////////////////////////////////////////////////
/// \brief Expects values in range, beyond far (+inf) and in front of near
/// (-inf) in _v.
void ExpectFiniteAndInfinities(const std::vector<float> &_v)
{
  EXPECT_TRUE(std::any_of(_v.begin(), _v.end(),
      [](float _x) { return std::isfinite(_x); }));
  EXPECT_TRUE(std::any_of(_v.begin(), _v.end(),
      [](float _x) { return std::isinf(_x) && _x > 0; }));
  EXPECT_TRUE(std::any_of(_v.begin(), _v.end(),
      [](float _x) { return std::isinf(_x) && _x < 0; }));
}

/////////////////////////////////////////////////
class DepthCameraTest: public CommonRenderingTest
{
};

/////////////////////////////////////////////////
TEST_F(DepthCameraTest, GZ_UTILS_TEST_DISABLED_ON_WIN32(DepthCameraBoxes))
{
  int imgWidth_ = 256;
  int imgHeight_ = 256;
  double aspectRatio_ = imgWidth_/imgHeight_;

  double unitBoxSize = 1.0;
  gz::math::Vector3d boxPosition(1.8, 0.0, 0.0);

  gz::rendering::ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);

  // red background
  scene->SetBackgroundColor(1.0, 0.0, 0.0);

  // Create an scene with a box in it
  scene->SetAmbientLight(1.0, 1.0, 1.0);
  gz::rendering::VisualPtr root = scene->RootVisual();

  // create blue material
  gz::rendering::MaterialPtr blue = scene->CreateMaterial();
  blue->SetAmbient(0.0, 0.0, 1.0);
  blue->SetDiffuse(0.0, 0.0, 1.0);
  blue->SetSpecular(0.0, 0.0, 1.0);

  // create box visual
  gz::rendering::VisualPtr box = scene->CreateVisual();
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

    gz::math::Pose3d testPose(gz::math::Vector3d(0, 0, 0),
        gz::math::Quaterniond::Identity);
    depthCamera->SetLocalPose(testPose);

    // Configure depth camera
    depthCamera->SetImageWidth(imgWidth_);
    EXPECT_EQ(depthCamera->ImageWidth(),
      static_cast<unsigned int>(imgWidth_));
    depthCamera->SetImageHeight(imgHeight_);
    EXPECT_EQ(depthCamera->ImageHeight(),
      static_cast<unsigned int>(imgHeight_));
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

    // Set a callback on the  camera sensor to get a depth camera frame
    float *scan = new float[imgHeight_ * imgWidth_];
    gz::common::ConnectionPtr connection =
      depthCamera->ConnectNewDepthFrame(
          std::bind(&::OnNewDepthFrame, scan,
            std::placeholders::_1, std::placeholders::_2, std::placeholders::_3,
            std::placeholders::_4, std::placeholders::_5));

    // rgb point cloud data callback
    unsigned int pointCloudChannelCount = 4u;
    float *pointCloudData = new float[
        imgHeight_ * imgWidth_ * pointCloudChannelCount];
    gz::common::ConnectionPtr connection2 =
      depthCamera->ConnectNewRgbPointCloud(
          std::bind(&::OnNewRgbPointCloud, pointCloudData,
            std::placeholders::_1, std::placeholders::_2, std::placeholders::_3,
            std::placeholders::_4, std::placeholders::_5));

    // update and verify we get new data
    g_depthCounter = 0u;
    g_pointCloudCounter = 0u;
    depthCamera->Update();
    scene->SetTime(scene->Time() + std::chrono::milliseconds(16));
    EXPECT_EQ(1u, g_depthCounter);
    EXPECT_EQ(1u, g_pointCloudCounter);

    // compute mid, left, and right indices to be used later for retrieving data
    // from depth and point cloud image

    // depth image indices
    int midWidth = static_cast<int>(depthCamera->ImageWidth() * 0.5);
    int midHeight = static_cast<int>(depthCamera->ImageHeight() * 0.5);
    int mid = midHeight * depthCamera->ImageWidth() + midWidth -1;
    double expectedRangeAtMidPoint = boxPosition.X() - unitBoxSize * 0.5;
    int left = midHeight * depthCamera->ImageWidth();
    int right = (midHeight+1) * depthCamera->ImageWidth() - 1;

    // point cloud image indices
    int pcMid = midHeight * depthCamera->ImageWidth() * pointCloudChannelCount
        + (midWidth-1) * pointCloudChannelCount;
    int pcLeft = midHeight * depthCamera->ImageWidth() * pointCloudChannelCount;
    int pcRight = (midHeight+1)
        * (depthCamera->ImageWidth() * pointCloudChannelCount)
        - pointCloudChannelCount;

    float minVal = -gz::math::INF_D;
    float maxVal = gz::math::INF_D;

    // Verify Depth
    // Depth sensor should see box in the middle of the image
    EXPECT_NEAR(expectedRangeAtMidPoint, scan[mid], DEPTH_TOL);
    // The left and right side of the depth frame should be max value
    EXPECT_FLOAT_EQ(maxVal, scan[left]);
    EXPECT_FLOAT_EQ(maxVal, scan[right]);

    // Verify Point Cloud XYZ values
    {
      // check mid point
      float mx = pointCloudData[pcMid];
      float my = pointCloudData[pcMid + 1];
      float mz = pointCloudData[pcMid + 2];
      EXPECT_FLOAT_EQ(scan[mid], mx);

      // check left and right points
      float lx = pointCloudData[pcLeft];
      float ly = pointCloudData[pcLeft + 1];
      float lz = pointCloudData[pcLeft + 2];
      EXPECT_FLOAT_EQ(maxVal, ly);
      EXPECT_FLOAT_EQ(maxVal, lz);
      EXPECT_FLOAT_EQ(scan[left], lx);

      float rx = pointCloudData[pcRight];
      float ry = pointCloudData[pcRight + 1];
      float rz = pointCloudData[pcRight + 2];
      EXPECT_FLOAT_EQ(maxVal, rx);
      EXPECT_FLOAT_EQ(maxVal, ry);
      EXPECT_FLOAT_EQ(maxVal, rz);
      EXPECT_FLOAT_EQ(scan[right], rx);

      // point to the left of mid point should have larger y value than mid
      // point, which in turn should have large y value than point to the
      // right of mid point
      float midLeftY = pointCloudData[pcMid + 1 - pointCloudChannelCount];
      float midRightY = pointCloudData[pcMid + 1 + pointCloudChannelCount];
      EXPECT_GT(midLeftY, my);
      EXPECT_GT(my, midRightY);
      EXPECT_GT(midLeftY, 0.0);
      EXPECT_LT(midRightY, 0.0);

      // all points on the box should have the same z position
      float midLeftZ = pointCloudData[pcMid + 2 - pointCloudChannelCount];
      float midRightZ = pointCloudData[pcMid + 2 + pointCloudChannelCount];
      EXPECT_NEAR(mz, midLeftZ, DEPTH_TOL);
      EXPECT_NEAR(mz, midRightZ, DEPTH_TOL);

      // Verify Point Cloud RGB values
      // The mid point should be blue
      float mc = pointCloudData[pcMid + 3];
      uint32_t *mrgba = reinterpret_cast<uint32_t *>(&mc);
      unsigned int mr = *mrgba >> 24 & 0xFF;
      unsigned int mg = *mrgba >> 16 & 0xFF;
      unsigned int mb = *mrgba >> 8 & 0xFF;
      unsigned int ma = *mrgba >> 0 & 0xFF;
      EXPECT_EQ(0u, mr);
      EXPECT_EQ(0u, mg);
      EXPECT_GT(mb, 0u);

      // Far left and right points should be red (background color)
      float lc = pointCloudData[pcLeft + 3];
      uint32_t *lrgba = reinterpret_cast<uint32_t *>(&lc);
      unsigned int lr = *lrgba >> 24 & 0xFF;
      unsigned int lg = *lrgba >> 16 & 0xFF;
      unsigned int lb = *lrgba >> 8 & 0xFF;
      unsigned int la = *lrgba >> 0 & 0xFF;

      EXPECT_EQ(255u, lr);
      EXPECT_EQ(0u, lg);
      EXPECT_EQ(0u, lb);

      float rc = pointCloudData[pcRight + 3];
      uint32_t *rrgba = reinterpret_cast<uint32_t *>(&rc);
      unsigned int rr = *rrgba >> 24 & 0xFF;
      unsigned int rg = *rrgba >> 16 & 0xFF;
      unsigned int rb = *rrgba >> 8 & 0xFF;
      unsigned int ra = *rrgba >> 0 & 0xFF;

      EXPECT_EQ(255u, rr);
      EXPECT_EQ(0u, rg);
      EXPECT_EQ(0u, rb);

      // Note: internal texture format used is RGB with no alpha channel
      // We observed the values can be either 255 or 0 but graphics card
      // drivers are free to fill it with any value they want.
      // This should be fixed in ogre 2.2 in gz-rendering6 which forbids
      // the use of RGB format.
      // see https://github.com/gazebosim/gz-rendering/issues/315
      EXPECT_TRUE(255u == ma || 0u == ma);
      EXPECT_TRUE(255u == la || 0u == la);
      EXPECT_TRUE(255u == ra || 0u == ra);
    }

    // Check that for a box really close it returns it is not seen
    gz::math::Vector3d boxPositionNear(
        unitBoxSize * 0.5 + nearDist * 0.5, 0.0, 0.0);
    box->SetLocalPosition(boxPositionNear);

    // update and verify we get new data
    g_depthCounter = 0u;
    g_pointCloudCounter = 0u;
    depthCamera->Update();
    scene->SetTime(scene->Time() + std::chrono::milliseconds(16));
    EXPECT_EQ(1u, g_depthCounter);
    EXPECT_EQ(1u, g_pointCloudCounter);

    // Verify Depth
    // box not detected
    EXPECT_FLOAT_EQ(minVal, scan[mid]);
    EXPECT_FLOAT_EQ(minVal, scan[left]);
    EXPECT_FLOAT_EQ(minVal, scan[right]);

    // Verify Point Cloud XYZ
    {
      // all points should be min val
      for (unsigned int i = 0; i < depthCamera->ImageHeight(); ++i)
      {
        unsigned int step = i*depthCamera->ImageWidth()*pointCloudChannelCount;
        for (unsigned int j = 0; j < depthCamera->ImageWidth(); ++j)
        {
          float x = pointCloudData[step + j*pointCloudChannelCount];
          float y = pointCloudData[step + j*pointCloudChannelCount + 1];
          float z = pointCloudData[step + j*pointCloudChannelCount + 2];
          EXPECT_FLOAT_EQ(minVal, x);
          EXPECT_FLOAT_EQ(minVal, y);
          EXPECT_FLOAT_EQ(minVal, z);
         }
      }

      // Verify Point Cloud RGB
      // all points should be red (background color)
      for (unsigned int i = 0; i < depthCamera->ImageHeight(); ++i)
      {
        unsigned int step = i*depthCamera->ImageWidth()*pointCloudChannelCount;
        for (unsigned int j = 0; j < depthCamera->ImageWidth(); ++j)
        {
          float color = pointCloudData[step + j*pointCloudChannelCount + 3];
          uint32_t *rgba = reinterpret_cast<uint32_t *>(&color);
          unsigned int r = *rgba >> 24 & 0xFF;
          unsigned int g = *rgba >> 16 & 0xFF;
          unsigned int b = *rgba >> 8 & 0xFF;
          unsigned int a = *rgba >> 0 & 0xFF;
          EXPECT_EQ(255u, r);
          EXPECT_EQ(0u, g);
          EXPECT_EQ(0u, b);
          EXPECT_EQ(255u, a);
        }
      }
    }

    // Check that for a box really far it returns max val
    gz::math::Vector3d boxPositionFar(
        unitBoxSize * 0.5 + farDist * 1.5, 0.0, 0.0);
    box->SetLocalPosition(boxPositionFar);

    // update and verify we get new data
    g_depthCounter = 0u;
    g_pointCloudCounter = 0u;
    depthCamera->Update();
    scene->SetTime(scene->Time() + std::chrono::milliseconds(16));
    EXPECT_EQ(1u, g_depthCounter);
    EXPECT_EQ(1u, g_pointCloudCounter);

    // Verify Depth
    {
      // box not detected so all should return max val
      EXPECT_FLOAT_EQ(maxVal, scan[mid]);
      EXPECT_FLOAT_EQ(maxVal, scan[left]);
      EXPECT_FLOAT_EQ(maxVal, scan[right]);
    }

    // Verify Point Cloud XYZ
    {
      // all points should be maxVal
      for (unsigned int i = 0; i < depthCamera->ImageHeight(); ++i)
      {
        unsigned int step = i*depthCamera->ImageWidth()*pointCloudChannelCount;
        for (unsigned int j = 0; j < depthCamera->ImageWidth(); ++j)
        {
          float x = pointCloudData[step + j*pointCloudChannelCount];
          float y = pointCloudData[step + j*pointCloudChannelCount + 1];
          float z = pointCloudData[step + j*pointCloudChannelCount + 2];
          EXPECT_FLOAT_EQ(maxVal, x);
          EXPECT_FLOAT_EQ(maxVal, y);
          EXPECT_FLOAT_EQ(maxVal, z);
        }
      }

      // Verify Point Cloud RGB
      // all points should be red (background color)
      for (unsigned int i = 0; i < depthCamera->ImageHeight(); ++i)
      {
        unsigned int step = i*depthCamera->ImageWidth()*pointCloudChannelCount;
        for (unsigned int j = 0; j < depthCamera->ImageWidth(); ++j)
        {
          float color = pointCloudData[step + j*pointCloudChannelCount + 3];
          uint32_t *rgba = reinterpret_cast<uint32_t *>(&color);
          unsigned int r = *rgba >> 24 & 0xFF;
          unsigned int g = *rgba >> 16 & 0xFF;
          unsigned int b = *rgba >> 8 & 0xFF;
          unsigned int a = *rgba >> 0 & 0xFF;
          EXPECT_EQ(255u, r);
          EXPECT_EQ(0u, g);
          EXPECT_EQ(0u, b);
          EXPECT_EQ(255u, a);
        }
      }
    }

    // Check that the depth values for a box do not warp.
    gz::math::Vector3d boxPositionFillFrame(
        unitBoxSize * 0.5 + 0.2, 0.0, 0.0);
    box->SetLocalPosition(boxPositionFillFrame);

    // update and verify we get new data
    g_depthCounter = 0u;
    g_pointCloudCounter = 0u;
    depthCamera->Update();
    scene->SetTime(scene->Time() + std::chrono::milliseconds(16));
    EXPECT_EQ(1u, g_depthCounter);
    EXPECT_EQ(1u, g_pointCloudCounter);

    double expectedRange = boxPositionFillFrame.X() - unitBoxSize * 0.5;

    // Verify Depth
    {
      // all points should have the same depth value
      EXPECT_FLOAT_EQ(expectedRange, scan[mid]);
      EXPECT_FLOAT_EQ(expectedRange, scan[left]);
      // Used a slightly higher tol for ogre1.x on arm-based mac
      EXPECT_NEAR(expectedRange, scan[right], 1e-6);
    }
    // Verify Point Cloud XYZ
    {
      // all points should have the same X value
      for (unsigned int i = 0; i < depthCamera->ImageHeight(); ++i)
      {
        unsigned int step = i*depthCamera->ImageWidth()*pointCloudChannelCount;
        for (unsigned int j = 0; j < depthCamera->ImageWidth(); ++j)
        {
          float x = pointCloudData[step + j*pointCloudChannelCount];
          EXPECT_NEAR(expectedRange, x, DOUBLE_TOL);
        }
      }

      // Verify Point Cloud RGB
      // all points should be blue
      for (unsigned int i = 0; i < depthCamera->ImageHeight(); ++i)
      {
        unsigned int step = i*depthCamera->ImageWidth()*pointCloudChannelCount;
        for (unsigned int j = 0; j < depthCamera->ImageWidth(); ++j)
        {
          float color = pointCloudData[step + j*pointCloudChannelCount + 3];
          uint32_t *rgba = reinterpret_cast<uint32_t *>(&color);
          unsigned int r = *rgba >> 24 & 0xFF;
          unsigned int g = *rgba >> 16 & 0xFF;
          unsigned int b = *rgba >> 8 & 0xFF;
          unsigned int a = *rgba >> 0 & 0xFF;
          EXPECT_EQ(0u, r);
          EXPECT_EQ(0u, g);
          EXPECT_GT(b, 0u);
          EXPECT_EQ(255u, a);
        }
      }
    }

    // Clean up
    connection.reset();
    delete [] scan;
    if (pointCloudData)
      delete [] pointCloudData;
  }

  engine->DestroyScene(scene);
}


/////////////////////////////////////////////////
TEST_F(DepthCameraTest, DepthCameraParticles)
{
  // particle emitter is only supported in ogre2
  CHECK_SUPPORTED_ENGINE("ogre2");

  int imgWidth_ = 256;
  int imgHeight_ = 256;
  double aspectRatio_ = imgWidth_ / imgHeight_;

  // box should fill camera view
  // we will add particle emitter in between box and depth camera later
  gz::math::Vector3d boxSize(1.0, 10.0, 10.0);
  gz::math::Vector3d boxPosition(1.8, 0.0, 0.0);

  gz::rendering::ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);

  // red background
  scene->SetBackgroundColor(1.0, 0.0, 0.0);

  // Create an scene with a box in it
  scene->SetAmbientLight(1.0, 1.0, 1.0);
  gz::rendering::VisualPtr root = scene->RootVisual();

  // create blue material
  gz::rendering::MaterialPtr blue = scene->CreateMaterial();
  blue->SetAmbient(0.0, 0.0, 1.0);
  blue->SetDiffuse(0.0, 0.0, 1.0);
  blue->SetSpecular(0.0, 0.0, 1.0);

  // create box visual
  gz::rendering::VisualPtr box = scene->CreateVisual();
  box->AddGeometry(scene->CreateBox());
  box->SetOrigin(0.0, 0.0, 0.0);
  box->SetLocalPosition(boxPosition);
  box->SetLocalRotation(0, 0, 0);
  box->SetLocalScale(boxSize);
  box->SetMaterial(blue);
  root->AddChild(box);
  {
    double farDist = 10.0;
    double nearDist = 0.01;
    double hfov_ = 1.05;
    // Create depth camera
    auto depthCamera = scene->CreateDepthCamera("DepthCamera");
    ASSERT_NE(depthCamera, nullptr);

    gz::math::Pose3d testPose(gz::math::Vector3d(0, 0, 0),
        gz::math::Quaterniond::Identity);
    depthCamera->SetLocalPose(testPose);

    // Configure depth camera
    depthCamera->SetImageWidth(imgWidth_);
    EXPECT_EQ(depthCamera->ImageWidth(),
      static_cast<unsigned int>(imgWidth_));
    depthCamera->SetImageHeight(imgHeight_);
    EXPECT_EQ(depthCamera->ImageHeight(),
      static_cast<unsigned int>(imgHeight_));
    depthCamera->SetFarClipPlane(farDist);
    EXPECT_DOUBLE_EQ(depthCamera->FarClipPlane(), farDist);
    depthCamera->SetNearClipPlane(nearDist);
    EXPECT_DOUBLE_EQ(depthCamera->NearClipPlane(), nearDist);
    depthCamera->SetAspectRatio(aspectRatio_);
    EXPECT_DOUBLE_EQ(depthCamera->AspectRatio(), aspectRatio_);
    depthCamera->SetHFOV(hfov_);
    EXPECT_DOUBLE_EQ(depthCamera->HFOV().Radian(), hfov_);

    depthCamera->CreateDepthTexture();
    scene->RootVisual()->AddChild(depthCamera);

    // Set a callback on the camera sensor to get a depth camera frame
    float *scan = new float[imgHeight_ * imgWidth_];
    gz::common::ConnectionPtr connection =
      depthCamera->ConnectNewDepthFrame(
          std::bind(&::OnNewDepthFrame, scan,
            std::placeholders::_1, std::placeholders::_2, std::placeholders::_3,
            std::placeholders::_4, std::placeholders::_5));

    // rgb point cloud data callback
    unsigned int pointCloudChannelCount = 4u;
    float *pointCloudData = new float[
        imgHeight_ * imgWidth_ * pointCloudChannelCount];
    gz::common::ConnectionPtr connection2 =
      depthCamera->ConnectNewRgbPointCloud(
          std::bind(&::OnNewRgbPointCloud, pointCloudData,
            std::placeholders::_1, std::placeholders::_2, std::placeholders::_3,
            std::placeholders::_4, std::placeholders::_5));

    // update and verify we get new data
    g_depthCounter = 0u;
    g_pointCloudCounter = 0u;
    depthCamera->Update();
    scene->SetTime(scene->Time() + std::chrono::milliseconds(16));
    EXPECT_EQ(1u, g_depthCounter);
    EXPECT_EQ(1u, g_pointCloudCounter);

    double expectedDepth = boxPosition.X() - boxSize.X() * 0.5;

    double pointAvg = 0.0;
    double depthAvg = 0.0;
    // Verify depth and point cloud data before particle effects
    for (unsigned int i = 0u; i < depthCamera->ImageHeight(); ++i)
    {
      unsigned int step =
          i * depthCamera->ImageWidth() * pointCloudChannelCount;
      for (unsigned int j = 0u; j < depthCamera->ImageWidth(); ++j)
      {
        float x = pointCloudData[step + j * pointCloudChannelCount];
        float y = pointCloudData[step + j * pointCloudChannelCount + 1];
        float z = pointCloudData[step + j * pointCloudChannelCount + 2];
        EXPECT_NEAR(expectedDepth, x, DEPTH_TOL);
        float d = scan[i * depthCamera->ImageWidth() + j];
        EXPECT_NEAR(expectedDepth, d, DEPTH_TOL);

        pointAvg += gz::math::Vector3d(x, y, z).Length();
        depthAvg += d;
      }
    }

    // create particle emitter between depth camera and box
    gz::math::Vector3d particlePosition(1.0, 0, 0);
    gz::math::Quaterniond particleRotation(
        gz::math::Vector3d(0, -1.57, 0));
    gz::math::Vector3d particleSize(0.2, 0.2, 0.2);
    gz::rendering::ParticleEmitterPtr emitter =
        scene->CreateParticleEmitter();
    emitter->SetLocalPosition(particlePosition);
    emitter->SetLocalRotation(particleRotation);
    emitter->SetParticleSize(particleSize);
    emitter->SetRate(100);
    emitter->SetLifetime(2);
    emitter->SetVelocityRange(0.1, 0.1);
    emitter->SetScaleRate(0.0);
    emitter->SetColorRange(gz::math::Color::Red,
        gz::math::Color::Black);
    emitter->SetEmitting(true);
    root->AddChild(emitter);

    // update and verify we get new data
    // make sure to update for a few frames for particles for flow into
    // camera view.
    g_depthCounter = 0u;
    g_pointCloudCounter = 0u;
    for (unsigned int i = 0; i < 100; ++i)
    {
      depthCamera->Update();
      scene->SetTime(scene->Time() + std::chrono::milliseconds(16));
    }
    EXPECT_EQ(100u, g_depthCounter);
    EXPECT_EQ(100u, g_pointCloudCounter);

    double pointParticleAvg = 0.0;
    double depthParticleAvg = 0.0;

    // set a larger tol for particle depth
    // depth noise is computed based on particle size
    double depthNoiseTol = particleSize.X() + particleSize.X() * 0.5;
    double expectedParticleDepth = particlePosition.X();

    // Verify depth and point cloud data after particle effects
    for (unsigned int i = 0u; i < depthCamera->ImageHeight(); ++i)
    {
      unsigned int step =
          i * depthCamera->ImageWidth() * pointCloudChannelCount;
      for (unsigned int j = 0u; j < depthCamera->ImageWidth(); ++j)
      {
        float x = pointCloudData[step + j * pointCloudChannelCount];
        float y = pointCloudData[step + j * pointCloudChannelCount + 1];
        float z = pointCloudData[step + j * pointCloudChannelCount + 2];

        double xd = static_cast<double>(x);
        // depth camera sees only certain percentage of particles
        // so the values should be either
        //   * box depth (depth camera does not see particles), or
        //   * noisy particle depth (depth camera see particles but values
        //     are affected by noise)
        EXPECT_TRUE(
            gz::math::equal(expectedParticleDepth, xd, depthNoiseTol) ||
            gz::math::equal(expectedDepth, xd, DEPTH_TOL))
            << "actual vs expected particle depth: "
            << xd << " vs " << expectedParticleDepth;
        float depth = scan[i * depthCamera->ImageWidth() + j];
        double depthd = static_cast<double>(depth);
        EXPECT_TRUE(
            gz::math::equal(expectedParticleDepth, depthd, depthNoiseTol)
            || gz::math::equal(expectedDepth, depthd, DEPTH_TOL))
            << "actual vs expected particle depth: "
            << depthd << " vs " << expectedParticleDepth;

        pointParticleAvg += gz::math::Vector3d(x, y, z).Length();
        depthParticleAvg += depthd;
      }
    }

    // compare point and depth data before and after particle effects
    // the avg point length and depth values in the image with particle effects
    // should be lower than the image without particle effects
    double pixelCount = depthCamera->ImageWidth() * depthCamera->ImageHeight();
    pointAvg /= pixelCount;
    depthAvg /= pixelCount;
    pointParticleAvg /= pixelCount;
    depthParticleAvg /= pixelCount;
    EXPECT_LT(pointParticleAvg, pointAvg);
    EXPECT_LT(depthParticleAvg, depthAvg);

    // test setting particle scatter ratio
    // reduce particle scatter ratio - this creates a "less dense" particle
    // emitter so we should have larger depth values on avg since fewers
    // depth readings are occluded by particles
    emitter->SetParticleScatterRatio(0.1f);

    g_depthCounter = 0u;
    g_pointCloudCounter = 0u;
    for (unsigned int i = 0; i < 100; ++i)
    {
      depthCamera->Update();
      scene->SetTime(scene->Time() + std::chrono::milliseconds(16));
    }
    EXPECT_EQ(100u, g_depthCounter);
    EXPECT_EQ(100u, g_pointCloudCounter);

    double pointParticleLowScatterAvg = 0.0;
    double depthParticleLowScatterAvg = 0.0;

    // Verify depth and point cloud data after setting particle scatter ratio
    for (unsigned int i = 0u; i < depthCamera->ImageHeight(); ++i)
    {
      unsigned int step =
          i * depthCamera->ImageWidth() * pointCloudChannelCount;
      for (unsigned int j = 0u; j < depthCamera->ImageWidth(); ++j)
      {
        float x = pointCloudData[step + j * pointCloudChannelCount];
        float y = pointCloudData[step + j * pointCloudChannelCount + 1];
        float z = pointCloudData[step + j * pointCloudChannelCount + 2];

        double xd = static_cast<double>(x);
        // depth camera sees only certain percentage of particles
        // so the values should be either
        //   * box depth (depth camera does not see particles), or
        //   * noisy particle depth (depth camera see particles but values
        //     are affected by noise)
        EXPECT_TRUE(
            gz::math::equal(expectedParticleDepth, xd, depthNoiseTol) ||
            gz::math::equal(expectedDepth, xd, DEPTH_TOL))
            << "actual vs expected particle depth: "
            << xd << " vs " << expectedParticleDepth;
        float depth = scan[i * depthCamera->ImageWidth() + j];
        double depthd = static_cast<double>(depth);
        EXPECT_TRUE(
            gz::math::equal(expectedParticleDepth, depthd, depthNoiseTol)
            || gz::math::equal(expectedDepth, depthd, DEPTH_TOL))
            << "actual vs expected particle depth: "
            << depthd << " vs " << expectedParticleDepth;

        pointParticleLowScatterAvg +=
            gz::math::Vector3d(x, y, z).Length();
        depthParticleLowScatterAvg += depthd;
      }
    }

    // compare point and depth data before and after setting particle scatter
    // ratio. The avg point length and depth values in the image with low
    // particle scatter ratio should be should be higher than the previous
    // images with particle effects
    pointParticleLowScatterAvg /= pixelCount;
    depthParticleLowScatterAvg /= pixelCount;
    EXPECT_LT(pointParticleAvg, pointParticleLowScatterAvg);
    EXPECT_LT(depthParticleAvg, depthParticleLowScatterAvg);

    // Clean up
    connection.reset();
    delete [] scan;
    if (pointCloudData)
      delete [] pointCloudData;
  }

  engine->DestroyScene(scene);
}

/////////////////////////////////////////////////
TEST_F(DepthCameraTest, GZ_UTILS_TEST_DISABLED_ON_WIN32(DepthCameraProjection))
{
  CHECK_SUPPORTED_ENGINE("ogre2");

  int imgWidth = 256;
  int imgHeight = 256;
  double aspectRatio = imgWidth / imgHeight;

  double unitBoxSize = 1.0;
  gz::math::Vector3d boxPosition(1.8, 0.0, 0.0);

  gz::rendering::ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);

  // Create an scene with a box in it
  gz::rendering::VisualPtr root = scene->RootVisual();

  // create box visual
  gz::rendering::VisualPtr box = scene->CreateVisual();
  box->AddGeometry(scene->CreateBox());
  box->SetLocalPosition(boxPosition);
  box->SetLocalScale(unitBoxSize, unitBoxSize, unitBoxSize);
  root->AddChild(box);
  {
    // Create depth camera
    auto depthCamera = scene->CreateDepthCamera("DepthCamera");
    ASSERT_NE(depthCamera, nullptr);

    gz::math::Pose3d testPose(gz::math::Vector3d(0, 0, 0),
        gz::math::Quaterniond::Identity);
    depthCamera->SetLocalPose(testPose);

    // Set initial camera parameters using a wide horizontal FOV
    double farDist = 100.0;
    double nearDist = 0.01;
    double hfov = 1.5;
    depthCamera->SetImageWidth(imgWidth);
    EXPECT_EQ(depthCamera->ImageWidth(),
      static_cast<unsigned int>(imgWidth));
    depthCamera->SetImageHeight(imgHeight);
    EXPECT_EQ(depthCamera->ImageHeight(),
      static_cast<unsigned int>(imgHeight));
    depthCamera->SetFarClipPlane(farDist);
    EXPECT_DOUBLE_EQ(depthCamera->FarClipPlane(), farDist);
    depthCamera->SetNearClipPlane(nearDist);
    EXPECT_DOUBLE_EQ(depthCamera->NearClipPlane(), nearDist);
    depthCamera->SetAspectRatio(aspectRatio);
    EXPECT_DOUBLE_EQ(depthCamera->AspectRatio(), aspectRatio);
    depthCamera->SetHFOV(hfov);
    EXPECT_DOUBLE_EQ(depthCamera->HFOV().Radian(), hfov);

    depthCamera->CreateDepthTexture();
    scene->RootVisual()->AddChild(depthCamera);

    // Set a callback on the  camera sensor to get a depth camera frame
    float *scan = new float[imgHeight * imgWidth];
    gz::common::ConnectionPtr connection =
      depthCamera->ConnectNewDepthFrame(
          std::bind(&::OnNewDepthFrame, scan,
            std::placeholders::_1, std::placeholders::_2, std::placeholders::_3,
            std::placeholders::_4, std::placeholders::_5));

    g_depthCounter = 0u;
    depthCamera->Update();
    scene->SetTime(scene->Time() + std::chrono::milliseconds(16));
    EXPECT_EQ(1u, g_depthCounter);

    float expectedRange = boxPosition.X() - unitBoxSize * 0.5;
    unsigned int hasInfValues = 0u;
    unsigned int hasBoxValues = 0u;
    for (unsigned int i = 0; i < depthCamera->ImageHeight(); ++i)
    {
      for (unsigned int j = 0; j < depthCamera->ImageWidth(); ++j)
      {
        float x = scan[i * depthCamera->ImageWidth() + j];
        if (gz::math::equal(expectedRange, x))
          hasBoxValues++;
        else if (std::isinf(x))
          hasInfValues++;
        else
          FAIL() << "Unexpected range value: " << x;
      }
    }
    EXPECT_LT(0u, hasBoxValues);
    EXPECT_LT(0u, hasInfValues);

    // Now override with a custom projection matrix
    // This projection matrix corresponds to a small horizontal FOV
    // (hfov = 0.5)
    gz::math::Matrix4d projectionMatrix(
        3.91632, 0, 0, 0,
        0, 3.91632, 0, 0,
        0, 0, -1.0002, -0.20002,
        0, 0, -1, 0);
    depthCamera->SetProjectionMatrix(projectionMatrix);
    depthCamera->Update();
    scene->SetTime(scene->Time() + std::chrono::milliseconds(16));
    EXPECT_EQ(2u, g_depthCounter);

    // The camera should use the updated projection matrix and
    // the box should fill the whole image.
    // Verify all range values at the expected box range
    for (unsigned int i = 0; i < depthCamera->ImageHeight(); ++i)
    {
      for (unsigned int j = 0; j < depthCamera->ImageWidth(); ++j)
      {
        float x = scan[i * depthCamera->ImageWidth() + j];
        EXPECT_FLOAT_EQ(expectedRange, x);
       }
    }

    // Clean up
    connection.reset();
    delete [] scan;
  }

  engine->DestroyScene(scene);
}

/////////////////////////////////////////////////
/// \brief On OpenGL the readback waits for the GPU by polling its fence
/// with short sleeps while the render thread has idle time;
/// GZ_RENDERING_OGRE2_BLOCKING_READBACK_WAIT makes map() wait instead. Both
/// must give the same depth image and point cloud bytes, frame for frame,
/// while a box moves in front of the camera.
TEST_F(DepthCameraTest,
    GZ_UTILS_TEST_DISABLED_ON_WIN32(ReadbackWaitMatchesBlocking))
{
  CHECK_SUPPORTED_ENGINE("ogre2");
  const char *kBlockingEnv = "GZ_RENDERING_OGRE2_BLOCKING_READBACK_WAIT";
  const unsigned int width = 160;
  const unsigned int height = 120;

  gz::rendering::ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);
  scene->SetAmbientLight(1.0, 1.0, 1.0);
  gz::rendering::VisualPtr root = scene->RootVisual();

  gz::rendering::VisualPtr box = scene->CreateVisual();
  box->AddGeometry(scene->CreateBox());
  root->AddChild(box);

  auto depthCamera = scene->CreateDepthCamera("DepthCamera");
  ASSERT_NE(nullptr, depthCamera);
  depthCamera->SetImageWidth(width);
  depthCamera->SetImageHeight(height);
  depthCamera->SetNearClipPlane(0.15);
  depthCamera->SetFarClipPlane(10.0);
  depthCamera->SetAspectRatio(static_cast<double>(width) / height);
  depthCamera->SetHFOV(1.05);
  depthCamera->CreateDepthTexture();
  root->AddChild(depthCamera);

  std::vector<float> depth(width * height);
  std::vector<float> cloud(width * height * 4);
  int depthFrames = 0;
  int cloudFrames = 0;
  gz::common::ConnectionPtr depthConn = depthCamera->ConnectNewDepthFrame(
      [&depth, &depthFrames](const float *_data, unsigned int _w,
          unsigned int _h, unsigned int _c, const std::string &)
      {
        ASSERT_EQ(depth.size(), static_cast<size_t>(_w) * _h * _c);
        memcpy(depth.data(), _data, depth.size() * sizeof(float));
        ++depthFrames;
      });
  gz::common::ConnectionPtr cloudConn = depthCamera->ConnectNewRgbPointCloud(
      [&cloud, &cloudFrames](const float *_data, unsigned int _w,
          unsigned int _h, unsigned int _c, const std::string &)
      {
        ASSERT_EQ(cloud.size(), static_cast<size_t>(_w) * _h * _c);
        memcpy(cloud.data(), _data, cloud.size() * sizeof(float));
        ++cloudFrames;
      });

  // Render once with the given wait into depth and cloud.
  auto render = [&](bool _blocking)
  {
    if (_blocking)
      EXPECT_TRUE(gz::utils::setenv(kBlockingEnv, "1"));
    else
      EXPECT_TRUE(gz::utils::unsetenv(kBlockingEnv));
    std::fill(depth.begin(), depth.end(), -1.0f);
    std::fill(cloud.begin(), cloud.end(), -1.0f);
    const int depthBefore = depthFrames;
    const int cloudBefore = cloudFrames;
    depthCamera->Update();
    EXPECT_EQ(depthBefore + 1, depthFrames);
    EXPECT_EQ(cloudBefore + 1, cloudFrames);
    EXPECT_TRUE(gz::utils::unsetenv(kBlockingEnv));
  };

  // The polled wait sleeps only while the thread has idle time: idle 20 ms
  // before each polled frame, and long enough for the wait policy to see it.
  const auto idle = []()
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
  };
  for (int i = 0; i < 20; ++i)
  {
    idle();
    render(false);
  }

  std::vector<float> previous;
  std::vector<float> previousCloud;
  for (int i = 0; i < 6; ++i)
  {
    box->SetLocalPosition(1.5 + 0.3 * i, -0.6 + 0.2 * i, 0.1 * i);
    idle();
    render(false);
    const std::vector<float> polled = depth;
    const std::vector<float> polledCloud = cloud;
    render(true);
    EXPECT_EQ(0, memcmp(polled.data(), depth.data(),
        depth.size() * sizeof(float))) << "depth, frame " << i;
    EXPECT_EQ(0, memcmp(polledCloud.data(), cloud.data(),
        cloud.size() * sizeof(float))) << "point cloud, frame " << i;
    // The box moved: a stale frame would not match the new one.
    if (!previous.empty())
    {
      EXPECT_NE(0, memcmp(previous.data(), depth.data(),
          depth.size() * sizeof(float))) << "frame " << i;
      EXPECT_NE(0, memcmp(previousCloud.data(), cloud.data(),
          cloud.size() * sizeof(float))) << "point cloud, frame " << i;
    }
    previous = depth;
    previousCloud = cloud;
  }

  depthConn.reset();
  cloudConn.reset();
  scene->DestroySensor(depthCamera);
  engine->DestroyScene(scene);
}

/////////////////////////////////////////////////
// Rendering two depth cameras, then issuing both readbacks before either
// PostRender, delivers the same bytes as Render + PostRender on each in
// turn; and a readback issued for a frame that was never post-rendered is
// not delivered for the next one.
TEST_F(DepthCameraTest,
    GZ_UTILS_TEST_DISABLED_ON_WIN32(IssueReadbackMatchesSerial))
{
  CHECK_SUPPORTED_ENGINE("ogre2");
  if (std::getenv("GZ_RENDERING_OGRE2_LEGACY_READBACK"))
    GTEST_SKIP() << "The legacy readback can't be issued ahead.";
  const unsigned int width = 160;
  const unsigned int height = 120;

  gz::rendering::ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);
  gz::rendering::VisualPtr root = scene->RootVisual();

  gz::rendering::VisualPtr box = scene->CreateVisual();
  box->AddGeometry(scene->CreateBox());
  root->AddChild(box);

  struct Output
  {
    gz::rendering::DepthCameraPtr camera;
    std::vector<float> depth;
    std::vector<float> cloud;
    int frames = 0;
    gz::common::ConnectionPtr depthConn;
    gz::common::ConnectionPtr cloudConn;
  };
  std::vector<Output> cams(2);
  for (unsigned int i = 0; i < cams.size(); ++i)
  {
    Output &c = cams[i];
    c.camera = scene->CreateDepthCamera("DepthCamera" + std::to_string(i));
    ASSERT_NE(nullptr, c.camera);
    c.camera->SetImageWidth(width);
    c.camera->SetImageHeight(height);
    c.camera->SetNearClipPlane(0.15);
    c.camera->SetFarClipPlane(10.0);
    c.camera->SetAspectRatio(static_cast<double>(width) / height);
    c.camera->SetHFOV(1.05);
    c.camera->SetLocalRotation(0, 0, i == 0 ? -0.2 : 0.2);
    c.camera->CreateDepthTexture();
    root->AddChild(c.camera);
    c.depth.resize(width * height);
    c.cloud.resize(width * height * 4);
    c.depthConn = c.camera->ConnectNewDepthFrame(
        [&c](const float *_data, unsigned int _w, unsigned int _h,
            unsigned int _c, const std::string &)
        {
          ASSERT_EQ(c.depth.size(), static_cast<size_t>(_w) * _h * _c);
          memcpy(c.depth.data(), _data, c.depth.size() * sizeof(float));
          ++c.frames;
        });
    c.cloudConn = c.camera->ConnectNewRgbPointCloud(
        [&c](const float *_data, unsigned int _w, unsigned int _h,
            unsigned int _c, const std::string &)
        {
          ASSERT_EQ(c.cloud.size(), static_cast<size_t>(_w) * _h * _c);
          memcpy(c.cloud.data(), _data, c.cloud.size() * sizeof(float));
        });
  }

  // One frame of both cameras, serial or with the readbacks issued ahead;
  // returns each camera's depth and point cloud.
  auto frame = [&](bool _ahead)
  {
    for (auto &c : cams)
    {
      std::fill(c.depth.begin(), c.depth.end(), -1.0f);
      std::fill(c.cloud.begin(), c.cloud.end(), -1.0f);
    }
    scene->PreRender();
    for (auto &c : cams)
    {
      const int before = c.frames;
      c.camera->Render();
      if (_ahead)
        EXPECT_TRUE(c.camera->IssueReadback());
      else
        c.camera->PostRender();
      EXPECT_EQ(before + (_ahead ? 0 : 1), c.frames);
    }
    for (auto &c : cams)
    {
      if (_ahead)
        c.camera->PostRender();
    }
    scene->PostRender();
    std::vector<std::vector<float>> out;
    for (auto &c : cams)
    {
      out.push_back(c.depth);
      out.push_back(c.cloud);
    }
    return out;
  };

  std::vector<std::vector<float>> previous;
  for (int i = 0; i < 4; ++i)
  {
    box->SetLocalPosition(2.0 + 0.3 * i, -0.4 + 0.25 * i, 0.1 * i);
    const auto ahead = frame(true);
    const auto serial = frame(false);
    for (unsigned int k = 0; k < serial.size(); ++k)
    {
      EXPECT_EQ(0, memcmp(ahead[k].data(), serial[k].data(),
          serial[k].size() * sizeof(float)))
          << "camera " << k / 2 << (k % 2 ? " cloud" : " depth")
          << ", frame " << i;
      // The box moved: a stale frame would not match the new one.
      if (!previous.empty())
      {
        EXPECT_NE(0, memcmp(previous[k].data(), serial[k].data(),
            serial[k].size() * sizeof(float)))
            << "camera " << k / 2 << ", frame " << i;
      }
    }
    previous = serial;
  }

  // Depth-only frames (no point cloud subscribers) read back another
  // target; the same holds for them.
  for (auto &c : cams)
    c.cloudConn.reset();
  // A released connection leaves the event at its next signal: the frame
  // after a disconnect still counts it (a point cloud frame without
  // callbacks). Render that one first.
  frame(false);
  for (int i = 0; i < 2; ++i)
  {
    box->SetLocalPosition(1.8 - 0.2 * i, 0.3 * i, 0.0);
    const auto ahead = frame(true);
    const auto serial = frame(false);
    for (unsigned int k = 0; k < serial.size(); k += 2)
    {
      EXPECT_EQ(0, memcmp(ahead[k].data(), serial[k].data(),
          serial[k].size() * sizeof(float)))
          << "depth only, camera " << k / 2 << ", frame " << i;
      EXPECT_NE(0, memcmp(previous[k].data(), serial[k].data(),
          serial[k].size() * sizeof(float)))
          << "depth only, camera " << k / 2 << ", frame " << i;
    }
    previous = serial;
  }

  // A readback issued for a frame that is never post-rendered: the next
  // frame delivers its own data, not that one's.
  scene->PreRender();
  cams[0].camera->Render();
  EXPECT_TRUE(cams[0].camera->IssueReadback());
  scene->PostRender();
  box->SetLocalPosition(1.5, 0.3, 0.0);
  const auto next = frame(false);
  const auto again = frame(false);
  for (unsigned int k = 0; k < next.size(); ++k)
  {
    EXPECT_EQ(0, memcmp(next[k].data(), again[k].data(),
        next[k].size() * sizeof(float))) << "camera " << k / 2;
  }

  for (auto &c : cams)
  {
    c.depthConn.reset();
    c.cloudConn.reset();
    scene->DestroySensor(c.camera);
  }
  engine->DestroyScene(scene);
}

/////////////////////////////////////////////////
/// \brief A point cloud subscriber that connects between the scene's
/// PreRender and the camera's Render (gz-sim's order: Scene::PreRender,
/// then each sensor's Update connects its callbacks and renders) gets a
/// point cloud from that very frame: the same xyz as a frame rendered with
/// the subscriber connected throughout, and the colour cleared, as for a
/// frame whose PreRender saw no subscriber (never a stale colour). None
/// once it disconnects.
TEST_F(DepthCameraTest,
    GZ_UTILS_TEST_DISABLED_ON_WIN32(PointCloudFromFrameOfConnect))
{
  CHECK_SUPPORTED_ENGINE("ogre2");
  const unsigned int width = 160;
  const unsigned int height = 120;

  gz::rendering::ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);
  scene->SetAmbientLight(1.0, 1.0, 1.0);
  gz::rendering::VisualPtr root = scene->RootVisual();
  gz::rendering::MaterialPtr red = scene->CreateMaterial();
  red->SetAmbient(1.0, 0.0, 0.0);
  red->SetDiffuse(1.0, 0.0, 0.0);
  gz::rendering::VisualPtr box = scene->CreateVisual();
  box->AddGeometry(scene->CreateBox());
  box->SetMaterial(red);
  box->SetLocalPosition(2.0, 0.2, 0.0);
  root->AddChild(box);

  auto depthCamera = NewDepthCamera(scene, "DepthCamera", width, height,
      0.15, 10.0);
  ASSERT_NE(nullptr, depthCamera);
  depthCamera->CreateDepthTexture();
  root->AddChild(depthCamera);

  std::vector<float> depth(width * height);
  std::vector<float> cloud(width * height * 4);
  int depthFrames = 0;
  int cloudFrames = 0;
  gz::common::ConnectionPtr depthConn = depthCamera->ConnectNewDepthFrame(
      CaptureInto(depth, depthFrames));
  auto onCloud = CaptureInto(cloud, cloudFrames);
  // whether the colour channel holds the same bytes everywhere
  auto uniformColour = [&cloud]()
  {
    for (size_t i = 7; i < cloud.size(); i += 4)
    {
      if (memcmp(&cloud[i], &cloud[3], sizeof(float)) != 0)
        return false;
    }
    return true;
  };
  // whether the cloud's x, y, z are bit for bit those of _other
  auto sameXyz = [&cloud](const std::vector<float> &_other)
  {
    const std::vector<float> a = Xyz(cloud);
    const std::vector<float> b = Xyz(_other);
    return a.size() == b.size() &&
        memcmp(a.data(), b.data(), a.size() * sizeof(float)) == 0;
  };
  // the scene's PreRender, a connection change, then the camera's frame
  auto lateFrame = [&](const std::function<void()> &_change)
  {
    std::fill(cloud.begin(), cloud.end(), -1.0f);
    scene->PreRender();
    _change();
    depthCamera->Render();
    depthCamera->PostRender();
    scene->PostRender();
  };

  // Frames with the subscriber connected throughout: the colour target
  // holds the red box on black.
  gz::common::ConnectionPtr cloudConn =
      depthCamera->ConnectNewRgbPointCloud(onCloud);
  depthCamera->Update();
  depthCamera->Update();
  ASSERT_EQ(2, cloudFrames);
  const std::vector<float> full = cloud;
  const std::vector<float> fullDepth = depth;
  ASSERT_FALSE(uniformColour()) << "no box colour in view";

  // Frames without a subscriber.
  cloudConn.reset();
  depthCamera->Update();
  depthCamera->Update();
  EXPECT_EQ(4, depthFrames);
  EXPECT_EQ(2, cloudFrames);
  EXPECT_EQ(0, memcmp(fullDepth.data(), depth.data(),
      depth.size() * sizeof(float)));

  // The subscriber connects after the scene's PreRender.
  lateFrame([&]()
      { cloudConn = depthCamera->ConnectNewRgbPointCloud(onCloud); });
  EXPECT_EQ(5, depthFrames);
  EXPECT_EQ(3, cloudFrames) << "no point cloud from the frame of connect";
  EXPECT_TRUE(sameXyz(full));
  EXPECT_TRUE(uniformColour()) << "stale colour in the frame of connect";
  EXPECT_EQ(0, memcmp(fullDepth.data(), depth.data(),
      depth.size() * sizeof(float)));

  // The next frame is a full one again.
  std::fill(cloud.begin(), cloud.end(), -1.0f);
  depthCamera->Update();
  EXPECT_EQ(4, cloudFrames);
  EXPECT_EQ(0, memcmp(full.data(), cloud.data(),
      cloud.size() * sizeof(float)));

  // The subscriber disconnects after the scene's PreRender: no point cloud,
  // the same depth frame.
  lateFrame([&]() { cloudConn.reset(); });
  EXPECT_EQ(7, depthFrames);
  EXPECT_EQ(4, cloudFrames);
  EXPECT_EQ(0, memcmp(fullDepth.data(), depth.data(),
      depth.size() * sizeof(float)));

  depthConn.reset();
  scene->DestroySensor(depthCamera);
  engine->DestroyScene(scene);
}


/////////////////////////////////////////////////
/// \brief A depth frame rendered without point cloud subscribers (read
/// back from the depth-only target: one quad pass, or two with a noise
/// pass in between) is bit for bit the depth frame of a point cloud frame
/// (channel 0 of the xyz + colour output, which the test checks too), with
/// pixels in range, beyond the far clip plane and in front of the near clip
/// plane. One scene; per variant a camera: 0 plain, 1 with a noise render
/// pass.
TEST_F(DepthCameraTest,
    GZ_UTILS_TEST_DISABLED_ON_WIN32(DepthOnlyFramesMatchPointCloudFrames))
{
  CHECK_SUPPORTED_ENGINE("ogre2");
  const unsigned int width = 160;
  const unsigned int height = 120;

  gz::rendering::ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);
  gz::rendering::VisualPtr box = AddBoxes(scene);

  std::vector<std::vector<float>> plain;
  for (int variant : {0, 1})
  {
    const bool noise = variant == 1;
    auto camera = NewDepthCamera(scene,
        "DepthCamera" + std::to_string(variant), width, height, 0.3, 4.0);
    ASSERT_NE(nullptr, camera);
    if (noise)
    {
      // a render pass between the base and the final node: the depth-only
      // frame takes the two-pass path
      auto pass = std::dynamic_pointer_cast<gz::rendering::GaussianNoisePass>(
          engine->RenderPassSystem()->Create<
          gz::rendering::GaussianNoisePass>());
      ASSERT_NE(nullptr, pass);
      pass->SetMean(0.1);
      pass->SetStdDev(0.01);
      camera->AddRenderPass(pass);
    }
    camera->CreateDepthTexture();
    scene->RootVisual()->AddChild(camera);

    std::vector<float> depth(width * height);
    int frames = 0;
    gz::common::ConnectionPtr depthConn = camera->ConnectNewDepthFrame(
        CaptureInto(depth, frames));
    int clouds = 0;
    std::vector<float> cloudDepth(width * height);
    auto onCloud = [&clouds, &cloudDepth](const float *_data, unsigned int _w,
        unsigned int _h, unsigned int _c, const std::string &)
    {
      // the depth is channel 0 of the point cloud
      ASSERT_EQ(cloudDepth.size(), static_cast<size_t>(_w) * _h);
      for (size_t i = 0; i < cloudDepth.size(); ++i)
        cloudDepth[i] = _data[i * _c];
      ++clouds;
    };

    // One frame with or without a point cloud subscriber, with the same
    // noise; returns the depth frame.
    auto frame = [&](bool _cloud)
    {
      gz::common::ConnectionPtr cloudConn;
      if (_cloud)
      {
        cloudConn = camera->ConnectNewRgbPointCloud(onCloud);
      }
      else
      {
        // A released connection leaves the event at its next signal: the
        // frame after a disconnect still counts it (a point cloud frame
        // without callbacks). Render that one first.
        camera->Update();
      }
      const int before = clouds;
      std::fill(depth.begin(), depth.end(), -1.0f);
      gz::math::Rand::Seed(11);
      camera->Update();
      EXPECT_EQ(before + (_cloud ? 1 : 0), clouds);
      if (_cloud)
      {
        EXPECT_EQ(0, memcmp(cloudDepth.data(), depth.data(),
            depth.size() * sizeof(float))) << "depth frame vs point cloud";
      }
      return depth;
    };

    std::vector<float> previous;
    for (int i = 0; i < 4; ++i)
    {
      // (raised: background (+inf) below it in every frame)
      box->SetLocalPosition(1.5 + 0.4 * i, -0.3 + 0.2 * i, 0.1 + 0.1 * i);
      const std::vector<float> full = frame(true);
      const std::vector<float> depthOnly = frame(false);
      SCOPED_TRACE("variant " + std::to_string(variant) + ", frame " +
          std::to_string(i));
      EXPECT_EQ(0, memcmp(full.data(), depthOnly.data(),
          full.size() * sizeof(float)));
      ExpectFiniteAndInfinities(full);
      // The box moved: a stale frame would not match the new one.
      if (!previous.empty())
      {
        EXPECT_NE(0, memcmp(previous.data(), depthOnly.data(),
            full.size() * sizeof(float)));
      }
      previous = depthOnly;
      // the noise pass is applied (to both kinds of frames)
      if (noise)
      {
        EXPECT_NE(0, memcmp(plain[i].data(), depthOnly.data(),
            full.size() * sizeof(float)));
      }
      else
      {
        plain.push_back(full);
      }
    }
    EXPECT_EQ(12, frames);

    depthConn.reset();
  }
  // (the cameras go with the scene: a depth camera created after another
  // one was destroyed trips an Ogre datablock assertion)
  engine->DestroyScene(scene);
}
