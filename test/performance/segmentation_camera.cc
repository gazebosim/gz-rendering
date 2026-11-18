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

// Segmentation camera benchmark: time per frame for several scene sizes,
// resolutions and segmentation modes. Each frame is split into:
//   pre      Scene::PreRender()
//   render   SegmentationCamera::Render() (includes coloring the objects)
//   post     SegmentationCamera::PostRender() (readback + callback)
//   convert  SegmentationCamera::LabelMapFromColoredBuffer() (colored maps
//            only; gz-sensors calls it every frame)
//   flush    Scene::PostRender()
//
// Not a pass/fail test: it prints a table, and optionally writes a CSV so
// runs before and after a change can be compared. Use an optimized build
// (e.g. CMAKE_BUILD_TYPE=RelWithDebInfo), otherwise the numbers say little.
// Each row also has a hash of the last image (and label map), to check that
// a change doesn't change the output.
//
// Environment variables:
//   GZ_SEG_BENCH_FRAMES  timed frames per configuration (default 30)
//   GZ_SEG_BENCH_CSV     if set, append results as CSV to this file
//   GZ_SEG_BENCH_FULL    if set, run all configurations. By default only a
//                        small subset runs (a smoke test, fast enough for
//                        CI)
//   GZ_SEG_BENCH_FILTER  if set, run all configurations whose
//                        "geometry/objects/WxH/mode" contains this text,
//                        e.g. "multilink4/1000/" or "/panoptic_colored"

#include <gtest/gtest.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <random>
#include <string>
#include <vector>

#include "CommonRenderingTest.hh"

#include <gz/common/Console.hh>
#include <gz/utils/Environment.hh>

#include "gz/rendering/Scene.hh"
#include "gz/rendering/SegmentationCamera.hh"
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
  /// \brief MULTILINK models spread all around the camera, so only ~1/8
  /// of them are in view (a large world seen by one camera)
  SPARSE_MULTILINK
};

std::string ToString(ObjectKind _g)
{
  switch (_g)
  {
    case ObjectKind::BOX: return "box";
    case ObjectKind::MULTILINK: return "multilink4";
    case ObjectKind::SPARSE_MULTILINK: return "sparse_multilink4";
    default: return "?";
  }
}

/// \brief Segmentation type and map kind.
struct Mode
{
  SegmentationType type;
  bool colored;
};

std::string ToString(const Mode &_m)
{
  return std::string(_m.type == SegmentationType::ST_SEMANTIC ?
      "semantic" : "panoptic") + (_m.colored ? "_colored" : "");
}

/// \brief Per-frame timings, in milliseconds.
struct Sample
{
  double pre{0};
  double render{0};
  double post{0};
  double convert{0};
  double flush{0};
  double Total() const
  {
    return this->pre + this->render + this->post + this->convert +
        this->flush;
  }
};

double Ms(Clock::time_point _a, Clock::time_point _b)
{
  return std::chrono::duration<double, std::milli>(_b - _a).count();
}

double Median(std::vector<double> _v)
{
  if (_v.empty())
    return 0;
  std::sort(_v.begin(), _v.end());
  const size_t n = _v.size();
  return n % 2 ? _v[n / 2] : 0.5 * (_v[n / 2 - 1] + _v[n / 2]);
}

double Percentile(std::vector<double> _v, double _p)
{
  if (_v.empty())
    return 0;
  std::sort(_v.begin(), _v.end());
  const size_t idx = std::min(_v.size() - 1,
      static_cast<size_t>(_p * (_v.size() - 1) + 0.5));
  return _v[idx];
}

/// \brief FNV-1a hash
uint64_t Hash(const std::vector<uint8_t> &_data,
    uint64_t _h = 1469598103934665603ull)
{
  for (uint8_t b : _data)
  {
    _h ^= b;
    _h *= 1099511628211ull;
  }
  return _h;
}

/// \brief Fill the camera's view with _count objects at random (but
/// repeatable) positions 4 to 20 m in front of it. SPARSE_MULTILINK objects
/// are instead spread all around the camera, so only ~1/8 of them are in
/// view. Names follow Gazebo's scoping ("model_0001::link_2"), labels
/// cycle through 1-200, and every 10th model has no label.
void Populate(ScenePtr _scene, ObjectKind _geometry, int _count)
{
  std::mt19937 rng(42);
  std::uniform_real_distribution<double> distX(4.0, 20.0);
  std::uniform_real_distribution<double> unit(-1.0, 1.0);
  std::uniform_real_distribution<double> angle(-GZ_PI, GZ_PI);

  // camera HFOV is 90 deg; keep objects inside ~80% of the view
  const double tanH = std::tan(GZ_PI / 4.0) * 0.8;
  const double tanV = tanH * 9.0 / 16.0;

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
    const bool labeled = i % 10 != 9;
    const int label = 1 + i % 200;

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
        if (labeled)
          link->SetUserData("label", label);
        model->AddChild(link);
      }
    }
    else
    {
      model->AddGeometry(_scene->CreateBox());
      model->SetLocalScale(0.6, 0.6, 0.6);
      if (labeled)
        model->SetUserData("label", label);
    }
  }
}
}  // namespace

//////////////////////////////////////////////////
class SegmentationCameraPerfTest : public CommonRenderingTest
{
};

//////////////////////////////////////////////////
TEST_F(SegmentationCameraPerfTest, FrameTime)
{
  CHECK_SUPPORTED_ENGINE("ogre2");

  int frames = 30;
  std::string value;
  if (utils::env("GZ_SEG_BENCH_FRAMES", value) && !value.empty())
    frames = std::max(1, std::stoi(value));
  std::string csvPath;
  utils::env("GZ_SEG_BENCH_CSV", csvPath);
  std::string filter;
  utils::env("GZ_SEG_BENCH_FILTER", filter);
  const bool quick =
      !utils::env("GZ_SEG_BENCH_FULL", value) && filter.empty();
  auto selected = [&filter](ObjectKind _g, int _count,
      unsigned int _w, unsigned int _h, const Mode &_m)
  {
    const std::string key = ToString(_g) + "/" + std::to_string(_count) +
        "/" + std::to_string(_w) + "x" + std::to_string(_h) + "/" +
        ToString(_m);
    return filter.empty() || key.find(filter) != std::string::npos;
  };
  const int warmup = 3;

  struct Resolution
  {
    unsigned int width;
    unsigned int height;
  };
  std::vector<ObjectKind> geometries = {ObjectKind::BOX,
      ObjectKind::MULTILINK, ObjectKind::SPARSE_MULTILINK};
  std::vector<int> counts = {10, 100, 1000};
  std::vector<Resolution> resolutions = {{320, 180}, {1280, 720}};
  const std::vector<Mode> modes = {
      {SegmentationType::ST_SEMANTIC, false},
      {SegmentationType::ST_SEMANTIC, true},
      {SegmentationType::ST_PANOPTIC, false},
      {SegmentationType::ST_PANOPTIC, true}};
  if (quick)
  {
    geometries = {ObjectKind::BOX, ObjectKind::MULTILINK};
    counts = {10};
    resolutions = {{320, 180}};
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
      csv << "geometry,objects,width,height,mode,frames,hash,"
          << "total_median_ms,total_mean_ms,total_p90_ms,pre_median_ms,"
          << "render_median_ms,post_median_ms,convert_median_ms,"
          << "flush_median_ms\n";
    }
  }

  std::cout << std::left << std::setw(18) << "geometry" << std::right
            << std::setw(6) << "objs" << std::setw(11) << "resolution"
            << std::setw(18) << "mode" << std::setw(10) << "total"
            << std::setw(10) << "p90" << std::setw(9) << "pre"
            << std::setw(9) << "render" << std::setw(9) << "post"
            << std::setw(9) << "convert" << std::setw(9) << "flush"
            << "   hash   (median ms/frame)" << std::endl;

  int sceneCount = 0;
  for (ObjectKind geometry : geometries)
  {
    for (int count : counts)
    {
      bool any = false;
      for (const Resolution &res : resolutions)
        for (const Mode &mode : modes)
          any = any || selected(geometry, count, res.width, res.height, mode);
      if (!any)
        continue;

      ScenePtr scene = engine->CreateScene(
          "bench_scene_" + std::to_string(sceneCount++));
      ASSERT_NE(nullptr, scene);
      Populate(scene, geometry, count);

      for (const Resolution &res : resolutions)
      {
        for (const Mode &mode : modes)
        {
          if (!selected(geometry, count, res.width, res.height, mode))
            continue;

          // A new camera per mode: the mode is only read when the camera
          // renders, but this keeps every configuration independent.
          auto camera = scene->CreateSegmentationCamera();
          ASSERT_NE(nullptr, camera);
          camera->SetLocalPosition(0, 0, 0);
          camera->SetLocalRotation(0, 0, 0);
          camera->SetImageWidth(res.width);
          camera->SetImageHeight(res.height);
          camera->SetAspectRatio(
              static_cast<double>(res.width) / res.height);
          camera->SetHFOV(GZ_PI / 2.0);
          camera->SetNearClipPlane(0.1);
          camera->SetFarClipPlane(100);
          camera->SetBackgroundLabel(0);
          camera->SetSegmentationType(mode.type);
          camera->EnableColoredMap(mode.colored);
          scene->RootVisual()->AddChild(camera);

          std::vector<uint8_t> image(res.width * res.height * 3);
          std::vector<uint8_t> labels(image.size());
          // PostRender skips the readback when nobody is listening
          auto connection = camera->ConnectNewSegmentationFrame(
              [&image](const uint8_t *_data, unsigned int _w,
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
            if (mode.colored)
              camera->LabelMapFromColoredBuffer(labels.data());
            auto t4 = Clock::now();
            if (!scene->LegacyAutoGpuFlush())
              scene->PostRender();
            auto t5 = Clock::now();
            s.pre = Ms(t0, t1);
            s.render = Ms(t1, t2);
            s.post = Ms(t2, t3);
            s.convert = Ms(t3, t4);
            s.flush = Ms(t4, t5);
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
          const double mean = std::accumulate(totals.begin(), totals.end(),
              0.0) / totals.size();
          const double median = Median(totals);
          const double p90 = Percentile(totals, 0.9);
          const double pre = Median(column(&Sample::pre));
          const double render = Median(column(&Sample::render));
          const double post = Median(column(&Sample::post));
          const double convert = Median(column(&Sample::convert));
          const double flush = Median(column(&Sample::flush));
          uint64_t hash = Hash(image);
          if (mode.colored)
            hash = Hash(labels, hash);
          char hashStr[17];
          std::snprintf(hashStr, sizeof(hashStr), "%016llx",
              static_cast<unsigned long long>(hash));

          const std::string resStr = std::to_string(res.width) + "x" +
              std::to_string(res.height);
          std::cout << std::fixed << std::setprecision(2) << std::left
                    << std::setw(18) << ToString(geometry) << std::right
                    << std::setw(6) << count << std::setw(11) << resStr
                    << std::setw(18) << ToString(mode)
                    << std::setw(10) << median << std::setw(10) << p90
                    << std::setw(9) << pre << std::setw(9) << render
                    << std::setw(9) << post << std::setw(9) << convert
                    << std::setw(9) << flush << "   " << hashStr
                    << std::endl;

          if (csv.is_open())
          {
            csv << ToString(geometry) << ',' << count << ',' << res.width
                << ',' << res.height << ',' << ToString(mode) << ','
                << frames << ',' << hashStr << ',' << median << ','
                << mean << ',' << p90 << ',' << pre << ',' << render << ','
                << post << ',' << convert << ',' << flush << '\n';
          }

          // sanity check that the camera produced something other than
          // background
          EXPECT_TRUE(std::any_of(image.begin(), image.end(),
              [](uint8_t _v) { return _v != 0; }));

          connection.reset();
          scene->DestroySensor(camera);
        }
      }
      engine->DestroyScene(scene);
    }
  }
}
