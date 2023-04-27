/*
 * Copyright (C) 2023 Open Source Robotics Foundation
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

#include "CommonRenderingTest.hh"

#include <gz/common/Image.hh>

#include "gz/rendering/Camera.hh"
#include "gz/rendering/Material.hh"
#include "gz/rendering/Projector.hh"
#include "gz/rendering/Scene.hh"
#include "gz/rendering/Visual.hh"

#include <gz/utils/ExtraTestMacros.hh>

using namespace gz;
using namespace rendering;

class ProjectorTest: public CommonRenderingTest
{
  // Path to test media directory
  public: const std::string TEST_MEDIA_PATH =
          gz::common::joinPaths(std::string(PROJECT_SOURCE_PATH),
                "test", "media");
};


/////////////////////////////////////////////////
TEST_F(ProjectorTest, GZ_UTILS_TEST_DISABLED_ON_WIN32(Visibility))
{
  ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);

  // Projector is experimental feature in gz-rendering7 and can only be accessed
  // by the scene extension API
  if (!scene->Extension())
    return;

  scene->SetBackgroundColor(0, 0, 0);
  scene->SetAmbientLight(1, 1, 1);

  VisualPtr root = scene->RootVisual();
  ASSERT_NE(nullptr, root);

  DirectionalLightPtr light0 = scene->CreateDirectionalLight();
  light0->SetDirection(0.0, 0.0, -1);
  light0->SetDiffuseColor(1.0, 1.0, 1.0);
  light0->SetSpecularColor(1.0, 1.0, 1.0);
  root->AddChild(light0);

  CameraPtr cameraA = scene->CreateCamera();
  ASSERT_NE(nullptr, cameraA);
  cameraA->SetWorldPosition(0, 0, -2);
  cameraA->SetWorldRotation(0, GZ_PI / 2.0, 0);
  cameraA->SetVisibilityMask(0x01);
  cameraA->SetImageWidth(256);
  cameraA->SetImageHeight(256);
  root->AddChild(cameraA);

  CameraPtr cameraB = scene->CreateCamera();
  ASSERT_NE(nullptr, cameraB);
  cameraB->SetWorldPosition(0, 0, -2);
  cameraB->SetWorldRotation(0, GZ_PI / 2.0, 0);
  cameraB->SetVisibilityMask(0x02);
  cameraB->SetImageWidth(256);
  cameraB->SetImageHeight(256);
  root->AddChild(cameraB);

  // create projectors with different visibility flags
   std::string textureRed = common::joinPaths(
       TEST_MEDIA_PATH, "materials", "textures",
       "red_texture.png");
  // ProjectorPtr projectorA = std::dynamic_pointer_cast<Projector>(
  //     scene->Extension()->CreateExt("projector"));
  // \todo(iche033) uncomment and use official API in gz-rendering8
  ProjectorPtr projectorA = scene->CreateProjector();
  ASSERT_NE(nullptr, projectorA);
  projectorA->SetNearClipPlane(1.0);
  projectorA->SetFarClipPlane(6.0);
  projectorA->SetTexture(textureRed);
  projectorA->SetVisibilityFlags(0x01);
  projectorA->SetWorldRotation(0, GZ_PI / 2.0, 0);
  root->AddChild(projectorA);

  std::string textureBlue = common::joinPaths(
       TEST_MEDIA_PATH, "materials", "textures",
       "blue_texture.png");
  // \todo(iche033) uncomment and use official API in gz-rendering8
  ProjectorPtr projectorB = scene->CreateProjector();
  // ProjectorPtr projectorB = std::dynamic_pointer_cast<Projector>(
  //     scene->Extension()->CreateExt("projector"));
  ASSERT_NE(nullptr, projectorB);

  projectorB->SetNearClipPlane(1.0);
  projectorB->SetFarClipPlane(6.0);
  projectorB->SetTexture(textureBlue);
  projectorB->SetVisibilityFlags(0x02);
  projectorB->SetWorldRotation(0, GZ_PI / 2.0, 0);
  root->AddChild(projectorB);

  // create background wall visual for projection
  VisualPtr visual = scene->CreateVisual();
  visual->AddGeometry(scene->CreateBox());
  visual->SetWorldPosition(0.0, 0.0, -5);
  visual->SetLocalScale(10.0, 10.0, 1.0);
  root->AddChild(visual);

  // create green material and assign to wall
  MaterialPtr green = scene->CreateMaterial();
  green->SetAmbient(0.0, 1.0, 0.0);
  green->SetDiffuse(0.0, 1.0, 0.0);
  green->SetSpecular(0.0, 1.0, 0.0);
  visual->SetMaterial(green);

  // create images to store camera data
  Image imageA = cameraA->CreateImage();
  Image imageB = cameraB->CreateImage();

  unsigned int height = cameraA->ImageHeight();
  unsigned int width = cameraA->ImageWidth();
  unsigned int bpp = PixelUtil::BytesPerPixel(cameraA->ImageFormat());
  unsigned int step = width * bpp;

  ASSERT_GT(height, 0u);
  ASSERT_GT(width, 0u);
  ASSERT_GT(bpp, 0u);

  // verify that cameraA only sees red texture from projector A and
  // cameraB only sees texture from projector B
  // \todo(anyone) ogre requires rendering a few frames to get correct output
  // need to investigate and make sure we can generate correct output by
  // rendering only once
  unsigned int iterations = 1u;
  if (engine->Name() == "ogre")
    iterations = 3u;
  for (unsigned int i = 0; i < iterations; ++i)
  {
    cameraA->Capture(imageA);
    cameraB->Capture(imageB);
  }

  unsigned char *dataA = imageA.Data<unsigned char>();
  unsigned char *dataB = imageB.Data<unsigned char>();

  common::Image imgA;
  imgA.SetFromData(dataA, width, height, common::Image::RGB_INT8);
  imgA.SavePNG("imageA.png");

  common::Image imgB;
  imgB.SetFromData(dataB, width, height, common::Image::RGB_INT8);
  imgB.SavePNG("imageB.png");

  for (unsigned int i = 0; i < height; ++i)
  {
    for (unsigned int j = 0; j < step; j+=bpp)
    {
      unsigned int idx = i * step + j;
      unsigned int rA = dataA[idx];
      unsigned int gA = dataA[idx+1];
      unsigned int bA = dataA[idx+2];

      // color should be predominantly red
      EXPECT_GT(rA, gA);
      EXPECT_GT(rA, bA);

      unsigned int rB = dataB[idx];
      unsigned int gB = dataB[idx+1];
      unsigned int bB = dataB[idx+2];

      // color should be predominantly blue
      EXPECT_GT(bB, gB);
      EXPECT_GT(bB, rB);
    }
  }

  // Clean up
  engine->DestroyScene(scene);
}
