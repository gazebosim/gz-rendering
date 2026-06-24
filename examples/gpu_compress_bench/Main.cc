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
#include <chrono>
#include <iostream>
#include <map>
#include <string>

#include <gz/rendering.hh>

using namespace gz;
using namespace rendering;
using Clock = std::chrono::steady_clock;

static ScenePtr buildScene(const std::string &_engineName)
{
  std::map<std::string, std::string> params;
  params["vulkan"] = "1";  // NV12 path requires Vulkan
  RenderEngine *engine = rendering::engine(_engineName, params);
  if (!engine)
  {
    std::cerr << "Engine '" << _engineName << "' unavailable\n";
    return nullptr;
  }
  ScenePtr scene = engine->CreateScene("scene");
  scene->SetBackgroundColor(0.2, 0.6, 0.9);
  VisualPtr root = scene->RootVisual();

  VisualPtr box = scene->CreateVisual("box");
  box->AddGeometry(scene->CreateBox());
  box->SetLocalPosition(3.0, 0.0, 0.0);
  root->AddChild(box);
  return scene;
}

static double benchBlocking(CameraPtr _cam, int _frames)
{
  Image img = _cam->CreateImage();
  auto t0 = Clock::now();
  for (int i = 0; i < _frames; ++i)
    _cam->Capture(img);  // Update + blocking Copy
  auto t1 = Clock::now();
  return std::chrono::duration<double, std::milli>(t1 - t0).count() / _frames;
}

static double benchNv12(CameraPtr _cam, int _frames, size_t &_bytesOut,
                        int &_gotOut)
{
  if (!_cam->IsEncodingSupported(IE_NV12))
  {
    std::cerr << "IE_NV12 unsupported on this backend\n";
    return -1.0;
  }
  size_t bytes = 0; int got = 0;
  auto conn = _cam->ConnectNewCompressedImageFrame(
      [&](const CompressedImage &_i){ bytes = _i.Size(); ++got; });
  _cam->SetImageEncoding(IE_NV12);
  auto t0 = Clock::now();
  for (int i = 0; i < _frames; ++i)
    _cam->Update();
  auto t1 = Clock::now();
  _bytesOut = bytes;
  _gotOut = got;
  return std::chrono::duration<double, std::milli>(t1 - t0).count() / _frames;
}

int main(int _argc, char **_argv)
{
  common::Console::SetVerbosity(2);
  const int frames = std::max(1, (_argc > 1) ? std::atoi(_argv[1]) : 200);

  ScenePtr scene = buildScene("ogre2");
  if (!scene) return 1;
  CameraPtr cam = scene->CreateCamera("camera");
  cam->SetImageWidth(1280u);
  cam->SetImageHeight(720u);
  cam->SetImageFormat(PF_R8G8B8A8);
  cam->SetHFOV(GZ_PI / 2);
  scene->RootVisual()->AddChild(cam);

  double blockMs = benchBlocking(cam, frames);
  size_t nv12Bytes = 0;
  int nv12Got = 0;
  double nv12Ms = benchNv12(cam, frames, nv12Bytes, nv12Got);

  const size_t rawBytes = 1280u * 720u * 4u;
  std::cout << "frames=" << frames << "\n"
            << "blocking ms/frame: " << blockMs << " (raw bytes/frame "
            << rawBytes << ")\n";
  if (nv12Ms < 0.0)
  {
    std::cout << "nv12: unsupported on this backend (needs Vulkan)\n";
  }
  else
  {
    std::cout << "nv12 ms/frame:     " << nv12Ms << " (nv12 bytes/frame "
              << nv12Bytes << ", "
              << (100.0 * (double)nv12Bytes / (double)rawBytes) << "% of raw)\n"
              << "frames received:   " << nv12Got << "\n";
  }
  return 0;
}
