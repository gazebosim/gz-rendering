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
#include <ignition/common/Material.hh>

#include "ignition/rendering/Camera.hh"
#include "ignition/rendering/Material.hh"
#include "ignition/rendering/RenderEngine.hh"
#include "ignition/rendering/RenderingIface.hh"
#include "ignition/rendering/ShaderType.hh"
#include "ignition/rendering/Scene.hh"

using namespace ignition;
using namespace rendering;

class MaterialTest : public testing::Test,
                     public testing::WithParamInterface<const char *>
{
  /// \brief Test material basic API
  public: void MaterialProperties(const std::string &_renderEngine);

  /// \brief Test copying and cloning a material
  public: void CopyFrom(const std::string &_renderEngine);
};

/////////////////////////////////////////////////
void MaterialTest::MaterialProperties(const std::string &_renderEngine)
{
  RenderEngine *engine = rendering::engine(_renderEngine);
  if (!engine)
  {
    igndbg << "Engine '" << _renderEngine
              << "' is not supported" << std::endl;
    return;
  }

  ScenePtr scene = engine->CreateScene("scene");

  MaterialPtr material = scene->CreateMaterial();
  EXPECT_TRUE(material != nullptr);

  material = scene->CreateMaterial("unique");
  EXPECT_TRUE(material != nullptr);
  EXPECT_TRUE(scene->MaterialRegistered("unique"));

  // ambient
  math::Color ambient(0.5, 0.2, 0.4, 1.0);
  material->SetAmbient(ambient);
  EXPECT_EQ(ambient, material->Ambient());

  ambient.Set(0.55, 0.22, 0.44, 1.0);
  material->SetAmbient(ambient.R(), ambient.G(), ambient.B(), ambient.A());
  EXPECT_EQ(ambient, material->Ambient());

  // diffuse
  math::Color diffuse(0.1, 0.9, 0.3, 1.0);
  material->SetDiffuse(diffuse);
  EXPECT_EQ(diffuse, material->Diffuse());

  diffuse.Set(0.11, 0.99, 0.33, 1.0);
  material->SetDiffuse(diffuse.R(), diffuse.G(), diffuse.B(), diffuse.A());
  EXPECT_EQ(diffuse, material->Diffuse());

  // specular
  math::Color specular(0.8, 0.7, 0.0, 1.0);
  material->SetSpecular(specular);
  EXPECT_EQ(specular, material->Specular());

  specular.Set(0.88, 0.77, 0.66, 1.0);
  material->SetSpecular(specular.R(), specular.G(), specular.B(), specular.A());
  EXPECT_EQ(specular, material->Specular());

  // emissive
  math::Color emissive(0.6, 0.4, 0.2, 1.0);
  material->SetEmissive(emissive);
  EXPECT_EQ(emissive, material->Emissive());

  emissive.Set(0.66, 0.44, 0.22, 1.0);
  material->SetEmissive(emissive.R(), emissive.G(), emissive.B(), emissive.A());
  EXPECT_EQ(emissive, material->Emissive());

  // shininess
  double shininess = 0.83457;
  material->SetShininess(shininess);
  EXPECT_DOUBLE_EQ(shininess, material->Shininess());

  // transparency
  double transparency = 0.3;
  material->SetTransparency(transparency);
  EXPECT_DOUBLE_EQ(transparency, material->Transparency());

  // reflectivity
  double reflectivity = 0.5;
  material->SetReflectivity(reflectivity);
  EXPECT_DOUBLE_EQ(reflectivity, material->Reflectivity());

  // cast shadows
  bool castShadows = false;
  material->SetCastShadows(castShadows);
  EXPECT_EQ(castShadows, material->CastShadows());

  // receive shadows
  bool receiveShadows = false;
  material->SetReceiveShadows(receiveShadows);
  EXPECT_EQ(receiveShadows, material->ReceiveShadows());

  // reflection
  bool reflectionEnabled = true;
  material->SetReflectionEnabled(reflectionEnabled);
  EXPECT_EQ(reflectionEnabled, material->ReflectionEnabled());

  // texture
  std::string textureName = "texture.png";
  material->SetTexture(textureName);
  EXPECT_EQ(textureName, material->Texture());
  EXPECT_TRUE(material->HasTexture())'

  material->ClearTexture();
  EXPECT_FALSE(material->HasTexture())'

  // normal map
  std::string normalMapName = "normal.png";
  material->SetNormalMap(normalMapName);
  EXPECT_EQ(normalMapName, material->NormalMap());
  EXPECT_TRUE(material->HasNormalMap())'

  material->ClearNormalMap();
  EXPECT_FALSE(material->HasNormalMap())'

  // shader type
  enum ShaderType shaderType = ShaderType::ST_PIXEL;
  material->SetShaderType(shaderType);
  EXPECT_EQ(shaderType, material->ShaderType());
}

/////////////////////////////////////////////////
void MaterialTest::CopyFrom(const std::string &_renderEngine)
{
  RenderEngine *engine = rendering::engine(_renderEngine);
  if (!engine)
  {
    igndbg << "Engine '" << _renderEngine
              << "' is not supported" << std::endl;
    return;
  }

  ScenePtr scene = engine->CreateScene("scene");

  MaterialPtr material = scene->CreateMaterial();
  EXPECT_TRUE(material != nullptr);

  math::Color ambient(0.5, 0.2, 0.4, 1.0);
  math::Color diffuse(0.1, 0.9, 0.3, 1.0);
  math::Color specular(0.8, 0.7, 0.0, 1.0);
  math::Color emissive(0.6, 0.4, 0.2, 1.0);
  double shininess = 0.83457;
  double transparency = 0.3;
  double reflectivity = 0.5;
  bool castShadows = false;
  bool receiveShadows = false;
  bool reflectionEnabled = true;
  std::string textureName = "texture.png";
  std::string normalMapName = "normal.png";
  enum ShaderType shaderType = ShaderType::ST_PIXEL;

  material->SetAmbient(ambient);
  material->SetDiffuse(diffuse);
  matieral->SetSpecular(specular);
  material->SetEmissive(emissive);
  material->SetShininess(shininess);
  material->SetTransparency(transparency);
  material->SetReflectivity(reflectivity);
  material->SetCastShadows(castShadows);
  material->SetReceiveShadows(receiveShadows);
  material->SetReflectionEnabled(reflectionEnabled);
  material->SetTexture(textureName);
  material->SetNormalMap(normalMapName);
  material->SetShaderType(shaderType);

  // test cloning a material
  MaterialPtr clone = material->Clone("clone");
  EXPECT_TRUE(scene->MaterialRegistered("clone"));
  EXPECT_EQ(ambient, clone->Ambient());
  EXPECT_EQ(diffuse, clone->Diffuse());
  EXPECT_EQ(specular, clone->Specular());
  EXPECT_EQ(emissive, clone->Emissive());
  EXPECT_DOUBLE_EQ(shininess, clone->Shininess());
  EXPECT_DOUBLE_EQ(transparency, clone->Transparency());
  EXPECT_DOUBLE_EQ(reflectivity, clone->Reflectivity());
  EXPECT_EQ(castShadows, clone->CastShadows());
  EXPECT_EQ(receiveShadows, clone->ReceiveShadows());
  EXPECT_EQ(reflectionEnabled, clone->ReflectionEnabled());
  EXPECT_EQ(textureName, clone->Texture());
  EXPECT_TRUE(clone->HasTexture());
  EXPECT_EQ(normalMapName, clone->NormalMap());
  EXPECT_TRUE(clone->HasNormalMap());
  EXPECT_EQ(shaderType, clone->ShaderType());

  // test copying a material
  MaterialPtr copy = scene->CreateMaterial("copy");
  EXPECT_TRUE(scene->MaterialRegistered("clone"));
  material->CopyFrom(clone);
  EXPECT_EQ(ambient, copy->Ambient());
  EXPECT_EQ(diffuse, copy->Diffuse());
  EXPECT_EQ(specular, copy->Specular());
  EXPECT_EQ(emissive, copy->Emissive());
  EXPECT_DOUBLE_EQ(shininess, copy->Shininess());
  EXPECT_DOUBLE_EQ(transparency, copy->Transparency());
  EXPECT_DOUBLE_EQ(reflectivity, copy->Reflectivity());
  EXPECT_EQ(castShadows, copy->CastShadows());
  EXPECT_EQ(receiveShadows, copy->ReceiveShadows());
  EXPECT_EQ(reflectionEnabled, copy->ReflectionEnabled());
  EXPECT_EQ(textureName, copy->Texture());
  EXPECT_TRUE(copy->HasTexture());
  EXPECT_EQ(normalMapName, copy->NormalMap());
  EXPECT_TRUE(copy->HasNormalMap());
  EXPECT_EQ(shaderType, copy->ShaderType());

  // test copying from a common material

}

/////////////////////////////////////////////////
TEST_P(MaterialTest, MaterialProperties)
{
  MaterialProperties(GetParam());
}

/////////////////////////////////////////////////
TEST_P(MaterialTest, Copy)
{
  Copy(GetParam());
}

INSTANTIATE_TEST_CASE_P(Material, MaterialTest,
    ::testing::Values("ogre", "optix"));

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
