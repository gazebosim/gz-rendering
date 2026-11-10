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

// Smoke test for render engines: builds the simple_demo scene, saves an RGB
// frame and prints a few depth-camera samples. Usage:
//   filament_smoke <engine> [out_prefix] [frames]

#include <algorithm>
#include <chrono>
#include <vector>
#include <cmath>
#include <iostream>
#include <limits>
#include <map>
#include <string>

#include <gz/common/Console.hh>
#include <gz/common/Image.hh>

#include <gz/rendering/Camera.hh>
#include <gz/rendering/DepthCamera.hh>
#include <gz/rendering/Image.hh>
#include <gz/rendering/Material.hh>
#include <gz/rendering/RenderEngine.hh>
#include <gz/rendering/RenderingIface.hh>
#include <gz/rendering/Scene.hh>
#include <gz/rendering/Visual.hh>
#include <gz/rendering/Light.hh>

using namespace gz;
using namespace rendering;

namespace
{
  void BuildScene(ScenePtr _scene)
  {
    _scene->SetAmbientLight(0.3, 0.3, 0.3);
    _scene->SetBackgroundColor(0.3, 0.3, 0.3);
    VisualPtr root = _scene->RootVisual();

    DirectionalLightPtr light0 = _scene->CreateDirectionalLight();
    light0->SetDirection(-0.5, 0.5, -1);
    light0->SetDiffuseColor(0.8, 0.8, 0.8);
    light0->SetSpecularColor(0.5, 0.5, 0.5);
    root->AddChild(light0);

    PointLightPtr light2 = _scene->CreatePointLight();
    light2->SetDiffuseColor(0.5, 0.5, 0.5);
    light2->SetSpecularColor(0.5, 0.5, 0.5);
    light2->SetLocalPosition(3, 5, 5);
    root->AddChild(light2);

    MaterialPtr green = _scene->CreateMaterial();
    green->SetAmbient(0.0, 0.5, 0.0);
    green->SetDiffuse(0.0, 0.7, 0.0);
    green->SetSpecular(0.5, 0.5, 0.5);
    green->SetShininess(50);
    green->SetReflectivity(0);

    MaterialPtr red = _scene->CreateMaterial();
    red->SetAmbient(0.5, 0.0, 0.0);
    red->SetDiffuse(1.0, 0.0, 0.0);
    red->SetSpecular(0.5, 0.5, 0.5);

    MaterialPtr blue = _scene->CreateMaterial();
    blue->SetAmbient(0.0, 0.0, 0.3);
    blue->SetDiffuse(0.0, 0.0, 0.8);
    blue->SetSpecular(0.5, 0.5, 0.5);

    MaterialPtr white = _scene->CreateMaterial();
    white->SetAmbient(0.5, 0.5, 0.5);
    white->SetDiffuse(0.8, 0.8, 0.8);
    white->SetReceiveShadows(true);
    white->SetReflectivity(0);

    VisualPtr center = _scene->CreateVisual("center");
    center->AddGeometry(_scene->CreateSphere());
    center->SetLocalPosition(3, 0, 0);
    center->SetLocalScale(0.1, 0.1, 0.1);
    center->SetMaterial(green);
    root->AddChild(center);

    VisualPtr sphere = _scene->CreateVisual("sphere");
    sphere->AddGeometry(_scene->CreateSphere());
    sphere->SetOrigin(0.0, -0.5, 0.0);
    sphere->SetLocalPosition(3, 0, 0);
    sphere->SetLocalRotation(0, 0, 0);
    sphere->SetLocalScale(1, 2.5, 1);
    sphere->SetMaterial(red);
    root->AddChild(sphere);

    VisualPtr box = _scene->CreateVisual("box");
    box->AddGeometry(_scene->CreateBox());
    box->SetOrigin(0.0, 0.5, 0.0);
    box->SetLocalPosition(3, 0, 0);
    box->SetLocalRotation(GZ_PI / 4, 0, GZ_PI / 3);
    box->SetLocalScale(1, 2.5, 1);
    box->SetMaterial(blue);
    root->AddChild(box);

    VisualPtr cylinder = _scene->CreateVisual("cylinder");
    cylinder->AddGeometry(_scene->CreateCylinder());
    cylinder->SetLocalPosition(4, -1.5, 0);
    cylinder->SetLocalScale(0.5, 0.5, 1.5);
    cylinder->SetMaterial(green);
    root->AddChild(cylinder);

    VisualPtr plane = _scene->CreateVisual("plane");
    plane->AddGeometry(_scene->CreatePlane());
    plane->SetLocalScale(5, 8, 1);
    plane->SetLocalPosition(3, 0, -0.5);
    plane->SetMaterial(white);
    root->AddChild(plane);
  }

  void Stats(const float *_d, unsigned int _w, unsigned int _h,
      const std::string &_label)
  {
    float mn = std::numeric_limits<float>::infinity();
    float mx = -std::numeric_limits<float>::infinity();
    unsigned int nInf = 0, nNegInf = 0, nNan = 0;
    for (unsigned int i = 0; i < _w * _h; ++i)
    {
      const float v = _d[i];
      if (std::isnan(v)) { ++nNan; continue; }
      if (std::isinf(v)) { if (v > 0) ++nInf; else ++nNegInf; continue; }
      mn = std::min(mn, v);
      mx = std::max(mx, v);
    }
    const unsigned int mid = (_h / 2) * _w + _w / 2;
    const unsigned int left = (_h / 2) * _w;
    const unsigned int right = (_h / 2 + 1) * _w - 1;
    const unsigned int top = _w / 2;
    const unsigned int bottom = (_h - 1) * _w + _w / 2;
    std::cout << _label << ": center=" << _d[mid] << " left=" << _d[left]
              << " right=" << _d[right] << " top=" << _d[top]
              << " bottom=" << _d[bottom] << " finite[min,max]=[" << mn
              << "," << mx << "] +inf=" << nInf << " -inf=" << nNegInf
              << " nan=" << nNan << std::endl;
  }
}

int main(int _argc, char **_argv)
{
  common::Console::SetVerbosity(4);
  const std::string engineName = _argc > 1 ? _argv[1] : "filament";
  const std::string prefix = _argc > 2 ? _argv[2] : engineName;
  const int frames = _argc > 3 ? std::atoi(_argv[3]) : 20;

  std::map<std::string, std::string> params;
  if (_argc > 4 && std::string(_argv[4]) != "-")
    params[_argv[4]] = "1";
  RenderEngine *engine = rendering::engine(engineName, params);
  if (!engine)
  {
    std::cerr << "Engine '" << engineName << "' is not supported" << std::endl;
    return 1;
  }
  ScenePtr scene = engine->CreateScene("scene");
  BuildScene(scene);
  // optional grid of extra boxes behind the demo objects, to see how the
  // per-frame cost scales with object count
  const int extra = _argc > 5 ? std::atoi(_argv[5]) : 0;
  {
    MaterialPtr gray = scene->CreateMaterial();
    gray->SetDiffuse(0.6, 0.6, 0.6);
    const int side = static_cast<int>(std::ceil(std::sqrt(extra)));
    for (int i = 0; i < extra; ++i)
    {
      VisualPtr v = scene->CreateVisual();
      v->AddGeometry(scene->CreateBox());
      v->SetLocalScale(0.15, 0.15, 0.15);
      v->SetLocalPosition(6 + 0.25 * (i / side), -0.25 * side / 2.0 +
          0.25 * (i % side), 0.5);
      v->SetMaterial(gray);
      scene->RootVisual()->AddChild(v);
    }
  }
  VisualPtr root = scene->RootVisual();

  // RGB camera
  CameraPtr camera = scene->CreateCamera("camera");
  camera->SetLocalPosition(0.0, 0.0, 0.5);
  camera->SetLocalRotation(0.0, 0.0, 0.0);
  camera->SetImageWidth(640);
  camera->SetImageHeight(480);
  camera->SetAntiAliasing(0);
  camera->SetAspectRatio(640.0 / 480.0);
  camera->SetHFOV(GZ_PI / 2);
  root->AddChild(camera);

  Image image = camera->CreateImage();
  camera->Capture(image);
  {
    common::Image out;
    out.SetFromData(image.Data<unsigned char>(), 640, 480,
        common::Image::RGB_INT8);
    out.SavePNG(prefix + "_rgb.png");
  }
  {
    const unsigned char *d = image.Data<unsigned char>();
    auto px = [&](unsigned x, unsigned y) {
      const unsigned char *p = d + (y * 640 + x) * 3;
      return std::to_string(p[0]) + "," + std::to_string(p[1]) + "," +
             std::to_string(p[2]);
    };
    std::cout << "rgb: center=" << px(320, 240) << " top-left=" << px(5, 5)
              << " bottom-center=" << px(320, 470)
              << " right-middle=" << px(630, 240) << std::endl;
  }

  // Depth camera
  DepthCameraPtr depth = scene->CreateDepthCamera("depth");
  depth->SetLocalPosition(0.0, 0.0, 0.5);
  depth->SetImageWidth(320);
  depth->SetImageHeight(240);
  depth->SetAspectRatio(320.0 / 240.0);
  depth->SetHFOV(GZ_PI / 2);
  depth->SetNearClipPlane(0.15);
  depth->SetFarClipPlane(10.0);
  depth->CreateDepthTexture();
  root->AddChild(depth);

  unsigned int depthFrames = 0;
  auto conn = depth->ConnectNewDepthFrame(
      [&](const float *_d, unsigned int _w, unsigned int _h, unsigned int,
          const std::string &)
      {
        ++depthFrames;
        if (depthFrames == 1)
          Stats(_d, _w, _h, "depth");
      });
  depth->Update();
  std::cout << "depth frames: " << depthFrames << std::endl;

  // Timing: warm up, then per-frame times for RGB capture and depth update
  {
    using Clock = std::chrono::steady_clock;
    auto measure = [&](auto &&_fn)
    {
      for (int i = 0; i < 10; ++i)
        _fn();
      std::vector<double> t;
      for (int i = 0; i < frames; ++i)
      {
        const auto a = Clock::now();
        _fn();
        t.push_back(std::chrono::duration<double, std::milli>(
            Clock::now() - a).count());
      }
      std::sort(t.begin(), t.end());
      return std::make_pair(t[t.size() / 2], t[t.size() * 9 / 10]);
    };
    // phase split of the RGB frame: scene update (plugin CPU), render
    // submission (engine CPU), readback (GPU + transfer)
    {
      std::vector<double> pre, ren, cpy;
      for (int i = 0; i < frames + 10; ++i)
      {
        const auto a = Clock::now();
        scene->PreRender();
        const auto b = Clock::now();
        camera->Render();
        camera->PostRender();
        scene->PostRender();
        const auto c = Clock::now();
        camera->Copy(image);
        const auto d = Clock::now();
        if (i < 10)
          continue;
        pre.push_back(std::chrono::duration<double, std::milli>(b - a).count());
        ren.push_back(std::chrono::duration<double, std::milli>(c - b).count());
        cpy.push_back(std::chrono::duration<double, std::milli>(d - c).count());
      }
      for (auto *v : {&pre, &ren, &cpy})
        std::sort(v->begin(), v->end());
      std::cout << "rgb phases (median ms): scene PreRender "
                << pre[pre.size() / 2] << ", render " << ren[ren.size() / 2]
                << ", readback " << cpy[cpy.size() / 2] << std::endl;
    }
    const auto rgb = measure([&]{ camera->Capture(image); });
    const auto dep = measure([&]{ depth->Update(); });
    std::cout << "timing (" << frames << " frames, median/p90 ms): rgb 640x480 "
              << rgb.first << "/" << rgb.second << ", depth 320x240 "
              << dep.first << "/" << dep.second << std::endl;
  }

  // Save a depth preview (normalized to [near, far])
  {
    const float *d = depth->DepthData();
    common::Image out;
    std::vector<unsigned char> buf(320 * 240 * 3);
    for (unsigned int i = 0; i < 320 * 240; ++i)
    {
      float v = d[i];
      unsigned char g = 0;
      if (std::isfinite(v))
        g = static_cast<unsigned char>(255.0f * (1.0f - std::min(v, 10.0f) / 10.0f));
      buf[i * 3] = buf[i * 3 + 1] = buf[i * 3 + 2] = g;
    }
    out.SetFromData(buf.data(), 320, 240, common::Image::RGB_INT8);
    out.SavePNG(prefix + "_depth.png");
  }

  engine->DestroyScene(scene);
  rendering::unloadEngine(engineName);
  std::cout << "done" << std::endl;
  return 0;
}
