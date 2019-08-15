/*
 * Copyright (C) 2019 Open Source Robotics Foundation
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

#include <benchmark/benchmark.h>

#include <ignition/common/Console.hh>

#include "ignition/rendering/Camera.hh"
#include "ignition/rendering/RenderEngine.hh"
#include "ignition/rendering/RenderingIface.hh"
#include "ignition/rendering/Scene.hh"

using namespace ignition;
using namespace rendering;

// NOLINTNEXTLINE
void BM_Update(benchmark::State &_st)
{
  auto imgWidth = _st.range(0);
  auto imgHeight = _st.range(1);

  auto engine = rendering::engine("ogre2");
  if (!engine)
  {
    igndbg << "Engine 'ogre2' is not supported" << std::endl;
    return;
  }

  auto scene = engine->CreateScene("scene");

  auto root = scene->RootVisual();

  {
    // create box visual
    VisualPtr box = scene->CreateVisual("box");
    box->AddGeometry(scene->CreateBox());
    box->SetOrigin(0.0, 0.5, 0.0);
    box->SetLocalPosition(3, 0, 0);
    box->SetLocalRotation(IGN_PI / 4, 0, IGN_PI / 3);
    box->SetLocalScale(1, 2.5, 1);
    root->AddChild(box);

    // create sphere visual
    VisualPtr sphere = scene->CreateVisual("sphere");
    sphere->AddGeometry(scene->CreateSphere());
    sphere->SetOrigin(0.0, -0.5, 0.0);
    sphere->SetLocalPosition(3, 0, 0);
    sphere->SetLocalRotation(0, 0, 0);
    sphere->SetLocalScale(1, 2.5, 1);
    root->AddChild(sphere);

    // create camera
    CameraPtr camera = scene->CreateCamera("camera");
    camera->SetLocalPosition(0.0, 0.0, 0.0);
    camera->SetLocalRotation(0.0, 0.0, 0.0);
    camera->SetImageWidth(imgWidth);
    camera->SetImageHeight(imgHeight);
    camera->SetAntiAliasing(2);
    camera->SetAspectRatio(1.333);
    camera->SetHFOV(IGN_PI / 2);
    root->AddChild(camera);

    Image img(imgWidth, imgHeight, PixelFormat::PF_B8G8R8);

    for (auto _ : _st)
    {
      camera->Update();
    }
  }

  // Clean up
  engine->DestroyScene(scene);
  unloadEngine(engine->Name());
}

// NOLINTNEXTLINE
void BM_Copy(benchmark::State &_st)
{
  auto imgWidth = _st.range(0);
  auto imgHeight = _st.range(1);

  auto engine = rendering::engine("ogre2");
  if (!engine)
  {
    igndbg << "Engine 'ogre2' is not supported" << std::endl;
    return;
  }

  auto scene = engine->CreateScene("scene");

  auto root = scene->RootVisual();

  {
    // create box visual
    VisualPtr box = scene->CreateVisual("box");
    box->AddGeometry(scene->CreateBox());
    box->SetOrigin(0.0, 0.5, 0.0);
    box->SetLocalPosition(3, 0, 0);
    box->SetLocalRotation(IGN_PI / 4, 0, IGN_PI / 3);
    box->SetLocalScale(1, 2.5, 1);
    root->AddChild(box);

    // create sphere visual
    VisualPtr sphere = scene->CreateVisual("sphere");
    sphere->AddGeometry(scene->CreateSphere());
    sphere->SetOrigin(0.0, -0.5, 0.0);
    sphere->SetLocalPosition(3, 0, 0);
    sphere->SetLocalRotation(0, 0, 0);
    sphere->SetLocalScale(1, 2.5, 1);
    root->AddChild(sphere);

    // create camera
    CameraPtr camera = scene->CreateCamera("camera");
    camera->SetLocalPosition(0.0, 0.0, 0.0);
    camera->SetLocalRotation(0.0, 0.0, 0.0);
    camera->SetImageWidth(imgWidth);
    camera->SetImageHeight(imgHeight);
    camera->SetAntiAliasing(2);
    camera->SetAspectRatio(1.333);
    camera->SetHFOV(IGN_PI / 2);
    root->AddChild(camera);

    Image img(imgWidth, imgHeight, PixelFormat::PF_B8G8R8);

    camera->Update();
    for (auto _ : _st)
    {
      camera->Copy(img);
    }
  }

  // Clean up
  engine->DestroyScene(scene);
  unloadEngine(engine->Name());
}

BENCHMARK(BM_Update)
  ->Args({320, 240})    // QVGA
  ->Args({640, 480})    // VGA
  ->Args({800, 600})    // SVGA
  ->Args({1280, 1024})  // SXGA
  ->Args({1600, 1200})  // UXGA
  ->Args({1920, 1080})  // HD1080
  ->Args({3840, 2160})  // 4KUHD
  ->Args({256, 256})
  ->Args({512, 512})
  ->Args({1024, 1024})
  ->Args({2048, 2048})
  ->Args({4096, 4096})
  ->Unit(benchmark::kMicrosecond);

BENCHMARK(BM_Copy)
  ->Args({320, 240})    // QVGA
  ->Args({640, 480})    // VGA
  ->Args({800, 600})    // SVGA
  ->Args({1280, 1024})  // SXGA
  ->Args({1600, 1200})  // UXGA
  ->Args({1920, 1080})  // HD1080
  ->Args({3840, 2160})  // 4KUHD
  ->Args({256, 256})
  ->Args({512, 512})
  ->Args({1024, 1024})
  ->Args({2048, 2048})
  ->Args({4096, 4096})
  ->Unit(benchmark::kMicrosecond);


// OSX needs the semicolon, Ubuntu complains that there's an extra ';'
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
BENCHMARK_MAIN();
#pragma GCC diagnostic pop

