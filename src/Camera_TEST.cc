/*
 * Copyright (C) 2017 Open Source Robotics Foundation
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

#include "ignition/rendering/Camera.hh"
#include "ignition/rendering/Camera.hh"
#include "ignition/rendering/RenderEngine.hh"
#include "ignition/rendering/RenderingIface.hh"
#include "ignition/rendering/Scene.hh"

using namespace ignition;
using namespace rendering;

class CameraTest : public testing::Test,
                   public testing::WithParamInterface<const char*>
{
  public: void Camera(const std::string &_renderEngine);
};

/////////////////////////////////////////////////
void CameraTest::Camera(const std::string &_renderEngine)
{
  // create and populate scene
  RenderEngine *engine = rendering::engine(_renderEngine);
  if (!engine)
  {
    igndbg << "Engine '" << _renderEngine
              << "' is not supported" << std::endl;
    return;
  }
  ScenePtr scene = engine->CreateScene("scene");

  CameraPtr camera = scene->CreateCamera();
  EXPECT_TRUE(camera != nullptr);

  // projection parameters
  math::Matrix4d projMatrix = camera->ProjectionMatrix();

  EXPECT_GT(camera->HFOV(), 0);
  math::Angle hfov(1.57);
  camera->SetHFOV(hfov);
  EXPECT_EQ(hfov.Radian(), camera->HFOV().Radian());

  EXPECT_GT(camera->AspectRatio(), 0);
  camera->SetAspectRatio(1.7777);
  EXPECT_FLOAT_EQ(1.7777, camera->AspectRatio());

  camera->SetAntiAliasing(1u);
  EXPECT_EQ(1u, camera->AntiAliasing());

  EXPECT_GT(camera->NearClipPlane(), 0);
  camera->SetNearClipPlane(0.1);
  EXPECT_EQ(0.1, camera->NearClipPlane());

  EXPECT_GT(camera->FarClipPlane(), 0);
  camera->SetFarClipPlane(800);
  EXPECT_EQ(800, camera->FarClipPlane());

  EXPECT_NE(projMatrix, camera->ProjectionMatrix());

  // view matrix
  math::Matrix4d viewMatrix = camera->ViewMatrix();
  EXPECT_EQ(math::Vector3d::Zero, camera->LocalPosition());
  EXPECT_EQ(math::Quaterniond::Identity, camera->LocalRotation());

  math::Vector3d pos(3.0, -2.0, 5.0);
  math::Quaterniond rot(0.0, 1.5, 3.14);
  camera->SetLocalPosition(pos);
  EXPECT_EQ(pos, camera->LocalPosition());
  camera->SetLocalRotation(rot);
  EXPECT_EQ(rot, camera->LocalRotation());

  EXPECT_NE(viewMatrix, camera->ViewMatrix());

  // render texture parameters
  EXPECT_GT(camera->ImageWidth(), 0u);
  camera->SetImageWidth(100u);
  EXPECT_EQ(100u, camera->ImageWidth());

  EXPECT_GT(camera->ImageHeight(), 0u);
  camera->SetImageHeight(80u);
  EXPECT_EQ(80u, camera->ImageHeight());

  EXPECT_NE(PixelFormat::PF_UNKNOWN, camera->ImageFormat());
  camera->SetImageFormat(PixelFormat::PF_B8G8R8);
  EXPECT_EQ(PixelFormat::PF_B8G8R8, camera->ImageFormat());
  EXPECT_EQ(100u*80u*3u, camera->ImageMemorySize());
}

/////////////////////////////////////////////////
TEST_P(CameraTest, Camera)
{
  Camera(GetParam());
}

INSTANTIATE_TEST_CASE_P(Camera, CameraTest,
    ::testing::Values("ogre", "optix"));

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
