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

#include <string>

#include <gz/utils/ExtraTestMacros.hh>

#include "gz/rendering/Camera.hh"
#include "gz/rendering/DepthCamera.hh"
#include "gz/rendering/GaussianNoisePass.hh"
#include "gz/rendering/Image.hh"
#include "gz/rendering/RenderEngine.hh"
#include "gz/rendering/RenderingIface.hh"
#include "gz/rendering/RenderPassSystem.hh"
#include "gz/rendering/Scene.hh"
#include "gz/rendering/Visual.hh"

#ifdef _MSC_VER
  #pragma warning(push, 0)
#endif
#include <Compositor/OgreCompositorManager2.h>
#include <Compositor/OgreCompositorNodeDef.h>
#include <OgreMaterialManager.h>
#include <OgreRoot.h>
#ifdef _MSC_VER
  #pragma warning(pop)
#endif

using namespace gz;
using namespace rendering;

namespace
{
/// \brief Number of Ogre materials whose name starts with a prefix
std::size_t CountMaterials(const std::string &_prefix)
{
  std::size_t count = 0u;
  auto it = Ogre::MaterialManager::getSingleton().getResourceIterator();
  while (it.hasMoreElements())
  {
    if (it.getNext()->getName().rfind(_prefix, 0) == 0)
      ++count;
  }
  return count;
}

/// \brief Number of compositor node definitions whose name starts with a
/// prefix
std::size_t CountNodeDefinitions(const std::string &_prefix)
{
  std::size_t count = 0u;
  const auto &nodeDefs = Ogre::Root::getSingleton().getCompositorManager2()
      ->getNodeDefinitions();
  for (const auto &nodeDef : nodeDefs)
  {
    if (nodeDef.second->getNameStr().rfind(_prefix, 0) == 0)
      ++count;
  }
  return count;
}
}  // namespace

/////////////////////////////////////////////////
/// \brief Noise passes clone a material, which must be removed when the
/// pass goes away. Otherwise every camera with noise that is created and
/// destroyed leaks one.
TEST(Ogre2GaussianNoisePass, GZ_UTILS_TEST_DISABLED_ON_WIN32(NoMaterialLeak))
{
  RenderEngine *engine = rendering::engine("ogre2");
  if (!engine)
    GTEST_SKIP() << "ogre2 engine is not available";

  const std::size_t colorBefore = CountMaterials("GaussianNoise_");
  const std::size_t depthBefore = CountMaterials("GaussianNoiseDepth_");

  for (int i = 0; i < 3; ++i)
  {
    ScenePtr scene = engine->CreateScene("scene");
    ASSERT_NE(nullptr, scene);
    VisualPtr box = scene->CreateVisual();
    box->AddGeometry(scene->CreateBox());
    box->SetLocalPosition(2, 0, 0);
    scene->RootVisual()->AddChild(box);

    RenderPassPtr colorPass =
        engine->RenderPassSystem()->Create<GaussianNoisePass>();
    CameraPtr camera = scene->CreateCamera("camera");
    camera->SetImageWidth(32);
    camera->SetImageHeight(32);
    camera->AddRenderPass(colorPass);
    scene->RootVisual()->AddChild(camera);
    camera->Update();

    RenderPassPtr depthPass =
        engine->RenderPassSystem()->Create<GaussianNoisePass>();
    DepthCameraPtr depthCamera = scene->CreateDepthCamera("depth");
    depthCamera->SetImageWidth(32);
    depthCamera->SetImageHeight(32);
    depthCamera->SetNearClipPlane(0.1);
    depthCamera->SetFarClipPlane(10);
    depthCamera->CreateDepthTexture();
    depthCamera->AddRenderPass(depthPass);
    scene->RootVisual()->AddChild(depthCamera);
    depthCamera->Update();
    // The color noise pass is now the last quad pass drawn, so Ogre's
    // shared fullscreen quad still uses its material when it is removed
    camera->Update();

    EXPECT_EQ(colorBefore + 1u, CountMaterials("GaussianNoise_"));
    EXPECT_EQ(depthBefore + 1u, CountMaterials("GaussianNoiseDepth_"));

    engine->DestroyScene(scene);
    camera.reset();
    depthCamera.reset();
    colorPass.reset();
    depthPass.reset();

    EXPECT_EQ(colorBefore, CountMaterials("GaussianNoise_"));
    EXPECT_EQ(depthBefore, CountMaterials("GaussianNoiseDepth_"));
  }

  unloadEngine(engine->Name());
}

/////////////////////////////////////////////////
/// \brief Noise passes create a compositor node definition, which can't be
/// removed with the pass: a workspace may still use it. A new pass must
/// reuse it instead, so the number of definitions doesn't keep growing.
TEST(Ogre2GaussianNoisePass,
     GZ_UTILS_TEST_DISABLED_ON_WIN32(NodeDefinitionsAreReused))
{
  RenderEngine *engine = rendering::engine("ogre2");
  if (!engine)
    GTEST_SKIP() << "ogre2 engine is not available";

  std::size_t colorNodes = 0u;
  std::size_t depthNodes = 0u;
  unsigned char background = 0u;
  for (int i = 0; i < 3; ++i)
  {
    ScenePtr scene = engine->CreateScene("scene");
    ASSERT_NE(nullptr, scene);
    scene->SetBackgroundColor(0, 0, 0);

    // Noise of mean 0.5 and no deviation turns the black background grey
    RenderPassPtr colorPass =
        engine->RenderPassSystem()->Create<GaussianNoisePass>();
    auto noise = std::dynamic_pointer_cast<GaussianNoisePass>(colorPass);
    ASSERT_NE(nullptr, noise);
    noise->SetMean(0.5);
    noise->SetStdDev(0.0);
    CameraPtr camera = scene->CreateCamera("camera");
    camera->SetImageWidth(32);
    camera->SetImageHeight(32);
    camera->AddRenderPass(colorPass);
    scene->RootVisual()->AddChild(camera);
    Image image = camera->CreateImage();
    camera->Capture(image);
    const unsigned char *data = image.Data<unsigned char>();
    ASSERT_NE(nullptr, data);

    RenderPassPtr depthPass =
        engine->RenderPassSystem()->Create<GaussianNoisePass>();
    DepthCameraPtr depthCamera = scene->CreateDepthCamera("depth");
    depthCamera->SetImageWidth(32);
    depthCamera->SetImageHeight(32);
    depthCamera->SetNearClipPlane(0.1);
    depthCamera->SetFarClipPlane(10);
    depthCamera->CreateDepthTexture();
    depthCamera->AddRenderPass(depthPass);
    scene->RootVisual()->AddChild(depthCamera);
    depthCamera->Update();

    if (i == 0)
    {
      colorNodes = CountNodeDefinitions("GaussianNoiseNodeNode_");
      depthNodes = CountNodeDefinitions("GaussianDepthNoiseNode_");
      background = data[0];
      EXPECT_GT(background, 100u);
      EXPECT_LT(background, 160u);
    }
    else
    {
      // a reused node still draws the noise
      EXPECT_EQ(background, data[0]);
      EXPECT_EQ(colorNodes, CountNodeDefinitions("GaussianNoiseNodeNode_"));
      EXPECT_EQ(depthNodes, CountNodeDefinitions("GaussianDepthNoiseNode_"));
    }

    engine->DestroyScene(scene);
    camera.reset();
    depthCamera.reset();
    colorPass.reset();
    depthPass.reset();
  }

  unloadEngine(engine->Name());
}
