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

#include <array>
#include <cstring>
#include <fstream>
#include <map>
#include <string>

#include <gz/common/Filesystem.hh>
#include <gz/common/TempDirectory.hh>

#include "gz/rendering/Camera.hh"
#include "gz/rendering/Image.hh"
#include "gz/rendering/Scene.hh"
#include "gz/rendering/Visual.hh"
#include "gz/rendering/ogre2/Ogre2RenderEngine.hh"

#ifdef _MSC_VER
  #pragma warning(push, 0)
#endif
#include <OgreHlmsListener.h>
#ifdef _MSC_VER
  #pragma warning(pop)
#endif

using namespace gz;
using namespace rendering;

namespace
{
/// \brief Image size [px]
constexpr unsigned int kSize = 32u;

/// \brief A listener that sends a colour in the pass buffer
class TintListener : public Ogre::HlmsListener
{
  // Documentation inherited
  public: Ogre::uint32 getPassBufferSize(
      const Ogre::CompositorShadowNode *, bool, bool,
      Ogre::SceneManager *) const override
  {
    return sizeof(this->tint);
  }

  // Documentation inherited
  public: float *preparePassBuffer(const Ogre::CompositorShadowNode *,
      bool, bool, Ogre::SceneManager *, float *_passBufferPtr) override
  {
    for (float value : this->tint)
      *_passBufferPtr++ = value;
    return _passBufferPtr;
  }

  /// \brief The colour the shader paints with
  public: std::array<float, 4> tint{{1.0f, 0.0f, 0.0f, 1.0f}};
};

/// \brief Write a file.
/// \param[in] _path Path of the file.
/// \param[in] _text Its contents.
void WriteFile(const std::string &_path, const std::string &_text)
{
  std::ofstream(_path) << _text;
}
}  // namespace

/// \brief A camera looking at a lit white box, and two customizations of
/// the PBS shaders: one paints with the colour its listener sends, the other
/// sets the blue channel after it.
class Ogre2HlmsCustomizationTest : public testing::Test
{
  // Documentation inherited
  protected: void SetUp() override
  {
    this->engine = Ogre2RenderEngine::Instance();
    if (!this->engine->IsInitialized())
    {
      std::map<std::string, std::string> params;
      if (!this->engine->Load(params) || !this->engine->Init())
        GTEST_SKIP() << "ogre2 could not start: no GPU?";
    }

    this->scene = this->engine->CreateScene("hlms_customization");
    ASSERT_NE(nullptr, this->scene);
    this->scene->SetAmbientLight(1.0, 1.0, 1.0);
    this->scene->SetBackgroundColor(0.0, 0.0, 0.0);

    VisualPtr box = this->scene->CreateVisual();
    box->AddGeometry(this->scene->CreateBox());
    box->SetLocalPosition(2.0, 0.0, 0.0);
    this->scene->RootVisual()->AddChild(box);

    this->camera = this->scene->CreateCamera();
    ASSERT_NE(nullptr, this->camera);
    this->camera->SetImageWidth(kSize);
    this->camera->SetImageHeight(kSize);
    this->scene->RootVisual()->AddChild(this->camera);

    // Pieces for the hooks, named <hook>_<name>
    this->tintFolder = common::joinPaths(this->tempDir.Path(), "tint");
    ASSERT_TRUE(common::createDirectories(this->tintFolder));
    WriteFile(common::joinPaths(this->tintFolder,
        "500.TestTint_piece_all.any"),
        "@piece( custom_passBuffer_test_tint )\n"
        "  float4 gzTestTint;\n"
        "@end\n");
    WriteFile(common::joinPaths(this->tintFolder, "800.TestTint_piece_ps.any"),
        "@piece( custom_ps_functions_test_tint )\n"
        "  float4 gzTestTintOf( float4 tint ) { return tint; }\n"
        "@end\n"
        "@piece( custom_ps_posExecution_test_tint )\n"
        "  @property( !hlms_shadowcaster )\n"
        "    outPs_colour0 = gzTestTintOf( passBuf.gzTestTint );\n"
        "  @end\n"
        "@end\n");

    this->blueFolder = common::joinPaths(this->tempDir.Path(), "blue");
    ASSERT_TRUE(common::createDirectories(this->blueFolder));
    WriteFile(common::joinPaths(this->blueFolder, "800.TestBlue_piece_ps.any"),
        "@piece( custom_ps_posExecution_test_blue )\n"
        "  @property( !hlms_shadowcaster )\n"
        "    outPs_colour0.z = 1.0;\n"
        "  @end\n"
        "@end\n");
  }

  // Documentation inherited
  protected: void TearDown() override
  {
    if (!this->scene)
      return;
    this->engine->RemoveHlmsCustomization(Ogre2HlmsType::PBS, "test_tint");
    this->engine->RemoveHlmsCustomization(Ogre2HlmsType::PBS, "test_blue");
    this->engine->DestroyScene(this->scene);
  }

  /// \brief Render a frame.
  /// \return The colour at the centre of the image, where the box is.
  protected: std::array<int, 3> CentreColour()
  {
    this->image = this->camera->CreateImage();
    this->camera->Capture(this->image);
    const unsigned char *data = this->image.Data<unsigned char>();
    const unsigned int i = ((kSize / 2u) * kSize + kSize / 2u) * 3u;
    return {data[i], data[i + 1], data[i + 2]};
  }

  /// \brief The engine
  protected: Ogre2RenderEngine *engine = nullptr;

  /// \brief The scene
  protected: ScenePtr scene;

  /// \brief The camera
  protected: CameraPtr camera;

  /// \brief The last image
  protected: Image image;

  /// \brief Where the pieces are written
  protected: common::TempDirectory tempDir{
      std::string("hlms_customization"), std::string("gz"), true};

  /// \brief Folder of the tint pieces
  protected: std::string tintFolder;

  /// \brief Folder of the blue pieces
  protected: std::string blueFolder;
};

/////////////////////////////////////////////////
/// Pieces and a listener extend the shaders next to gz-rendering's own, in
/// the order they were added, and come out again.
TEST_F(Ogre2HlmsCustomizationTest, PiecesAndListener)
{
  const std::array<int, 3> white{{255, 255, 255}};
  EXPECT_EQ(white, this->CentreColour());
  const Image before = this->image;

  // Its pieces, its pass data from the listener
  TintListener listener;
  ASSERT_TRUE(this->engine->AddHlmsCustomization(Ogre2HlmsType::PBS,
      "test_tint", &listener, this->tintFolder));
  EXPECT_EQ((std::array<int, 3>{{255, 0, 0}}), this->CentreColour());

  // New pass data, no reload
  listener.tint = {{0.0f, 1.0f, 0.0f, 1.0f}};
  EXPECT_EQ((std::array<int, 3>{{0, 255, 0}}), this->CentreColour());

  // Pieces only, after the first in the same hook
  ASSERT_TRUE(this->engine->AddHlmsCustomization(Ogre2HlmsType::PBS,
      "test_blue", nullptr, this->blueFolder));
  EXPECT_EQ((std::array<int, 3>{{0, 255, 255}}), this->CentreColour());

  // Out again, one then the other
  EXPECT_TRUE(this->engine->RemoveHlmsCustomization(Ogre2HlmsType::PBS,
      "test_tint"));
  EXPECT_EQ((std::array<int, 3>{{255, 255, 255}}), this->CentreColour());
  EXPECT_TRUE(this->engine->RemoveHlmsCustomization(Ogre2HlmsType::PBS,
      "test_blue"));
  EXPECT_EQ(white, this->CentreColour());
  EXPECT_EQ(0, std::memcmp(before.Data<unsigned char>(),
      this->image.Data<unsigned char>(), before.MemorySize()));
}

/////////////////////////////////////////////////
/// Names are checked, and each Hlms can be extended.
TEST_F(Ogre2HlmsCustomizationTest, Names)
{
  TintListener listener;
  EXPECT_FALSE(this->engine->AddHlmsCustomization(Ogre2HlmsType::PBS,
      "test tint", &listener, this->tintFolder));
  EXPECT_FALSE(this->engine->AddHlmsCustomization(Ogre2HlmsType::PBS,
      "", &listener, this->tintFolder));
  for (const char *reserved :
       {"customizations", "solid_color", "spherical_clip_min_distance",
        "terra"})
  {
    EXPECT_FALSE(this->engine->AddHlmsCustomization(Ogre2HlmsType::PBS,
        reserved, &listener, this->tintFolder)) << reserved;
  }
  EXPECT_FALSE(this->engine->AddHlmsCustomization(Ogre2HlmsType::PBS,
      "test_tint", &listener, common::joinPaths(this->tintFolder, "none")));

  ASSERT_TRUE(this->engine->AddHlmsCustomization(Ogre2HlmsType::PBS,
      "test_tint", &listener, this->tintFolder));
  EXPECT_FALSE(this->engine->AddHlmsCustomization(Ogre2HlmsType::PBS,
      "test_tint", &listener, this->tintFolder));
  EXPECT_TRUE(this->engine->RemoveHlmsCustomization(Ogre2HlmsType::PBS,
      "test_tint"));
  EXPECT_FALSE(this->engine->RemoveHlmsCustomization(Ogre2HlmsType::PBS,
      "test_tint"));

  // The same name is free in another Hlms
  for (Ogre2HlmsType type : {Ogre2HlmsType::UNLIT, Ogre2HlmsType::TERRA})
  {
    EXPECT_TRUE(this->engine->AddHlmsCustomization(type, "test_tint",
        nullptr, this->blueFolder));
    EXPECT_TRUE(this->engine->RemoveHlmsCustomization(type, "test_tint"));
  }

  // The scene still renders with all of them gone
  EXPECT_EQ((std::array<int, 3>{{255, 255, 255}}), this->CentreColour());
}
