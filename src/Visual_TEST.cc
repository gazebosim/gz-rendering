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
#include <string>

#include <ignition/common/Console.hh>

#include "test_config.h"  // NOLINT(build/include)

#include "ignition/rendering/Visual.hh"
#include "ignition/rendering/RenderEngine.hh"
#include "ignition/rendering/RenderingIface.hh"
#include "ignition/rendering/Scene.hh"

using namespace ignition;
using namespace rendering;

class VisualTest : public testing::Test,
                   public testing::WithParamInterface<const char *>
{
  /// \brief Test visual material
  public: void Material(const std::string &_renderEngine);

  /// \brief Test adding removing children
  public: void Children(const std::string &_renderEngine);

  /// \brief Test visual scale
  public: void Scale(const std::string &_renderEngine);
};

/////////////////////////////////////////////////
void VisualTest::Material(const std::string &_renderEngine)
{
  RenderEngine *engine = rendering::engine(_renderEngine);
  if (!engine)
  {
    igndbg << "Engine '" << _renderEngine
              << "' is not supported" << std::endl;
    return;
  }

  ScenePtr scene = engine->CreateScene("scene");

  // create visual
  VisualPtr visual = scene->CreateVisual();
  ASSERT_NE(nullptr, visual);

  // check initial material
  EXPECT_EQ(nullptr, visual->Material());

  // create material
  math::Color ambient(0.5, 0.2, 0.4, 1.0);
  math::Color diffuse(0.1, 0.9, 0.3, 1.0);
  math::Color specular(0.8, 0.7, 0.0, 1.0);
  double transparency = 0.3;
  MaterialPtr material = scene->CreateMaterial("unique");
  ASSERT_NE(nullptr, material);
  EXPECT_TRUE(scene->MaterialRegistered("unique"));
  material->SetAmbient(ambient);
  material->SetDiffuse(diffuse);
  material->SetSpecular(specular);
  material->SetTransparency(transparency);

  // Set visual material but do not clone the material
  visual->SetMaterial(material, false);
  EXPECT_EQ(material, visual->Material());

  // Set visual material and clone the material
  visual->SetMaterial(material, true);
  MaterialPtr cloneMat = visual->Material();
  EXPECT_NE(material, cloneMat);
  EXPECT_NE(material->Name(), cloneMat->Name());

  // verify clone material properties
  EXPECT_EQ(material->Type(), cloneMat->Type());
  EXPECT_EQ(ambient, cloneMat->Ambient());
  EXPECT_EQ(diffuse, cloneMat->Diffuse());
  EXPECT_EQ(specular, cloneMat->Specular());
  EXPECT_DOUBLE_EQ(transparency, cloneMat->Transparency());

  // create another material
  math::Color ambient2(0.0, 0.0, 1.0, 1.0);
  math::Color diffuse2(1.0, 0.0, 1.0, 1.0);
  math::Color specular2(0.0, 1.0, 0.0, 1.0);
  double transparency2 = 0;
  MaterialPtr material2 = scene->CreateMaterial("unique2");
  ASSERT_NE(nullptr, material2);
  EXPECT_TRUE(scene->MaterialRegistered("unique2"));
  material2->SetAmbient(ambient2);
  material2->SetDiffuse(diffuse2);
  material2->SetSpecular(specular2);
  material2->SetTransparency(transparency2);

  // Set material to the same visual using its name and verify material changed
  visual->SetMaterial("unique2", true);
  MaterialPtr cloneMat2 = visual->Material();
  EXPECT_NE(material2, cloneMat);
  EXPECT_NE(material2->Name(), cloneMat2->Name());
  EXPECT_EQ(material2->Type(), cloneMat2->Type());
  EXPECT_EQ(ambient2, cloneMat2->Ambient());
  EXPECT_EQ(diffuse2, cloneMat2->Diffuse());
  EXPECT_EQ(specular2, cloneMat2->Specular());
  EXPECT_DOUBLE_EQ(transparency2, cloneMat2->Transparency());

  // Clean up
  engine->DestroyScene(scene);
  rendering::unloadEngine(engine->Name());
}

/////////////////////////////////////////////////
TEST_P(VisualTest, Material)
{
  Material(GetParam());
}

/////////////////////////////////////////////////
void VisualTest::Children(const std::string &_renderEngine)
{
  RenderEngine *engine = rendering::engine(_renderEngine);
  if (!engine)
  {
    igndbg << "Engine '" << _renderEngine
              << "' is not supported" << std::endl;
    return;
  }

  ScenePtr scene = engine->CreateScene("scene2");

  // create visual
  VisualPtr visual = scene->CreateVisual();
  ASSERT_NE(nullptr, visual);

  VisualPtr child = scene->CreateVisual();
  ASSERT_NE(nullptr, child);

  // no child by default
  EXPECT_EQ(0u, visual->ChildCount());

  // attach child and verify
  visual->AddChild(child);
  EXPECT_EQ(1u, visual->ChildCount());
  EXPECT_TRUE(visual->HasChild(child));
  EXPECT_TRUE(visual->HasChildId(child->Id()));
  EXPECT_TRUE(visual->HasChildName(child->Name()));

  EXPECT_EQ(child, visual->ChildById(child->Id()));
  EXPECT_EQ(child, visual->ChildByName(child->Name()));
  EXPECT_EQ(child, visual->ChildByIndex(0u));

  // attempt to attach itself and verify self not added
  visual->AddChild(visual);
  EXPECT_EQ(1u, visual->ChildCount());
  EXPECT_FALSE(visual->HasChild(visual));

  // attach more than one child
  VisualPtr child2 = scene->CreateVisual();
  ASSERT_NE(nullptr, child2);
  visual->AddChild(child2);
  EXPECT_EQ(2u, visual->ChildCount());
  VisualPtr child3 = scene->CreateVisual();
  ASSERT_NE(nullptr, child3);
  visual->AddChild(child3);
  EXPECT_EQ(3u, visual->ChildCount());

  // test different child removal methods
  EXPECT_EQ(child, visual->RemoveChild(child));
  EXPECT_EQ(child2, visual->RemoveChildById(child2->Id()));
  EXPECT_EQ(child3, visual->RemoveChildByName(child3->Name()));

  // attach previously removed child and remove again
  visual->AddChild(child);
  EXPECT_EQ(child, visual->RemoveChildByIndex(0u));

  // Clean up
  engine->DestroyScene(scene);
  rendering::unloadEngine(engine->Name());
}

/////////////////////////////////////////////////
TEST_P(VisualTest, Children)
{
  Children(GetParam());
}

/////////////////////////////////////////////////
void VisualTest::Scale(const std::string &_renderEngine)
{
  RenderEngine *engine = rendering::engine(_renderEngine);
  if (!engine)
  {
    igndbg << "Engine '" << _renderEngine
              << "' is not supported" << std::endl;
    return;
  }

  ScenePtr scene = engine->CreateScene("scene2");

  // create visual
  VisualPtr visual = scene->CreateVisual();
  ASSERT_NE(nullptr, visual);

  VisualPtr child = scene->CreateVisual();
  ASSERT_NE(nullptr, child);

  // verify initial scale properties
  EXPECT_TRUE(visual->InheritScale());
  EXPECT_EQ(math::Vector3d::One, visual->LocalScale());
  EXPECT_EQ(math::Vector3d::One, visual->WorldScale());

  // set visual scale and verify
  math::Vector3d scale(4, 2, 3);
  visual->SetLocalScale(scale);
  EXPECT_EQ(scale, visual->LocalScale());
  EXPECT_EQ(scale, visual->WorldScale());

  math::Vector3d worldScale(6, 1, 4);
  visual->SetWorldScale(worldScale);
  EXPECT_EQ(worldScale, visual->LocalScale());
  EXPECT_EQ(worldScale, visual->WorldScale());

  // attach child and verify
  visual->AddChild(child);
  EXPECT_EQ(1u, visual->ChildCount());
  EXPECT_EQ(child, visual->ChildById(child->Id()));

  // verify child initial scale
  EXPECT_TRUE(child->InheritScale());
  EXPECT_EQ(math::Vector3d::One, child->LocalScale());
  EXPECT_EQ(worldScale, child->WorldScale());

  // Set child scale and verify
  math::Vector3d childScale(5, 1, 9);
  child->SetLocalScale(childScale);
  EXPECT_EQ(childScale, child->LocalScale());
  EXPECT_EQ(childScale * worldScale, child->WorldScale());

  math::Vector3d childWorldScale(5, 1, 9);
  child->SetWorldScale(childWorldScale);
  math::Vector3d childLocalScale = childWorldScale / worldScale;
  EXPECT_EQ(childLocalScale, child->LocalScale());
  EXPECT_EQ(childWorldScale, child->WorldScale());

  // Set visual scale and verify visual and child scale
  math::Vector3d scale2(3, 5, 9);
  visual->SetLocalScale(scale2);
  EXPECT_EQ(scale2, visual->LocalScale());
  EXPECT_EQ(scale2, visual->WorldScale());
  EXPECT_EQ(childLocalScale, child->LocalScale());
  EXPECT_EQ(scale2 * childLocalScale, child->WorldScale());

  // test setting child inherit scale to false
  child->SetInheritScale(false);
  EXPECT_FALSE(child->InheritScale());
  EXPECT_EQ(childLocalScale, child->LocalScale());
  EXPECT_EQ(childLocalScale, child->WorldScale());

  // set parent visual scale and verify the child is not affected.
  math::Vector3d scale3(1, 9, 8);
  visual->SetLocalScale(scale3);
  EXPECT_EQ(scale3, visual->LocalScale());
  EXPECT_EQ(scale3, visual->WorldScale());
  EXPECT_EQ(childLocalScale, child->LocalScale());
  EXPECT_EQ(childLocalScale, child->WorldScale());

  // Clean up
  engine->DestroyScene(scene);
  rendering::unloadEngine(engine->Name());
}

/////////////////////////////////////////////////
TEST_P(VisualTest, Scale)
{
  Scale(GetParam());
}

INSTANTIATE_TEST_CASE_P(Visual, VisualTest,
    RENDER_ENGINE_VALUES,
    ignition::rendering::PrintToStringParam());

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
