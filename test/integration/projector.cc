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
#include <gz/common/geospatial/ImageHeightmap.hh>

#include "gz/rendering/Camera.hh"
#include "gz/rendering/Heightmap.hh"
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
/// \brief Count the pixels of an RGB image where one channel is larger
/// than the other two
/// \param[in] _image Image captured by a camera
/// \param[in] _channel 0 for red, 1 for green, 2 for blue
/// \return Number of pixels where _channel is the largest
static unsigned int DominantPixelCount(Image &_image, unsigned int _channel)
{
  const unsigned int bpp = PixelUtil::BytesPerPixel(_image.Format());
  const unsigned int count = _image.Width() * _image.Height();
  const unsigned char *data = _image.Data<unsigned char>();
  unsigned int result = 0u;
  for (unsigned int i = 0; i < count; ++i)
  {
    const unsigned char *px = data + i * bpp;
    const unsigned char value = px[_channel];
    if (value > px[(_channel + 1) % 3] && value > px[(_channel + 2) % 3])
      ++result;
  }
  return result;
}

/////////////////////////////////////////////////
TEST_F(ProjectorTest, GZ_UTILS_TEST_DISABLED_ON_WIN32(Visibility))
{
  ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);

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
  ProjectorPtr projectorB = scene->CreateProjector();
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
  // ogre requires rendering a couple of frames to get correct output
  unsigned int iterations = 1u;
  if (engine->Name() == "ogre")
    iterations = 2u;
  for (unsigned int i = 0; i < iterations; ++i)
  {
    cameraA->Capture(imageA);
    cameraB->Capture(imageB);
  }

  unsigned char *dataA = imageA.Data<unsigned char>();
  unsigned char *dataB = imageB.Data<unsigned char>();

  common::Image imgA;
  imgA.SetFromData(dataA, width, height, common::Image::RGB_INT8);

  common::Image imgB;
  imgB.SetFromData(dataB, width, height, common::Image::RGB_INT8);

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

/////////////////////////////////////////////////
TEST_F(ProjectorTest, GZ_UTILS_TEST_DISABLED_ON_WIN32(Heightmap))
{
  // This test checks that projectors and heightmaps can co-exist
  ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);

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

  // create projector
   std::string textureRed = common::joinPaths(
       TEST_MEDIA_PATH, "materials", "textures",
       "red_texture.png");
  ProjectorPtr projectorA = scene->CreateProjector();
  ASSERT_NE(nullptr, projectorA);
  projectorA->SetNearClipPlane(1.0);
  projectorA->SetFarClipPlane(6.0);
  projectorA->SetTexture(textureRed);
  projectorA->SetVisibilityFlags(0x01);
  projectorA->SetWorldRotation(0, GZ_PI / 2.0, 0);
  root->AddChild(projectorA);

  // create ImageHeightmap
  auto data = std::make_shared<common::ImageHeightmap>();
  data->Load(common::joinPaths(TEST_MEDIA_PATH, "heightmap_bowl.png"));

  HeightmapDescriptor desc;
  desc.SetName("example_bowl");
  desc.SetData(data);
  desc.SetSize({ 17, 17, 7.0 });
  desc.SetSampling(2u);
  desc.SetUseTerrainPaging(false);

  const auto textureImage =
    common::joinPaths(TEST_MEDIA_PATH, "materials", "textures", "texture.png");
  const auto normalImage = common::joinPaths(TEST_MEDIA_PATH, "materials",
                                             "textures", "flat_normal.png");

  HeightmapTexture textureA;
  textureA.SetSize(1.0);
  textureA.SetDiffuse(textureImage);
  textureA.SetNormal(normalImage);
  desc.AddTexture(textureA);

  HeightmapBlend blendA;
  blendA.SetMinHeight(2.0);
  blendA.SetFadeDistance(5.0);
  desc.AddBlend(blendA);

  HeightmapTexture textureB;
  textureB.SetSize(1.0);
  textureB.SetDiffuse(textureImage);
  textureB.SetNormal(normalImage);
  desc.AddTexture(textureB);

  HeightmapBlend blendB;
  blendB.SetMinHeight(4.0);
  blendB.SetFadeDistance(5.0);
  desc.AddBlend(blendB);

  HeightmapTexture textureC;
  textureC.SetSize(1.0);
  textureC.SetDiffuse(textureImage);
  textureC.SetNormal(normalImage);
  desc.AddTexture(textureC);
  auto heightmapGeom = scene->CreateHeightmap(desc);
  auto vis = scene->CreateVisual();
  vis->AddGeometry(heightmapGeom);
  root->AddChild(vis);

  // render once to update scene graph and make sure
  // there are no crashes
  Image imageA = cameraA->CreateImage();
  EXPECT_NO_THROW(cameraA->Capture(imageA));

  // Clean up
  engine->DestroyScene(scene);
}

/////////////////////////////////////////////////
// The decal must come back when the projector turns away from a wall and
// back again. In ogre, the RTSS can rebuild the shader techniques of the
// cameras that see the decal while it is off, for example after a shadow
// texture pass or a change in the scene lights.
TEST_F(ProjectorTest,
    GZ_UTILS_TEST_DISABLED_ON_WIN32(DecalAfterTurningAwayAndBack))
{
  ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);

  scene->SetBackgroundColor(0, 0, 0);
  scene->SetAmbientLight(1, 1, 1);

  VisualPtr root = scene->RootVisual();
  ASSERT_NE(nullptr, root);

  DirectionalLightPtr light0 = scene->CreateDirectionalLight();
  light0->SetDirection(0.0, 0.0, -1);
  light0->SetDiffuseColor(1.0, 1.0, 1.0);
  light0->SetSpecularColor(1.0, 1.0, 1.0);
  root->AddChild(light0);

  CameraPtr camera = scene->CreateCamera();
  ASSERT_NE(nullptr, camera);
  camera->SetWorldPosition(0, 0, -2);
  camera->SetWorldRotation(0, GZ_PI / 2.0, 0);
  camera->SetVisibilityMask(0x01);
  camera->SetImageWidth(256);
  camera->SetImageHeight(256);
  root->AddChild(camera);

  // custom visibility flags, so that only cameras with a matching mask see
  // the decal
  std::string textureRed = common::joinPaths(
      TEST_MEDIA_PATH, "materials", "textures", "red_texture.png");
  ProjectorPtr projector = scene->CreateProjector();
  ASSERT_NE(nullptr, projector);
  projector->SetNearClipPlane(1.0);
  projector->SetFarClipPlane(6.0);
  projector->SetTexture(textureRed);
  projector->SetVisibilityFlags(0x01);
  projector->SetWorldRotation(0, GZ_PI / 2.0, 0);
  root->AddChild(projector);

  // green wall that fills the camera view
  VisualPtr wall = scene->CreateVisual();
  wall->AddGeometry(scene->CreateBox());
  wall->SetWorldPosition(0.0, 0.0, -5);
  wall->SetLocalScale(10.0, 10.0, 1.0);
  root->AddChild(wall);

  MaterialPtr green = scene->CreateMaterial();
  green->SetAmbient(0.0, 1.0, 0.0);
  green->SetDiffuse(0.0, 1.0, 0.0);
  green->SetSpecular(0.0, 1.0, 0.0);
  wall->SetMaterial(green);

  Image image = camera->CreateImage();
  const unsigned int pixelCount = image.Width() * image.Height();
  ASSERT_GT(pixelCount, 0u);

  // ogre requires rendering a couple of frames to get correct output
  auto capture = [&]()
  {
    for (unsigned int i = 0; i < 2u; ++i)
      camera->Capture(image);
  };

  // the red decal covers the whole view
  capture();
  EXPECT_EQ(pixelCount, DominantPixelCount(image, 0u));

  // turn the projector away, the camera sees the green wall only
  projector->SetWorldRotation(0, -GZ_PI / 2.0, 0);
  capture();
  EXPECT_EQ(0u, DominantPixelCount(image, 0u));
  EXPECT_EQ(pixelCount, DominantPixelCount(image, 1u));

  // add a light while the decal is off
  DirectionalLightPtr light1 = scene->CreateDirectionalLight();
  light1->SetDirection(0.0, 0.0, -1);
  light1->SetDiffuseColor(0.2, 0.2, 0.2);
  light1->SetSpecularColor(0.2, 0.2, 0.2);
  root->AddChild(light1);
  capture();
  EXPECT_EQ(0u, DominantPixelCount(image, 0u));

  // turn the projector back, the decal is drawn again
  projector->SetWorldRotation(0, GZ_PI / 2.0, 0);
  capture();
  EXPECT_EQ(pixelCount, DominantPixelCount(image, 0u));

  // Clean up
  engine->DestroyScene(scene);
}

/////////////////////////////////////////////////
// A projector with the default visibility flags must not hide the decal of
// a projector with custom flags on the surfaces they both cover.
TEST_F(ProjectorTest,
    GZ_UTILS_TEST_DISABLED_ON_WIN32(DefaultAndCustomFlagsOnSameWall))
{
  ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);

  scene->SetBackgroundColor(0, 0, 0);
  scene->SetAmbientLight(1, 1, 1);

  VisualPtr root = scene->RootVisual();
  ASSERT_NE(nullptr, root);

  DirectionalLightPtr light0 = scene->CreateDirectionalLight();
  light0->SetDirection(0.0, 0.0, -1);
  light0->SetDiffuseColor(1.0, 1.0, 1.0);
  light0->SetSpecularColor(1.0, 1.0, 1.0);
  root->AddChild(light0);

  // camera that matches the custom flags of the blue projector
  CameraPtr camera = scene->CreateCamera();
  ASSERT_NE(nullptr, camera);
  camera->SetWorldPosition(0, 0, -2);
  camera->SetWorldRotation(0, GZ_PI / 2.0, 0);
  camera->SetVisibilityMask(0x02);
  camera->SetImageWidth(256);
  camera->SetImageHeight(256);
  root->AddChild(camera);

  // red projector seen by all cameras, covering one side of the view
  std::string textureRed = common::joinPaths(
      TEST_MEDIA_PATH, "materials", "textures", "red_texture.png");
  ProjectorPtr projectorAll = scene->CreateProjector();
  ASSERT_NE(nullptr, projectorAll);
  projectorAll->SetNearClipPlane(1.0);
  projectorAll->SetFarClipPlane(6.0);
  projectorAll->SetTexture(textureRed);
  projectorAll->SetVisibilityFlags(GZ_VISIBILITY_ALL);
  projectorAll->SetWorldPosition(0, 3.0, 0);
  projectorAll->SetWorldRotation(0, GZ_PI / 2.0, 0);
  root->AddChild(projectorAll);

  // blue projector with custom flags, covering the other side of the view
  std::string textureBlue = common::joinPaths(
      TEST_MEDIA_PATH, "materials", "textures", "blue_texture.png");
  ProjectorPtr projectorCustom = scene->CreateProjector();
  ASSERT_NE(nullptr, projectorCustom);
  projectorCustom->SetNearClipPlane(1.0);
  projectorCustom->SetFarClipPlane(6.0);
  projectorCustom->SetTexture(textureBlue);
  projectorCustom->SetVisibilityFlags(0x02);
  projectorCustom->SetWorldPosition(0, -3.0, 0);
  projectorCustom->SetWorldRotation(0, GZ_PI / 2.0, 0);
  root->AddChild(projectorCustom);

  // green wall that fills the camera view
  VisualPtr wall = scene->CreateVisual();
  wall->AddGeometry(scene->CreateBox());
  wall->SetWorldPosition(0.0, 0.0, -5);
  wall->SetLocalScale(10.0, 10.0, 1.0);
  root->AddChild(wall);

  MaterialPtr green = scene->CreateMaterial();
  green->SetAmbient(0.0, 1.0, 0.0);
  green->SetDiffuse(0.0, 1.0, 0.0);
  green->SetSpecular(0.0, 1.0, 0.0);
  wall->SetMaterial(green);

  Image image = camera->CreateImage();
  const unsigned int pixelCount = image.Width() * image.Height();
  ASSERT_GT(pixelCount, 0u);

  // ogre requires rendering a couple of frames to get correct output
  for (unsigned int i = 0; i < 2u; ++i)
    camera->Capture(image);

  // both decals are visible, each on its side of the view
  EXPECT_GT(DominantPixelCount(image, 0u), pixelCount / 8u);
  EXPECT_GT(DominantPixelCount(image, 2u), pixelCount / 8u);

  // Clean up. Release the projectors first so that they are destroyed with
  // the scene: the one with the default flags removes its decal from the
  // wall's material, which does not exist after the scene is destroyed.
  projectorAll.reset();
  projectorCustom.reset();
  engine->DestroyScene(scene);
}
