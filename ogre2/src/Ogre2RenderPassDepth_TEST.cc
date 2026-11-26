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

#include <algorithm>
#include <cmath>
#include <cstring>
#include <map>
#include <memory>
#include <string>

#include "gz/rendering/Camera.hh"
#include "gz/rendering/Image.hh"
#include "gz/rendering/Scene.hh"
#include "gz/rendering/Visual.hh"
#include "gz/rendering/ogre2/Ogre2Camera.hh"
#include "gz/rendering/ogre2/Ogre2RenderEngine.hh"
#include "gz/rendering/ogre2/Ogre2RenderPass.hh"

#ifdef _MSC_VER
  #pragma warning(push, 0)
#endif
#include <Compositor/OgreCompositorManager2.h>
#include <Compositor/OgreCompositorNodeDef.h>
#include <Compositor/Pass/PassQuad/OgreCompositorPassQuadDef.h>
#include <OgreCamera.h>
#include <OgreRoot.h>
#ifdef _MSC_VER
  #pragma warning(pop)
#endif

using namespace gz;
using namespace rendering;

namespace
{
/// \brief A render pass that copies its input, or the depth of the scene,
/// to its output.
class CopyPass : public Ogre2RenderPass
{
  /// \brief Constructor
  /// \param[in] _depth True to copy the depth of the scene, false to copy
  /// the image from the previous node.
  public: explicit CopyPass(bool _depth)
    : depth(_depth)
  {
  }

  // Documentation inherited
  public: bool RequiresDepth() const override
  {
    return this->depth;
  }

  // Documentation inherited
  public: void CreateRenderPass() override
  {
    if (!this->ogreCompositorNodeDefName.empty())
      return;

    static int counter = 0;
    const std::string nodeDefName =
        "RenderPassDepthTestNode_" + std::to_string(counter++);

    Ogre::CompositorManager2 *compositorManager =
        Ogre2RenderEngine::Instance()->OgreRoot()->getCompositorManager2();
    Ogre::CompositorNodeDef *nodeDef =
        compositorManager->addNodeDefinition(nodeDefName);
    nodeDef->addTextureSourceName("rt_input", 0u,
        Ogre::TextureDefinitionBase::TEXTURE_INPUT);
    nodeDef->addTextureSourceName("rt_output", 1u,
        Ogre::TextureDefinitionBase::TEXTURE_INPUT);
    if (this->depth)
    {
      nodeDef->addTextureSourceName("depth", kDepthInputChannel,
          Ogre::TextureDefinitionBase::TEXTURE_INPUT);
    }

    nodeDef->setNumTargetPass(1);
    Ogre::CompositorTargetDef *targetDef = nodeDef->addTargetPass("rt_output");
    targetDef->setNumPasses(1);
    Ogre::CompositorPassQuadDef *passQuad =
        static_cast<Ogre::CompositorPassQuadDef *>(
        targetDef->addPass(Ogre::PASS_QUAD));
    passQuad->mMaterialName = "Ogre/Copy/4xFP32";
    passQuad->addQuadTextureSource(0, this->depth ? "depth" : "rt_input");

    nodeDef->mapOutputChannel(0, "rt_output");
    nodeDef->mapOutputChannel(1, "rt_input");
    this->ogreCompositorNodeDefName = nodeDefName;
  }

  /// \brief True to copy the depth of the scene
  private: bool depth;
};

/// \brief Image size [px]
constexpr unsigned int kSize = 64u;

/// \brief Far clip distance of the camera [m]
constexpr double kFar = 100.0;

/// \brief A value in [0, 1] as an 8 bit sRGB target stores it: a camera
/// renders to one, and its image is that target's bytes.
/// \param[in] _value The value.
/// \return The byte.
int SrgbByte(double _value)
{
  const double v = std::min(std::max(_value, 0.0), 1.0);
  const double encoded = v <= 0.0031308 ?
      12.92 * v : 1.055 * std::pow(v, 1.0 / 2.4) - 0.055;
  return static_cast<int>(std::lround(255.0 * encoded));
}
}  // namespace

/// \brief A camera looking along +x at a box, with and without
/// anti-aliasing
class Ogre2RenderPassDepthTest : public testing::TestWithParam<unsigned int>
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

    static int sceneCounter = 0;
    this->scene = this->engine->CreateScene(
        "render_pass_depth_" + std::to_string(sceneCounter++));
    ASSERT_NE(nullptr, this->scene);
    this->scene->SetAmbientLight(1.0, 1.0, 1.0);
    this->scene->SetBackgroundColor(0.0, 0.0, 0.0);

    this->box = this->scene->CreateVisual();
    this->box->AddGeometry(this->scene->CreateBox());
    this->scene->RootVisual()->AddChild(this->box);
    this->PlaceBox(1.0);

    this->camera = this->scene->CreateCamera();
    ASSERT_NE(nullptr, this->camera);
    this->camera->SetImageWidth(kSize);
    this->camera->SetImageHeight(kSize);
    this->camera->SetNearClipPlane(0.1);
    this->camera->SetFarClipPlane(kFar);
    this->camera->SetAntiAliasing(GetParam());
    this->scene->RootVisual()->AddChild(this->camera);
  }

  // Documentation inherited
  protected: void TearDown() override
  {
    if (this->scene)
      this->engine->DestroyScene(this->scene);
  }

  /// \brief Put the face of the unit box that the camera sees at a
  /// distance.
  /// \param[in] _distance Distance [m].
  protected: void PlaceBox(double _distance)
  {
    this->box->SetLocalPosition(_distance + 0.5, 0.0, 0.0);
  }

  /// \brief Render a frame.
  /// \return The image.
  protected: Image Capture()
  {
    Image image = this->camera->CreateImage();
    this->camera->Capture(image);
    return image;
  }

  /// \brief The byte of a copy of the hardware depth at a distance along
  /// the camera's axis: the depth is `A + B / distance`, with `(A, B)` from
  /// the camera's projection.
  /// \param[in] _distance Distance [m].
  /// \return The byte.
  protected: int DepthByte(double _distance) const
  {
    auto *ogre2Camera = dynamic_cast<Ogre2Camera *>(this->camera.get());
    const Ogre::Vector2 ab =
        ogre2Camera->OgreCamera()->getProjectionParamsAB();
    return SrgbByte(ab.x + ab.y / _distance);
  }

  /// \brief The red byte of a pixel.
  /// \param[in] _image The image.
  /// \param[in] _x Column.
  /// \param[in] _y Row.
  /// \return The byte.
  protected: static int Red(const Image &_image, unsigned int _x,
      unsigned int _y)
  {
    return _image.Data<unsigned char>()[(_y * _image.Width() + _x) * 3u];
  }

  /// \brief Whether two images are the same, byte for byte.
  /// \param[in] _a One image.
  /// \param[in] _b The other.
  /// \return True if they are.
  protected: static bool Same(const Image &_a, const Image &_b)
  {
    return _a.MemorySize() == _b.MemorySize() &&
        std::memcmp(_a.Data<unsigned char>(), _b.Data<unsigned char>(),
        _a.MemorySize()) == 0;
  }

  /// \brief The engine
  protected: Ogre2RenderEngine *engine = nullptr;

  /// \brief The scene
  protected: ScenePtr scene;

  /// \brief The box the camera looks at
  protected: VisualPtr box;

  /// \brief The camera
  protected: CameraPtr camera;
};

/////////////////////////////////////////////////
/// A pass that reads the depth gets the depth the scene was rendered with,
/// whatever pass comes before it and also when added after the camera has
/// rendered; once it is removed the camera renders the scene as before.
TEST_P(Ogre2RenderPassDepthTest, DepthReachesThePass)
{
  // A pass that does not read the depth, before and after
  RenderPassPtr copyPass = std::make_shared<CopyPass>(false);
  this->camera->AddRenderPass(copyPass);
  const Image before = this->Capture();

  RenderPassPtr depthPass = std::make_shared<CopyPass>(true);
  this->camera->AddRenderPass(depthPass);
  for (double distance : {1.0, 2.0, 4.5})
  {
    SCOPED_TRACE(distance);
    this->PlaceBox(distance);
    const Image depth = this->Capture();
    // The face of the box; one step of the 8 bit target either way
    EXPECT_NEAR(this->DepthByte(distance), Red(depth, kSize / 2, kSize / 2),
        1);
    // Where nothing was drawn: the clear value, the far clip distance
    EXPECT_EQ(this->DepthByte(kFar), Red(depth, 0, 0));
  }
  // The depth, not the image: a lit box is white
  EXPECT_LT(this->DepthByte(1.0), 200);

  // Taken out again: the image of the scene, not its depth
  this->camera->RemoveRenderPass(depthPass);
  this->PlaceBox(1.0);
  EXPECT_TRUE(Same(before, this->Capture()));
}

/////////////////////////////////////////////////
/// A pass that reads the depth can be turned off and on.
TEST_P(Ogre2RenderPassDepthTest, DisabledAndEnabledAgain)
{
  const Image plain = this->Capture();

  RenderPassPtr depthPass = std::make_shared<CopyPass>(true);
  this->camera->AddRenderPass(depthPass);
  EXPECT_NEAR(this->DepthByte(1.0),
      Red(this->Capture(), kSize / 2, kSize / 2), 1);

  // Off: the image of the scene, as without the pass
  depthPass->SetEnabled(false);
  EXPECT_TRUE(Same(plain, this->Capture()));

  depthPass->SetEnabled(true);
  EXPECT_NEAR(this->DepthByte(1.0),
      Red(this->Capture(), kSize / 2, kSize / 2), 1);
}

INSTANTIATE_TEST_SUITE_P(AntiAliasing, Ogre2RenderPassDepthTest,
    testing::Values(0u, 4u),
    [](const testing::TestParamInfo<unsigned int> &_info)
    {
      return "AA" + std::to_string(_info.param);
    });
