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

#include <gz/common/Console.hh>
#include <gz/common/MeshManager.hh>
#include <gz/common/Skeleton.hh>
#include <gz/common/SkeletonAnimation.hh>

#include "test_config.h"  // NOLINT(build/include)
#include "gz/rendering/Camera.hh"
#include "gz/rendering/Mesh.hh"
#include "gz/rendering/RenderEngine.hh"
#include "gz/rendering/RenderingIface.hh"
#include "gz/rendering/Scene.hh"

using namespace gz;
using namespace rendering;

class MeshTest : public testing::Test,
                 public testing::WithParamInterface<const char *>
{
  /// \brief Test mesh and submesh basic API
  public: void MeshSubMesh(const std::string &_renderEngine);

  /// \brief Test mesh skeleton
  public: void MeshSkeleton(const std::string &_renderEngine);

  public: const std::string TEST_MEDIA_PATH =
        common::joinPaths(std::string(PROJECT_SOURCE_PATH),
        "test", "media", "skeleton");
};

/////////////////////////////////////////////////
void MeshTest::MeshSubMesh(const std::string &_renderEngine)
{
#ifdef __APPLE__                                                                              
  std::cerr << "Skipping test for apple, see issue #847." << std::endl;                       
  return;
#endif
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

  EXPECT_FALSE(mesh->HasSkeleton());

  std::map<std::string, ignition::math::Matrix4d> skinFrames;
  mesh->SetSkeletonLocalTransforms(skinFrames);
  auto skeletonLocalTransforms = mesh->SkeletonLocalTransforms();
  EXPECT_EQ(0u, skeletonLocalTransforms.size());

  // test submesh API
  MaterialPtr mat = submesh->Material();
  ASSERT_TRUE(mat != nullptr);

  // set submesh non-unique material
  MaterialPtr matClone = mat->Clone();
  submesh->SetMaterial(matClone, false);

  EXPECT_EQ(matClone, submesh->Material());
  EXPECT_NE(mat, submesh->Material());

  submesh->SetMaterial(MaterialPtr(), false);

  submesh->SetMaterial("Default/White", false);
  EXPECT_EQ("Default/White", submesh->Material()->Name());

  // unique material
  submesh->SetMaterial(matClone, true);
  EXPECT_NE(matClone, submesh->Material());

  // Clean up
  engine->DestroyScene(scene);
  unloadEngine(engine->Name());
}

/////////////////////////////////////////////////
void MeshTest::MeshSkeleton(const std::string &_renderEngine)
{
#ifdef __APPLE__                                                                              
  std::cerr << "Skipping test for apple, see issue #847." << std::endl;                       
  return;
#endif
  RenderEngine *engine = rendering::engine(_renderEngine);
  if (!engine)
  {
    igndbg << "Engine '" << _renderEngine
              << "' is not supported" << std::endl;
    return;
  }

  ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);

  VisualPtr root = scene->RootVisual();

  // create a visual for the actor, attach mesh and get skeleton
  // Skeleton will be animated by GlutWindow
  VisualPtr actorVisual = scene->CreateVisual("actor");
  actorVisual->SetLocalPosition(0, 0, 0);
  actorVisual->SetLocalRotation(0, 0, 0);

  MeshDescriptor descriptor;
  descriptor.meshName = common::joinPaths(TEST_MEDIA_PATH, "walk.dae");
  common::MeshManager *meshManager = common::MeshManager::Instance();
  descriptor.mesh = meshManager->Load(descriptor.meshName);

  MeshPtr mesh = scene->CreateMesh(descriptor);
  actorVisual->AddGeometry(mesh);
  root->AddChild(actorVisual);

  common::SkeletonPtr skel;

  if (mesh && descriptor.mesh->HasSkeleton())
  {
    skel = descriptor.mesh->MeshSkeleton();

    if (!skel || skel->AnimationCount() == 0)
    {
      FAIL();
    }
  }
  else
  {
    FAIL();
  }

  EXPECT_TRUE(mesh->HasSkeleton());

  std::string bvhFile = common::joinPaths(TEST_MEDIA_PATH, "cmu-13_26.bvh");

  double scale = 0.055;
  skel->AddBvhAnimation(bvhFile, scale);

  int g_animIdx = 1;
  auto * skelAnim = skel->Animation(g_animIdx);
  for (double i = 0; i < 10; i+=0.01)
  {
    std::map<std::string, ignition::math::Matrix4d> animFrames;
    animFrames = skelAnim->PoseAt(i, true);

    std::map<std::string, ignition::math::Matrix4d> skinFrames;

    for (auto pair : animFrames)
    {
      std::string animName = pair.first;
      auto animTf = pair.second;

      std::string skinName = skel->NodeNameAnimToSkin(g_animIdx, animName);
      ignition::math::Matrix4d skinTf =
              skel->AlignTranslation(g_animIdx, animName)
              * animTf * skel->AlignRotation(g_animIdx, animName);

      skinFrames[skinName] = skinTf;
    }

    mesh->SetSkeletonLocalTransforms(skinFrames);
  }

  auto skeletonLocalTransforms = mesh->SkeletonLocalTransforms();
  EXPECT_EQ(31u, skeletonLocalTransforms.size());

  // Clean up
  engine->DestroyScene(scene);
  unloadEngine(engine->Name());
}

/////////////////////////////////////////////////
TEST_P(MeshTest, MeshSubMesh)
{
  MeshSubMesh(GetParam());
}

/////////////////////////////////////////////////
TEST_P(MeshTest, MeshSkeleton)
{
  MeshSkeleton(GetParam());
}

INSTANTIATE_TEST_CASE_P(Mesh, MeshTest,
    RENDER_ENGINE_VALUES,
    PrintToStringParam());

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
