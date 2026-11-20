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
#include <memory>
#include <string>
#include <vector>

#include "CommonRenderingTest.hh"

#include <gz/common/Filesystem.hh>
#include <gz/common/geospatial/ImageHeightmap.hh>
#include <gz/utils/ExtraTestMacros.hh>

#include "gz/rendering/Heightmap.hh"
#include "gz/rendering/Scene.hh"
#include "gz/rendering/ThermalCamera.hh"
#include "gz/rendering/Visual.hh"

using namespace gz;
using namespace rendering;

namespace
{
constexpr unsigned int kWidth = 80;
constexpr unsigned int kHeight = 60;
constexpr double kHfov = 1.05;
constexpr float kResolution = 0.01f;
constexpr float kAmbient = 296.0f;
constexpr float kAmbientRange = 4.0f;
constexpr float kHeatRange = 3.0f;

/// \brief A thermal camera at the origin looking along +X, and its last
/// image.
struct ThermalRig
{
  ThermalCameraPtr camera;
  std::vector<uint16_t> image;
  common::ConnectionPtr connection;

  /// \brief Render one frame
  void Update()
  {
    this->camera->Update();
  }

  /// \brief Temperature (K) at a pixel of the last image
  float Temperature(unsigned int _x, unsigned int _y) const
  {
    return this->image[_y * kWidth + _x] * kResolution;
  }

  /// \brief Temperature (K) at the center of the last image
  float Center() const
  {
    return this->Temperature(kWidth / 2, kHeight / 2);
  }
};

/// \brief Create a thermal camera in _scene
/// \param[in] _scene The scene
/// \param[out] _rig The camera and its image
void CreateRig(ScenePtr _scene, ThermalRig &_rig)
{
  _rig.camera = _scene->CreateThermalCamera();
  ASSERT_NE(nullptr, _rig.camera);
  _rig.camera->SetImageWidth(kWidth);
  _rig.camera->SetImageHeight(kHeight);
  _rig.camera->SetAspectRatio(static_cast<double>(kWidth) / kHeight);
  _rig.camera->SetHFOV(kHfov);
  _rig.camera->SetNearClipPlane(0.15);
  _rig.camera->SetFarClipPlane(10.0);
  _rig.camera->SetAmbientTemperature(kAmbient);
  _rig.camera->SetAmbientTemperatureRange(kAmbientRange);
  _rig.camera->SetLinearResolution(kResolution);
  _rig.camera->SetHeatSourceTemperatureRange(kHeatRange);
  _scene->RootVisual()->AddChild(_rig.camera);

  _rig.image.assign(kWidth * kHeight, 0u);
  auto *image = &_rig.image;
  _rig.connection = _rig.camera->ConnectNewThermalFrame(
      [image](const uint16_t *_data, unsigned int _w, unsigned int _h,
          unsigned int _channels, const std::string &)
      {
        std::copy(_data, _data + _w * _h * _channels, image->begin());
      });
}

/// \brief Create a unit box visual with a temperature
VisualPtr CreateBox(ScenePtr _scene, const math::Vector3d &_pos,
    float _temperature)
{
  VisualPtr box = _scene->CreateVisual();
  box->AddGeometry(_scene->CreateBox());
  box->SetLocalPosition(_pos);
  box->SetUserData("temperature", _temperature);
  return box;
}
}  // namespace

/// \brief Tests of what the thermal camera shows, mostly of cases its
/// material switcher has to handle: objects at the edge of the view,
/// heightmaps without a visual, objects added and removed.
class ThermalCameraAccuracyTest : public CommonRenderingTest
{
  /// \brief Path to test media files
  public: const std::string kMediaPath = common::joinPaths(
      std::string(PROJECT_SOURCE_PATH), "test", "media");
};

/////////////////////////////////////////////////
/// \brief A heightmap that isn't attached to a visual must not crash.
TEST_F(ThermalCameraAccuracyTest,
       GZ_UTILS_TEST_DISABLED_ON_WIN32(UnattachedHeightmapDoesNotCrash))
{
  CHECK_SUPPORTED_ENGINE("ogre2");
  ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);
  scene->SetAmbientLight(1.0, 1.0, 1.0);
  scene->RootVisual()->AddChild(CreateBox(scene, {2, 0, 0}, 310.0f));

  auto data = std::make_shared<common::ImageHeightmap>();
  ASSERT_EQ(0, data->Load(
      common::joinPaths(this->kMediaPath, "heightmap_bowl.png")));
  HeightmapDescriptor desc;
  desc.SetData(data);
  desc.SetSize({17, 17, 7});
  desc.SetSampling(2u);
  desc.SetUseTerrainPaging(false);
  // never added to a visual
  auto heightmap = scene->CreateHeightmap(desc);
  ASSERT_NE(nullptr, heightmap);

  ThermalRig rig;
  CreateRig(scene, rig);
  rig.Update();
  rig.Update();
  EXPECT_NEAR(310.0f, rig.Center(), kHeatRange);

  engine->DestroyScene(scene);
}

/////////////////////////////////////////////////
/// \brief Objects whose center is outside the view, but that are partly in
/// view, must be drawn with their temperature (heat sources) or as
/// background objects.
TEST_F(ThermalCameraAccuracyTest,
       GZ_UTILS_TEST_DISABLED_ON_WIN32(ObjectsOnTheEdgeOfTheView))
{
  CHECK_SUPPORTED_ENGINE("ogre2");
  ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);
  scene->SetAmbientLight(1.0, 1.0, 1.0);

  // The view's half width at x = 3 is 3 * tan(kHfov / 2) = 1.74 m: the box
  // centers are outside it, the boxes' inner sides are inside.
  const double y = 3.0 * std::tan(kHfov / 2.0) + 0.3;
  scene->RootVisual()->AddChild(CreateBox(scene, {3, y, 0}, 310.0f));
  // a background object (no temperature) on the other side
  VisualPtr background = scene->CreateVisual();
  background->AddGeometry(scene->CreateBox());
  background->SetLocalPosition(3, -y, 0);
  scene->RootVisual()->AddChild(background);

  ThermalRig rig;
  CreateRig(scene, rig);
  rig.Update();
  rig.Update();

  // +Y is on the left of the image
  EXPECT_NEAR(310.0f, rig.Temperature(0, kHeight / 2), kHeatRange);
  EXPECT_NEAR(kAmbient, rig.Temperature(kWidth - 1, kHeight / 2),
      kAmbientRange);
  EXPECT_NEAR(kAmbient, rig.Center(), kAmbientRange);

  engine->DestroyScene(scene);
}

/////////////////////////////////////////////////
/// \brief Objects moved into view, and objects replaced by others, must
/// show their current temperature.
TEST_F(ThermalCameraAccuracyTest,
       GZ_UTILS_TEST_DISABLED_ON_WIN32(MovedAndReplacedObjects))
{
  CHECK_SUPPORTED_ENGINE("ogre2");
  ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);
  scene->SetAmbientLight(1.0, 1.0, 1.0);

  // behind the camera
  VisualPtr box = CreateBox(scene, {-2, 0, 0}, 310.0f);
  scene->RootVisual()->AddChild(box);

  ThermalRig rig;
  CreateRig(scene, rig);
  rig.Update();
  EXPECT_NEAR(kAmbient, rig.Center(), kAmbientRange);

  // in front
  box->SetLocalPosition(2, 0, 0);
  rig.Update();
  EXPECT_NEAR(310.0f, rig.Center(), kHeatRange);

  // new temperature
  box->SetUserData("temperature", 320.0f);
  rig.Update();
  EXPECT_NEAR(320.0f, rig.Center(), kHeatRange);

  // replaced by another box
  scene->DestroyVisual(box);
  rig.Update();
  EXPECT_NEAR(kAmbient, rig.Center(), kAmbientRange);
  scene->RootVisual()->AddChild(CreateBox(scene, {2, 0, 0}, 330.0f));
  rig.Update();
  EXPECT_NEAR(330.0f, rig.Center(), kHeatRange);

  engine->DestroyScene(scene);
}

/////////////////////////////////////////////////
/// \brief Objects with a heat signature can be added and removed many
/// times.
TEST_F(ThermalCameraAccuracyTest,
       GZ_UTILS_TEST_DISABLED_ON_WIN32(HeatSignatureAddedAndRemoved))
{
  CHECK_SUPPORTED_ENGINE("ogre2");
  ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);
  scene->SetAmbientLight(1.0, 1.0, 1.0);

  ThermalRig rig;
  CreateRig(scene, rig);

  const std::string texture = common::joinPaths(this->kMediaPath,
      "materials", "textures", "gray_texture.png");
  for (int i = 0; i < 4; ++i)
  {
    VisualPtr box = scene->CreateVisual();
    box->AddGeometry(scene->CreateBox());
    box->SetLocalPosition(1.8, 0, 0);
    box->SetUserData("temperature", texture);
    box->SetUserData("minTemp", 100.0f);
    box->SetUserData("maxTemp", 200.0f);
    scene->RootVisual()->AddChild(box);

    rig.Update();
    // a gray texture: midway between min and max temperature
    EXPECT_NEAR(150.0f, rig.Center(), kHeatRange) << "iteration " << i;

    scene->DestroyVisual(box);
    rig.Update();
    EXPECT_NEAR(kAmbient, rig.Center(), kAmbientRange) << "iteration " << i;
  }

  engine->DestroyScene(scene);
}
