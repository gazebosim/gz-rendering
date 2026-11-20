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

// Thermal camera benchmark: time per frame for several scene sizes and
// resolutions. Each frame is split into:
//   pre      Scene::PreRender()
//   render   ThermalCamera::Render() (includes setting the temperatures)
//   post     ThermalCamera::PostRender() (readback + callback)
//   flush    Scene::PostRender()
//
// Not a pass/fail test: it prints a table, and optionally writes a CSV so
// runs before and after a change can be compared. Use an optimized build
// (e.g. CMAKE_BUILD_TYPE=RelWithDebInfo), otherwise the numbers say little.
// Each row also has a hash of the last image, to check that a change
// doesn't change the output.
//
// Environment variables:
//   GZ_THERMAL_BENCH_FRAMES  timed frames per configuration (default 30)
//   GZ_THERMAL_BENCH_CSV     if set, append results as CSV to this file
//   GZ_THERMAL_BENCH_FULL    if set, run all configurations. By default only
//                            a small subset runs (a smoke test, fast enough
//                            for CI)
//   GZ_THERMAL_BENCH_FILTER  if set, run all configurations whose
//                            "geometry/objects/WxH" contains this text,
//                            e.g. "multilink4/1000/"

#include <gtest/gtest.h>

#include <algorithm>
#include <chrono>
#include <cinttypes>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <vector>

#include "CommonRenderingTest.hh"

#include <gz/common/Console.hh>
#include <gz/utils/Environment.hh>

#include "gz/rendering/Scene.hh"
#include "gz/rendering/ThermalCamera.hh"
#include "gz/rendering/Visual.hh"

using namespace gz;
using namespace rendering;

namespace
{
using Clock = std::chrono::steady_clock;

/// \brief Kind of objects in the scene.
enum class ObjectKind
{
  /// \brief Unit boxes
  BOX,
  /// \brief Models made of 4 box links
  MULTILINK,
  /// \brief Like MULTILINK, but spread all around the camera
  SPARSE_MULTILINK
};

/// \brief Name of an object kind
std::string ToString(ObjectKind _kind)
{
  switch (_kind)
  {
    case ObjectKind::BOX:
      return "box";
    case ObjectKind::MULTILINK:
      return "multilink4";
    case ObjectKind::SPARSE_MULTILINK:
    default:
      return "sparse_multilink4";
  }
}

/// \brief Times of one frame, in ms
struct Sample
{
  double pre{0};
  double render{0};
  double post{0};
  double flush{0};
  double Total() const { return pre + render + post + flush; }
};

/// \brief Milliseconds between two time points
double Ms(Clock::time_point _start, Clock::time_point _end)
{
  return std::chrono::duration<double, std::milli>(_end - _start).count();
}

/// \brief Median of a list of values
double Median(std::vector<double> _v)
{
  if (_v.empty())
    return 0;
  std::sort(_v.begin(), _v.end());
  return _v[_v.size() / 2];
}

/// \brief FNV-1a hash
uint64_t Hash(const unsigned char *_data, size_t _size,
    uint64_t _h = 1469598103934665603ull)
{
  for (size_t i = 0; i < _size; ++i)
  {
    _h ^= _data[i];
    _h *= 1099511628211ull;
  }
  return _h;
}

/// \brief Give a visual a temperature, cycling through: none (an object
/// that isn't a heat source), 250-349 K, and every 7th a negative value
/// (clamped to 0 K).
void SetTemperature(VisualPtr _visual, int _index)
{
  if (_index % 7 == 6)
    _visual->SetUserData("temperature", -5.0f);
  else if (_index % 2 == 0)
    _visual->SetUserData("temperature", 250.0f + (_index % 100));
}

/// \brief Fill the camera's view with _count objects at random (but
/// repeatable) positions 4 to 20 m in front of it. SPARSE_MULTILINK objects
/// are instead spread all around the camera, so only ~1/8 of them are in
/// view.
void Populate(ScenePtr _scene, ObjectKind _geometry, int _count)
{
  std::mt19937 rng(42);
  std::uniform_real_distribution<double> distX(4.0, 20.0);
  std::uniform_real_distribution<double> unit(-1.0, 1.0);
  std::uniform_real_distribution<double> angle(-GZ_PI, GZ_PI);

  // camera HFOV is 90 deg; keep objects inside ~80% of the view
  const double tanH = std::tan(GZ_PI / 4.0) * 0.8;
  const double tanV = tanH * 3.0 / 4.0;

  VisualPtr root = _scene->RootVisual();
  for (int i = 0; i < _count; ++i)
  {
    const double x = distX(rng);
    math::Pose3d pose(x, unit(rng) * x * tanH, unit(rng) * x * tanV,
        angle(rng), angle(rng), angle(rng));
    if (_geometry == ObjectKind::SPARSE_MULTILINK)
    {
      const double heading = angle(rng);
      pose.Pos() = {x * std::cos(heading), x * std::sin(heading),
          unit(rng) * x * tanV * 2.0};
    }

    char name[32];
    std::snprintf(name, sizeof(name), "model_%04d", i);
    VisualPtr model = _scene->CreateVisual(name);
    model->SetLocalPose(pose);
    root->AddChild(model);

    if (_geometry == ObjectKind::MULTILINK ||
        _geometry == ObjectKind::SPARSE_MULTILINK)
    {
      for (int l = 0; l < 4; ++l)
      {
        VisualPtr link = _scene->CreateVisual(
            std::string(name) + "::link_" + std::to_string(l));
        link->AddGeometry(_scene->CreateBox());
        link->SetLocalPosition((l % 2) * 0.6 - 0.3, (l / 2) * 0.6 - 0.3, 0);
        link->SetLocalScale(0.4, 0.4, 0.4);
        SetTemperature(link, i * 4 + l);
        model->AddChild(link);
      }
    }
    else
    {
      model->AddGeometry(_scene->CreateBox());
      model->SetLocalScale(0.6, 0.6, 0.6);
      SetTemperature(model, i);
    }
  }
}
}  // namespace

class ThermalCameraPerfTest : public CommonRenderingTest
{
};

//////////////////////////////////////////////////
TEST_F(ThermalCameraPerfTest, FrameTime)
{
  CHECK_SUPPORTED_ENGINE("ogre2");

  int frames = 30;
  std::string value;
  if (utils::env("GZ_THERMAL_BENCH_FRAMES", value) && !value.empty())
    frames = std::max(1, std::stoi(value));
  std::string csvPath;
  utils::env("GZ_THERMAL_BENCH_CSV", csvPath);
  std::string filter;
  utils::env("GZ_THERMAL_BENCH_FILTER", filter);
  const bool quick =
      !utils::env("GZ_THERMAL_BENCH_FULL", value) && filter.empty();
  const int warmup = 3;

  struct Resolution
  {
    unsigned int width;
    unsigned int height;
  };
  std::vector<ObjectKind> geometries = {ObjectKind::BOX,
      ObjectKind::MULTILINK, ObjectKind::SPARSE_MULTILINK};
  std::vector<int> counts = {10, 100, 1000};
  std::vector<Resolution> resolutions = {{320, 240}, {1280, 960}};
  if (quick)
  {
    geometries = {ObjectKind::BOX, ObjectKind::MULTILINK};
    counts = {10};
    resolutions = {{320, 240}};
  }

  // keep the console quiet while timing
  common::Console::SetVerbosity(1);

  std::ofstream csv;
  if (!csvPath.empty())
  {
    const bool exists = std::ifstream(csvPath).good();
    csv.open(csvPath, std::ios::app);
    if (!exists)
    {
      csv << "geometry,objects,width,height,frames,hash,total_median_ms,"
          << "pre_median_ms,render_median_ms,post_median_ms,"
          << "flush_median_ms\n";
    }
  }

  std::cout << std::left << std::setw(18) << "geometry" << std::right
            << std::setw(6) << "objs" << std::setw(11) << "resolution"
            << std::setw(10) << "total" << std::setw(9) << "pre"
            << std::setw(9) << "render" << std::setw(9) << "post"
            << std::setw(9) << "flush" << "   hash   (median ms/frame)"
            << std::endl;

  // Scenes are destroyed at the end: creating a thermal camera after
  // destroying another one trips an Ogre assertion (the shared fullscreen
  // quad still holds the destroyed camera's material).
  std::vector<ScenePtr> scenes;
  int sceneCount = 0;
  for (ObjectKind geometry : geometries)
  {
    for (int count : counts)
    {
      for (const Resolution &res : resolutions)
      {
        const std::string resStr = std::to_string(res.width) + "x" +
            std::to_string(res.height);
        const std::string key = ToString(geometry) + "/" +
            std::to_string(count) + "/" + resStr;
        if (!filter.empty() && key.find(filter) == std::string::npos)
          continue;

        ScenePtr scene = engine->CreateScene(
            "thermal_bench_" + std::to_string(sceneCount++));
        ASSERT_NE(nullptr, scene);
        scene->SetAmbientLight(0.3, 0.3, 0.3);
        Populate(scene, geometry, count);

        auto camera = scene->CreateThermalCamera(
            "thermal_" + std::to_string(sceneCount));
        ASSERT_NE(nullptr, camera);
        camera->SetLocalPosition(0, 0, 0);
        camera->SetLocalRotation(0, 0, 0);
        camera->SetImageWidth(res.width);
        camera->SetImageHeight(res.height);
        camera->SetAspectRatio(static_cast<double>(res.width) / res.height);
        camera->SetHFOV(GZ_PI / 2.0);
        camera->SetNearClipPlane(0.1);
        camera->SetFarClipPlane(100);
        camera->SetAmbientTemperature(296.0f);
        camera->SetAmbientTemperatureRange(4.0f);
        camera->SetLinearResolution(0.01f);
        scene->RootVisual()->AddChild(camera);

        std::vector<uint16_t> image(res.width * res.height);
        // PostRender skips the readback when nobody is listening
        auto connection = camera->ConnectNewThermalFrame(
            [&image](const uint16_t *_data, unsigned int _w,
                unsigned int _h, unsigned int _channels,
                const std::string &)
            {
              std::copy(_data, _data + _w * _h * _channels, image.begin());
            });

        std::vector<Sample> samples;
        for (int f = 0; f < warmup + frames; ++f)
        {
          Sample s;
          auto t0 = Clock::now();
          scene->PreRender();
          auto t1 = Clock::now();
          camera->Render();
          auto t2 = Clock::now();
          camera->PostRender();
          auto t3 = Clock::now();
          if (!scene->LegacyAutoGpuFlush())
            scene->PostRender();
          auto t4 = Clock::now();
          s.pre = Ms(t0, t1);
          s.render = Ms(t1, t2);
          s.post = Ms(t2, t3);
          s.flush = Ms(t3, t4);
          if (f >= warmup)
            samples.push_back(s);
        }

        auto column = [&samples](double Sample::*_field)
        {
          std::vector<double> v;
          for (const auto &s : samples)
            v.push_back(s.*_field);
          return v;
        };
        std::vector<double> totals;
        for (const auto &s : samples)
          totals.push_back(s.Total());
        const double median = Median(totals);
        const double pre = Median(column(&Sample::pre));
        const double render = Median(column(&Sample::render));
        const double post = Median(column(&Sample::post));
        const double flush = Median(column(&Sample::flush));
        const uint64_t hash = Hash(
            reinterpret_cast<const unsigned char *>(image.data()),
            image.size() * sizeof(uint16_t));
        char hashStr[17];
        std::snprintf(hashStr, sizeof(hashStr), "%016" PRIx64, hash);

        std::cout << std::fixed << std::setprecision(2) << std::left
                  << std::setw(18) << ToString(geometry) << std::right
                  << std::setw(6) << count << std::setw(11) << resStr
                  << std::setw(10) << median << std::setw(9) << pre
                  << std::setw(9) << render << std::setw(9) << post
                  << std::setw(9) << flush << "   " << hashStr
                  << std::endl;

        if (csv.is_open())
        {
          csv << ToString(geometry) << ',' << count << ',' << res.width
              << ',' << res.height << ',' << frames << ',' << hashStr << ','
              << median << ',' << pre << ',' << render << ',' << post << ','
              << flush << '\n';
        }

        connection.reset();
        // nothing to render in this scene anymore
        scene->RootVisual()->RemoveChildren();
        scenes.push_back(scene);
      }
    }
  }
  for (auto &scene : scenes)
    engine->DestroyScene(scene);
}
