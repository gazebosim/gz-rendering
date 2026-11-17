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

// Benchmark for the bounding box camera.
//
// Renders scenes of increasing size with each bounding box type and reports
// the time per frame, split into the phases of Camera::Update():
//   pre    Scene::PreRender()
//   render Camera::Render()          (material switch + GPU render)
//   post   Camera::PostRender()      (readback + box extraction)
//   flush  Scene::PostRender()
//
// Not a pass/fail test: it prints a table, and optionally writes a CSV so
// runs before and after a change can be compared. Use an optimized build
// (e.g. CMAKE_BUILD_TYPE=RelWithDebInfo), otherwise the numbers say little.
//
// Environment variables:
//   GZ_BBOX_BENCH_FRAMES  timed frames per configuration (default 30)
//   GZ_BBOX_BENCH_CSV     if set, append results as CSV to this file
//   GZ_BBOX_BENCH_QUICK   if set, only run a small subset (smoke test)
//   GZ_BBOX_BENCH_FILTER  if set, only run configurations whose
//                         "geometry/objects/WxH/type" contains this text,
//                         e.g. "multilink4/1000/" or "/1280x720/visible2d"
//   GZ_BBOX_BENCH_MOVE    if set, turn the camera slightly every frame, so
//                         nothing computed in a frame can be reused in the
//                         next one

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
#include <gz/common/MeshManager.hh>
#include <gz/utils/Environment.hh>

#include "gz/rendering/BoundingBoxCamera.hh"
#include "gz/rendering/Mesh.hh"
#include "gz/rendering/MeshDescriptor.hh"
#include "gz/rendering/Scene.hh"
#include "gz/rendering/Visual.hh"

using namespace gz;
using namespace rendering;

namespace
{
using Clock = std::chrono::steady_clock;

/// \brief Kind of objects in the scene.
enum class ObjectKind
{
  /// \brief Unit box, 24 vertices
  BOX,
  /// \brief Unit sphere, ~1100 vertices
  SPHERE,
  /// \brief Model made of 4 box links (exercises multi-link merging)
  MULTILINK,
  /// \brief A character mesh (test/media/meshes/walk.dae), ~7800 vertices,
  /// most of them inside its convex hull, like most real meshes
  WALKMESH,
  /// \brief MULTILINK models spread all around the camera, so only ~1/8
  /// of them are in view (a large world seen by one camera)
  SPARSE_MULTILINK
};

std::string ToString(ObjectKind _g)
{
  switch (_g)
  {
    case ObjectKind::BOX: return "box";
    case ObjectKind::SPHERE: return "sphere";
    case ObjectKind::MULTILINK: return "multilink4";
    case ObjectKind::WALKMESH: return "walkmesh";
    case ObjectKind::SPARSE_MULTILINK: return "sparse_multilink4";
    default: return "?";
  }
}

std::string ToString(BoundingBoxType _t)
{
  switch (_t)
  {
    case BoundingBoxType::BBT_VISIBLEBOX2D: return "visible2d";
    case BoundingBoxType::BBT_FULLBOX2D: return "full2d";
    case BoundingBoxType::BBT_BOX3D: return "box3d";
    default: return "?";
  }
}

/// \brief Per-frame timings, in milliseconds.
struct Sample
{
  double pre{0};
  double render{0};
  double post{0};
  double flush{0};
  double Total() const
  {
    return this->pre + this->render + this->post + this->flush;
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

/// \brief Fill the camera's view with _count objects at random (but
/// repeatable) positions 4 to 20 m in front of it. Objects overlap, so
/// visible and full boxes differ. SPARSE_MULTILINK objects are instead
/// spread all around the camera (any heading, twice the vertical view), so
/// only ~1/8 of them are in view.
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
    const int label = 1 + i % 200;

    VisualPtr model = _scene->CreateVisual();
    model->SetLocalPose(pose);
    root->AddChild(model);

    if (_geometry == ObjectKind::MULTILINK ||
        _geometry == ObjectKind::SPARSE_MULTILINK)
    {
      for (int l = 0; l < 4; ++l)
      {
        VisualPtr link = _scene->CreateVisual();
        link->AddGeometry(_scene->CreateBox());
        link->SetLocalPosition((l % 2) * 0.6 - 0.3, (l / 2) * 0.6 - 0.3, 0);
        link->SetLocalScale(0.4, 0.4, 0.4);
        link->SetUserData("label", label);
        model->AddChild(link);
      }
    }
    else if (_geometry == ObjectKind::WALKMESH)
    {
      const std::string path =
          std::string(PROJECT_SOURCE_PATH) + "/test/media/meshes/walk.dae";
      MeshDescriptor desc(common::MeshManager::Instance()->Load(path));
      desc.meshName = path;
      model->AddGeometry(_scene->CreateMesh(desc));
      model->SetLocalScale(0.5, 0.5, 0.5);
      model->SetUserData("label", label);
    }
    else
    {
      model->AddGeometry(_geometry == ObjectKind::BOX ?
          _scene->CreateBox() : _scene->CreateSphere());
      model->SetLocalScale(0.6, 0.6, 0.6);
      model->SetUserData("label", label);
    }
  }
}
}  // namespace

//////////////////////////////////////////////////
class BoundingBoxCameraPerfTest : public CommonRenderingTest
{
};

//////////////////////////////////////////////////
TEST_F(BoundingBoxCameraPerfTest, FrameTime)
{
  CHECK_SUPPORTED_ENGINE("ogre2");

  int frames = 30;
  std::string value;
  if (utils::env("GZ_BBOX_BENCH_FRAMES", value) && !value.empty())
    frames = std::max(1, std::stoi(value));
  std::string csvPath;
  utils::env("GZ_BBOX_BENCH_CSV", csvPath);
  const bool quick = utils::env("GZ_BBOX_BENCH_QUICK", value);
  const bool moveCamera = utils::env("GZ_BBOX_BENCH_MOVE", value);
  std::string filter;
  utils::env("GZ_BBOX_BENCH_FILTER", filter);
  auto selected = [&filter](ObjectKind _g, int _count,
      unsigned int _w, unsigned int _h, BoundingBoxType _t)
  {
    const std::string key = ToString(_g) + "/" + std::to_string(_count) +
        "/" + std::to_string(_w) + "x" + std::to_string(_h) + "/" +
        ToString(_t);
    return filter.empty() || key.find(filter) != std::string::npos;
  };
  const int warmup = 3;

  struct Resolution
  {
    unsigned int width;
    unsigned int height;
  };
  std::vector<ObjectKind> geometries = {ObjectKind::BOX, ObjectKind::SPHERE,
      ObjectKind::MULTILINK, ObjectKind::WALKMESH,
      ObjectKind::SPARSE_MULTILINK};
  std::vector<int> counts = {10, 100, 1000};
  std::vector<Resolution> resolutions = {{320, 180}, {1280, 720}};
  std::vector<BoundingBoxType> types = {BoundingBoxType::BBT_VISIBLEBOX2D,
      BoundingBoxType::BBT_FULLBOX2D, BoundingBoxType::BBT_BOX3D};
  if (quick)
  {
    geometries = {ObjectKind::BOX, ObjectKind::SPHERE};
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
      csv << "geometry,objects,width,height,type,frames,boxes,"
          << "total_median_ms,total_mean_ms,total_p90_ms,"
          << "pre_median_ms,render_median_ms,post_median_ms,flush_median_ms\n";
    }
  }

  std::cout << std::left << std::setw(11) << "geometry" << std::right
            << std::setw(6) << "objs" << std::setw(11) << "resolution"
            << std::setw(11) << "type" << std::setw(7) << "boxes"
            << std::setw(10) << "total" << std::setw(10) << "p90"
            << std::setw(9) << "pre" << std::setw(9) << "render"
            << std::setw(9) << "post" << std::setw(9) << "flush"
            << "   (median ms/frame)" << std::endl;

  int sceneCount = 0;
  for (ObjectKind geometry : geometries)
  {
    for (int count : counts)
    {
      bool any = false;
      for (const Resolution &res : resolutions)
        for (BoundingBoxType type : types)
          any = any || selected(geometry, count, res.width, res.height, type);
      if (!any)
        continue;

      ScenePtr scene = engine->CreateScene(
          "bench_scene_" + std::to_string(sceneCount++));
      ASSERT_NE(nullptr, scene);
      Populate(scene, geometry, count);

      for (const Resolution &res : resolutions)
      {
        auto camera = scene->CreateBoundingBoxCamera();
        ASSERT_NE(nullptr, camera);
        camera->SetLocalPosition(0, 0, 0);
        camera->SetLocalRotation(0, 0, 0);
        camera->SetImageWidth(res.width);
        camera->SetImageHeight(res.height);
        camera->SetAspectRatio(static_cast<double>(res.width) / res.height);
        camera->SetHFOV(GZ_PI / 2.0);
        camera->SetNearClipPlane(0.1);
        camera->SetFarClipPlane(100);
        scene->RootVisual()->AddChild(camera);

        size_t boxCount = 0;
        // PostRender skips the readback when nobody is listening
        auto connection = camera->ConnectNewBoundingBoxes(
            [&boxCount](const std::vector<BoundingBox> &_boxes)
            {
              boxCount = _boxes.size();
            });

        for (BoundingBoxType type : types)
        {
          if (!selected(geometry, count, res.width, res.height, type))
            continue;
          camera->SetBoundingBoxType(type);
          std::vector<Sample> samples;
          for (int f = 0; f < warmup + frames; ++f)
          {
            if (moveCamera)
              camera->SetLocalRotation(0, 0, (f % 2) * 0.001);
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
          const double mean = std::accumulate(totals.begin(), totals.end(),
              0.0) / totals.size();
          const double median = Median(totals);
          const double p90 = Percentile(totals, 0.9);
          const double pre = Median(column(&Sample::pre));
          const double render = Median(column(&Sample::render));
          const double post = Median(column(&Sample::post));
          const double flush = Median(column(&Sample::flush));

          const std::string resStr = std::to_string(res.width) + "x" +
              std::to_string(res.height);
          std::cout << std::fixed << std::setprecision(2) << std::left
                    << std::setw(11) << ToString(geometry) << std::right
                    << std::setw(6) << count << std::setw(11) << resStr
                    << std::setw(11) << ToString(type)
                    << std::setw(7) << boxCount
                    << std::setw(10) << median << std::setw(10) << p90
                    << std::setw(9) << pre << std::setw(9) << render
                    << std::setw(9) << post << std::setw(9) << flush
                    << std::endl;

          if (csv.is_open())
          {
            csv << ToString(geometry) << ',' << count << ',' << res.width
                << ',' << res.height << ',' << ToString(type) << ','
                << frames << ',' << boxCount << ',' << median << ','
                << mean << ',' << p90 << ',' << pre << ',' << render << ','
                << post << ',' << flush << '\n';
          }

          // sanity check that the camera is actually producing boxes
          EXPECT_GT(boxCount, 0u);
        }

        connection.reset();
        scene->DestroySensor(camera);
      }
      engine->DestroyScene(scene);
    }
  }
}
