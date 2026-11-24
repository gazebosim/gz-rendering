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

#include <memory>
#include <string>
#include <tuple>

#include "CommonRenderingTest.hh"

#include <gz/utils/ExtraTestMacros.hh>

#include "gz/rendering/Camera.hh"
#include "gz/rendering/DepthCamera.hh"
#include "gz/rendering/GaussianNoisePass.hh"
#include "gz/rendering/GpuRays.hh"
#include "gz/rendering/RenderPassSystem.hh"
#include "gz/rendering/Scene.hh"
#include "gz/rendering/ThermalCamera.hh"
#include "gz/rendering/Visual.hh"

using namespace gz;
using namespace rendering;

namespace
{
/// \brief Kinds of sensors to recreate
enum class SensorKind
{
  CAMERA_SELECTION,
  CAMERA_NOISE,
  DEPTH,
  GPU_RAYS,
  THERMAL
};

/// \brief How the first sensor goes away before the second is created
enum class Recreate
{
  /// \brief The sensor is destroyed, in the same scene
  SAME_SCENE,
  /// \brief The whole scene is destroyed, the second sensor is in a new one
  NEW_SCENE
};

/// \brief Name of a sensor kind, for test names
std::string ToString(SensorKind _kind)
{
  switch (_kind)
  {
    case SensorKind::CAMERA_SELECTION: return "CameraSelection";
    case SensorKind::CAMERA_NOISE: return "CameraNoisePass";
    case SensorKind::DEPTH: return "Depth";
    case SensorKind::GPU_RAYS: return "GpuRays";
    case SensorKind::THERMAL: return "Thermal";
    default: return "Unknown";
  }
}

/// \brief Create a scene with a box in front of the origin
ScenePtr CreateScene(RenderEngine *_engine, const std::string &_name)
{
  ScenePtr scene = _engine->CreateScene(_name);
  if (!scene)
    return scene;
  scene->SetAmbientLight(0.5, 0.5, 0.5);
  VisualPtr box = scene->CreateVisual();
  box->AddGeometry(scene->CreateBox());
  box->SetLocalPosition(2, 0, 0);
  box->SetUserData("temperature", 310.0f);
  box->SetUserData("label", 1);
  scene->RootVisual()->AddChild(box);
  return scene;
}

/// \brief Create a sensor of a kind in a scene and render a frame with it.
/// The sensor always has the same name, as when a simulation removes a
/// sensor and adds it again.
/// \return The sensor
SensorPtr CreateAndRender(RenderEngine *_engine, ScenePtr _scene,
    SensorKind _kind)
{
  const std::string name = "sensor";
  SensorPtr sensor;
  switch (_kind)
  {
    case SensorKind::CAMERA_SELECTION:
    case SensorKind::CAMERA_NOISE:
    {
      CameraPtr camera = _scene->CreateCamera(name);
      camera->SetImageWidth(64);
      camera->SetImageHeight(48);
      if (_kind == SensorKind::CAMERA_NOISE)
      {
        RenderPassPtr pass =
            _engine->RenderPassSystem()->Create<GaussianNoisePass>();
        camera->AddRenderPass(pass);
      }
      _scene->RootVisual()->AddChild(camera);
      camera->Update();
      if (_kind == SensorKind::CAMERA_SELECTION)
        camera->VisualAt({32, 24});
      sensor = camera;
      break;
    }
    case SensorKind::DEPTH:
    {
      DepthCameraPtr camera = _scene->CreateDepthCamera(name);
      camera->SetImageWidth(64);
      camera->SetImageHeight(48);
      camera->SetNearClipPlane(0.1);
      camera->SetFarClipPlane(10);
      camera->CreateDepthTexture();
      _scene->RootVisual()->AddChild(camera);
      camera->Update();
      sensor = camera;
      break;
    }
    case SensorKind::GPU_RAYS:
    {
      GpuRaysPtr rays = _scene->CreateGpuRays(name);
      rays->SetNearClipPlane(0.1);
      rays->SetFarClipPlane(10);
      rays->SetAngleMin(-1.0);
      rays->SetAngleMax(1.0);
      rays->SetRayCount(64);
      rays->SetVerticalRayCount(1);
      _scene->RootVisual()->AddChild(rays);
      rays->Update();
      sensor = rays;
      break;
    }
    case SensorKind::THERMAL:
    {
      ThermalCameraPtr camera = _scene->CreateThermalCamera(name);
      camera->SetImageWidth(64);
      camera->SetImageHeight(48);
      _scene->RootVisual()->AddChild(camera);
      camera->Update();
      sensor = camera;
      break;
    }
    default:
      break;
  }
  return sensor;
}
}  // namespace

/// \brief Removing a sensor and creating it again, as a simulation does
/// when a model is removed and added again, must work.
class SensorRecreateTest : public CommonRenderingTest,
    public testing::WithParamInterface<std::tuple<SensorKind, Recreate>>
{
};

/////////////////////////////////////////////////
TEST_P(SensorRecreateTest, GZ_UTILS_TEST_DISABLED_ON_WIN32(Recreate))
{
  CHECK_SUPPORTED_ENGINE("ogre2");
  const SensorKind kind = std::get<0>(this->GetParam());
  const Recreate recreate = std::get<1>(this->GetParam());

  ScenePtr scene = CreateScene(engine, "scene_1");
  ASSERT_NE(nullptr, scene);
  SensorPtr sensor = CreateAndRender(engine, scene, kind);
  ASSERT_NE(nullptr, sensor);

  if (recreate == Recreate::SAME_SCENE)
  {
    scene->DestroySensor(sensor);
  }
  else
  {
    engine->DestroyScene(scene);
    scene = CreateScene(engine, "scene_2");
    ASSERT_NE(nullptr, scene);
  }
  sensor.reset();

  // Twice: a new sensor, and that sensor rendering again
  sensor = CreateAndRender(engine, scene, kind);
  ASSERT_NE(nullptr, sensor);
  CameraPtr camera = std::dynamic_pointer_cast<Camera>(sensor);
  ASSERT_NE(nullptr, camera);
  camera->Update();

  engine->DestroyScene(scene);
}

INSTANTIATE_TEST_SUITE_P(Sensors, SensorRecreateTest,
    testing::Combine(
        testing::Values(SensorKind::CAMERA_SELECTION,
            SensorKind::CAMERA_NOISE, SensorKind::DEPTH,
            SensorKind::GPU_RAYS, SensorKind::THERMAL),
        testing::Values(Recreate::SAME_SCENE, Recreate::NEW_SCENE)),
    [](const testing::TestParamInfo<SensorRecreateTest::ParamType> &_info)
    {
      return ToString(std::get<0>(_info.param)) +
          (std::get<1>(_info.param) == Recreate::SAME_SCENE ?
           "SameScene" : "NewScene");
    });

/////////////////////////////////////////////////
/// \brief After sensors that draw fullscreen passes are destroyed, any
/// other sensor must still work.
TEST_F(SensorRecreateTest,
       GZ_UTILS_TEST_DISABLED_ON_WIN32(OtherSensorsAfterDestroy))
{
  CHECK_SUPPORTED_ENGINE("ogre2");
  ScenePtr scene = CreateScene(engine, "scene");
  ASSERT_NE(nullptr, scene);

  const SensorKind kinds[] = {SensorKind::THERMAL, SensorKind::DEPTH,
      SensorKind::CAMERA_SELECTION};
  for (SensorKind destroyed : kinds)
  {
    for (SensorKind created : kinds)
    {
      if (created == destroyed)
        continue;
      SCOPED_TRACE(ToString(destroyed) + " then " + ToString(created));
      SensorPtr first = CreateAndRender(engine, scene, destroyed);
      ASSERT_NE(nullptr, first);
      scene->DestroySensor(first);
      first.reset();

      SensorPtr second = CreateAndRender(engine, scene, created);
      ASSERT_NE(nullptr, second);
      scene->DestroySensor(second);
    }
  }

  engine->DestroyScene(scene);
}
