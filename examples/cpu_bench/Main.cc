/*
 * Headless CPU benchmark for the ogre2 (and ogre1) render path.
 *
 * Renders N boxes offscreen for a number of frames and reports per-frame wall
 * and CPU time. Designed to isolate WHERE the per-frame CPU goes for a static
 * many-object scene (the "400 static boxes burns insane CPU" report).
 *
 * All knobs are environment variables so a single binary drives the whole A/B
 * matrix without rebuilds:
 *
 *   BENCH_N          number of boxes                       (default 400)
 *   BENCH_FRAMES     timed frames                          (default 300)
 *   BENCH_WARMUP     warmup frames (not timed)             (default 30)
 *   BENCH_SHADOWS    1=dir light casts PSSM shadows, 0=off (default 1)
 *   BENCH_MOVE_POSES 1=re-push every box pose each frame   (default 1)
 *                    (mimics gz-sim RenderUtil unconditional pose update)
 *   BENCH_OFFSCREEN  1=place boxes BEHIND camera (frustum-culled, never drawn)
 *                                                          (default 0)
 *   BENCH_W BENCH_H  render target size                    (default 800x600)
 *   BENCH_VERBOSE    1=ogre console verbosity 4            (default 0)
 *   BENCH_CAPTURE    1=write bench_capture.png + count non-bg px once
 */

#include <sys/resource.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include <gz/common/Console.hh>
#include <gz/common/Image.hh>
#include <gz/rendering.hh>

using namespace gz;
using namespace rendering;

static int envi(const char *_k, int _def)
{
  const char *v = std::getenv(_k);
  return v ? std::atoi(v) : _def;
}

static double cpuSeconds()
{
  struct rusage ru;
  getrusage(RUSAGE_SELF, &ru);
  double u = ru.ru_utime.tv_sec + ru.ru_utime.tv_usec * 1e-6;
  double s = ru.ru_stime.tv_sec + ru.ru_stime.tv_usec * 1e-6;
  return u + s;
}

int main(int _argc, char **_argv)
{
  const std::string engineName = (_argc > 1) ? _argv[1] : "ogre2";

  const int N         = envi("BENCH_N", 400);
  const int frames    = envi("BENCH_FRAMES", 300);
  const int warmup    = envi("BENCH_WARMUP", 30);
  const int shadows   = envi("BENCH_SHADOWS", 1);
  const int movePoses = envi("BENCH_MOVE_POSES", 1);
  const int offscreen = envi("BENCH_OFFSCREEN", 0);
  const int W         = envi("BENCH_W", 800);
  const int H         = envi("BENCH_H", 600);
  const int capFps    = envi("BENCH_FPS", 0);   // 0 = uncapped; >0 caps frame rate

  if (envi("BENCH_VERBOSE", 0))
    common::Console::SetVerbosity(4);

  RenderEngine *engine = rendering::engine(engineName);
  if (!engine)
  {
    std::cerr << "engine '" << engineName << "' not available\n";
    return 1;
  }
  ScenePtr scene = engine->CreateScene("scene");
  scene->SetAmbientLight(0.3, 0.3, 0.3);
  scene->SetBackgroundColor(0.2, 0.2, 0.3);
  VisualPtr root = scene->RootVisual();

  // Directional light, optionally shadow-casting (PSSM, 3 splits in ogre2).
  DirectionalLightPtr light = scene->CreateDirectionalLight();
  light->SetDirection(-0.5, 0.5, -1);
  light->SetDiffuseColor(0.8, 0.8, 0.8);
  light->SetSpecularColor(0.5, 0.5, 0.5);
  light->SetCastShadows(shadows != 0);
  root->AddChild(light);

  MaterialPtr mat = scene->CreateMaterial();
  mat->SetAmbient(0.3, 0.3, 0.3);
  mat->SetDiffuse(0.7, 0.2, 0.2);
  mat->SetSpecular(0.3, 0.3, 0.3);

  // gz camera at the origin looks down +X by default. Build a wall of boxes in
  // the Y-Z plane in front of the camera, sized to roughly fill a 90deg HFOV.
  const int side = static_cast<int>(std::ceil(std::sqrt((double)N)));
  const double spacing = 1.0;
  const double halfExtent = 0.5 * side * spacing;
  // distance so the wall ~fits the 90deg frustum (half-width == distance), +margin
  const double depth = halfExtent / 0.8 + 2.0;

  std::vector<VisualPtr> boxes;
  boxes.reserve(N);
  for (int i = 0; i < N; ++i)
  {
    const int r = i / side;
    const int c = i % side;
    VisualPtr box = scene->CreateVisual();
    box->AddGeometry(scene->CreateBox());
    const double y = (c - side / 2.0) * spacing;
    const double z = (r - side / 2.0) * spacing;
    // offscreen => behind the camera (negative X) so the frustum cull drops them
    const double x = offscreen ? -50.0 : depth;
    box->SetLocalPosition(x, y, z);
    box->SetLocalScale(0.35, 0.35, 0.35);
    box->SetMaterial(mat);
    root->AddChild(box);
    boxes.push_back(box);
  }

  if (!offscreen)
  {
    VisualPtr ground = scene->CreateVisual();
    ground->AddGeometry(scene->CreatePlane());
    ground->SetLocalScale(depth * 4, side * spacing * 2, 1);
    ground->SetLocalPosition(depth, 0, -halfExtent - 1.0);
    MaterialPtr gmat = scene->CreateMaterial();
    gmat->SetDiffuse(0.5, 0.5, 0.5);
    gmat->SetReceiveShadows(true);
    ground->SetMaterial(gmat);
    root->AddChild(ground);
  }

  CameraPtr camera = scene->CreateCamera("camera");
  camera->SetLocalPosition(0, 0, 0);
  camera->SetLocalRotation(0, 0, 0);
  camera->SetImageWidth(W);
  camera->SetImageHeight(H);
  camera->SetAspectRatio((double)W / H);
  camera->SetHFOV(GZ_PI / 2.0);
  camera->SetAntiAliasing(2);
  root->AddChild(camera);

  auto reapply = [&]()
  {
    for (auto &b : boxes)
      b->SetLocalPose(b->LocalPose());
  };

  // Warmup (shader compile, first-frame allocations, shadow node build).
  for (int i = 0; i < warmup; ++i)
  {
    if (movePoses) reapply();
    camera->Update();
  }

  if (envi("BENCH_CAPTURE", 0))
  {
    Image img = camera->CreateImage();
    camera->Capture(img);
    unsigned int nonBg = 0;
    const unsigned char *d = img.Data<unsigned char>();
    const unsigned int px = camera->ImageWidth() * camera->ImageHeight();
    for (unsigned int p = 0; p < px; ++p)
    {
      // background is (0.2,0.2,0.3) ~ (51,51,76)
      unsigned char rr = d[p * 3], gg = d[p * 3 + 1], bb = d[p * 3 + 2];
      if (std::abs((int)rr - 51) > 12 || std::abs((int)gg - 51) > 12 ||
          std::abs((int)bb - 76) > 12)
        ++nonBg;
    }
    std::cerr << "[capture] non-background px = " << nonBg << " / " << px
              << " (" << (100.0 * nonBg / px) << "%)\n";
  }

  // Timed loop.
  std::vector<double> perFrame;
  perFrame.reserve(frames);
  const double cpu0 = cpuSeconds();
  const auto t0 = std::chrono::steady_clock::now();
  const std::chrono::duration<double> framePeriod(capFps > 0 ? 1.0 / capFps : 0.0);
  auto deadline = t0;
  for (int i = 0; i < frames; ++i)
  {
    const auto f0 = std::chrono::steady_clock::now();
    if (movePoses) reapply();
    camera->Update();
    const auto f1 = std::chrono::steady_clock::now();
    perFrame.push_back(
        std::chrono::duration<double, std::milli>(f1 - f0).count());
    if (capFps > 0)
    {
      deadline += std::chrono::duration_cast<std::chrono::steady_clock::duration>(
          framePeriod);
      std::this_thread::sleep_until(deadline);
    }
  }
  const auto t1 = std::chrono::steady_clock::now();
  const double cpu1 = cpuSeconds();

  const double wallTotal =
      std::chrono::duration<double, std::milli>(t1 - t0).count();
  std::sort(perFrame.begin(), perFrame.end());
  const double wMin = perFrame.front();
  const double wMed = perFrame[perFrame.size() / 2];
  const double wMean = wallTotal / frames;
  const double cpuPerFrame = (cpu1 - cpu0) * 1000.0 / frames;
  const double cpuTotalSec = (cpu1 - cpu0);
  const double wallTotalSec = wallTotal / 1000.0;
  const double coresBusy = cpuTotalSec / wallTotalSec;
  const double achievedFps = frames / wallTotalSec;

  std::cout << "BENCH engine=" << engineName << " N=" << N
            << " shadows=" << shadows << " poses=" << movePoses
            << " offscreen=" << offscreen << " capFps=" << capFps << " "
            << W << "x" << H << " frames=" << frames << "\n"
            << "  achieved fps:   " << achievedFps << "\n"
            << "  wall/frame ms:  min=" << wMin << "  median=" << wMed
            << "  mean=" << wMean << "\n"
            << "  cpu/frame  ms:  mean=" << cpuPerFrame << "\n"
            << "  CORES BUSY:     " << coresBusy
            << "  (total CPU " << cpuTotalSec << "s / wall " << wallTotalSec
            << "s)\n";

  // Stable, grep-friendly one-liner.
  std::cout << "CSV," << engineName << "," << N << "," << shadows << ","
            << movePoses << "," << offscreen << "," << W << "x" << H << ","
            << wMin << "," << wMed << "," << wMean << "," << cpuPerFrame
            << "\n";

  scene->DestroySensor(camera);
  engine->DestroyScene(scene);
  return 0;
}
