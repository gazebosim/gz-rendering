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

#include "test_config.h"  // NOLINT(build/include)

#include "ignition/rendering/Camera.hh"
#include "ignition/rendering/RenderEngine.hh"
#include "ignition/rendering/RenderingIface.hh"
#include "ignition/rendering/Scene.hh"

using namespace ignition;
using namespace rendering;

class CameraTest: public testing::Test,
                  public testing::WithParamInterface<const char *>
{
  // Test and verify camera tracking
  public: void Track(const std::string &_renderEngine);

  // Test and verify camera following
  public: void Follow(const std::string &_renderEngine);
};

/////////////////////////////////////////////////
void CameraTest::Track(const std::string &_renderEngine)
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
  ASSERT_TRUE(scene != nullptr);

  VisualPtr root = scene->RootVisual();

  CameraPtr camera = scene->CreateCamera();
  ASSERT_TRUE(camera != nullptr);
  root->AddChild(camera);

  // create visual to be tracked
  VisualPtr visual = scene->CreateVisual();
  visual->AddGeometry(scene->CreateBox());
  visual->SetWorldPosition(0.0, 0.0, 0.0);
  // rotate visual to test tracking in local and world frames
  visual->SetWorldRotation(0.0, 0.0, 3.14);
  root->AddChild(visual);

  // set camera initial pose
  math::Vector3d initPos(-2, 0.0, 5.0);
  math::Quaterniond initRot = math::Quaterniond::Identity;
  camera->SetWorldPosition(initPos);
  EXPECT_EQ(initPos, camera->WorldPosition());
  EXPECT_EQ(initRot, camera->WorldRotation());

  // track visual
  camera->SetTrackTarget(visual);
  EXPECT_EQ(visual, camera->TrackTarget());

  // render a frame
  camera->Update();

  EXPECT_EQ(initPos, camera->WorldPosition());
  EXPECT_NE(initRot, camera->WorldRotation());
  math::Vector3d rot = camera->WorldRotation().Euler();
  EXPECT_DOUBLE_EQ(rot.X(), 0.0);
  EXPECT_GT(rot.Y(), 0.0);
  EXPECT_DOUBLE_EQ(rot.Z(), 0.0);

  // store result that will be compared against camera tracking with p gain
  // later in the test
  math::Pose3d camPoseTrackNormal = camera->WorldPose();

  // track target with offset in world frame
  math::Vector3d trackOffset(0.0, 1.0, 0.0);
  camera->SetTrackTarget(visual, trackOffset, true);
  EXPECT_EQ(visual, camera->TrackTarget());
  EXPECT_EQ(trackOffset, camera->TrackOffset());

  // render a frame
  camera->Update();

  // verify camera orientation when tracking target with offset
  // in world frame
  // camera should be looking down and to the left
  EXPECT_EQ(initPos, camera->WorldPosition());
  EXPECT_NE(initRot, camera->WorldRotation());
  rot = camera->WorldRotation().Euler();
  EXPECT_NEAR(0.0, rot.X(), 1e-6);
  EXPECT_GT(rot.Y(), 0.0);
  EXPECT_GT(rot.Z(), 0.0);

  // track visual with offset in local frame
  camera->SetTrackTarget(visual, trackOffset, false);
  EXPECT_EQ(visual, camera->TrackTarget());
  EXPECT_EQ(trackOffset, camera->TrackOffset());

  // render a frame
  camera->Update();
  // verify camera orientation when tracking target with offset
  // in local frame
  // camera should be looking down and to the right
  EXPECT_EQ(initPos, camera->WorldPosition());
  EXPECT_NE(initRot, camera->WorldRotation());
  rot = camera->WorldRotation().Euler();
  EXPECT_NEAR(0.0, rot.X(), 1e-6);
  EXPECT_GT(rot.Y(), 0.0);
  EXPECT_LT(rot.Z(), 0.0);

  // disable target tracking
  camera->SetTrackTarget(nullptr);
  EXPECT_EQ(nullptr, camera->TrackTarget());

  // render a frame
  camera->Update();

  // reset camera pose
  camera->SetWorldPosition(initPos);
  camera->SetWorldRotation(initRot);
  EXPECT_EQ(initPos, camera->WorldPosition());
  EXPECT_EQ(initRot, camera->WorldRotation());

  // track visual with p gain
  camera->SetTrackPGain(0.01);
  EXPECT_DOUBLE_EQ(0.01, camera->TrackPGain());
  camera->SetTrackTarget(visual);
  EXPECT_EQ(visual, camera->TrackTarget());

  // render a frame
  camera->Update();

  // verify camera rotaion has pitch component
  // but not as large as before without p gain
  EXPECT_EQ(initPos, camera->WorldPosition());
  EXPECT_NE(initRot, camera->WorldRotation());
  rot = camera->WorldRotation().Euler();
  EXPECT_DOUBLE_EQ(0.0, rot.X());
  EXPECT_GT(rot.Y(), 0.0);
  EXPECT_LT(rot.Y(), camPoseTrackNormal.Rot().Euler().Y());
  EXPECT_DOUBLE_EQ(0.0, rot.Z());

  // Clean up
  engine->DestroyScene(scene);
}

/////////////////////////////////////////////////
void CameraTest::Follow(const std::string &_renderEngine)
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
  ASSERT_TRUE(scene != nullptr);

  VisualPtr root = scene->RootVisual();

  CameraPtr camera = scene->CreateCamera();
  ASSERT_TRUE(camera != nullptr);
  root->AddChild(camera);

  // create to be followed
  VisualPtr visual = scene->CreateVisual();
  visual->AddGeometry(scene->CreateBox());
  visual->SetWorldPosition(0.0, 0.0, 0.0);
  // give visual a rotation for testing camera following in target
  // node's local frame.
  visual->SetWorldRotation(0.0, 0.0, 0.8);
  root->AddChild(visual);

  // Set camera initial pose
  math::Vector3d initPos(-20, 0.0, 5.0);
  math::Quaterniond initRot = math::Quaterniond::Identity;
  camera->SetWorldPosition(initPos);
  EXPECT_EQ(initPos, camera->WorldPosition());
  EXPECT_EQ(initRot, camera->WorldRotation());

  // track visual
  camera->SetFollowTarget(visual);
  EXPECT_EQ(visual, camera->FollowTarget());

  // render a frame
  camera->Update();

  // verify camera is at same location as visual because
  // no offset is given
  EXPECT_EQ(visual->WorldPosition(), camera->WorldPosition());
  EXPECT_EQ(initRot, camera->WorldRotation());

  // follow target with offset in world frame
  math::Vector3d followOffset(-3.0, 2.0, 1.0);
  camera->SetFollowTarget(visual, followOffset, true);
  EXPECT_EQ(visual, camera->FollowTarget());
  EXPECT_EQ(followOffset, camera->FollowOffset());

  // render a frame
  camera->Update();

  // verify camera pose when following target with offset
  // in world frame
  EXPECT_NE(initPos, camera->WorldPosition());
  EXPECT_EQ(initRot, camera->WorldRotation());
  math::Vector3d cameraFollowPosWorld = visual->WorldPosition()
      + followOffset;
  EXPECT_EQ(cameraFollowPosWorld, camera->WorldPosition());

  // follow target with offset in target local frame
  camera->SetFollowTarget(visual, followOffset, false);
  EXPECT_EQ(visual, camera->FollowTarget());
  EXPECT_EQ(followOffset, camera->FollowOffset());

  // render a frame
  camera->Update();

  // verify camera pose when following target with offset
  // in local frame
  EXPECT_NE(initPos, camera->WorldPosition());
  EXPECT_EQ(initRot, camera->WorldRotation());
  math::Vector3d cameraFollowPosLocal =
      visual->WorldPosition() + visual->WorldRotation()*followOffset;
  EXPECT_EQ(cameraFollowPosLocal, camera->WorldPosition());
  EXPECT_NE(cameraFollowPosWorld, cameraFollowPosLocal);

  // disable target following
  camera->SetFollowTarget(nullptr);
  EXPECT_EQ(nullptr, camera->FollowTarget());

  // render a frame
  camera->Update();

  // reset camera pose
  camera->SetWorldPosition(initPos);
  camera->SetWorldRotation(initRot);
  EXPECT_EQ(camera->WorldPosition(), initPos);
  EXPECT_EQ(camera->WorldRotation(), initRot);

  // follow visual with p gain
  camera->SetFollowPGain(0.03);
  EXPECT_DOUBLE_EQ(0.03, camera->FollowPGain());
  camera->SetFollowTarget(visual, followOffset, true);
  EXPECT_EQ(visual, camera->FollowTarget());

  // render a frame
  camera->Update();

  // verify camera position has changed but
  // but not as close to the target as before without p gain
  EXPECT_NE(cameraFollowPosWorld, camera->WorldPosition());
  EXPECT_EQ(initRot, camera->WorldRotation());
  math::Vector3d offset = camera->WorldPosition() - visual->WorldPosition();
  EXPECT_GT(offset.Length(), followOffset.Length());

  // Clean up
  engine->DestroyScene(scene);
}
/////////////////////////////////////////////////
TEST_P(CameraTest, Track)
{
  Track(GetParam());
}

/////////////////////////////////////////////////
TEST_P(CameraTest, Follow)
{
  Follow(GetParam());
}

INSTANTIATE_TEST_CASE_P(Camera, CameraTest,
    ::testing::Values("ogre", "optix"));

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
