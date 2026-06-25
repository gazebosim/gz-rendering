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

// Headless GPU-compression benchmark with a per-frame cost breakdown and an
// N-camera scaling mode.
//
// Three measured paths (each processes ALL cameras once per "tick", averaged
// over `frames` ticks):
//   * update_only : render + submit + implicit sync, NO readback. The fixed
//                   engine overhead floor (present in every gz-rendering).
//   * raw         : update + blocking full-size RGBA8 Camera::Capture().
//                   readback cost = raw - update_only.
//   * nv12        : update + async NV12 compressed output
//                   (ConnectNewCompressedImageFrame). compute cost = nv12 -
//                   update_only. Requires Vulkan; compiled in only when the
//                   linked gz-rendering exposes gz/rendering/CompressedImage.hh
//                   (CMake -> GZ_RENDERING_HAS_COMPRESSED_IMAGE), so the same
//                   source builds raw-only against a system gz-rendering that
//                   predates the compression API.
//
// Output: one machine-parseable line per path, e.g.
//   RESULT engine=ogre2 backend=vulkan path=raw  ncams=4 w=1280 h=720 frames=200 \
//          ms_per_tick=15.8 ms_per_cam=3.95 bytes_per_cam=3686400 bytes_per_tick=14745600
//
// Usage: gpu_compress_bench [engine=ogre2] [backend=gl|vulkan] \
//          [width=1280] [height=720] [frames=200] [ncams=1]

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include <gz/rendering.hh>

#ifdef GZ_RENDERING_HAS_COMPRESSED_IMAGE
#include <gz/rendering/CompressedImage.hh>
#endif

using namespace gz;
using namespace rendering;
using Clock = std::chrono::steady_clock;

static double msPer(const std::chrono::steady_clock::time_point &a,
                    const std::chrono::steady_clock::time_point &b, int n)
{
  return std::chrono::duration<double, std::milli>(b - a).count() / n;
}

static ScenePtr buildScene(const std::string &_engineName,
                           const std::string &_backend)
{
  std::map<std::string, std::string> params;
  if (_backend == "vulkan")
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

// render+submit only, no readback; ms per tick (all cams once)
static double benchUpdateOnly(std::vector<CameraPtr> &_cams, int _frames)
{
  auto t0 = Clock::now();
  for (int f = 0; f < _frames; ++f)
    for (auto &c : _cams) c->Update();
  return msPer(t0, Clock::now(), _frames);
}

// update + blocking full-size Copy; ms per tick
static double benchRaw(std::vector<CameraPtr> &_cams,
                       std::vector<Image> &_imgs, int _frames)
{
  auto t0 = Clock::now();
  for (int f = 0; f < _frames; ++f)
    for (size_t i = 0; i < _cams.size(); ++i) _cams[i]->Capture(_imgs[i]);
  return msPer(t0, Clock::now(), _frames);
}

#ifdef GZ_RENDERING_HAS_COMPRESSED_IMAGE
// enable NV12 on every camera, then update; ms per tick
static double benchNv12(std::vector<CameraPtr> &_cams, int _frames,
                        size_t &_bytesPerCam, int &_received)
{
  std::vector<common::ConnectionPtr> conns;
  size_t bytes = 0; int got = 0;
  for (auto &c : _cams)
  {
    conns.push_back(c->ConnectNewCompressedImageFrame(
        [&](const CompressedImage &_i){ bytes = _i.Size(); ++got; }));
    c->SetImageEncoding(IE_NV12);
  }
  auto t0 = Clock::now();
  for (int f = 0; f < _frames; ++f)
    for (auto &c : _cams) c->Update();
  double r = msPer(t0, Clock::now(), _frames);
  _bytesPerCam = bytes;
  _received = got;
  return r;
}
#endif

static void emitTick(const std::string &eng, const std::string &bk,
    const std::string &path, int ncams, unsigned w, unsigned h, int frames,
    double msTick, const std::string &extra = "")
{
  std::cout << "RESULT engine=" << eng << " backend=" << bk
            << " path=" << path << " ncams=" << ncams
            << " w=" << w << " h=" << h << " frames=" << frames
            << " ms_per_tick=" << msTick
            << " ms_per_cam=" << (msTick / ncams);
  if (!extra.empty()) std::cout << " " << extra;
  std::cout << "\n";
}

int main(int _argc, char **_argv)
{
  common::Console::SetVerbosity(2);

  const std::string engineName = (_argc > 1) ? _argv[1] : "ogre2";
  const std::string backend    = (_argc > 2) ? _argv[2] : "vulkan";
  const unsigned int width  = (_argc > 3) ?
      static_cast<unsigned int>(std::max(1, std::atoi(_argv[3]))) : 1280u;
  const unsigned int height = (_argc > 4) ?
      static_cast<unsigned int>(std::max(1, std::atoi(_argv[4]))) : 720u;
  const int frames = std::max(1, (_argc > 5) ? std::atoi(_argv[5]) : 200);
  const int ncams  = std::max(1, (_argc > 6) ? std::atoi(_argv[6]) : 1);

  ScenePtr scene = buildScene(engineName, backend);
  if (!scene) return 1;

  std::vector<CameraPtr> cams;
  std::vector<Image> imgs;
  for (int i = 0; i < ncams; ++i)
  {
    CameraPtr cam = scene->CreateCamera("camera" + std::to_string(i));
    cam->SetImageWidth(width);
    cam->SetImageHeight(height);
    cam->SetImageFormat(PF_R8G8B8A8);
    cam->SetHFOV(GZ_PI / 2);
    scene->RootVisual()->AddChild(cam);
    cams.push_back(cam);
    imgs.push_back(cam->CreateImage());
  }

  for (int i = 0; i < 10; ++i)            // warmup
    for (auto &c : cams) c->Update();

  const size_t rawBytesPerCam =
      static_cast<size_t>(width) * static_cast<size_t>(height) * 4u;

  // 1) render+submit floor (no readback)
  double updMs = benchUpdateOnly(cams, frames);
  emitTick(engineName, backend, "update_only", ncams, width, height, frames, updMs);

  // 2) raw blocking readback
  double rawMs = benchRaw(cams, imgs, frames);
  emitTick(engineName, backend, "raw", ncams, width, height, frames, rawMs,
      "bytes_per_cam=" + std::to_string(rawBytesPerCam) +
      " bytes_per_tick=" + std::to_string(rawBytesPerCam * ncams));

#ifdef GZ_RENDERING_HAS_COMPRESSED_IMAGE
  if (!cams[0]->IsEncodingSupported(IE_NV12))
  {
    std::cout << "RESULT engine=" << engineName << " backend=" << backend
              << " path=nv12 status=unsupported needs_vulkan=1\n";
  }
  else
  {
    size_t nv12Bytes = 0; int nv12Got = 0;
    double nv12Ms = benchNv12(cams, frames, nv12Bytes, nv12Got);
    const double pct = 100.0 * static_cast<double>(nv12Bytes) /
                       static_cast<double>(rawBytesPerCam);
    emitTick(engineName, backend, "nv12", ncams, width, height, frames, nv12Ms,
        "bytes_per_cam=" + std::to_string(nv12Bytes) +
        " bytes_per_tick=" + std::to_string(nv12Bytes * ncams) +
        " pct_of_raw=" + std::to_string(pct) +
        " frames_received=" + std::to_string(nv12Got));
  }
#else
  std::cout << "RESULT engine=" << engineName << " backend=" << backend
            << " path=nv12 status=not_built no_compressed_image_api=1\n";
#endif
  return 0;
}
