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

#include <algorithm>

#include "CommonRenderingTest.hh"

#include <gz/common/Image.hh>
#include <gz/math/Helpers.hh>

#include "gz/rendering/Camera.hh"
#include "gz/rendering/Image.hh"
#include "gz/rendering/PixelFormat.hh"
#include "gz/rendering/Scene.hh"

#include <gz/utils/Environment.hh>
#include <gz/utils/ExtraTestMacros.hh>

using namespace gz;
using namespace rendering;

class ShadowsTest: public CommonRenderingTest
{
};

/////////////////////////////////////////////////
TEST_F(ShadowsTest, GZ_UTILS_TEST_DISABLED_ON_WIN32(Shadows))
{
  // override and make sure not to look for resources in installed share dir
  std::string projectSrcPath = PROJECT_SOURCE_PATH;
  // (setenv copies the value: putenv would keep a pointer to a local)
  ASSERT_TRUE(gz::utils::setenv("GZ_RENDERING_RESOURCE_PATH",
      projectSrcPath));

  // add resources in build dir
  engine->AddResourcePath(
      common::joinPaths(std::string(PROJECT_BUILD_PATH), "src"));

  ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);
  scene->SetAmbientLight(0.3, 0.3, 0.3);

  VisualPtr root = scene->RootVisual();
  ASSERT_NE(nullptr, root);

  CameraPtr camera = scene->CreateCamera();
  ASSERT_NE(nullptr, camera);

  // downward looking camera
  camera->SetImageWidth(10);
  camera->SetImageHeight(10);
  camera->SetLocalRotation(0, 1.57, 0);
  root->AddChild(camera);


  // create downward directional light
  DirectionalLightPtr light = scene->CreateDirectionalLight();
  light->SetDirection(0.0, 0.0, -1);
  light->SetDiffuseColor(0.5, 0.5, 0.5);
  light->SetSpecularColor(0.5, 0.5, 0.5);
  root->AddChild(light);

  MaterialPtr white = scene->CreateMaterial();
  white->SetAmbient(1.0, 1.0, 1.0);
  white->SetDiffuse(1.0, 1.0, 1.0);
  white->SetSpecular(1.0, 1.0, 1.0);
  white->SetCastShadows(true);

  // create box that casts shadows
  VisualPtr boxTop = scene->CreateVisual();
  boxTop->AddGeometry(scene->CreateBox());
  boxTop->SetLocalPosition(0.0, 0.5, 0.55);
  boxTop->SetMaterial(white, false);
  root->AddChild(boxTop);

  // create green material
  MaterialPtr green = scene->CreateMaterial();
  green->SetAmbient(0.0, 0.5, 0.0);
  green->SetDiffuse(0.0, 0.7, 0.0);
  green->SetSpecular(0.5, 0.5, 0.5);
  green->SetShininess(50);
  green->SetReflectivity(0);

  // create box at origin that receives shadows
  VisualPtr boxBottom = scene->CreateVisual();
  boxBottom->AddGeometry(scene->CreateBox());
  boxBottom->SetLocalPosition(0.0, 0.0, -1.0);
  boxBottom->SetMaterial(green);
  root->AddChild(boxBottom);

  Image image = camera->CreateImage();
  unsigned int height = camera->ImageHeight();
  unsigned int width = camera->ImageWidth();
  unsigned int bpp = PixelUtil::BytesPerPixel(camera->ImageFormat());
  unsigned int step = width * bpp;

  // verify shadows are generated for a number of images
  for (unsigned int k = 0; k < 10; ++k)
  {
    // sum of shaded / unshaded pixel values
    unsigned shaded = 0;
    unsigned unshaded = 0;

    camera->Capture(image);
    unsigned char *data = image.Data<unsigned char>();
    for (unsigned int i = 0; i < height; ++i)
    {
      for (unsigned int j = 0; j < step; j+=bpp)
      {
        unsigned int idx = i * step + j;
        unsigned int r = data[idx];
        unsigned int g = data[idx+1];
        unsigned int b = data[idx+2];

        // color should be a shade of green
        EXPECT_GT(g, r);
        EXPECT_GT(g, b);

        if (j < step /2)
          shaded += r + g + b;
        else
          unshaded += r + g + b;
      }
    }
    // Test currently fails on macOS
#ifndef __APPLE__
    // left side of image should be significanly darker than the right side
    EXPECT_LT(shaded, unshaded);
#endif
  }

  // disable shadows and verify shades
  white->SetCastShadows(false);
  boxTop->SetMaterial(white);


  // verify no shadows are generated for a number of images
  for (unsigned int k = 0; k < 10; ++k)
  {
    // sum of left / right pixel values
    unsigned left = 0;
    unsigned right = 0;

    camera->Capture(image);
    unsigned char *data = image.Data<unsigned char>();
    for (unsigned int i = 0; i < height; ++i)
    {
      for (unsigned int j = 0; j < step; j+=bpp)
      {
        unsigned int idx = i * step + j;
        unsigned int r = data[idx];
        unsigned int g = data[idx+1];
        unsigned int b = data[idx+2];

        // color should be a shade of green
        EXPECT_GT(g, r);
        EXPECT_GT(g, b);

        if (j < step /2)
          left += r + g + b;
        else
          right += r + g + b;
      }
    }
    // Test currently fails on macOS
#ifndef __APPLE__
    // left side of image should be similar to right side
    EXPECT_NEAR(left, right, 5);
#endif
  }

  // Clean up materials
  scene->DestroyMaterial(white);
  scene->DestroyMaterial(green);

  // Clean up
  engine->DestroyScene(scene);
}

/////////////////////////////////////////////////
// A very large shadow-casting ground plane (gz-sim worlds use planes of
// up to 100 km) seen from a camera close to the ground. Nothing occludes
// the sun, so the ground must be evenly lit: no shadow acne, no dark
// patch from a badly fitted shadow map.
TEST_F(ShadowsTest, GZ_UTILS_TEST_DISABLED_ON_WIN32(LargeGroundPlane))
{
  // ogre does not light this ground evenly: one of the views below fails
  CHECK_SUPPORTED_ENGINE("ogre2", "filament");

  ScenePtr scene = engine->CreateScene("largeGroundPlane");
  ASSERT_NE(nullptr, scene);
  scene->SetAmbientLight(0.4, 0.4, 0.4);

  VisualPtr root = scene->RootVisual();
  ASSERT_NE(nullptr, root);

  DirectionalLightPtr light = scene->CreateDirectionalLight();
  light->SetDirection(-0.5, 0.1, -0.9);
  light->SetDiffuseColor(1.0, 1.0, 1.0);
  light->SetSpecularColor(0.5, 0.5, 0.5);
  light->SetCastShadows(true);
  root->AddChild(light);

  MaterialPtr grey = scene->CreateMaterial();
  grey->SetAmbient(0.8, 0.8, 0.8);
  grey->SetDiffuse(0.8, 0.8, 0.8);
  grey->SetSpecular(0.8, 0.8, 0.8);
  grey->SetCastShadows(true);

  // 100 km x 100 m, like gz-sim's plane_propeller_demo world
  VisualPtr ground = scene->CreateVisual();
  ground->AddGeometry(scene->CreatePlane());
  ground->SetLocalScale(100000.0, 100.0, 1.0);
  ground->SetMaterial(grey);
  root->AddChild(ground);

  // 1.5 m above the ground, far clip beyond 500 m
  CameraPtr camera = scene->CreateCamera();
  ASSERT_NE(nullptr, camera);
  camera->SetImageWidth(320);
  camera->SetImageHeight(240);
  camera->SetHFOV(1.347);
  camera->SetNearClipPlane(0.1);
  camera->SetFarClipPlane(1000.0);
  root->AddChild(camera);

  Image image = camera->CreateImage();
  unsigned int height = camera->ImageHeight();
  unsigned int width = camera->ImageWidth();
  unsigned int bpp = PixelUtil::BytesPerPixel(camera->ImageFormat());

  // a few headings: how the shadow map is fit depends on the view
  for (double yaw : {2.5, 3.0, GZ_PI})
  {
    camera->SetLocalPosition(0.0, 0.0, 1.5);
    camera->SetLocalRotation(0.0, 0.0, yaw);
    for (unsigned int k = 0; k < 3; ++k)
      camera->Capture(image);

    // the bottom quarter of the image is ground within 10 m of the camera
    const unsigned char *data = image.Data<unsigned char>();
    unsigned int minSum = 3 * 255;
    unsigned int maxSum = 0;
    for (unsigned int i = height * 3 / 4; i < height; ++i)
    {
      for (unsigned int j = 0; j < width; ++j)
      {
        unsigned int idx = (i * width + j) * bpp;
        unsigned int sum = data[idx] + data[idx + 1] + data[idx + 2];
        minSum = std::min(minSum, sum);
        maxSum = std::max(maxSum, sum);
      }
    }
    EXPECT_GT(minSum, 0u) << "yaw " << yaw;
    EXPECT_LT(maxSum - minSum, 30u) << "yaw " << yaw;
  }

  scene->DestroyMaterial(grey);
  engine->DestroyScene(scene);
}
