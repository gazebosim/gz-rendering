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

// Lidar visual benchmark: time per scan for each visual type, with
// non-hitting rays shown or hidden, for several lidar sizes. Each frame
// gives the visual a new scan (alternating between two scans) and renders
// it with a camera:
//   update   LidarVisual::Update()
//   render   Camera::Update() (pre render, render, readback)
//
// Not a pass/fail test: it prints a table, and optionally writes a CSV so
// runs before and after a change can be compared. Use an optimized build
// (e.g. CMAKE_BUILD_TYPE=RelWithDebInfo), otherwise the numbers say little.
// Each row also has a hash of the last image, to check that a change
// doesn't change the output.
//
// Environment variables:
//   GZ_LIDAR_VIS_BENCH_FRAMES  timed frames per configuration (default 30,
//                              rounded up to an even number)
//   GZ_LIDAR_VIS_BENCH_CSV     if set, append results as CSV to this file
//   GZ_LIDAR_VIS_BENCH_FULL    if set, run all configurations. By default
//                              only a small subset runs (a smoke test,
//                              fast enough for CI)
//   GZ_LIDAR_VIS_BENCH_FILTER  if set, run all configurations whose
//                              "type/nonhitting/HxV" contains this text,
//                              e.g. "points/" or "/hidden/"

#include <gtest/gtest.h>

#include <algorithm>
#include <chrono>
#include <cinttypes>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <random>
#include <string>
#include <vector>

#include "CommonRenderingTest.hh"

#include <gz/common/Console.hh>
#include <gz/utils/Environment.hh>

#include "gz/rendering/Camera.hh"
#include "gz/rendering/Image.hh"
#include "gz/rendering/Light.hh"
#include "gz/rendering/LidarVisual.hh"
#include "gz/rendering/Scene.hh"

using namespace gz;
using namespace rendering;

namespace
{
using Clock = std::chrono::steady_clock;

/// \brief Lidar size
struct Rays
{
  unsigned int horizontal;
  unsigned int vertical;
};

/// \brief Name of a visual type
std::string ToString(LidarVisualType _type)
{
  switch (_type)
  {
    case LidarVisualType::LVT_RAY_LINES:
      return "ray_lines";
    case LidarVisualType::LVT_TRIANGLE_STRIPS:
      return "triangle_strips";
    case LidarVisualType::LVT_POINTS:
      return "points";
    default:
      return "none";
  }
}

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

/// \brief A repeatable scan: 30% of the rays hit nothing (inf), the others
/// hit something 1 to 9 m away.
std::vector<double> Scan(unsigned int _seed, const Rays &_rays)
{
  std::mt19937 rng(_seed);
  std::uniform_real_distribution<double> range(1.0, 9.0);
  std::uniform_real_distribution<double> unit(0.0, 1.0);
  std::vector<double> points(_rays.horizontal * _rays.vertical);
  for (auto &p : points)
  {
    const bool hit = unit(rng) >= 0.3;
    const double r = range(rng);
    p = hit ? r : std::numeric_limits<double>::infinity();
  }
  return points;
}
}  // namespace

class LidarVisualPerfTest : public CommonRenderingTest
{
};

//////////////////////////////////////////////////
TEST_F(LidarVisualPerfTest, FrameTime)
{
  CHECK_SUPPORTED_ENGINE("ogre2");

  int frames = 30;
  std::string value;
  if (utils::env("GZ_LIDAR_VIS_BENCH_FRAMES", value) && !value.empty())
    frames = std::max(1, std::stoi(value));
  // even, so the last scan is always the same one
  frames += frames % 2;
  std::string csvPath;
  utils::env("GZ_LIDAR_VIS_BENCH_CSV", csvPath);
  std::string filter;
  utils::env("GZ_LIDAR_VIS_BENCH_FILTER", filter);
  const bool quick =
      !utils::env("GZ_LIDAR_VIS_BENCH_FULL", value) && filter.empty();
  const int warmup = 4;

  std::vector<LidarVisualType> types = {LidarVisualType::LVT_RAY_LINES,
      LidarVisualType::LVT_TRIANGLE_STRIPS, LidarVisualType::LVT_POINTS};
  std::vector<bool> nonHittings = {true, false};
  std::vector<Rays> sizes = {{440, 16}, {1800, 16}, {1024, 64}};
  if (quick)
    sizes = {{440, 16}};

  // keep the console quiet while timing
  common::Console::SetVerbosity(1);

  std::ofstream csv;
  if (!csvPath.empty())
  {
    const bool exists = std::ifstream(csvPath).good();
    csv.open(csvPath, std::ios::app);
    if (!exists)
    {
      csv << "type,nonhitting,horizontal,vertical,frames,hash,"
          << "total_median_ms,update_median_ms,render_median_ms\n";
    }
  }

  std::cout << std::left << std::setw(17) << "type" << std::setw(9)
            << "nonhit" << std::right << std::setw(9) << "rays"
            << std::setw(10) << "total" << std::setw(10) << "update"
            << std::setw(10) << "render" << "   hash   (median ms/frame)"
            << std::endl;

  int sceneCount = 0;
  for (const auto &size : sizes)
  {
    const std::vector<double> scanA = Scan(1, size);
    const std::vector<double> scanB = Scan(2, size);
    for (auto type : types)
    {
      for (bool nonHitting : nonHittings)
      {
        const std::string sizeName = std::to_string(size.horizontal) + "x" +
            std::to_string(size.vertical);
        const std::string key = ToString(type) + "/" +
            (nonHitting ? "shown" : "hidden") + "/" + sizeName;
        if (!filter.empty() && key.find(filter) == std::string::npos)
          continue;

        ScenePtr scene = engine->CreateScene(
            "lidar_vis_bench_" + std::to_string(sceneCount++));
        ASSERT_NE(nullptr, scene);
        scene->SetBackgroundColor(0, 0, 0);
        scene->SetAmbientLight(0.3, 0.3, 0.3);
        VisualPtr root = scene->RootVisual();

        DirectionalLightPtr light = scene->CreateDirectionalLight();
        light->SetDirection(0.2, 0.3, -1);
        light->SetDiffuseColor(0.8, 0.8, 0.8);
        root->AddChild(light);

        LidarVisualPtr lidar = scene->CreateLidarVisual();
        ASSERT_NE(nullptr, lidar);
        lidar->SetMinRange(0.1);
        lidar->SetMaxRange(10.0);
        lidar->SetMinHorizontalAngle(-GZ_PI);
        lidar->SetMaxHorizontalAngle(GZ_PI);
        lidar->SetHorizontalRayCount(size.horizontal);
        lidar->SetMinVerticalAngle(-0.26);
        lidar->SetMaxVerticalAngle(0.26);
        lidar->SetVerticalRayCount(size.vertical);
        lidar->SetType(type);
        lidar->SetDisplayNonHitting(nonHitting);
        lidar->SetSize(2.0);
        root->AddChild(lidar);

        // looking down at the lidar from above
        CameraPtr camera = scene->CreateCamera();
        ASSERT_NE(nullptr, camera);
        camera->SetImageWidth(640);
        camera->SetImageHeight(480);
        camera->SetHFOV(GZ_PI / 2.0);
        camera->SetLocalPosition(0, 0, 12);
        camera->SetLocalRotation(0, GZ_PI / 2.0, 0);
        root->AddChild(camera);
        Image image = camera->CreateImage();

        std::vector<double> updateMs;
        std::vector<double> renderMs;
        std::vector<double> totalMs;
        for (int i = 0; i < warmup + frames; ++i)
        {
          lidar->SetPoints(i % 2 == 0 ? scanA : scanB);
          const auto t0 = Clock::now();
          lidar->Update();
          const auto t1 = Clock::now();
          camera->Capture(image);
          const auto t2 = Clock::now();
          if (i >= warmup)
          {
            updateMs.push_back(Ms(t0, t1));
            renderMs.push_back(Ms(t1, t2));
            totalMs.push_back(Ms(t0, t2));
          }
        }

        const uint64_t hash = Hash(image.Data<unsigned char>(),
            image.MemorySize());
        char hashStr[17];
        std::snprintf(hashStr, sizeof(hashStr), "%016" PRIx64, hash);

        std::cout << std::left << std::setw(17) << ToString(type)
                  << std::setw(9) << (nonHitting ? "shown" : "hidden")
                  << std::right << std::setw(9) << sizeName << std::fixed
                  << std::setprecision(3) << std::setw(10)
                  << Median(totalMs) << std::setw(10) << Median(updateMs)
                  << std::setw(10) << Median(renderMs) << "   " << hashStr
                  << std::endl;
        if (csv.is_open())
        {
          csv << ToString(type) << "," << (nonHitting ? "shown" : "hidden")
              << "," << size.horizontal << "," << size.vertical << ","
              << frames << "," << hashStr << "," << Median(totalMs) << ","
              << Median(updateMs) << "," << Median(renderMs) << "\n";
        }

        engine->DestroyScene(scene);
      }
    }
  }
}
