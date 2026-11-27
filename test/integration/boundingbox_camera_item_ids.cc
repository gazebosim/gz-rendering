/*
 * Copyright (C) 2026 Open Source Robotics Foundation
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

// Bounding box camera with Ogre item ids above 16 bits.
//
// Ogre item ids come from a process-wide counter that only grows (it is not
// reset when the render engine is unloaded). The bounding box camera used to
// encode the id in 16 bits, so once 65536 items had been created in the
// process it stopped working: full 2D and 3D boxes returned nothing and every
// visible 2D box was merged into one.
//
// This lives in its own test binary because the test has to push the global
// id counter past 65535, which would break every test that runs after it in
// the same process.

#include "boundingbox_camera_test_utils.hh"

using namespace gz;
using namespace rendering;
using namespace bbox_test;

//////////////////////////////////////////////////
// Box A is created before the id counter passes 65535, boxes B and C after.
// All three box types must still report the three boxes, with the right
// labels and positions.
TEST_F(BoundingBoxCameraAccuracyTest, ItemIdsAbove16Bits)
{
  CHECK_SUPPORTED_ENGINE("ogre2");
  ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);

  const CameraModel model;
  auto camera = this->CreateCamera(scene, model,
      BoundingBoxType::BBT_VISIBLEBOX2D);
  ASSERT_NE(nullptr, camera);

  const math::Vector3d size(1, 1, 1);
  const std::vector<math::Pose3d> poses = {
    {5, 1.5, 0, 0, 0, 0},
    {5, -1.5, 0, 0, 0, 0},
    {7, 0, 1.5, 0, 0, 0},
  };

  AddBox(scene, scene->RootVisual(), poses[0], size, 1);
  for (int i = 0; i < 65536; ++i)
  {
    VisualPtr tmp = AddBox(scene, scene->RootVisual(),
        {50, 0, 0, 0, 0, 0}, size, std::nullopt);
    scene->DestroyVisual(tmp, true);
  }
  AddBox(scene, scene->RootVisual(), poses[1], size, 2);
  AddBox(scene, scene->RootVisual(), poses[2], size, 3);

  for (auto type : {BoundingBoxType::BBT_VISIBLEBOX2D,
      BoundingBoxType::BBT_FULLBOX2D, BoundingBoxType::BBT_BOX3D})
  {
    SCOPED_TRACE(static_cast<int>(type));
    auto result = this->Render(camera, type);
    ASSERT_EQ(3u, result.size());
    for (uint32_t label = 1; label <= 3; ++label)
    {
      auto b = WithLabel(result, label);
      ASSERT_EQ(1u, b.size()) << "label " << label;
      const math::Pose3d &pose = poses[label - 1];
      if (type == BoundingBoxType::BBT_BOX3D)
      {
        EXPECT_NEAR(ToOptical(pose.Pos()).X(), b[0].Center().X(), 1e-3);
        EXPECT_NEAR(ToOptical(pose.Pos()).Y(), b[0].Center().Y(), 1e-3);
        EXPECT_NEAR(ToOptical(pose.Pos()).Z(), b[0].Center().Z(), 1e-3);
      }
      else
      {
        ExpectRectNear(ToRect(b[0]),
            *ExpectedRect(BoxCorners(pose, size), model), kVisibleTol,
            "label " + std::to_string(label));
      }
    }
  }

  engine->DestroyScene(scene);
}
