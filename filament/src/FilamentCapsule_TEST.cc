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

#include <gtest/gtest.h>

#include <cstdlib>
#include <map>
#include <memory>
#include <string>

#include "gz/rendering/Capsule.hh"
#include "gz/rendering/Material.hh"
#include "gz/rendering/RenderEngine.hh"
#include "gz/rendering/RenderingIface.hh"
#include "gz/rendering/Scene.hh"
#include "gz/rendering/Visual.hh"
#include "gz/rendering/base/BaseMaterial.hh"
#include "gz/rendering/base/BaseObject.hh"

using namespace gz;
using namespace rendering;

namespace
{
/// \brief An object of no scene, as BaseMaterial needs for Clone
class ForeignObject : public BaseObject
{
  // Documentation inherited
  public: ScenePtr Scene() const override
  {
    return this->owner;
  }

  /// \brief The scene, never set
  private: ScenePtr owner;
};

/// \brief A material that no scene of the filament engine made, as one of
/// another render engine would be
class ForeignMaterial : public BaseMaterial<ForeignObject>
{
  // Documentation inherited
  public: MaterialPtr Clone(const std::string &) const override
  {
    return std::make_shared<ForeignMaterial>();
  }
};
}

/////////////////////////////////////////////////
// A material the capsule can't use is refused: the capsule keeps the
// material it had, which stays alive
TEST(FilamentCapsuleTest, RejectedMaterialKeepsCurrent)
{
  std::map<std::string, std::string> params;
  if (const char *backend = std::getenv("GZ_ENGINE_BACKEND"))
    params["backend"] = backend;
  // through the plugin loader: the engine singleton of the library
  RenderEngine *engine = rendering::engine("filament", params);
  ASSERT_NE(nullptr, engine);
  {
    ScenePtr scene = engine->CreateScene("scene");
    ASSERT_NE(nullptr, scene);
    VisualPtr visual = scene->CreateVisual();
    scene->RootVisual()->AddChild(visual);
    CapsulePtr capsule = scene->CreateCapsule();
    ASSERT_NE(nullptr, capsule);
    visual->AddGeometry(capsule);

    MaterialPtr red = scene->CreateMaterial();
    red->SetDiffuse(1.0, 0.0, 0.0);
    capsule->SetMaterial(red, true);
    // a clone, owned by the capsule
    const MaterialPtr current = capsule->Material();
    ASSERT_NE(nullptr, current);
    ASSERT_NE(red, current);

    // from another render engine, as is and cloned
    for (const bool unique : {false, true})
    {
      capsule->SetMaterial(std::make_shared<ForeignMaterial>(), unique);
      EXPECT_EQ(current, capsule->Material()) << unique;
      EXPECT_TRUE(scene->MaterialRegistered(current->Name())) << unique;
    }

    // none
    capsule->SetMaterial(MaterialPtr(), true);
    EXPECT_EQ(current, capsule->Material());
    EXPECT_TRUE(scene->MaterialRegistered(current->Name()));

    // still takes a good one
    MaterialPtr green = scene->CreateMaterial();
    green->SetDiffuse(0.0, 1.0, 0.0);
    capsule->SetMaterial(green, false);
    EXPECT_EQ(green, capsule->Material());
    EXPECT_FALSE(scene->MaterialRegistered(current->Name()));
    engine->DestroyScene(scene);
  }
  rendering::unloadEngine(engine->Name());
}
