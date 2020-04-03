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
#include <string>

#include <ignition/common/Console.hh>
#include <ignition/common/MeshManager.hh>
#include <ignition/common/Skeleton.hh>
#include <ignition/common/SkeletonAnimation.hh>

#include "test_config.h"  // NOLINT(build/include)
#include "ignition/rendering/Camera.hh"
#include "ignition/rendering/Mesh.hh"
#include "ignition/rendering/RenderEngine.hh"
#include "ignition/rendering/RenderingIface.hh"
#include "ignition/rendering/Scene.hh"

using namespace ignition;
using namespace rendering;

class MeshTest : public testing::Test,
                 public testing::WithParamInterface<const char *>
{
  /// \brief Test mesh and submesh basic API
  public: void MeshSubMesh(const std::string &_renderEngine);

  /// \brief Test mesh skeleton animation API
  public: void MeshSkeletonAnimation(const std::string &_renderEngine);

  public: const std::string TEST_MEDIA_PATH =
        common::joinPaths(std::string(PROJECT_SOURCE_PATH),
        "test", "media", "meshes");
};

/////////////////////////////////////////////////
void MeshTest::MeshSubMesh(const std::string &_renderEngine)
{
  RenderEngine *engine = rendering::engine(_renderEngine);
  if (!engine)
  {
    igndbg << "Engine '" << _renderEngine
              << "' is not supported" << std::endl;
    return;
  }

  ScenePtr scene = engine->CreateScene("scene");
  ASSERT_TRUE(scene != nullptr);

  // create the mesh using mesh descriptor
  MeshDescriptor descriptor("unit_box");
  MeshPtr mesh = scene->CreateMesh(descriptor);
  ASSERT_TRUE(mesh!= nullptr);

  // test mesh API
  EXPECT_EQ(mesh->SubMeshCount(), 1u);

  SubMeshPtr submesh = mesh->SubMeshByIndex(0u);
  ASSERT_TRUE(submesh != nullptr);

  EXPECT_TRUE(mesh->HasSubMesh(submesh));
  EXPECT_TRUE(mesh->HasSubMeshName(submesh->Name()));

  EXPECT_EQ(submesh, mesh->SubMeshByName(submesh->Name()));

  // test submesh API
  MaterialPtr mat = submesh->Material();
  ASSERT_TRUE(mat != nullptr);

  // set submesh non-unique material
  MaterialPtr matClone = mat->Clone();
  submesh->SetMaterial(matClone, false);

  EXPECT_EQ(matClone, submesh->Material());
  EXPECT_NE(mat, submesh->Material());

  submesh->SetMaterial("Default/White", false);
  EXPECT_EQ("Default/White", submesh->Material()->Name());

  // unique material
  submesh->SetMaterial(matClone, true);
  EXPECT_NE(matClone, submesh->Material());

  // Clean up
  engine->DestroyScene(scene);
  rendering::unloadEngine(engine->Name());
}

/////////////////////////////////////////////////
TEST_P(MeshTest, MeshSubMesh)
{
  MeshSubMesh(GetParam());
}

/////////////////////////////////////////////////
void MeshTest::MeshSkeletonAnimation(const std::string &_renderEngine)
{
  RenderEngine *engine = rendering::engine(_renderEngine);
  if (!engine)
  {
    igndbg << "Engine '" << _renderEngine
              << "' is not supported" << std::endl;
    return;
  }

  ScenePtr scene = engine->CreateScene("scene");
  ASSERT_TRUE(scene != nullptr);

  // test box mesh with no skeleton animation
  MeshDescriptor boxDescriptor("unit_box");
  MeshPtr boxMesh = scene->CreateMesh(boxDescriptor);
  ASSERT_TRUE(boxMesh != nullptr);

  EXPECT_FALSE(boxMesh->HasSkeleton());
  EXPECT_NO_THROW(boxMesh->SetSkeletonAnimationEnabled("invalid", false));
  EXPECT_FALSE(boxMesh->SkeletonAnimationEnabled("invalid"));
  EXPECT_TRUE(boxMesh->SkeletonLocalTransforms().empty());

  // test mesh with skeleton animation
  MeshDescriptor descriptor;
  descriptor.meshName = common::joinPaths(TEST_MEDIA_PATH, "walk.dae");
  common::MeshManager *meshManager = common::MeshManager::Instance();
  descriptor.mesh = meshManager->Load(descriptor.meshName);
  MeshPtr mesh = scene->CreateMesh(descriptor);

  EXPECT_TRUE(mesh->HasSkeleton());
  EXPECT_FALSE(mesh->SkeletonLocalTransforms().empty());

  auto skel = descriptor.mesh->MeshSkeleton();
  ASSERT_EQ(1u, skel->AnimationCount());

  std::string animName = skel->Animation(0u)->Name();
  EXPECT_FALSE(mesh->SkeletonAnimationEnabled(animName));

  mesh->SetSkeletonAnimationEnabled(animName, true);
  EXPECT_TRUE(mesh->SkeletonAnimationEnabled(animName));

  EXPECT_NO_THROW(mesh->UpdateSkeletonAnimation(
      std::chrono::duration_cast<std::chrono::steady_clock::duration>(
      std::chrono::duration<double>(0.0))));
  EXPECT_NO_THROW(mesh->UpdateSkeletonAnimation(
      std::chrono::duration_cast<std::chrono::steady_clock::duration>(
      std::chrono::duration<double>(1.0))));
  EXPECT_NO_THROW(mesh->UpdateSkeletonAnimation(
      std::chrono::duration_cast<std::chrono::steady_clock::duration>(
      std::chrono::duration<double>(-11.0))));
  EXPECT_NO_THROW(mesh->UpdateSkeletonAnimation(
      std::chrono::duration_cast<std::chrono::steady_clock::duration>(
      std::chrono::duration<double>(1234.5))));

  // verify default skeleton bone weight
  std::unordered_map<std::string, float> weights = mesh->SkeletonWeights();
  EXPECT_FALSE(weights.empty());
  EXPECT_EQ(skel->NodeCount(), weights.size());
  for (auto &it : weights)
  {
    EXPECT_NE(nullptr, skel->NodeByName(it.first));
    EXPECT_FLOAT_EQ(1.0, it.second);
  }

  // change a bone weight and verify
  std::string nodeName  = skel->RootNode()->Name();
  EXPECT_EQ(1u, weights.count(nodeName));
  weights[nodeName] = 0.5;
  mesh->SetSkeletonWeights(weights);

  auto newWeights = mesh->SkeletonWeights();
  EXPECT_FALSE(newWeights.empty());
  EXPECT_EQ(skel->NodeCount(), newWeights.size());
  EXPECT_FLOAT_EQ(0.5, newWeights[nodeName]);
  for (auto &it : newWeights)
  {
    EXPECT_NE(nullptr, skel->NodeByName(it.first));
    if (it.first != nodeName)
    {
      EXPECT_FLOAT_EQ(1.0, it.second);
    }
  }

  // Clean up
  engine->DestroyScene(scene);
  rendering::unloadEngine(engine->Name());
}

/////////////////////////////////////////////////
TEST_P(MeshTest, MeshSkeletonAnimation)
{
  MeshSkeletonAnimation(GetParam());
}

INSTANTIATE_TEST_CASE_P(Mesh, MeshTest,
    RENDER_ENGINE_VALUES,
    ignition::rendering::PrintToStringParam());

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
