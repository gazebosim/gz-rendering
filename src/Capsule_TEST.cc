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

#include <ignition/common/Console.hh>

#include "test_config.h"  // NOLINT(build/include)
#include "ignition/rendering/RenderEngine.hh"
#include "ignition/rendering/RenderingIface.hh"
#include "ignition/rendering/Capsule.hh"
#include "ignition/rendering/Scene.hh"

using namespace ignition;
using namespace rendering;

class CapsuleTest : public testing::Test,
                 public testing::WithParamInterface<const char *>
{
  public: void Capsule(const std::string &_renderEngine);
};

/////////////////////////////////////////////////
void CapsuleTest::Capsule(const std::string &_renderEngine)
{
  if (_renderEngine != "ogre" && _renderEngine != "ogre2")
  {
    igndbg << "Capsule not supported yet in rendering engine: "
            << _renderEngine << std::endl;
    return;
  }

  RenderEngine *engine = rendering::engine(_renderEngine);
  if (!engine)
  {
    igndbg << "Engine '" << _renderEngine
           << "' is not supported" << std::endl;
    return;
  }

  ScenePtr scene = engine->CreateScene("scene");

  CapsulePtr capsule = scene->CreateCapsule();
  ASSERT_NE(nullptr, capsule);

  EXPECT_DOUBLE_EQ(capsule->Radius(), 0.5);
  EXPECT_DOUBLE_EQ(capsule->Length(), 0.5);

  capsule->SetRadius(0.7);
  capsule->SetLength(1.9);

  EXPECT_DOUBLE_EQ(capsule->Radius(), 0.7);
  EXPECT_DOUBLE_EQ(capsule->Length(), 1.9);

  // create material
  MaterialPtr mat = scene->CreateMaterial();
  mat->SetAmbient(0.6, 0.7, 0.8);
  mat->SetDiffuse(0.3, 0.8, 0.2);
  mat->SetSpecular(0.4, 0.9, 1.0);

  capsule->SetMaterial(mat);
  MaterialPtr capsuleMat = capsule->Material();
  ASSERT_NE(nullptr, capsuleMat);
  EXPECT_EQ(math::Color(0.6f, 0.7f, 0.8f), capsuleMat->Ambient());
  EXPECT_EQ(math::Color(0.3f, 0.8f, 0.2f), capsuleMat->Diffuse());
  EXPECT_EQ(math::Color(0.4f, 0.9f, 1.0f), capsuleMat->Specular());

  // Clean up
  engine->DestroyScene(scene);
  rendering::unloadEngine(engine->Name());
}

/////////////////////////////////////////////////
TEST_P(CapsuleTest, Capsule)
{
  Capsule(GetParam());
}

INSTANTIATE_TEST_CASE_P(Capsule, CapsuleTest,
    RENDER_ENGINE_VALUES,
    ignition::rendering::PrintToStringParam());

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
