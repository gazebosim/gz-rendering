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

// RGBD-camera throughput + resource benchmark. Models the GPU->CPU readback a
// gz-sensors RgbdCameraSensor performs each update: a float DEPTH frame
// (ConnectNewDepthFrame, 4 bytes/px) plus an XYZRGBA POINT CLOUD
// (ConnectNewRgbPointCloud, 16 bytes/px -- the RGBA word carries the colour).
// Together 20 bytes/px, which is what makes many RGBD sensors bandwidth-bound.
//
// Default config matches the case of interest: 8 RGBD cameras at 640x320.
//
// Two measured points per run:
//   * depth_only : render + DEPTH readback only (ConnectNewDepthFrame).
//   * rgbd       : render + depth + POINT CLOUD readback (full RGBD).
//
// Output: one machine-parseable line per path, e.g.
//   RESULT engine=ogre2 backend=vulkan path=rgbd ncams=8 w=640 h=320 \
//     frames=200 ms_per_tick=.. ms_per_cam=.. fps_per_cam=.. fps_total=.. \
//     cpu_pct=.. rss_mb=.. peak_rss_mb=.. depth_mb_s=.. cloud_mb_s=.. \
//     total_mb_s=.. depth_frames=.. cloud_frames=..
//
// Usage: rgbd_throughput_bench [engine=ogre2] [backend=gl|vulkan] \
//          [width=640] [height=320] [frames=200] [ncams=8]

#include <sys/resource.h>

#include <algorithm>
#include <cmath>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include <gz/rendering.hh>

using namespace gz;
using namespace rendering;
using Clock = std::chrono::steady_clock;

static double cpuSeconds()
{
  struct rusage ru;
  getrusage(RUSAGE_SELF, &ru);
  return ru.ru_utime.tv_sec + ru.ru_utime.tv_usec * 1e-6 +
         ru.ru_stime.tv_sec + ru.ru_stime.tv_usec * 1e-6;
}
static double procMemMb(const char *_field)
{
  FILE *fp = std::fopen("/proc/self/status", "r");
  if (!fp) return 0.0;
  char line[256]; double mb = 0.0;
  const size_t flen = std::strlen(_field);
  while (std::fgets(line, sizeof(line), fp))
  {
    if (std::strncmp(line, _field, flen) == 0)
    {
      long kb = 0;
      if (std::sscanf(line + flen, " %ld", &kb) == 1) mb = kb / 1024.0;
      break;
    }
  }
  std::fclose(fp);
  return mb;
}
static double rssMb()     { return procMemMb("VmRSS:"); }
static double peakRssMb() { return procMemMb("VmHWM:"); }

static ScenePtr buildScene(const std::string &_engineName,
                           const std::string &_backend)
{
  std::map<std::string, std::string> params;
  if (_backend == "vulkan")
    params["vulkan"] = "1";
  RenderEngine *engine = rendering::engine(_engineName, params);
  if (!engine)
  {
    std::cerr << "Engine '" << _engineName << "' unavailable\n";
    return nullptr;
  }
  ScenePtr scene = engine->CreateScene("scene");
  scene->SetAmbientLight(0.3, 0.3, 0.3);
  scene->SetBackgroundColor(0.2, 0.6, 0.9);
  VisualPtr root = scene->RootVisual();

  // A few objects at varying depth so the depth/point-cloud passes do real work.
  for (int i = 0; i < 5; ++i)
  {
    VisualPtr v = scene->CreateVisual("obj" + std::to_string(i));
    v->AddGeometry(i % 2 ? scene->CreateSphere() : scene->CreateBox());
    v->SetLocalPosition(2.0 + i, (i - 2) * 0.6, 0.0);
    root->AddChild(v);
  }
  return scene;
}

int main(int _argc, char **_argv)
{
  common::Console::SetVerbosity(2);

  const std::string engineName = (_argc > 1) ? _argv[1] : "ogre2";
  const std::string backend    = (_argc > 2) ? _argv[2] : "vulkan";
  const unsigned int width  = (_argc > 3) ?
      static_cast<unsigned int>(std::max(1, std::atoi(_argv[3]))) : 640u;
  const unsigned int height = (_argc > 4) ?
      static_cast<unsigned int>(std::max(1, std::atoi(_argv[4]))) : 320u;
  const int frames = std::max(1, (_argc > 5) ? std::atoi(_argv[5]) : 200);
  const int ncams  = std::max(1, (_argc > 6) ? std::atoi(_argv[6]) : 8);

  ScenePtr scene = buildScene(engineName, backend);
  if (!scene) return 1;

  std::vector<DepthCameraPtr> cams;
  for (int i = 0; i < ncams; ++i)
  {
    DepthCameraPtr cam = scene->CreateDepthCamera("rgbd" + std::to_string(i));
    cam->SetImageWidth(width);
    cam->SetImageHeight(height);
    cam->SetAspectRatio(static_cast<double>(width) / height);
    cam->SetHFOV(GZ_PI / 2);
    cam->SetImageFormat(PF_FLOAT32_RGBA);
    cam->SetNearClipPlane(0.15);
    cam->SetFarClipPlane(20.0);
    cam->CreateDepthTexture();
    scene->RootVisual()->AddChild(cam);
    cams.push_back(cam);
  }

  // Per-pixel byte volumes: depth = 1 float; point cloud = 4 floats (XYZRGBA).
  const size_t depthBytesPerCam =
      static_cast<size_t>(width) * height * sizeof(float);
  const size_t cloudBytesPerCam =
      static_cast<size_t>(width) * height * 4u * sizeof(float);

  long depthFrames = 0;
  long cloudFrames = 0;
  double lastDepthSum = 0.0;   // sampled checksum to compare sync vs async data
  std::vector<common::ConnectionPtr> conns;
  for (auto &c : cams)
  {
    conns.push_back(c->ConnectNewDepthFrame(
        [&](const float *_data, unsigned int _w, unsigned int _h, unsigned int,
            const std::string &)
        {
          ++depthFrames;
          double s = 0.0;
          const unsigned int n = _w * _h;
          for (unsigned int i = 0; i < n; i += 251u)
            if (std::isfinite(_data[i])) s += _data[i];
          lastDepthSum = s;
        }));
  }

  auto emit = [&](const std::string &path, double secs, double cpu0,
                  bool withCloud)
  {
    const double cpuPct = 100.0 * (cpuSeconds() - cpu0) / secs;
    const double msTick = (secs * 1000.0) / frames;
    const double msCam  = msTick / ncams;
    const double fpsCam = 1000.0 / msTick;        // achieved per-sensor Hz
    const double depthMbs =
        static_cast<double>(depthBytesPerCam) * ncams * fpsCam / 1e6;
    const double cloudMbs = withCloud ?
        static_cast<double>(cloudBytesPerCam) * ncams * fpsCam / 1e6 : 0.0;
    std::cout << "RESULT engine=" << engineName << " backend=" << backend
              << " path=" << path << " ncams=" << ncams
              << " w=" << width << " h=" << height << " frames=" << frames
              << " ms_per_tick=" << msTick << " ms_per_cam=" << msCam
              << " fps_per_cam=" << fpsCam << " fps_total=" << (fpsCam * ncams)
              << " cpu_pct=" << cpuPct << " rss_mb=" << rssMb()
              << " peak_rss_mb=" << peakRssMb()
              << " depth_mb_s=" << depthMbs << " cloud_mb_s=" << cloudMbs
              << " total_mb_s=" << (depthMbs + cloudMbs)
              << " depth_frames=" << depthFrames
              << " cloud_frames=" << cloudFrames
              << " depth_checksum=" << lastDepthSum << "\n";
  };

  for (int i = 0; i < 10; ++i)            // warmup (depth only)
    for (auto &c : cams) c->Update();

  // --- depth_only: render + depth readback ---
  depthFrames = 0; cloudFrames = 0;
  {
    double cpu0 = cpuSeconds();
    auto t0 = Clock::now();
    for (int f = 0; f < frames; ++f)
      for (auto &c : cams) c->Update();
    double secs = std::chrono::duration<double>(Clock::now() - t0).count();
    emit("depth_only", secs, cpu0, false);
  }

  // --- rgbd: also read back the XYZRGBA point cloud (full RGBD) ---
  for (auto &c : cams)
  {
    conns.push_back(c->ConnectNewRgbPointCloud(
        [&](const float *, unsigned int, unsigned int, unsigned int,
            const std::string &){ ++cloudFrames; }));
  }
  for (int i = 0; i < 10; ++i)            // warmup with cloud enabled
    for (auto &c : cams) c->Update();

  depthFrames = 0; cloudFrames = 0;
  {
    double cpu0 = cpuSeconds();
    auto t0 = Clock::now();
    for (int f = 0; f < frames; ++f)
      for (auto &c : cams) c->Update();
    double secs = std::chrono::duration<double>(Clock::now() - t0).count();
    emit("rgbd", secs, cpu0, true);
  }

  // --- rgbd_async: same depth + cloud readback, but deferred (non-blocking) ---
  // so each camera's readback DMA overlaps the next camera's render.
  for (auto &c : cams) c->SetAsyncReadback(true);
  for (int i = 0; i < 12; ++i)            // warmup: fill + reach steady state
    for (auto &c : cams) c->Update();

  depthFrames = 0; cloudFrames = 0;
  {
    double cpu0 = cpuSeconds();
    auto t0 = Clock::now();
    for (int f = 0; f < frames; ++f)
      for (auto &c : cams) c->Update();
    double secs = std::chrono::duration<double>(Clock::now() - t0).count();
    emit("rgbd_async", secs, cpu0, true);
  }
  for (auto &c : cams) c->SetAsyncReadback(false);

  return 0;
}
