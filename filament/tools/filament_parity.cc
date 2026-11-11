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

// Sensor parity dump: builds a fixed scene, renders depth cameras (depth +
// point cloud) and GPU lidars with the given engine and writes the raw float
// outputs plus per-sensor timings to <outdir>. Compare two engines with
// filament/tools/parity_compare.py.
//
// usage: filament_parity <engine> <outdir> [timingFrames] [engineParam]
// engineParam is set to "1", e.g. "opengl" for filament
// PARITY_HEATSIG=1: adds heat signature objects (thermal outputs)
// PARITY_CAPSULE=1: adds a tilted capsule, resized after it is attached
// PARITY_TEXBOX=<png>: textured box in the RGB overview (texture orientation)
// PARITY_POINTLIGHTS=1: RGB overview lit like a house interior (dim sun,
// point and spot lights near surfaces, specular 0.01)
// PARITY_PBR=1: PBR material row in the RGB overview (roughness, metal,
// normal, emissive, light and environment maps, wireframe); PARITY_PBR=sky also
// enables the sky
// PARITY_WIDEANGLE=1: only the wide-angle camera geometry checks (RGB images,
// per-object centroids, cutoff circle radius, Project3d), see WideAngle()
// PARITY_PARTICLES=1: only particle statistics, with a particle emitter in
// front of the depth cameras, lidars and thermal cameras, see Particles()
// PARITY_NOISE=1: only the Gaussian noise pass outputs (RGB, wide angle,
// depth + point cloud, thermal), see Noise()
// PARITY_LENSFLARE=1: only lens flare pictures (lf_*.png), see LensFlare()
// PARITY_MARKERS=1: only what each sensor sees of a MT_POINTS marker, printed,
// see Markers()
// PARITY_PROJECTOR=<png>: only a projector of the picture over a floor seen
// from above, written to projector.png, see ProjectTexture()
// PARITY_SHADER=1: only the custom_shaders_uniforms box (GLSL on ogre2, its
// .mat port on filament), shader.png and shader_updated.png (new params);
// PARITY_SHADER=waves: the Gerstner waves of test/integration/waves.cc
// PARITY_USERCAM=1: the rgb camera is flagged as the GUI's user camera
// (user data "user-camera", 8x AA), see UpdateQualityMode in FilamentCamera;
// PARITY_USERCAM=<n>: also prints its ms per capture over n captures
// PARITY_GUI=1: only gui.png, the gz-sim GUI overlays (gizmo, grid, light
// visual, transparent and wireframe visuals), see GuiOverlays()
// PARITY_DEPTHEDGE=1: only depth cameras crossing both clip ranges with
//   off-center intrinsics: _depth, _cloud and _depthonly (see DepthEdges)
// PARITY_PROJECTION=1: only custom (off-center intrinsics) and orthographic
// projections on depth, bbox and RGB cameras, see Projection()
// PARITY_ALPHATEST=1: adds an alpha tested (alpha from texture) checker
// card in front of the sensors, and alpha_plate.png / alpha_shadow.png, an
// alpha tested plate and its shadow seen from above, see AlphaTest()
// PARITY_SPOT="h range c l q inner outer falloff shadows [rough] [spec]": only
// spot.png (top down) and spot_oblique.png, one spot light h m above a grey
// floor with these SDF parameters, see Spot()
// PARITY_ACTOR=1: adds two skinned walking actors (test/media/meshes/
// walk.dae) posed at a fixed animation time, one through the animation API
// (with a bone weight) and one through SetSkeletonLocalTransforms; writes
// actor.txt: ogre2's visible-2D boxes draw actors in bind pose (see
// parity_compare.py)

#include <algorithm>
#include <chrono>
#include <cstring>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <sstream>

#include <gz/common/Console.hh>
#include <gz/common/Image.hh>
#include <gz/common/geospatial/ImageHeightmap.hh>
#include <gz/common/Mesh.hh>
#include <gz/common/MeshManager.hh>
#include <gz/common/Skeleton.hh>
#include <gz/common/SkeletonAnimation.hh>
#include <gz/math/Pose3.hh>

#include <gz/rendering/BoundingBoxCamera.hh>
#include <gz/rendering/Camera.hh>
#include <gz/rendering/Capsule.hh>
#include <gz/rendering/DepthCamera.hh>
#include <gz/rendering/GaussianNoisePass.hh>
#include <gz/rendering/GizmoVisual.hh>
#include <gz/rendering/Grid.hh>
#include <gz/rendering/Heightmap.hh>
#include <gz/rendering/Light.hh>
#include <gz/rendering/Mesh.hh>
#include <gz/rendering/ShaderParams.hh>
#include <gz/rendering/Image.hh>
#include <gz/rendering/LensFlarePass.hh>
#include <gz/rendering/LightVisual.hh>
#include <gz/rendering/GpuRays.hh>
#include <gz/rendering/RenderEngine.hh>
#include <gz/rendering/RenderPassSystem.hh>
#include <gz/rendering/RenderingIface.hh>
#include <gz/rendering/Scene.hh>
#include <gz/rendering/SegmentationCamera.hh>
#include <gz/rendering/ThermalCamera.hh>
#include <gz/rendering/Marker.hh>
#include <gz/rendering/Material.hh>
#include <gz/rendering/Projector.hh>
#include <gz/rendering/ParticleEmitter.hh>
#include <gz/rendering/Visual.hh>
#include <gz/rendering/WideAngleCamera.hh>

using namespace gz;
using namespace rendering;

namespace
{
  /// \brief Counter used to give every other object a segmentation label
  int g_added = 0;

  VisualPtr Add(ScenePtr _scene, GeometryPtr _geom, const math::Pose3d &_pose,
      const math::Vector3d &_scale)
  {
    VisualPtr v = _scene->CreateVisual();
    v->AddGeometry(_geom);
    v->SetLocalPose(_pose);
    v->SetLocalScale(_scale);
    // labels 1..3 on most objects, some left unlabeled (background)
    if (g_added++ % 4 != 0)
      v->SetUserData("label", 1 + g_added % 3);
    // assorted diffuse colors (thermal uses them for non heat sources),
    // every third object is a heat source
    MaterialPtr mat = _scene->CreateMaterial();
    mat->SetDiffuse(0.1 + 0.08 * g_added, 0.9 - 0.07 * g_added,
        0.3 + 0.05 * (g_added % 5));
    v->SetMaterial(mat);
    if (g_added % 3 == 0)
      v->SetUserData("temperature", 280.0f + 7.3f * g_added);
    _scene->RootVisual()->AddChild(v);
    return v;
  }

  /// \brief Red checker with fully transparent cells, alpha from texture
  MaterialPtr CheckerMaterial(ScenePtr _scene, bool _twoSided)
  {
    const unsigned int n = 64;
    std::vector<unsigned char> px(n * n * 4);
    for (unsigned int i = 0; i < n * n; ++i)
    {
      unsigned char *p = &px[i * 4];
      p[0] = 220;
      p[1] = 30;
      p[2] = 30;
      p[3] = ((i % n) / 16 + (i / n) / 16) % 2 ? 255 : 0;
    }
    auto img = std::make_shared<common::Image>();
    img->SetFromData(px.data(), n, n, common::Image::RGBA_INT8);
    MaterialPtr m = _scene->CreateMaterial();
    m->SetDiffuse(1, 1, 1);
    m->SetTexture("parity_alpha_checker", img);
    m->SetAlphaFromTexture(true, 0.5, _twoSided);
    return m;
  }

  void Write(const std::string &_dir, const std::string &_name,
      const float *_data, unsigned int _w, unsigned int _h, unsigned int _c,
      std::ofstream &_index)
  {
    std::ofstream f(_dir + "/" + _name + ".f32", std::ios::binary);
    f.write(reinterpret_cast<const char *>(_data),
        static_cast<std::streamsize>(sizeof(float)) * _w * _h * _c);
    _index << _name << " " << _w << " " << _h << " " << _c << "\n";
  }
  void BuildScene(ScenePtr _scene)
  {
    // floor and a back wall (the wall sits inside every far clip); with
    // PARITY_HEIGHTMAP=<image> a 20 x 20 x 3 m heightmap replaces the floor,
    // its mid height at z = 0
    if (const char *hm = std::getenv("PARITY_HEIGHTMAP"))
    {
      auto data = std::make_shared<common::ImageHeightmap>();
      if (data->Load(hm) != 0)
        std::cerr << "cannot load heightmap " << hm << std::endl;
      HeightmapDescriptor desc;
      desc.SetData(data);
      desc.SetSize({20, 20, 3});
      desc.SetPosition({0, 0, -1.5});
      // PARITY_HEIGHTMAP_POS="x y z": an off-center heightmap
      const char *p = std::getenv("PARITY_HEIGHTMAP_POS");
      if (p && *p)
      {
        std::istringstream in(p);
        math::Vector3d pos;
        in >> pos;
        desc.SetPosition(pos);
      }
      desc.SetSampling(2u);
      HeightmapTexture tex;
      tex.SetSize(2.0);
      tex.SetDiffuse(std::string(PROJECT_SOURCE_PATH) +
          "/test/media/materials/textures/texture.png");
      desc.AddTexture(tex);
      VisualPtr v = _scene->CreateVisual();
      v->AddGeometry(_scene->CreateHeightmap(desc));
      v->SetUserData("label", 9);
      _scene->RootVisual()->AddChild(v);
      ++g_added;
    }
    else
    {
      Add(_scene, _scene->CreateBox(), {0, 0, -0.05, 0, 0, 0}, {20, 20, 0.1});
    }
    Add(_scene, _scene->CreateBox(), {7, 0, 1.5, 0, 0, 0}, {0.2, 10, 3});
    // primitives at assorted poses, near and far
    Add(_scene, _scene->CreateBox(), {2, 0.5, 0.5, 0, 0, 0.4}, {1, 1, 1});
    Add(_scene, _scene->CreateBox(), {3.5, -1.5, 0.8, 0.3, 0.5, 0.9},
        {0.6, 1.2, 0.8});
    Add(_scene, _scene->CreateSphere(), {2.5, -0.8, 1.2, 0, 0, 0},
        {0.8, 0.8, 0.8});
    Add(_scene, _scene->CreateCylinder(), {4, 1.8, 0.75, 0, 0, 0},
        {0.5, 0.5, 1.5});
    Add(_scene, _scene->CreateCylinder(), {-2, 1, 0.3, GZ_PI / 2, 0, 0.3},
        {0.4, 0.4, 2});
    Add(_scene, _scene->CreateSphere(), {-1.5, -2, 0.6, 0, 0, 0},
        {1.2, 1.2, 1.2});
    Add(_scene, _scene->CreateBox(), {0, -3, 1, 0, 0, 0.7}, {2, 0.3, 2});
    // thin pole and an object closer than some near clips
    Add(_scene, _scene->CreateCylinder(), {1.2, -0.6, 1, 0, 0, 0},
        {0.05, 0.05, 2});
    Add(_scene, _scene->CreateBox(), {0.35, 0.3, 0.9, 0, 0, 0},
        {0.1, 0.1, 0.1});
    // a two-link model: one panoptic instance spread over child visuals
    VisualPtr model = _scene->CreateVisual("model");
    model->SetLocalPosition(3, 0.8, 0);
    for (int i = 0; i < 2; ++i)
    {
      VisualPtr link = _scene->CreateVisual();
      link->AddGeometry(_scene->CreateBox());
      link->SetLocalPosition(0, 0.5 * i, 0.3 + 0.6 * i);
      link->SetLocalScale(0.4, 0.4, 0.6);
      link->SetUserData("label", 7);
      model->AddChild(link);
    }
    _scene->RootVisual()->AddChild(model);

    if (std::getenv("PARITY_CAPSULE"))
    {
      CapsulePtr capsule = _scene->CreateCapsule();
      Add(_scene, capsule, {1.6, 1.3, 0.7, 0.5, 0.3, 0.2}, {1, 1, 1});
      capsule->SetRadius(0.3);
      capsule->SetLength(0.8);
    }

    // PARITY_ALPHATEST=1: a two-sided alpha tested card facing the sensors,
    // a labelled heat source: the cut-out cells are see-through for the
    // depth, lidar, segmentation and thermal cameras (ogre2 keeps the alpha
    // test there), not for the bounding box camera
    if (std::getenv("PARITY_ALPHATEST"))
    {
      VisualPtr v = _scene->CreateVisual();
      v->AddGeometry(_scene->CreatePlane());
      v->SetLocalPose({1.4, 0.6, 1.0, 0, -GZ_PI / 2, 0});
      v->SetLocalScale(1.2, 1.2, 1);
      v->SetMaterial(CheckerMaterial(_scene, true));
      v->SetUserData("label", 5);
      v->SetUserData("temperature", 310.0f);
      _scene->RootVisual()->AddChild(v);
    }

    // PARITY_HEATSIG=1: heat signature textures in the thermal view, a
    // uniform gray one (magnified) and texture.png's red channel
    if (std::getenv("PARITY_HEATSIG"))
    {
      const std::string tex = std::string(PROJECT_SOURCE_PATH) +
          "/test/media/materials/textures/";
      auto heat = [&](GeometryPtr _geom, const math::Pose3d &_pose,
          double _scale, const std::string &_png, float _min, float _max)
      {
        VisualPtr v = _scene->CreateVisual();
        v->AddGeometry(_geom);
        v->SetLocalPose(_pose);
        v->SetLocalScale(_scale);
        v->SetUserData("temperature", tex + _png);
        v->SetUserData("minTemp", _min);
        v->SetUserData("maxTemp", _max);
        _scene->RootVisual()->AddChild(v);
      };
      heat(_scene->CreateBox(), {1.3, 1.4, 0.4, 0, 0, 0.3}, 0.5,
          "gray_texture.png", 280.0f, 330.0f);
      heat(_scene->CreateSphere(), {1.5, -1.3, 1.0, 0, 0, 0.5}, 0.7,
          "texture.png", 260.0f, 390.0f);
      heat(_scene->CreateBox(), {2.2, -0.2, 1.6, 0.2, 0.1, 0.4}, 0.5,
          "texture.png", 270.0f, 380.0f);
    }

    // optional asset meshes (PARITY_MESHES=a.dae:b.obj:...), with their own
    // materials, scaled to ~0.9 m and set on the floor in an arc in view
    const char *list = std::getenv("PARITY_MESHES");
    std::vector<std::string> paths;
    std::stringstream ss(list ? list : "");
    for (std::string p; std::getline(ss, p, ':');)
      if (!p.empty())
        paths.push_back(p);
    for (size_t k = 0; k < paths.size(); ++k)
    {
      const common::Mesh *m = common::MeshManager::Instance()->Load(paths[k]);
      if (!m)
      {
        std::cerr << "cannot load " << paths[k] << std::endl;
        continue;
      }
      const math::Vector3d size = m->Max() - m->Min();
      const double scale = 0.9 / std::max({size.X(), size.Y(), size.Z()});
      const double a = paths.size() > 1 ?
          -0.7 + 1.4 * k / (paths.size() - 1) : 0.0;
      VisualPtr v = _scene->CreateVisual();
      v->AddGeometry(_scene->CreateMesh(paths[k]));
      v->SetLocalPose({1.8 + 0.8 * std::cos(a), 2.6 * std::sin(a),
          -m->Min().Z() * scale, 0, 0, 0.5 * k});
      v->SetLocalScale(scale);
      v->SetUserData("label", 4 + static_cast<int>(k % 2));
      if (k % 2 == 0)
        v->SetUserData("temperature", 300.0f + 4.1f * k);
      _scene->RootVisual()->AddChild(v);
    }

    // PARITY_ACTOR=1: the two ways gz-sim poses an actor, at a fixed time
    if (std::getenv("PARITY_ACTOR"))
    {
      MeshDescriptor desc(std::string(PROJECT_SOURCE_PATH) +
          "/test/media/meshes/walk.dae");
      desc.mesh = common::MeshManager::Instance()->Load(desc.meshName);
      common::SkeletonPtr skel = desc.mesh ? desc.mesh->MeshSkeleton() :
          nullptr;
      if (!skel || skel->AnimationCount() == 0)
      {
        std::cerr << "cannot load " << desc.meshName << std::endl;
        return;
      }
      // animation API as gz-sim's RenderUtil with trajectory following:
      // root at weight 0 and set by hand; plus a bone at half weight
      MeshPtr walking = _scene->CreateMesh(desc);
      const std::string anim = skel->Animation(0)->Name();
      const std::string root = skel->RootNode()->Name();
      walking->SetSkeletonAnimationEnabled(anim, true, true, 1.0);
      walking->SetSkeletonWeights({{root, 0.0f},
          {skel->NodeByHandle(skel->NodeCount() / 2)->Name(), 0.5f}});
      walking->UpdateSkeletonAnimation(std::chrono::milliseconds(730));
      math::Matrix4d rootTf = skel->RootNode()->Transform();
      rootTf.SetTranslation(rootTf.Translation() + math::Vector3d(0, 0.1, 0));
      walking->SetSkeletonLocalTransforms({{root, rootTf}});
      // feet on the floor, both actors fully inside the depthA / bbox view
      const double z = -desc.mesh->Min().Z();
      Add(_scene, walking, {1.8, 0.1, z, 0, 0, GZ_PI / 2}, {1, 1, 1});
      // explicit local bone transforms, as in MeshTest.MeshSkeleton
      MeshPtr posed = _scene->CreateMesh(desc);
      std::map<std::string, math::Matrix4d> tfs;
      for (const auto &[name, tf] : skel->Animation(0)->PoseAt(1.1, true))
      {
        tfs[skel->NodeNameAnimToSkin(0, name)] =
            skel->AlignTranslation(0, name) * tf *
            skel->AlignRotation(0, name);
      }
      posed->SetSkeletonLocalTransforms(tfs);
      Add(_scene, posed, {2.1, -0.5, z, 0, 0, 0.5}, {1, 1, 1})
          ->SetUserData("label", 6);
    }
  }

  void WriteBytes(const std::string &_dir, const std::string &_name,
      const uint8_t *_data, unsigned int _w, unsigned int _h,
      std::ofstream &_index)
  {
    std::vector<float> f(_data, _data + static_cast<size_t>(_w) * _h * 3);
    Write(_dir, _name, f.data(), _w, _h, 3, _index);
  }



  /// \brief Median and p90 wall time of _frames calls, in ms
  std::pair<double, double> Time(const std::function<void()> &_f, int _frames)
  {
    for (int i = 0; i < 5; ++i)
      _f();
    std::vector<double> t;
    for (int i = 0; i < _frames; ++i)
    {
      auto t0 = std::chrono::steady_clock::now();
      _f();
      t.push_back(std::chrono::duration<double, std::milli>(
          std::chrono::steady_clock::now() - t0).count());
    }
    std::sort(t.begin(), t.end());
    return {t[t.size() / 2], t[t.size() * 9 / 10]};
  }

  /// \brief sRGB byte to linear [0, 1]
  double Linear(uint8_t _v)
  {
    const double x = _v / 255.0;
    return x <= 0.04045 ? x / 12.92 : std::pow((x + 0.055) / 1.055, 2.4);
  }

  /// \brief Wide-angle camera geometry, comparable across engines whose
  /// shading differs: for every lens, an RGB image of the main scene
  /// (wa_<lens>_rgb), then in an empty scene the cutoff circle radius along
  /// the center row on a white background (wa_cutoff.txt), the centroid and
  /// area of every sphere rendered alone on black next to Project3d of its
  /// center (wa_geom.txt), and an L8 image of the first lens (wa_l8).
  void WideAngle(RenderEngine *_engine, ScenePtr _scene,
      const std::string &_dir, std::ofstream &_index,
      std::ofstream &_timing, int _frames)
  {
    struct Cfg
    {
      std::string name; MappingFunctionType type; double hfov, cutoff;
      unsigned int w, h;
    };
    const std::vector<Cfg> cfgs = {
      // the integration test lens
      {"custom", MFT_CUSTOM, 2.6, GZ_PI, 320, 240},
      {"equidistant", MFT_EQUIDISTANT, 3.0, 1.4, 320, 320},
      // HFOV pi trips an assert in ogre2 (degenerate frustum)
      {"stereographic", MFT_STEREOGRAPHIC, 3.0, 1.3, 320, 320},
      {"equisolid", MFT_EQUISOLID_ANGLE, 2.8, 1.2, 480, 320},
      {"orthographic", MFT_ORTHOGRAPHIC, 2.4, 1.0, 320, 320},
    };
    auto make = [&](ScenePtr _s, const Cfg &_c, const std::string &_name)
    {
      WideAngleCameraPtr cam = _s->CreateWideAngleCamera(_name);
      CameraLens lens;
      if (_c.type == MFT_CUSTOM)
        lens.SetCustomMappingFunction(1.05, 4.0, AFT_TAN, 1.0, 0.0);
      lens.SetType(_c.type);
      lens.SetCutOffAngle(_c.cutoff);
      lens.SetScaleToHFOV(true);
      cam->SetLens(lens);
      cam->SetHFOV(_c.hfov);
      cam->SetImageWidth(_c.w);
      cam->SetImageHeight(_c.h);
      cam->SetAspectRatio(static_cast<double>(_c.w) / _c.h);
      _s->RootVisual()->AddChild(cam);
      return cam;
    };

    // lit main scene, for images worth looking at
    _scene->SetAmbientLight(0.3, 0.3, 0.3);
    _scene->SetBackgroundColor(0.35, 0.45, 0.6);
    DirectionalLightPtr sun = _scene->CreateDirectionalLight();
    sun->SetDirection(-0.5, 0.3, -1);
    sun->SetDiffuseColor(0.9, 0.9, 0.85);
    _scene->RootVisual()->AddChild(sun);
    for (const auto &c : cfgs)
    {
      WideAngleCameraPtr cam = make(_scene, c, "wa_" + c.name);
      cam->SetLocalPose({0, 0, 1, 0, 0.2, 0});
      Image img = cam->CreateImage();
      auto tm = Time([&] { cam->Capture(img); }, _frames);
      _timing << "wa_" << c.name << " " << tm.first << " " << tm.second
              << "\n";
      WriteBytes(_dir, "wa_" + c.name + "_rgb", img.Data<uint8_t>(), c.w,
          c.h, _index);
      if (&c == &cfgs[0])
      {
        WideAngleCameraPtr l8 = make(_scene, c, "wa_l8cam");
        l8->SetLocalPose({0, 0, 1, 0, 0.2, 0});
        l8->SetImageFormat(PF_L8);
        Image mono = l8->CreateImage();
        l8->Capture(mono);
        std::vector<float> f(mono.Data<uint8_t>(),
            mono.Data<uint8_t>() + c.w * c.h);
        Write(_dir, "wa_l8", f.data(), c.w, c.h, 1, _index);
      }
    }

    ScenePtr geom = _engine->CreateScene("wa_geom");
    geom->SetAmbientLight(1, 1, 1);
    std::vector<WideAngleCameraPtr> cams;
    for (const auto &c : cfgs)
    {
      cams.push_back(make(geom, c, "wa_geom_" + c.name));
      cams.back()->SetLocalPose({0, 0, 0, 0.1, 0.15, 0.3});
    }

    // cutoff circle: left / right crossing of half intensity, center row
    std::ofstream cut(_dir + "/wa_cutoff.txt");
    cut.precision(10);
    geom->SetBackgroundColor(1, 1, 1);
    for (size_t k = 0; k < cfgs.size(); ++k)
    {
      Image img = cams[k]->CreateImage();
      cams[k]->Capture(img);
      const unsigned int w = cfgs[k].w;
      const uint8_t *row = img.Data<uint8_t>() + (cfgs[k].h / 2) * w * 3;
      auto cross = [&](int _from, int _step)
      {
        for (int x = _from; x >= 0 && x + _step >= 0 &&
             x + _step < static_cast<int>(w); x += _step)
        {
          const double a = Linear(row[x * 3]);
          const double b = Linear(row[(x + _step) * 3]);
          if (a >= 0.5 && b < 0.5)
            return x + 0.5 + _step * (a - 0.5) / (a - b);
        }
        return -1.0;
      };
      const double left = cross(w / 2, -1);
      const double right = cross(w / 2 - 1, 1);
      cut << cfgs[k].name << " " << left << " " << right << " "
          << (left < 0 || right < 0 ? -1.0 : (right - left) / 2) << "\n";
    }

    // spheres one at a time on black: centroid of the pixels brighter than
    // half the brightest one, next to Project3d of the sphere center
    geom->SetBackgroundColor(0, 0, 0);
    const std::vector<math::Vector3d> points = {
      {3, 0, 0}, {2, 2, 0.5}, {1, -2, 1}, {0.5, 1, -1.5}, {2, 0, 2},
      {-0.5, 2.5, 0.3}, {1, -1.2, -1.2}};
    std::ofstream out(_dir + "/wa_geom.txt");
    out.precision(10);
    MaterialPtr white = geom->CreateMaterial();
    white->SetAmbient(1, 1, 1);
    white->SetDiffuse(1, 1, 1);
    // flat color, so the masks do not depend on the shading model
    white->SetLightingEnabled(false);
    for (size_t p = 0; p < points.size(); ++p)
    {
      VisualPtr v = geom->CreateVisual();
      v->AddGeometry(geom->CreateSphere());
      v->SetLocalPosition(points[p]);
      v->SetLocalScale(0.6);
      v->SetMaterial(white);
      geom->RootVisual()->AddChild(v);
      for (size_t k = 0; k < cfgs.size(); ++k)
      {
        Image img = cams[k]->CreateImage();
        cams[k]->Capture(img);
        const uint8_t *d = img.Data<uint8_t>();
        const size_t n = static_cast<size_t>(cfgs[k].w) * cfgs[k].h;
        int maxSum = 0;
        for (size_t i = 0; i < n; ++i)
          maxSum = std::max(maxSum, d[i * 3] + d[i * 3 + 1] + d[i * 3 + 2]);
        double sx = 0, sy = 0, area = 0;
        for (size_t i = 0; i < n && maxSum > 0; ++i)
        {
          if (2 * (d[i * 3] + d[i * 3 + 1] + d[i * 3 + 2]) > maxSum)
          {
            sx += i % cfgs[k].w + 0.5;
            sy += i / cfgs[k].w + 0.5;
            area += 1;
          }
        }
        const math::Vector3d proj = cams[k]->Project3d(points[p]);
        out << cfgs[k].name << " " << p << " "
            << (area > 0 ? sx / area : -1) << " "
            << (area > 0 ? sy / area : -1) << " " << area << " "
            << proj.X() << " " << proj.Y() << " " << proj.Z() << "\n";
      }
      geom->DestroyVisual(v);
    }
  }

  /// \brief PARITY_LENSFLARE=1: lens_flare.png pictures for eyeballing, not
  /// compared: lf_ref (no pass), lf_open (sun in view), lf_partial (the sun
  /// half behind a box), lf_wide (wide-angle camera, after stitching)
  void LensFlare(RenderEngine *_engine, const std::string &_dir)
  {
    ScenePtr scene = _engine->CreateScene("lensflare");
    scene->SetAmbientLight(0.3, 0.3, 0.3);
    scene->SetBackgroundColor(0.4, 0.55, 0.8);
    VisualPtr root = scene->RootVisual();
    DirectionalLightPtr sun = scene->CreateDirectionalLight();
    sun->SetDirection(-1.0, -0.25, -0.25);
    sun->SetDiffuseColor(0.8, 0.8, 0.8);
    root->AddChild(sun);
    MaterialPtr green = scene->CreateMaterial();
    green->SetDiffuse(0.1, 0.6, 0.1);
    VisualPtr ground = scene->CreateVisual();
    ground->AddGeometry(scene->CreatePlane());
    ground->SetLocalScale(40, 40, 1);
    ground->SetMaterial(green);
    root->AddChild(ground);
    MaterialPtr red = scene->CreateMaterial();
    red->SetDiffuse(0.7, 0.1, 0.1);
    VisualPtr box = scene->CreateVisual();
    box->AddGeometry(scene->CreateBox());
    box->SetLocalPosition(4.0, 0.48, 2.0);
    box->SetMaterial(red);
    root->AddChild(box);

    auto shot = [&](CameraPtr _cam, const std::string &_name)
    {
      Image img = _cam->CreateImage();
      _cam->Capture(img);
      common::Image png;
      png.SetFromData(img.Data<unsigned char>(), img.Width(), img.Height(),
          common::Image::RGB_INT8);
      png.SavePNG(_dir + "/" + _name + ".png");
    };
    auto pass = [&](bool _afterStitching)
    {
      LensFlarePassPtr p = std::dynamic_pointer_cast<LensFlarePass>(
          _engine->RenderPassSystem()->Create<LensFlarePass>());
      p->Init(scene);
      p->SetLight(sun);
      p->SetWideAngleCameraAfterStitching(_afterStitching);
      return p;
    };

    CameraPtr cam = scene->CreateCamera();
    cam->SetImageWidth(320);
    cam->SetImageHeight(240);
    cam->SetLocalPosition(0, 0, 1);
    root->AddChild(cam);
    box->SetVisible(false);
    shot(cam, "lf_ref");
    cam->AddRenderPass(pass(true));
    shot(cam, "lf_open");
    box->SetVisible(true);
    shot(cam, "lf_partial");

    CameraPtr wide = scene->CreateWideAngleCamera();
    wide->SetImageWidth(320);
    wide->SetImageHeight(240);
    wide->SetLocalPosition(0, 0, 1);
    root->AddChild(wide);
    box->SetVisible(false);
    wide->AddRenderPass(pass(true));
    shot(wide, "lf_wide");
    _engine->DestroyScene(scene);
  }

  /// \brief PARITY_MARKERS=1: three points of size 5 and one of size 1 at 3
  /// m in front of every sensor, printing how many pixels each sees
  void Markers(RenderEngine *_engine)
  {
    ScenePtr scene = _engine->CreateScene("markers");
    scene->SetAmbientLight(0.3, 0.3, 0.3);
    scene->SetBackgroundColor(0, 0, 0);
    VisualPtr root = scene->RootVisual();
    for (double size : {5.0, 1.0})
    {
      MarkerPtr marker = scene->CreateMarker();
      marker->SetType(MT_POINTS);
      marker->SetSize(size);
      const double z = size > 1.0 ? 0.0 : 0.5;
      for (double y : {-0.5, 0.0, 0.5})
        marker->AddPoint(math::Vector3d(3.0, y, z), math::Color::Green);
      VisualPtr v = scene->CreateVisual();
      v->AddGeometry(marker);
      v->SetUserData("label", 3);
      v->SetUserData("temperature", 400.0f);
      root->AddChild(v);
    }
    const math::Pose3d pose(0, 0, 0, 0, 0, 0);

    DepthCameraPtr depth = scene->CreateDepthCamera("depth");
    depth->SetLocalPose(pose);
    depth->SetImageWidth(320);
    depth->SetImageHeight(240);
    depth->SetAspectRatio(320.0 / 240.0);
    depth->SetHFOV(GZ_PI / 2);
    depth->SetNearClipPlane(0.1);
    depth->SetFarClipPlane(10);
    depth->CreateDepthTexture();
    root->AddChild(depth);
    std::vector<float> d(320 * 240);
    auto dc = depth->ConnectNewDepthFrame(
        [&](const float *_d, unsigned int, unsigned int, unsigned int,
            const std::string &) {
          std::memcpy(d.data(), _d, d.size() * sizeof(float)); });
    depth->Update();
    size_t n = 0;
    float lo = 1e9f, hi = 0;
    for (float v : d)
    {
      if (std::isfinite(v))
      {
        ++n;
        lo = std::min(lo, v);
        hi = std::max(hi, v);
      }
    }
    std::cout << "markers depth: " << n << " finite pixels, range " << lo
              << " .. " << hi << std::endl;
    dc.reset();

    CameraPtr rgb = scene->CreateCamera("rgb");
    rgb->SetLocalPose(pose);
    rgb->SetImageWidth(320);
    rgb->SetImageHeight(240);
    rgb->SetHFOV(GZ_PI / 2);
    root->AddChild(rgb);
    Image img = rgb->CreateImage();
    rgb->Capture(img);
    n = 0;
    const unsigned char *px = img.Data<unsigned char>();
    for (size_t i = 0; i < 320u * 240u; ++i)
      n += (px[3 * i] || px[3 * i + 1] || px[3 * i + 2]) ? 1 : 0;
    std::cout << "markers rgb: " << n << " lit pixels" << std::endl;

    SegmentationCameraPtr seg = scene->CreateSegmentationCamera("seg");
    seg->SetLocalPose(pose);
    seg->SetImageWidth(320);
    seg->SetImageHeight(240);
    seg->SetAspectRatio(320.0 / 240.0);
    seg->SetHFOV(GZ_PI / 2);
    seg->SetBackgroundLabel(23);
    root->AddChild(seg);
    std::vector<uint8_t> labels(320 * 240 * 3);
    auto sc = seg->ConnectNewSegmentationFrame(
        [&](const uint8_t *_d, unsigned int, unsigned int, unsigned int,
            const std::string &) {
          std::memcpy(labels.data(), _d, labels.size()); });
    seg->SetSegmentationType(SegmentationType::ST_SEMANTIC);
    seg->EnableColoredMap(false);
    seg->Update();
    n = 0;
    for (size_t i = 0; i < 320u * 240u; ++i)
      n += (labels[3 * i] | (labels[3 * i + 1] << 8)) == 3 ? 1 : 0;
    std::cout << "markers segmentation: " << n << " label-3 pixels";
    std::map<unsigned int, unsigned int> values;
    for (size_t i = 0; i < 320u * 240u; ++i)
      ++values[labels[3 * i] | (labels[3 * i + 1] << 8)];
    for (const auto &v : values)
      std::cout << ", label " << v.first << ": " << v.second << " px";
    std::cout << std::endl;
    sc.reset();

    ThermalCameraPtr th = scene->CreateThermalCamera("thermal");
    th->SetLocalPose(pose);
    th->SetImageWidth(320);
    th->SetImageHeight(240);
    th->SetAspectRatio(320.0 / 240.0);
    th->SetHFOV(GZ_PI / 2);
    th->SetNearClipPlane(0.5);
    th->SetFarClipPlane(10);
    th->SetImageFormat(PF_L16);
    th->SetAmbientTemperature(296.0f);
    th->SetAmbientTemperatureRange(0.0f);
    th->SetHeatSourceTemperatureRange(2.0f);
    th->SetLinearResolution(0.01f);
    th->SetMinTemperature(250.0f);
    th->SetMaxTemperature(450.0f);
    root->AddChild(th);
    std::vector<uint16_t> t(320 * 240);
    auto tc = th->ConnectNewThermalFrame(
        [&](const uint16_t *_d, unsigned int, unsigned int, unsigned int,
            const std::string &) {
          std::memcpy(t.data(), _d, t.size() * 2); });
    th->Update();
    n = 0;
    uint16_t base = t[0];
    for (uint16_t v : t)
      n += v > base + 100 ? 1 : 0;
    std::cout << "markers thermal: " << n << " hot pixels (ambient " << base
              << ")" << std::endl;
    tc.reset();

    BoundingBoxCameraPtr bb = scene->CreateBoundingBoxCamera("bbox");
    bb->SetLocalPose(pose);
    bb->SetImageWidth(320);
    bb->SetImageHeight(240);
    bb->SetAspectRatio(320.0 / 240.0);
    bb->SetHFOV(GZ_PI / 2);
    bb->SetBoundingBoxType(BoundingBoxType::BBT_VISIBLEBOX2D);
    root->AddChild(bb);
    std::vector<BoundingBox> boxes;
    auto bc = bb->ConnectNewBoundingBoxes(
        [&](const std::vector<BoundingBox> &_b) { boxes = _b; });
    bb->Update();
    std::cout << "markers bbox: " << boxes.size() << " boxes" << std::endl;
    bc.reset();
    // sensors stay alive: ogre2 asserts in DestroySensor on depth cameras
  }

  /// \brief PARITY_PROJECTOR=<png>: projector.png, the picture projected onto
  /// a floor, seen by a camera above with the same orientation
  void ProjectTexture(RenderEngine *_engine, const std::string &_png,
      const std::string &_dir)
  {
    ScenePtr scene = _engine->CreateScene("projector");
    scene->SetBackgroundColor(0, 0, 0);
    scene->SetAmbientLight(1, 1, 1);
    VisualPtr root = scene->RootVisual();
    DirectionalLightPtr light = scene->CreateDirectionalLight();
    light->SetDirection(0, 0, -1);
    root->AddChild(light);
    MaterialPtr gray = scene->CreateMaterial();
    gray->SetDiffuse(0.5, 0.5, 0.5);
    gray->SetAmbient(0.5, 0.5, 0.5);
    VisualPtr floor = scene->CreateVisual();
    floor->AddGeometry(scene->CreateBox());
    floor->SetLocalScale(20, 20, 1);
    floor->SetLocalPosition(0, 0, -0.5);
    floor->SetMaterial(gray);
    root->AddChild(floor);

    ProjectorPtr projector = scene->CreateProjector();
    projector->SetNearClipPlane(0.5);
    projector->SetFarClipPlane(4.0);
    projector->SetHFOV(0.8);
    projector->SetTexture(_png);
    projector->SetWorldPosition(0, 0, 3);
    projector->SetWorldRotation(0, GZ_PI / 2.0, 0);
    root->AddChild(projector);

    CameraPtr cam = scene->CreateCamera();
    cam->SetImageWidth(256);
    cam->SetImageHeight(256);
    cam->SetWorldPosition(0, 0, 6);
    cam->SetWorldRotation(0, GZ_PI / 2.0, 0);
    root->AddChild(cam);
    Image img = cam->CreateImage();
    cam->Capture(img);
    cam->Capture(img);
    common::Image png;
    png.SetFromData(img.Data<unsigned char>(), img.Width(), img.Height(),
        common::Image::RGB_INT8);
    png.SavePNG(_dir + "/projector.png");
    _engine->DestroyScene(scene);
  }

  /// \brief PARITY_SPOT: spot.png (top down, 50 m across) and
  /// spot_oblique.png, a downward spot light over a grey floor, ambient 0.3
  void Spot(RenderEngine *_engine, const char *_args, const std::string &_dir)
  {
    double h = 10, range = 30, c = 0.02, l = 0.03, q = 0, inner = 0,
        outer = 2.5, falloff = 0.5, rough = 1, spec = 0;
    int shadows = 1;
    std::sscanf(_args, "%lf %lf %lf %lf %lf %lf %lf %lf %d %lf %lf", &h,
        &range, &c, &l, &q, &inner, &outer, &falloff, &shadows, &rough, &spec);
    ScenePtr scene = _engine->CreateScene("spot");
    scene->SetBackgroundColor(0, 0, 0);
    scene->SetAmbientLight(0.3, 0.3, 0.3);
    VisualPtr root = scene->RootVisual();
    MaterialPtr gray = scene->CreateMaterial();
    gray->SetDiffuse(0.36, 0.36, 0.36);
    gray->SetAmbient(0.36, 0.36, 0.36);
    gray->SetRoughness(static_cast<float>(rough));
    gray->SetMetalness(0);
    gray->SetSpecular(spec, spec, spec);
    VisualPtr floor = scene->CreateVisual();
    // PARITY_SPOT_MESH=<mesh>: that mesh (own materials, unscaled) replaces
    // the floor, e.g. a Fuel warehouse
    if (const char *mesh = std::getenv("PARITY_SPOT_MESH"))
    {
      common::MeshManager::Instance()->Load(mesh);
      floor->AddGeometry(scene->CreateMesh(mesh));
    }
    else
    {
      floor->AddGeometry(scene->CreatePlane());
      floor->SetLocalScale(200, 200, 1);
      floor->SetMaterial(gray);
    }
    root->AddChild(floor);
    SpotLightPtr spot = scene->CreateSpotLight();
    // PARITY_SPOT_XY="x y": the spot light's horizontal position
    double sx = 0, sy = 0;
    if (const char *xy = std::getenv("PARITY_SPOT_XY"))
      std::sscanf(xy, "%lf %lf", &sx, &sy);
    spot->SetLocalPosition(sx, sy, h);
    spot->SetDirection(0, 0, -1);
    spot->SetDiffuseColor(1, 1, 1);
    spot->SetSpecularColor(1, 1, 1);
    spot->SetAttenuationRange(range);
    spot->SetAttenuationConstant(c);
    spot->SetAttenuationLinear(l);
    spot->SetAttenuationQuadratic(q);
    spot->SetInnerAngle(inner);
    spot->SetOuterAngle(outer);
    spot->SetFalloff(falloff);
    spot->SetCastShadows(shadows != 0);
    root->AddChild(spot);
    auto shoot = [&](const math::Pose3d &_pose, const std::string &_name)
    {
      CameraPtr cam = scene->CreateCamera();
      cam->SetImageWidth(512);
      cam->SetImageHeight(512);
      cam->SetAspectRatio(1.0);
      cam->SetHFOV(GZ_PI / 2);
      cam->SetFarClipPlane(1000);
      cam->SetLocalPose(_pose);
      root->AddChild(cam);
      Image img = cam->CreateImage();
      for (int i = 0; i < 3; ++i)
        cam->Capture(img);
      common::Image png;
      png.SetFromData(img.Data<unsigned char>(), img.Width(), img.Height(),
          common::Image::RGB_INT8);
      png.SavePNG(_dir + "/" + _name);
    };
    // PARITY_SPOT_CAM="ex ey ez tx ty tz": spot_cam.png from that eye
    if (const char *c = std::getenv("PARITY_SPOT_CAM"))
    {
      math::Vector3d e, t;
      std::stringstream(c) >> e.X() >> e.Y() >> e.Z() >> t.X() >> t.Y() >>
          t.Z();
      shoot(math::Matrix4d::LookAt(e, t).Pose(), "spot_cam.png");
    }
    shoot(math::Pose3d(0, 0, 25, 0, GZ_PI / 2, 0), "spot.png");
    shoot(math::Pose3d(-20, 0, 8, 0, 0.35, 0), "spot_oblique.png");
    _engine->DestroyScene(scene);
  }

  /// \brief PARITY_GUI=1: gui.png, what the gz-sim GUI draws over a scene:
  /// a translation gizmo, a grid, a spot light visual, a lit box at 50%
  /// transparency and a wireframe sphere
  void GuiOverlays(RenderEngine *_engine, const std::string &_dir)
  {
    ScenePtr scene = _engine->CreateScene("gui");
    scene->SetBackgroundColor(0.8, 0.8, 0.8);
    scene->SetAmbientLight(0.4, 0.4, 0.4);
    VisualPtr root = scene->RootVisual();
    DirectionalLightPtr sun = scene->CreateDirectionalLight();
    sun->SetDirection(-0.5, 0.5, -1);
    sun->SetDiffuseColor(0.8, 0.8, 0.8);
    root->AddChild(sun);

    VisualPtr box = scene->CreateVisual();
    box->AddGeometry(scene->CreateBox());
    MaterialPtr red = scene->CreateMaterial();
    red->SetDiffuse(0.8, 0, 0);
    red->SetAmbient(0.8, 0, 0);
    box->SetMaterial(red);
    root->AddChild(box);

    // GzSceneManager's transparent view: lit material, transparency 0.5
    VisualPtr glass = scene->CreateVisual();
    glass->AddGeometry(scene->CreateBox());
    MaterialPtr glassMat = scene->CreateMaterial();
    glassMat->SetDiffuse(0, 0, 0.8);
    glassMat->SetAmbient(0, 0, 0.8);
    glassMat->SetTransparency(0.5);
    glass->SetMaterial(glassMat);
    glass->SetLocalPosition(0, 2, 0);
    root->AddChild(glass);

    VisualPtr wire = scene->CreateVisual();
    wire->AddGeometry(scene->CreateSphere());
    wire->SetMaterial(red);
    wire->SetLocalPosition(0, -2, 0);
    wire->SetWireframe(true);
    root->AddChild(wire);

    // gz-gui GridConfig: 20 cells of 1 m, Default/TransGray-like material
    GridPtr grid = scene->CreateGrid();
    grid->SetCellCount(20);
    grid->SetCellLength(1);
    grid->SetVerticalCellCount(0);
    VisualPtr gridVis = scene->CreateVisual();
    gridVis->AddGeometry(grid);
    MaterialPtr gridMat = scene->CreateMaterial();
    gridMat->SetAmbient(0.7, 0.7, 0.7);
    gridMat->SetDiffuse(0.7, 0.7, 0.7);
    gridMat->SetSpecular(0.7, 0.7, 0.7);
    gridVis->SetMaterial(gridMat);
    gridVis->SetLocalPosition(0, 0, -0.5);
    root->AddChild(gridVis);

    // shapes.sdf ground_plane, 1.5 cm under the grid as in gz-sim RenderUtil
    VisualPtr ground = scene->CreateVisual();
    ground->AddGeometry(scene->CreatePlane());
    MaterialPtr groundMat = scene->CreateMaterial();
    groundMat->SetAmbient(0.8, 0.8, 0.8);
    groundMat->SetDiffuse(0.8, 0.8, 0.8);
    ground->SetMaterial(groundMat);
    ground->SetLocalScale(100, 100, 1);
    ground->SetLocalPosition(0, 0, -0.515);
    root->AddChild(ground);

    LightVisualPtr lightVis = scene->CreateLightVisual();
    lightVis->SetType(LightVisualType::LVT_SPOT);
    lightVis->SetLocalPosition(2, -1, 1.5);
    root->AddChild(lightVis);

    // gz-sim SceneManager: a spawned spot light, its visual a child of it
    SpotLightPtr spot = scene->CreateSpotLight();
    spot->SetLocalPosition(2, 2, 1.5);
    spot->SetDirection(0, 0, -1);
    spot->SetInnerAngle(0.1);
    spot->SetOuterAngle(0.5);
    root->AddChild(spot);
    LightVisualPtr spotVis = scene->CreateLightVisual();
    spotVis->SetType(LightVisualType::LVT_SPOT);
    spotVis->SetInnerAngle(0.1);
    spotVis->SetOuterAngle(0.5);
    spotVis->SetVisible(true);
    spotVis->RemoveParent();
    spot->AddChild(spotVis);

    GizmoVisualPtr gizmo = scene->CreateGizmoVisual();
    gizmo->SetTransformMode(TransformMode::TM_TRANSLATION);
    gizmo->SetLocalPosition(0, 0, 0.5);
    root->AddChild(gizmo);

    CameraPtr cam = scene->CreateCamera();
    cam->SetImageWidth(640);
    cam->SetImageHeight(480);
    cam->SetAspectRatio(640.0 / 480.0);
    cam->SetHFOV(GZ_PI / 3);
    cam->SetLocalPosition(6, 0, 3);
    cam->SetLocalRotation(0, 0.4, GZ_PI);
    root->AddChild(cam);
    Image img = cam->CreateImage();
    cam->Capture(img);
    cam->Capture(img);
    common::Image png;
    png.SetFromData(img.Data<unsigned char>(), img.Width(), img.Height(),
        common::Image::RGB_INT8);
    png.SavePNG(_dir + "/gui.png");
    _engine->DestroyScene(scene);
  }

  /// \brief PARITY_SHADER=1: shader.png, the custom_shaders_uniforms example
  /// box: its GLSL 330 shaders on ogre2, its .mat port on filament
  void CustomShader(RenderEngine *_engine, const std::string &_engineName,
      const std::string &_dir)
  {
    ScenePtr scene = _engine->CreateScene("shader");
    scene->SetBackgroundColor(0, 0, 0);
    scene->SetAmbientLight(0.3, 0.3, 0.3);
    VisualPtr root = scene->RootVisual();
    const std::string media = std::string(PROJECT_SOURCE_PATH) +
        "/examples/custom_shaders_uniforms/media/";
    MaterialPtr shader = scene->CreateMaterial();
    if (_engineName == "filament")
    {
      shader->SetVertexShader(media + "custom_shader.mat");
      shader->SetFragmentShader(media + "custom_shader.mat");
    }
    else
    {
      shader->SetVertexShader(media + "vertex_shader_330.glsl");
      shader->SetFragmentShader(media + "fragment_shader_330.glsl");
    }
    VisualPtr box = scene->CreateVisual();
    box->AddGeometry(scene->CreateBox());
    box->SetLocalPosition(0, 1, 0);
    box->SetLocalScale(2, 2, 2);
    box->SetMaterial(shader);
    // the example's uniforms, u_seed advanced a few frames
    int seed[1] = {3};
    float resolution[2] = {400, 200};
    float color[3] = {1.0f, 0.8f, 0.4f};
    // noise scale 0.02 instead of the example's 0.0005: visible grain
    float adjustments[16] = {0, 0, 0, 0.02f, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0};
    // the visual holds a clone: set the params on it
    ShaderParams &p = *box->Material()->FragmentShaderParams();
    p["u_seed"].InitializeBuffer(1);
    p["u_seed"].UpdateBuffer(seed);
    p["u_resolution"].InitializeBuffer(2);
    p["u_resolution"].UpdateBuffer(resolution);
    p["u_color"].InitializeBuffer(3);
    p["u_color"].UpdateBuffer(color);
    p["u_adjustments"].InitializeBuffer(16);
    p["u_adjustments"].UpdateBuffer(adjustments);
    // ogre2 auto-binds this one once named; filament has getClipFromWorld...
    if (_engineName == "ogre2")
      (*box->Material()->VertexShaderParams())["worldviewproj_matrix"] = 1;

    root->AddChild(box);

    CameraPtr cam = scene->CreateCamera();
    cam->SetImageWidth(400);
    cam->SetImageHeight(300);
    cam->SetAspectRatio(400.0 / 300.0);
    cam->SetHFOV(GZ_PI / 2);
    // the example's first camera pose, facing the box
    cam->SetLocalPosition(3, 3, 0);
    cam->SetLocalRotation(0, 0, std::atan2(-2.0, -3.0));
    root->AddChild(cam);
    Image img = cam->CreateImage();
    cam->Capture(img);
    cam->Capture(img);
    common::Image png;
    png.SetFromData(img.Data<unsigned char>(), img.Width(), img.Height(),
        common::Image::RGB_INT8);
    png.SavePNG(_dir + "/shader.png");

    // params updated between frames, like the example's key handlers
    seed[0] = 7;
    color[0] = 0.4f;
    color[2] = 1.0f;
    p["u_seed"].UpdateBuffer(seed);
    p["u_color"].UpdateBuffer(color);
    cam->Capture(img);
    png.SetFromData(img.Data<unsigned char>(), img.Width(), img.Height(),
        common::Image::RGB_INT8);
    png.SavePNG(_dir + "/shader_updated.png");
    _engine->DestroyScene(scene);
  }

  /// \brief PARITY_SHADER=waves: waves.png, test/integration/waves.cc's
  /// Gerstner waves at t = 5 s: GLSL 330 on ogre2, GerstnerWaves.mat on
  /// filament
  void Waves(RenderEngine *_engine, const std::string &_engineName,
      const std::string &_dir)
  {
    ScenePtr scene = _engine->CreateScene("waves");
    scene->SetAmbientLight(0.3, 0.3, 0.3);
    scene->SetBackgroundColor(1.0, 0.0, 0.0);
    VisualPtr root = scene->RootVisual();
    const std::string media = std::string(PROJECT_SOURCE_PATH) +
        "/test/media/";
    const std::string programs = media + "materials/programs/";
    MaterialPtr shader = scene->CreateMaterial();
    const bool fil = _engineName == "filament";
    shader->SetVertexShader(programs + (fil ? "GerstnerWaves.mat" :
        "GerstnerWaves_vs_330.glsl"));
    shader->SetFragmentShader(programs + (fil ? "GerstnerWaves.mat" :
        "GerstnerWaves_fs_330.glsl"));

    VisualPtr waves = scene->CreateVisual();
    MeshDescriptor descriptor;
    descriptor.meshName = media + "meshes/mesh.dae";
    descriptor.mesh = common::MeshManager::Instance()->Load(
        descriptor.meshName);
    waves->AddGeometry(scene->CreateMesh(descriptor));
    waves->SetLocalPosition(3, 0, 0);
    waves->SetMaterial(shader, false);
    root->AddChild(waves);

    ShaderParams &vs = *shader->VertexShaderParams();
    if (!fil)
    {
      vs["worldviewproj_matrix"] = 1;
      vs["camera_position_object_space"] = 1;
    }
    auto buffer = [](ShaderParams &_p, const std::string &_name,
        std::vector<float> _v)
    {
      _p[_name].InitializeBuffer(static_cast<uint32_t>(_v.size()));
      _p[_name].UpdateBuffer(_v.data());
    };
    vs["Nwaves"] = 3;
    vs["rescale"] = 0.5f;
    buffer(vs, "bumpScale", {25.0f, 25.0f});
    buffer(vs, "bumpSpeed", {0.01f, 0.01f});
    buffer(vs, "amplitude", {1.8f, 1.2f, 0.9f});
    buffer(vs, "wavenumber", {0.028f, 0.0896f, 0.0504f});
    buffer(vs, "omega", {0.5f, 1.7f, 1.0f});
    buffer(vs, "dir0", {-1.0f, 0.0f});
    buffer(vs, "dir1", {-0.7f, 0.7f});
    buffer(vs, "dir2", {0.7f, 0.7f});
    buffer(vs, "steepness", {1.0f, 1.5f, 0.8f});
    vs["tau"] = 2.0f;
    vs["t"] = 5.0f;
    ShaderParams &fs = *shader->FragmentShaderParams();
    fs["hdrMultiplier"] = 0.4f;
    fs["fresnelPower"] = 5.0f;
    buffer(fs, "shallowColor", {0.0f, 0.1f, 0.3f, 1.0f});
    buffer(fs, "deepColor", {0.0f, 0.05f, 0.2f, 1.0f});
    fs["bumpMap"].SetTexture(media + "materials/textures/wave_normals.dds");
    fs["cubeMap"].SetTexture(media + "materials/textures/skybox_lowres.dds",
        ShaderParam::ParamType::PARAM_TEXTURE_CUBE, 1u);

    CameraPtr cam = scene->CreateCamera();
    cam->SetLocalPosition(0, 0.0, 3.5);
    cam->SetImageWidth(640);
    cam->SetImageHeight(480);
    root->AddChild(cam);
    Image img = cam->CreateImage();
    cam->Capture(img);
    cam->Capture(img);
    common::Image png;
    png.SetFromData(img.Data<unsigned char>(), img.Width(), img.Height(),
        common::Image::RGB_INT8);
    png.SavePNG(_dir + "/waves.png");
    _engine->DestroyScene(scene);
  }

  /// \brief PARITY_NOISE=1: Gaussian noise render pass outputs, compared as
  /// distributions by parity_compare.py. Per case a clean image from a twin
  /// camera without the pass, then kNoiseFrames noisy frames:
  /// noise_rgb_<cfg>: background-only scene (flat colors, clamping at 0 and
  /// 1); noise_depth / noise_cloud: depthA in the main scene;
  /// noise_thermal: thermal camera; noise_wa_face / noise_wa_final: wide
  /// angle camera with the pass on the cube faces / after stitching.
  void Noise(RenderEngine *_engine, ScenePtr _scene, const std::string &_dir,
      std::ofstream &_index)
  {
    const int kNoiseFrames = 8;
    std::ofstream cfg(_dir + "/noise_cfg.txt");
    auto pass = [&](const std::string &_name, double _mean, double _stddev,
        bool _afterStitching = false)
    {
      cfg << _name << " " << _mean << " " << _stddev << "\n";
      GaussianNoisePassPtr p = std::dynamic_pointer_cast<GaussianNoisePass>(
          _engine->RenderPassSystem()->Create<GaussianNoisePass>());
      p->SetMean(_mean);
      p->SetStdDev(_stddev);
      p->SetWideAngleCameraAfterStitching(_afterStitching);
      return p;
    };
    auto rgb = [&](const std::string &_name, CameraPtr _cam)
    {
      Image img = _cam->CreateImage();
      _cam->Capture(img);
      WriteBytes(_dir, _name, img.Data<uint8_t>(), img.Width(),
          img.Height(), _index);
    };

    // RGB: flat background colors, linear (0.5, 0, 1) and (0.2, 0.6, 0.9)
    ScenePtr flat = _engine->CreateScene("noise_flat");
    struct RgbCfg { std::string name; math::Color bg; double mean, stddev; };
    const std::vector<RgbCfg> rgbCfgs = {
      {"wide", {0.5f, 0.0f, 1.0f}, 0.0, 0.3},
      {"biased", {0.2f, 0.6f, 0.9f}, 0.1, 0.05},
    };
    for (const auto &c : rgbCfgs)
    {
      flat->SetBackgroundColor(c.bg);
      CameraPtr cams[2];
      for (int k = 0; k < 2; ++k)
      {
        cams[k] = flat->CreateCamera("noise_rgb_" + c.name + std::to_string(k));
        cams[k]->SetImageWidth(320);
        cams[k]->SetImageHeight(240);
        cams[k]->SetAspectRatio(320.0 / 240.0);
        flat->RootVisual()->AddChild(cams[k]);
      }
      cams[1]->AddRenderPass(pass("rgb_" + c.name, c.mean, c.stddev));
      rgb("noise_rgb_" + c.name + "_clean", cams[0]);
      for (int f = 0; f < kNoiseFrames; ++f)
        rgb("noise_rgb_" + c.name + "_f" + std::to_string(f), cams[1]);
    }

    // a zero noise pass must not change a structured image (orientation,
    // sRGB round trip): the lit main scene
    {
      _scene->SetAmbientLight(0.3, 0.3, 0.3);
      DirectionalLightPtr sun = _scene->CreateDirectionalLight();
      sun->SetDirection(-0.5, -0.5, -1);
      sun->SetDiffuseColor(0.9, 0.9, 0.9);
      _scene->RootVisual()->AddChild(sun);
      CameraPtr cams[2];
      for (int k = 0; k < 2; ++k)
      {
        cams[k] = _scene->CreateCamera("noise_zero" + std::to_string(k));
        cams[k]->SetLocalPose({-8, 8, 5, 0, 0.4, -GZ_PI / 4});
        cams[k]->SetImageWidth(320);
        cams[k]->SetImageHeight(240);
        cams[k]->SetAspectRatio(320.0 / 240.0);
        cams[k]->SetHFOV(GZ_PI / 3);
        _scene->RootVisual()->AddChild(cams[k]);
      }
      cams[1]->AddRenderPass(pass("rgb_zero", 0.0, 0.0));
      rgb("noise_rgb_zero_clean", cams[0]);
      rgb("noise_rgb_zero_f0", cams[1]);
    }

    // wide angle, background only: pass on the faces / on the final image
    {
      flat->SetBackgroundColor(0.5, 0.0, 1.0);
      WideAngleCameraPtr cams[3];
      for (int k = 0; k < 3; ++k)
      {
        cams[k] = flat->CreateWideAngleCamera("noise_wa" + std::to_string(k));
        CameraLens lens;
        lens.SetCustomMappingFunction(1.05, 4.0, AFT_TAN, 1.0, 0.0);
        lens.SetType(MFT_CUSTOM);
        lens.SetCutOffAngle(GZ_PI);
        lens.SetScaleToHFOV(true);
        cams[k]->SetLens(lens);
        cams[k]->SetHFOV(2.6);
        cams[k]->SetImageWidth(320);
        cams[k]->SetImageHeight(240);
        cams[k]->SetAspectRatio(320.0 / 240.0);
        flat->RootVisual()->AddChild(cams[k]);
      }
      cams[1]->AddRenderPass(pass("wa_face", 0.0, 0.3, false));
      cams[2]->AddRenderPass(pass("wa_final", 0.0, 0.3, true));
      rgb("noise_wa_clean", cams[0]);
      for (int f = 0; f < kNoiseFrames; ++f)
      {
        rgb("noise_wa_face_f" + std::to_string(f), cams[1]);
        rgb("noise_wa_final_f" + std::to_string(f), cams[2]);
      }
    }

    // depth camera (depthA of the default run) in the main scene; besides
    // a realistic noise, strong ones pushing points past near / far
    {
      const unsigned int w = 320, h = 240;
      struct DepthNoiseCfg { std::string name; double mean, stddev; };
      const std::vector<DepthNoiseCfg> cfgs = {{"", 0.0, 0.0},
          {"depth", 0.1, 0.05}, {"depth_near", -1.0, 0.5},
          {"depth_far", 1.0, 0.5}};
      std::vector<float> depth(w * h), cloud(w * h * 4);
      for (const auto &c : cfgs)
      {
        DepthCameraPtr cam = _scene->CreateDepthCamera("noise_" + c.name);
        cam->SetLocalPose({0, 0, 1, 0, 0.2, 0});
        cam->SetImageWidth(w);
        cam->SetImageHeight(h);
        cam->SetAspectRatio(static_cast<double>(w) / h);
        cam->SetHFOV(GZ_PI / 2);
        cam->SetNearClipPlane(0.1);
        cam->SetFarClipPlane(10);
        cam->CreateDepthTexture();
        _scene->RootVisual()->AddChild(cam);
        if (!c.name.empty())
          cam->AddRenderPass(pass(c.name, c.mean, c.stddev));
        auto dc = cam->ConnectNewDepthFrame(
            [&](const float *_d, unsigned int, unsigned int, unsigned int,
                const std::string &) {
              std::memcpy(depth.data(), _d, depth.size() * sizeof(float)); });
        auto pc = cam->ConnectNewRgbPointCloud(
            [&](const float *_d, unsigned int, unsigned int, unsigned int,
                const std::string &) {
              std::memcpy(cloud.data(), _d, cloud.size() * sizeof(float)); });
        for (int f = 0; f < (c.name.empty() ? 1 : kNoiseFrames); ++f)
        {
          cam->Update();
          const std::string s = c.name.empty() ? "depth_clean" :
              c.name + "_f" + std::to_string(f);
          Write(_dir, "noise_" + s, depth.data(), w, h, 1, _index);
          Write(_dir, "noise_cloud_" + s, cloud.data(), w, h, 4, _index);
        }
      }
    }

    // thermal camera (thermal16 of the default run)
    {
      ThermalCameraPtr cams[2];
      std::vector<uint16_t> img(320 * 240);
      for (int k = 0; k < 2; ++k)
      {
        ThermalCameraPtr th = _scene->CreateThermalCamera(
            "noise_thermal" + std::to_string(k));
        th->SetLocalPose({0, 0, 1, 0, 0.2, 0});
        th->SetImageWidth(320);
        th->SetImageHeight(240);
        th->SetAspectRatio(320.0 / 240.0);
        th->SetHFOV(GZ_PI / 2);
        th->SetNearClipPlane(0.5);
        th->SetFarClipPlane(10);
        th->SetImageFormat(PF_L16);
        th->SetAmbientTemperature(296.0f);
        th->SetAmbientTemperatureRange(4.0f);
        th->SetHeatSourceTemperatureRange(2.0f);
        th->SetLinearResolution(0.01f);
        th->SetMinTemperature(250.0f);
        th->SetMaxTemperature(400.0f);
        _scene->RootVisual()->AddChild(th);
        cams[k] = th;
      }
      cams[1]->AddRenderPass(pass("thermal", 0.1, 0.05));
      for (int k = 0; k < 2; ++k)
      {
        auto conn = cams[k]->ConnectNewThermalFrame(
            [&](const uint16_t *_d, unsigned int, unsigned int, unsigned int,
                const std::string &) {
              std::memcpy(img.data(), _d, img.size() * sizeof(uint16_t)); });
        for (int f = 0; f < (k == 0 ? 1 : 2); ++f)
        {
          cams[k]->Update();
          std::vector<float> fl(img.begin(), img.end());
          Write(_dir, "noise_thermal" +
              (k == 0 ? std::string("_clean") : "_f" + std::to_string(f)),
              fl.data(), 320, 240, 1, _index);
        }
      }
    }
  }

  struct DepthCfg
  {
    std::string name; math::Pose3d pose; unsigned int w, h;
    double hfov, nearClip, farClip;
  };

  struct RaysCfg
  {
    std::string name; math::Pose3d pose;
    unsigned int hCount, vCount;
    double hMin, hMax, vMin, vMax, nearClip, farClip; bool clamp;
  };

  /// \brief Particle statistics. A box emitter rises in front of the
  /// sensors; after 3 s of warm-up, each depth camera, lidar and thermal
  /// camera renders one frame with the emitter hidden (baseline), then
  /// kFrames frames 16 ms apart with it. A pixel / ray is affected when
  /// its finite / inf class or its value (by more than 1 cm, or 1 count
  /// for thermal) differs from the baseline. particles.txt gets, per
  /// sensor: affected fraction, mean and std of the finite affected values,
  /// mean of the baseline where both are finite. rgb_particles.png is for
  /// eyeballing.
  /// \brief PARITY_MULTI: a depth camera and two lidars updated together,
  /// per frame scene PreRender, then either Render + PostRender per sensor
  /// (gz-sensors today, multi_seq) or every Render, then every PostRender
  /// (render ahead, multi_ahead). Timing lines: wall median and p90 per
  /// frame, then process CPU ms per frame (all threads, median).
  void Multi(ScenePtr _scene, const std::string &_dir, std::ofstream &_index,
      std::ofstream &_timing, int _frames,
      const std::vector<DepthCfg> &_depthCfgs,
      const std::vector<RaysCfg> &_raysCfgs)
  {
    std::vector<CameraPtr> sensors;
    std::vector<std::vector<float>> data;
    std::vector<std::string> names;
    std::vector<common::ConnectionPtr> conns;
    data.reserve(3);
    for (const auto &c : _depthCfgs)
    {
      if (c.name != "depthB")
        continue;
      DepthCameraPtr cam = _scene->CreateDepthCamera("multi_" + c.name);
      cam->SetLocalPose(c.pose);
      cam->SetImageWidth(c.w);
      cam->SetImageHeight(c.h);
      cam->SetAspectRatio(static_cast<double>(c.w) / c.h);
      cam->SetHFOV(c.hfov);
      cam->SetNearClipPlane(c.nearClip);
      cam->SetFarClipPlane(c.farClip);
      cam->CreateDepthTexture();
      _scene->RootVisual()->AddChild(cam);
      data.emplace_back(c.w * c.h);
      std::vector<float> *d = &data.back();
      conns.push_back(cam->ConnectNewDepthFrame(
          [d](const float *_d, unsigned int, unsigned int, unsigned int,
              const std::string &) {
            std::memcpy(d->data(), _d, d->size() * sizeof(float)); }));
      sensors.push_back(cam);
      names.push_back("multi_" + c.name + "_depth");
    }
    for (const auto &c : _raysCfgs)
    {
      if (c.name != "os1_64" && c.name != "s2l2")
        continue;
      GpuRaysPtr rays = _scene->CreateGpuRays("multi_" + c.name);
      rays->SetLocalPose(c.pose);
      rays->SetNearClipPlane(c.nearClip);
      rays->SetFarClipPlane(c.farClip);
      rays->SetClamp(c.clamp);
      rays->SetAngleMin(c.hMin);
      rays->SetAngleMax(c.hMax);
      rays->SetRayCount(c.hCount);
      rays->SetVerticalRayCount(c.vCount);
      rays->SetVerticalAngleMin(c.vMin);
      rays->SetVerticalAngleMax(c.vMax);
      _scene->RootVisual()->AddChild(rays);
      data.emplace_back(c.hCount * c.vCount * rays->Channels());
      std::vector<float> *d = &data.back();
      conns.push_back(rays->ConnectNewGpuRaysFrame(
          [d](const float *_d, unsigned int, unsigned int, unsigned int,
              const std::string &) {
            std::memcpy(d->data(), _d, d->size() * sizeof(float)); }));
      sensors.push_back(rays);
      names.push_back("multi_" + c.name);
    }
    auto cpuMs = [] {
      timespec ts;
      clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts);
      return ts.tv_sec * 1e3 + ts.tv_nsec * 1e-6; };
    for (bool ahead : {false, true})
    {
      std::vector<double> cpu;
      auto frame = [&] {
        const double c0 = cpuMs();
        _scene->PreRender();
        for (auto &s : sensors)
        {
          s->Render();
          if (!ahead)
            s->PostRender();
        }
        if (ahead)
        {
          for (auto &s : sensors)
            s->PostRender();
        }
        if (!_scene->LegacyAutoGpuFlush())
          _scene->PostRender();
        cpu.push_back(cpuMs() - c0);
      };
      auto t = Time(frame, _frames);
      std::sort(cpu.begin(), cpu.end());
      _timing << (ahead ? "multi_ahead " : "multi_seq ") << t.first << " "
              << t.second << " " << cpu[cpu.size() / 2] << "\n";
    }
    for (size_t i = 0; i < sensors.size(); ++i)
      Write(_dir, names[i], data[i].data(), data[i].size(), 1, 1, _index);
  }

  void Particles(ScenePtr _scene, const std::string &_dir,
      const std::vector<DepthCfg> &_depthCfgs,
      const std::vector<RaysCfg> &_raysCfgs)
  {
    constexpr int kFrames = 30;
    ParticleEmitterPtr emitter = _scene->CreateParticleEmitter();
    emitter->SetType(EM_BOX);
    // ogre2 axes: width (X) along the emitter's Y, height (Y) along its Z,
    // depth (Z) along its X, the emission direction; pitched so that the
    // particles rise
    emitter->SetEmitterSize({0.6, 1.2, 0.4});
    emitter->SetLocalPose({1.2, 0, 0.3, 0, -1.57, 0});
    emitter->SetParticleSize({0.15, 0.15, 0.15});
    emitter->SetRate(80);
    emitter->SetLifetime(3);
    emitter->SetVelocityRange(0.2, 0.5);
    emitter->SetScaleRate(0.1);
    emitter->SetEmitting(true);
    _scene->RootVisual()->AddChild(emitter);

    auto step = [&] {
      _scene->SetTime(_scene->Time() + std::chrono::milliseconds(16)); };
    CameraPtr cam = _scene->CreateCamera("rgb_particles");
    cam->SetLocalPose({-1, -2.5, 1.2, 0, 0.1, 0.85});
    cam->SetImageWidth(640);
    cam->SetImageHeight(480);
    cam->SetAspectRatio(640.0 / 480.0);
    cam->SetHFOV(GZ_PI / 3);
    _scene->RootVisual()->AddChild(cam);
    Image img = cam->CreateImage();
    for (int i = 0; i < 190; ++i)
    {
      step();
      cam->Update();
    }
    cam->Capture(img);
    common::Image png;
    png.SetFromData(img.Data<unsigned char>(), 640, 480,
        common::Image::RGB_INT8);
    png.SavePNG(_dir + "/rgb_particles.png");

    std::ofstream out(_dir + "/particles.txt");
    // _update renders one frame into _data
    auto stats = [&](const std::string &_name, size_t _n, double _tol,
        const std::function<void(std::vector<float> &)> &_update)
    {
      std::vector<float> base(_n), cur(_n);
      emitter->SetVisible(false);
      _update(base);
      emitter->SetVisible(true);
      double affected = 0, n = 0, sum = 0, sum2 = 0, nBase = 0, sumBase = 0;
      for (int f = 0; f < kFrames; ++f)
      {
        step();
        _update(cur);
        for (size_t i = 0; i < _n; ++i)
        {
          const bool fa = std::isfinite(base[i]);
          const bool fb = std::isfinite(cur[i]);
          if (fa == fb && (!fa || std::abs(cur[i] - base[i]) <= _tol))
            continue;
          affected += 1;
          if (fb)
          {
            n += 1;
            sum += cur[i];
            sum2 += cur[i] * cur[i];
          }
          if (fa && fb)
          {
            nBase += 1;
            sumBase += base[i];
          }
        }
      }
      const double mean = n > 0 ? sum / n : 0.0;
      out << _name << " " << affected / (_n * kFrames) << " " << mean << " "
          << (n > 0 ? std::sqrt(std::max(0.0, sum2 / n - mean * mean)) : 0.0)
          << " " << (nBase > 0 ? sumBase / nBase : 0.0) << "\n";
    };

    for (const auto &c : _depthCfgs)
    {
      DepthCameraPtr dc = _scene->CreateDepthCamera(c.name);
      dc->SetLocalPose(c.pose);
      dc->SetImageWidth(c.w);
      dc->SetImageHeight(c.h);
      dc->SetAspectRatio(static_cast<double>(c.w) / c.h);
      dc->SetHFOV(c.hfov);
      dc->SetNearClipPlane(c.nearClip);
      dc->SetFarClipPlane(c.farClip);
      dc->CreateDepthTexture();
      _scene->RootVisual()->AddChild(dc);
      std::vector<float> *target = nullptr;
      auto conn = dc->ConnectNewDepthFrame(
          [&](const float *_d, unsigned int, unsigned int, unsigned int,
              const std::string &) {
            std::memcpy(target->data(), _d, target->size() * sizeof(float));
          });
      stats(c.name + "_depth", c.w * c.h, 0.01, [&](std::vector<float> &_v)
          { target = &_v; dc->Update(); });
      conn.reset();
    }

    for (const auto &c : _raysCfgs)
    {
      GpuRaysPtr rays = _scene->CreateGpuRays(c.name);
      rays->SetLocalPose(c.pose);
      rays->SetNearClipPlane(c.nearClip);
      rays->SetFarClipPlane(c.farClip);
      rays->SetClamp(c.clamp);
      rays->SetAngleMin(c.hMin);
      rays->SetAngleMax(c.hMax);
      rays->SetRayCount(c.hCount);
      rays->SetVerticalRayCount(c.vCount);
      if (c.vCount > 1)
      {
        rays->SetVerticalAngleMin(c.vMin);
        rays->SetVerticalAngleMax(c.vMax);
      }
      _scene->RootVisual()->AddChild(rays);
      const unsigned int ch = rays->Channels();
      std::vector<float> *target = nullptr;
      auto conn = rays->ConnectNewGpuRaysFrame(
          [&](const float *_d, unsigned int, unsigned int, unsigned int,
              const std::string &) {
            for (size_t i = 0; i < target->size(); ++i)
              (*target)[i] = _d[i * ch];
          });
      stats(c.name, c.hCount * c.vCount, 0.01, [&](std::vector<float> &_v)
          { target = &_v; rays->Update(); });
      conn.reset();
    }

    for (PixelFormat fmt : {PF_L16, PF_L8})
    {
      const std::string name = fmt == PF_L16 ? "thermal16" : "thermal8";
      ThermalCameraPtr th = _scene->CreateThermalCamera(name);
      th->SetLocalPose({0, 0, 1, 0, 0.2, 0});
      th->SetImageWidth(320);
      th->SetImageHeight(240);
      th->SetAspectRatio(320.0 / 240.0);
      th->SetHFOV(GZ_PI / 2);
      th->SetNearClipPlane(0.5);
      th->SetFarClipPlane(10);
      th->SetImageFormat(fmt);
      th->SetAmbientTemperature(296.0f);
      th->SetAmbientTemperatureRange(4.0f);
      th->SetHeatSourceTemperatureRange(2.0f);
      th->SetLinearResolution(fmt == PF_L16 ? 0.01f : 3.0f);
      th->SetMinTemperature(250.0f);
      th->SetMaxTemperature(fmt == PF_L16 ? 400.0f : 600.0f);
      _scene->RootVisual()->AddChild(th);
      std::vector<float> *target = nullptr;
      auto conn = th->ConnectNewThermalFrame(
          [&](const uint16_t *_d, unsigned int, unsigned int, unsigned int,
              const std::string &) {
            std::copy(_d, _d + target->size(), target->begin()); });
      stats(name, 320 * 240, 0.5, [&](std::vector<float> &_v)
          { target = &_v; th->Update(); });
      conn.reset();
    }
  }

  /// \brief GL projection matrix of a pinhole camera (fx, fy, cx, cy in
  /// pixels, cy counted from the top row), like gz-sensors builds from SDF
  /// <intrinsics>
  math::Matrix4d Intrinsics(unsigned int _w, unsigned int _h, double _fx,
      double _fy, double _cx, double _cy, double _near, double _far)
  {
    return math::Matrix4d(
        2.0 * _fx / _w, 0, 1.0 - 2.0 * _cx / _w, 0,
        0, 2.0 * _fy / _h, 2.0 * _cy / _h - 1.0, 0,
        0, 0, -(_far + _near) / (_far - _near),
        -2.0 * _far * _near / (_far - _near),
        0, 0, -1, 0);
  }

  /// \brief PARITY_DEPTHEDGE=1 outputs: depth cameras whose views cross
  /// both clip ranges (floor nearer than near and beyond far), with
  /// off-center intrinsics and odd sizes; per camera <name>_depth and
  /// <name>_cloud of one frame with a point cloud subscriber, and
  /// <name>_depthonly of one without. The depth clamping must give the
  /// same bytes whichever way an engine computes it (compare dumps of two
  /// builds), and _depthonly must equal _depth.
  void DepthEdges(ScenePtr _scene, const std::string &_dir,
      std::ofstream &_index)
  {
    struct Cfg { std::string name; math::Pose3d pose; unsigned int w, h;
        double nearClip, farClip, fx, fy, cx, cy; };
    for (const Cfg &c : {
        Cfg{"edgeA", {0, 0, 1, 0, 0.9, 0}, 321, 239, 1.1, 2.5,
            150, 160, 60, 200},
        Cfg{"edgeB", {0.5, 0.3, 1.2, 0.2, 0.7, 0.4}, 255, 257, 1.3, 3.0,
            200, 180, 230, 30},
        Cfg{"edgeC", {-1, 0, 0.8, 0, 0.5, -0.3}, 317, 211, 0.9, 2.0,
            160, 160, 158.5, 105.5}})
    {
      DepthCameraPtr cam = _scene->CreateDepthCamera(c.name);
      cam->SetLocalPose(c.pose);
      cam->SetImageWidth(c.w);
      cam->SetImageHeight(c.h);
      cam->SetAspectRatio(static_cast<double>(c.w) / c.h);
      cam->SetHFOV(2.0 * std::atan(c.w * 0.5 / c.fx));
      cam->SetNearClipPlane(c.nearClip);
      cam->SetFarClipPlane(c.farClip);
      cam->CreateDepthTexture();
      cam->SetProjectionMatrix(Intrinsics(c.w, c.h, c.fx, c.fy, c.cx, c.cy,
          c.nearClip, c.farClip));
      _scene->RootVisual()->AddChild(cam);
      std::vector<float> depth(c.w * c.h), cloud(c.w * c.h * 4);
      auto dc = cam->ConnectNewDepthFrame(
          [&](const float *_d, unsigned int, unsigned int, unsigned int,
              const std::string &) {
            std::memcpy(depth.data(), _d, depth.size() * sizeof(float)); });
      auto pc = cam->ConnectNewRgbPointCloud(
          [&](const float *_d, unsigned int, unsigned int, unsigned int,
              const std::string &) {
            std::memcpy(cloud.data(), _d, cloud.size() * sizeof(float)); });
      cam->Update();
      Write(_dir, c.name + "_depth", depth.data(), c.w, c.h, 1, _index);
      Write(_dir, c.name + "_cloud", cloud.data(), c.w, c.h, 4, _index);
      pc.reset();
      cam->Update();
      Write(_dir, c.name + "_depthonly", depth.data(), c.w, c.h, 1, _index);
      dc.reset();
    }
  }

  /// \brief PARITY_PROJECTION=1 outputs. proj_*: depthA with an off-center
  /// intrinsics matrix; ortho_*: depthB set to CPT_ORTHOGRAPHIC (ogre2
  /// renders sensor cameras perspective whatever the type);
  /// bbox_proj_* / bboxdraw_proj_*: the bbox camera with an off-center
  /// intrinsics matrix; projmat_*: ProjectionMatrix() of those cameras and
  /// of an orthographic RGB camera (ogre's default ortho window).
  /// rgb_ortho.png: RGB camera with an OrthoViewController-style custom
  /// orthographic matrix, for eyeballing.
  void Projection(ScenePtr _scene, const std::string &_dir,
      std::ofstream &_index)
  {
    auto writeMatrix = [&](const std::string &_name, const math::Matrix4d &_m)
    {
      std::vector<float> f;
      for (int r = 0; r < 4; ++r)
        for (int c = 0; c < 4; ++c)
          f.push_back(static_cast<float>(_m(r, c)));
      Write(_dir, "projmat_" + _name, f.data(), 4, 4, 1, _index);
    };

    struct Cfg { std::string name; math::Pose3d pose; unsigned int w, h;
        double hfov, nearClip, farClip; bool ortho; };
    for (const Cfg &c : {
        Cfg{"proj", {0, 0, 1, 0, 0.2, 0}, 320, 240, GZ_PI / 2, 0.1, 10, false},
        Cfg{"ortho", {-1, 2, 0.5, 0, 0.1, -0.6}, 640, 480, GZ_PI / 3, 0.5, 6,
            true}})
    {
      DepthCameraPtr cam = _scene->CreateDepthCamera(c.name);
      cam->SetLocalPose(c.pose);
      cam->SetImageWidth(c.w);
      cam->SetImageHeight(c.h);
      cam->SetAspectRatio(static_cast<double>(c.w) / c.h);
      cam->SetHFOV(c.hfov);
      cam->SetNearClipPlane(c.nearClip);
      cam->SetFarClipPlane(c.farClip);
      cam->CreateDepthTexture();
      if (c.ortho)
        cam->SetProjectionType(CPT_ORTHOGRAPHIC);
      else
      {
        // wrong depth rows on purpose: the depth camera owns them
        cam->SetProjectionMatrix(Intrinsics(c.w, c.h, 180, 190, 185, 100,
            c.nearClip * 2, c.farClip * 3));
      }
      _scene->RootVisual()->AddChild(cam);
      std::vector<float> depth(c.w * c.h), cloud(c.w * c.h * 4);
      auto dc = cam->ConnectNewDepthFrame(
          [&](const float *_d, unsigned int, unsigned int, unsigned int,
              const std::string &) {
            std::memcpy(depth.data(), _d, depth.size() * sizeof(float)); });
      auto pc = cam->ConnectNewRgbPointCloud(
          [&](const float *_d, unsigned int, unsigned int, unsigned int,
              const std::string &) {
            std::memcpy(cloud.data(), _d, cloud.size() * sizeof(float)); });
      cam->Update();
      Write(_dir, c.name + "_depth", depth.data(), c.w, c.h, 1, _index);
      Write(_dir, c.name + "_cloud", cloud.data(), c.w, c.h, 4, _index);
      writeMatrix(c.name, cam->ProjectionMatrix());
      dc.reset();
      pc.reset();
    }

    {
      BoundingBoxCameraPtr bb = _scene->CreateBoundingBoxCamera("bbox");
      bb->SetLocalPose({0, 0, 1, 0, 0.2, 0});
      bb->SetImageWidth(320);
      bb->SetImageHeight(240);
      bb->SetAspectRatio(320.0 / 240.0);
      bb->SetHFOV(GZ_PI / 2);
      bb->SetProjectionMatrix(Intrinsics(320, 240, 150, 155, 175, 110,
          bb->NearClipPlane(), bb->FarClipPlane()));
      _scene->RootVisual()->AddChild(bb);
      writeMatrix("bbox", bb->ProjectionMatrix());
      std::vector<BoundingBox> boxes;
      auto conn = bb->ConnectNewBoundingBoxes(
          [&](const std::vector<BoundingBox> &_b) { boxes = _b; });
      for (auto type : {BoundingBoxType::BBT_VISIBLEBOX2D,
                        BoundingBoxType::BBT_FULLBOX2D,
                        BoundingBoxType::BBT_BOX3D})
      {
        const std::string t = type == BoundingBoxType::BBT_VISIBLEBOX2D ?
            "visible2d" : type == BoundingBoxType::BBT_FULLBOX2D ? "full2d" :
            "box3d";
        bb->SetBoundingBoxType(type);
        bb->Update();
        std::vector<float> rows;
        for (const BoundingBox &b : boxes)
        {
          const math::Quaterniond q = b.Orientation();
          for (double v : {b.Center().X(), b.Center().Y(), b.Center().Z(),
               b.Size().X(), b.Size().Y(), b.Size().Z(), q.W(), q.X(), q.Y(),
               q.Z(), static_cast<double>(b.Label())})
            rows.push_back(static_cast<float>(v));
        }
        Write(_dir, "bbox_proj_" + t, rows.data(), 11, boxes.size(), 1,
            _index);
        std::vector<uint8_t> img(320 * 240 * 3, 0);
        for (const BoundingBox &b : boxes)
          bb->DrawBoundingBox(img.data(), math::Color::Green, b);
        WriteBytes(_dir, "bboxdraw_proj_" + t, img.data(), 320, 240, _index);
      }
      conn.reset();
    }

    _scene->SetAmbientLight(0.3, 0.3, 0.3);
    DirectionalLightPtr sun = _scene->CreateDirectionalLight();
    sun->SetDirection(-0.5, -0.5, -1);
    sun->SetDiffuseColor(0.9, 0.9, 0.9);
    _scene->RootVisual()->AddChild(sun);
    for (bool custom : {false, true})
    {
      CameraPtr cam = _scene->CreateCamera(custom ? "rgb_ortho" : "rgb_ortho_default");
      cam->SetLocalPose({-8, 8, 5, 0, 0.4, -GZ_PI / 4});
      cam->SetImageWidth(640);
      cam->SetImageHeight(480);
      cam->SetAspectRatio(640.0 / 480.0);
      cam->SetHFOV(GZ_PI / 3);
      cam->SetNearClipPlane(0.1);
      cam->SetFarClipPlane(50);
      cam->SetProjectionType(CPT_ORTHOGRAPHIC);
      _scene->RootVisual()->AddChild(cam);
      if (!custom)
      {
        writeMatrix("rgb_ortho_default", cam->ProjectionMatrix());
        continue;
      }
      // 16 x 12 m window, like OrthoViewController's
      math::Matrix4d m = math::Matrix4d::Zero;
      m(0, 0) = 2.0 / 16.0;
      m(1, 1) = 2.0 / 12.0;
      m(2, 2) = -2.0 / (50 - 0.1);
      m(2, 3) = -(50 + 0.1) / (50 - 0.1);
      m(3, 3) = 1.0;
      cam->SetProjectionMatrix(m);
      writeMatrix("rgb_ortho", cam->ProjectionMatrix());
      Image img = cam->CreateImage();
      cam->Capture(img);
      common::Image png;
      png.SetFromData(img.Data<unsigned char>(), 640, 480,
          common::Image::RGB_INT8);
      png.SavePNG(_dir + "/rgb_ortho.png");
    }
  }
}

/// \brief PARITY_ALPHATEST: alpha_plate.png, a single-sided alpha tested
/// plate 3 m above the floor seen from above (red where drawn), and
/// alpha_shadow.png, its shadow under the overview's sun
void AlphaTest(ScenePtr _scene, const std::string &_dir)
{
  VisualPtr v = _scene->CreateVisual();
  v->AddGeometry(_scene->CreatePlane());
  v->SetLocalPose({-4, -4, 3, 0, 0, 0});
  v->SetLocalScale(1.5, 1.5, 1);
  v->SetMaterial(CheckerMaterial(_scene, false));
  _scene->RootVisual()->AddChild(v);
  auto shot = [&](const std::string &_name, const math::Pose3d &_pose)
  {
    CameraPtr cam = _scene->CreateCamera(_name);
    cam->SetLocalPose(_pose);
    cam->SetImageWidth(160);
    cam->SetImageHeight(160);
    cam->SetAspectRatio(1.0);
    cam->SetHFOV(GZ_PI / 4);
    _scene->RootVisual()->AddChild(cam);
    Image img = cam->CreateImage();
    cam->Capture(img);
    common::Image png;
    png.SetFromData(img.Data<unsigned char>(), 160, 160,
        common::Image::RGB_INT8);
    png.SavePNG(_dir + "/" + _name + ".png");
  };
  shot("alpha_plate", {-4, -4, 6, 0, GZ_PI / 2, 0});
  // the sun (-0.5, -0.5, -1) offsets the shadow by 1.5 m in x and y
  shot("alpha_shadow", {-5.5, -5.5, 2.5, 0, GZ_PI / 2, 0});
}

/// \brief PARITY_PBR: a row of material samples across the RGB overview,
/// spheres for the scalar parameters and boxes for the texture maps
void AddPbrRow(ScenePtr _scene, bool _sky)
{
  const std::string tex = std::string(PROJECT_SOURCE_PATH) +
      "/test/media/materials/textures/";
  // procedural bumps: tangent space normals of h = 0.5 sin(kx) sin(ky)
  const unsigned int n = 256;
  std::vector<unsigned char> px(n * n * 3);
  for (unsigned int y = 0; y < n; ++y)
  {
    for (unsigned int x = 0; x < n; ++x)
    {
      const double k = 8 * GZ_PI / n;
      const double dx = 0.5 * k * n / 8 * std::cos(k * x) * std::sin(k * y);
      const double dy = 0.5 * k * n / 8 * std::sin(k * x) * std::cos(k * y);
      math::Vector3d nrm(-dx, -dy, 1);
      nrm.Normalize();
      unsigned char *p = &px[(y * n + x) * 3];
      p[0] = static_cast<unsigned char>((nrm.X() * 0.5 + 0.5) * 255);
      p[1] = static_cast<unsigned char>((nrm.Y() * 0.5 + 0.5) * 255);
      p[2] = static_cast<unsigned char>((nrm.Z() * 0.5 + 0.5) * 255);
    }
  }
  auto bumps = std::make_shared<common::Image>();
  bumps->SetFromData(px.data(), n, n, common::Image::RGB_INT8);

  struct Sample
  {
    bool box;
    std::function<void(MaterialPtr)> set;
    bool wireframe = false;
  };
  const std::vector<Sample> samples = {
    // dielectric, rough 0.1 / 0.9
    {false, [](MaterialPtr _m) {
      _m->SetDiffuse(0.8, 0.1, 0.1); _m->SetRoughness(0.1f); }},
    {false, [](MaterialPtr _m) {
      _m->SetDiffuse(0.8, 0.1, 0.1); _m->SetRoughness(0.9f); }},
    // gz-sim's default "gz-grey" (primitives without a material)
    {false, [](MaterialPtr _m) {
      _m->SetAmbient(0.3, 0.3, 0.3); _m->SetDiffuse(0.7, 0.7, 0.7);
      _m->SetSpecular(1, 1, 1); _m->SetRoughness(0.2f);
      _m->SetMetalness(1.0f); }},
    // gold metal, roughness map
    {true, [tex](MaterialPtr _m) {
      _m->SetDiffuse(1.0, 0.75, 0.3); _m->SetMetalness(1.0f);
      _m->SetRoughness(1.0f); _m->SetRoughnessMap(tex + "texture.png"); }},
    // normal map
    {true, [bumps](MaterialPtr _m) {
      _m->SetDiffuse(0.7, 0.7, 0.7); _m->SetRoughness(0.4f);
      _m->SetNormalMap("parity_bumps", bumps); }},
    // emissive color
    {false, [](MaterialPtr _m) {
      _m->SetDiffuse(0.1, 0.1, 0.1); _m->SetEmissive(0.1, 0.9, 0.2); }},
    // emissive map
    {true, [tex](MaterialPtr _m) {
      _m->SetDiffuse(0.1, 0.1, 0.1);
      _m->SetEmissiveMap(tex + "texture.png"); }},
    // light map (uv set 0), modulates the diffuse color
    {true, [tex](MaterialPtr _m) {
      _m->SetDiffuse(1.0, 1.0, 1.0);
      _m->SetLightMap(tex + "texture.png", 0u); }},
    // environment (reflection) map on a smooth metal
    {false, [tex](MaterialPtr _m) {
      _m->SetDiffuse(0.9, 0.9, 0.9); _m->SetMetalness(1.0f);
      _m->SetRoughness(0.1f);
      _m->SetEnvironmentMap(tex + "skybox_lowres.dds"); }},
    // wireframe
    {false, [](MaterialPtr _m) { _m->SetDiffuse(0.9, 0.9, 0.2); }, true},
  };
  // a row facing the overview camera, above the sensor scene
  const double count = static_cast<double>(samples.size());
  for (size_t i = 0; i < samples.size(); ++i)
  {
    const double t = (static_cast<double>(i) - (count - 1) / 2) * 0.65 - 0.4;
    VisualPtr v = _scene->CreateVisual();
    v->AddGeometry(samples[i].box ? _scene->CreateBox() :
        _scene->CreateSphere());
    v->SetLocalPose({-4.5 + t * 0.7071, 4.5 + t * 0.7071, 2.6,
        0, 0, -GZ_PI / 4});
    v->SetLocalScale(0.6, 0.6, 0.6);
    MaterialPtr m = _scene->CreateMaterial();
    samples[i].set(m);
    v->SetMaterial(m);
    v->SetWireframe(samples[i].wireframe);
    _scene->RootVisual()->AddChild(v);
  }
  if (_sky)
    _scene->SetSkyEnabled(true);
}

int main(int _argc, char **_argv)
{
  if (_argc < 3)
  {
    std::cerr << "usage: filament_parity <engine> <outdir> [frames] [param]\n";
    return 1;
  }
  common::Console::SetVerbosity(2);
  const std::string dir = _argv[2];
  const int frames = _argc > 3 ? std::atoi(_argv[3]) : 50;
  std::map<std::string, std::string> params;
  if (_argc > 4)
    params[_argv[4]] = "1";
  RenderEngine *engine = rendering::engine(_argv[1], params);
  if (!engine)
    return 1;
  ScenePtr scene = engine->CreateScene("scene");
  BuildScene(scene);
  if (std::getenv("PARITY_ACTOR"))
    std::ofstream(dir + "/actor.txt") << "PARITY_ACTOR=1\n";
  if (std::getenv("PARITY_ALPHATEST"))
    std::ofstream(dir + "/alphatest.txt") << "PARITY_ALPHATEST=1\n";
  std::ofstream index(dir + "/index.txt");
  std::ofstream timing(dir + "/timing.txt");

  if (std::getenv("PARITY_WIDEANGLE"))
  {
    WideAngle(engine, scene, dir, index, timing, frames);
    std::cout << "wrote " << dir << std::endl;
    return 0;
  }

  if (std::getenv("PARITY_DEPTHEDGE"))
  {
    DepthEdges(scene, dir, index);
    std::cout << "wrote " << dir << std::endl;
    return 0;
  }

  if (std::getenv("PARITY_PROJECTION"))
  {
    Projection(scene, dir, index);
    std::cout << "wrote " << dir << std::endl;
    return 0;
  }

  if (std::getenv("PARITY_LENSFLARE"))
  {
    LensFlare(engine, dir);
    std::cout << "wrote " << dir << std::endl;
    return 0;
  }

  if (std::getenv("PARITY_MARKERS"))
  {
    Markers(engine);
    return 0;
  }

  if (const char *png = std::getenv("PARITY_PROJECTOR"))
  {
    ProjectTexture(engine, png, dir);
    std::cout << "wrote " << dir << std::endl;
    return 0;
  }

  if (const char *spotArgs = std::getenv("PARITY_SPOT"))
  {
    Spot(engine, spotArgs, dir);
    std::cout << "wrote " << dir << std::endl;
    return 0;
  }

  if (std::getenv("PARITY_GUI"))
  {
    GuiOverlays(engine, dir);
    std::cout << "wrote " << dir << std::endl;
    return 0;
  }

  if (const char *shader = std::getenv("PARITY_SHADER"))
  {
    if (std::string(shader) == "waves")
      Waves(engine, _argv[1], dir);
    else
      CustomShader(engine, _argv[1], dir);
    std::cout << "wrote " << dir << std::endl;
    return 0;
  }

  if (std::getenv("PARITY_NOISE"))
  {
    Noise(engine, scene, dir, index);
    std::cout << "wrote " << dir << std::endl;
    return 0;
  }

  {
    // RGB overview for eyeballing, not compared
    scene->SetAmbientLight(0.3, 0.3, 0.3);
    DirectionalLightPtr sun = scene->CreateDirectionalLight();
    sun->SetDirection(-0.5, -0.5, -1);
    sun->SetDiffuseColor(0.9, 0.9, 0.9);
    scene->RootVisual()->AddChild(sun);
    // PARITY_POINTLIGHTS=1: lit like the Harmonic house interior: a dim sun
    // and point/spot lights close to surfaces, all with specular 0.01
    if (std::getenv("PARITY_POINTLIGHTS"))
    {
      sun->SetDiffuseColor(0.3, 0.3, 0.3);
      sun->SetSpecularColor(0.01, 0.01, 0.01);
      auto add = [&](LightPtr _l, const math::Vector3d &_pos, double _range,
          double _lin, double _quad, bool _shadows)
      {
        _l->SetLocalPosition(_pos);
        _l->SetSpecularColor(0.01, 0.01, 0.01);
        _l->SetAttenuationRange(_range);
        _l->SetAttenuationConstant(0.8);
        _l->SetAttenuationLinear(_lin);
        _l->SetAttenuationQuadratic(_quad);
        _l->SetCastShadows(_shadows);
        scene->RootVisual()->AddChild(_l);
      };
      add(scene->CreatePointLight(), {0, 0, 1.8}, 10, 0.5, 0.001, false);
      add(scene->CreatePointLight(), {2, 0.5, 1.5}, 20, 0.2, 0.01, true);
      add(scene->CreatePointLight(), {6.5, 0, 2.5}, 10, 0.2, 0.001, false);
      SpotLightPtr spot = scene->CreateSpotLight();
      spot->SetDirection(0, 0, -1);
      add(spot, {-1.5, -2, 2.5}, 10, 0.2, 0.001, false);
    }
    // PARITY_TEXBOX=<png>: a large textured box facing the overview camera,
    // to check texture orientation (use an image with distinct top/bottom)
    if (const char *tex = std::getenv("PARITY_TEXBOX"))
    {
      VisualPtr v = scene->CreateVisual();
      v->AddGeometry(scene->CreateBox());
      v->SetLocalPose({-3, 3, 1.5, 0, 0, -GZ_PI / 4});
      v->SetLocalScale(2, 2, 2);
      MaterialPtr m = scene->CreateMaterial();
      m->SetDiffuse(1, 1, 1);
      m->SetTexture(tex);
      v->SetMaterial(m);
      scene->RootVisual()->AddChild(v);
    }
    if (const char *pbr = std::getenv("PARITY_PBR"))
      AddPbrRow(scene, std::string(pbr) == "sky");
    CameraPtr cam = scene->CreateCamera("rgb");
    if (std::getenv("PARITY_USERCAM"))
    {
      // like gz-gui's MinimalScene: filament renders this one in its GUI
      // quality mode
      cam->SetUserData("user-camera", true);
      cam->SetAntiAliasing(8);
    }
    cam->SetLocalPose({-8, 8, 5, 0, 0.4, -GZ_PI / 4});
    cam->SetImageWidth(640);
    cam->SetImageHeight(480);
    cam->SetAspectRatio(640.0 / 480.0);
    cam->SetHFOV(GZ_PI / 3);
    scene->RootVisual()->AddChild(cam);
    Image img = cam->CreateImage();
    cam->Capture(img);
    // PARITY_USERCAM=<n> with n > 1: time n more captures (render + read)
    const char *userCam = std::getenv("PARITY_USERCAM");
    if (userCam && std::atoi(userCam) > 1)
    {
      const int n = std::atoi(userCam);
      const auto t0 = std::chrono::steady_clock::now();
      for (int i = 0; i < n; ++i)
        cam->Capture(img);
      const std::chrono::duration<double, std::milli> dt =
          std::chrono::steady_clock::now() - t0;
      std::cout << "usercam ms/frame " << dt.count() / n << std::endl;
    }
    common::Image png;
    png.SetFromData(img.Data<unsigned char>(), 640, 480,
        common::Image::RGB_INT8);
    png.SavePNG(dir + "/rgb.png");
  }

  const std::vector<DepthCfg> depthCfgs = {
    {"depthA", {0, 0, 1, 0, 0.2, 0}, 320, 240, GZ_PI / 2, 0.1, 10},
    {"depthB", {-1, 2, 0.5, 0, 0.1, -0.6}, 640, 480, GZ_PI / 3, 0.5, 6},
    {"depthC", {0, 0, 0.9, 0.2, 0.3, 0.4}, 256, 256, 1.2, 0.3, 4},
  };
  std::vector<RaysCfg> raysCfgs = {
    {"lidar2d", {0, 0, 0.5, 0, 0, 0}, 720, 1,
        -GZ_PI, GZ_PI, 0, 0, 0.1, 12, false},
    {"vlp16", {0, 0, 1.0, 0, 0, 0}, 1800, 16,
        -GZ_PI, GZ_PI, -0.2618, 0.2618, 0.3, 10, false},
    {"vlp16clamp", {0, 0, 1.0, 0, 0, 0}, 1800, 16,
        -GZ_PI, GZ_PI, -0.2618, 0.2618, 0.3, 10, true},
    {"tilted", {0.5, -0.5, 1.2, 0.3, 0.2, 0.5}, 128, 64,
        -GZ_PI / 4, GZ_PI / 4, -0.5236, 0.1745, 0.2, 8, false},
    {"os1_64", {0, 0, 1.2, 0, 0, 0}, 1024, 64,
        -GZ_PI, GZ_PI, -0.3927, 0.3927, 0.3, 20, false},
  };

  // PARITY_S2L2=1: also the 2048x128 lidar of the gz-sim S2L2 benchmark
  // (quadruped-physics-bench lidar_eq worlds)
  if (std::getenv("PARITY_S2L2"))
  {
    raysCfgs.push_back({"s2l2", {0, 0, 1.2, 0, 0, 0}, 2048, 128,
        -GZ_PI, GZ_PI, -0.3927, 0.3927, 0.3, 8, false});
  }

  if (std::getenv("PARITY_MULTI"))
  {
    Multi(scene, dir, index, timing, frames, depthCfgs, raysCfgs);
    std::cout << "wrote " << dir << std::endl;
    return 0;
  }

  if (std::getenv("PARITY_PARTICLES"))
  {
    Particles(scene, dir, depthCfgs, raysCfgs);
    std::cout << "wrote " << dir << std::endl;
    return 0;
  }

  for (const auto &c : depthCfgs)
  {
    DepthCameraPtr cam = scene->CreateDepthCamera(c.name);
    cam->SetLocalPose(c.pose);
    cam->SetImageWidth(c.w);
    cam->SetImageHeight(c.h);
    cam->SetAspectRatio(static_cast<double>(c.w) / c.h);
    cam->SetHFOV(c.hfov);
    cam->SetNearClipPlane(c.nearClip);
    cam->SetFarClipPlane(c.farClip);
    cam->CreateDepthTexture();
    scene->RootVisual()->AddChild(cam);

    std::vector<float> depth(c.w * c.h), cloud(c.w * c.h * 4);
    auto dc = cam->ConnectNewDepthFrame(
        [&](const float *_d, unsigned int, unsigned int, unsigned int,
            const std::string &) {
          std::memcpy(depth.data(), _d, depth.size() * sizeof(float)); });
    auto t = Time([&] { cam->Update(); }, frames);
    timing << c.name << "_depth " << t.first << " " << t.second << "\n";
    // PARITY_DEPTHONLY=1: also the depth of a frame without a point cloud
    // subscriber
    if (std::getenv("PARITY_DEPTHONLY"))
      Write(dir, c.name + "_depthonly", depth.data(), c.w, c.h, 1, index);
    auto pc = cam->ConnectNewRgbPointCloud(
        [&](const float *_d, unsigned int, unsigned int, unsigned int,
            const std::string &) {
          std::memcpy(cloud.data(), _d, cloud.size() * sizeof(float)); });
    t = Time([&] { cam->Update(); }, frames);
    timing << c.name << "_depth+cloud " << t.first << " " << t.second << "\n";
    Write(dir, c.name + "_depth", depth.data(), c.w, c.h, 1, index);
    Write(dir, c.name + "_cloud", cloud.data(), c.w, c.h, 4, index);
    // sensors stay alive: ogre2 asserts in DestroySensor on depth cameras
    dc.reset();
    pc.reset();
  }

  {
    SegmentationCameraPtr seg = scene->CreateSegmentationCamera("seg");
    seg->SetLocalPose({0, 0, 1, 0, 0.2, 0});
    seg->SetImageWidth(320);
    seg->SetImageHeight(240);
    seg->SetAspectRatio(320.0 / 240.0);
    seg->SetHFOV(GZ_PI / 2);
    seg->SetBackgroundLabel(23);
    scene->RootVisual()->AddChild(seg);
    std::vector<uint8_t> img(320 * 240 * 3), labels(img.size());
    auto conn = seg->ConnectNewSegmentationFrame(
        [&](const uint8_t *_d, unsigned int, unsigned int, unsigned int,
            const std::string &) {
          std::memcpy(img.data(), _d, img.size()); });
    for (auto type : {SegmentationType::ST_SEMANTIC,
                      SegmentationType::ST_PANOPTIC})
    {
      const std::string t =
          type == SegmentationType::ST_SEMANTIC ? "sem" : "pan";
      seg->SetSegmentationType(type);
      seg->EnableColoredMap(false);
      auto tm = Time([&] { seg->Update(); }, frames);
      timing << "seg_" << t << " " << tm.first << " " << tm.second << "\n";
      WriteBytes(dir, "seg_" + t, img.data(), 320, 240, index);
      seg->EnableColoredMap(true);
      seg->Update();
      WriteBytes(dir, "seg_" + t + "_colored", img.data(), 320, 240, index);
      seg->LabelMapFromColoredBuffer(labels.data());
      WriteBytes(dir, "seg_" + t + "_fromcolored", labels.data(), 320, 240,
          index);
    }
    conn.reset();
  }

  {
    BoundingBoxCameraPtr bb = scene->CreateBoundingBoxCamera("bbox");
    bb->SetLocalPose({0, 0, 1, 0, 0.2, 0});
    bb->SetImageWidth(320);
    bb->SetImageHeight(240);
    bb->SetAspectRatio(320.0 / 240.0);
    bb->SetHFOV(GZ_PI / 2);
    scene->RootVisual()->AddChild(bb);
    std::vector<BoundingBox> boxes;
    auto conn = bb->ConnectNewBoundingBoxes(
        [&](const std::vector<BoundingBox> &_b) { boxes = _b; });
    for (auto type : {BoundingBoxType::BBT_VISIBLEBOX2D,
                      BoundingBoxType::BBT_FULLBOX2D,
                      BoundingBoxType::BBT_BOX3D})
    {
      const std::string t = type == BoundingBoxType::BBT_VISIBLEBOX2D ?
          "visible2d" : type == BoundingBoxType::BBT_FULLBOX2D ? "full2d" :
          "box3d";
      bb->SetBoundingBoxType(type);
      auto tm = Time([&] { bb->Update(); }, frames);
      timing << "bbox_" << t << " " << tm.first << " " << tm.second << "\n";
      // one row per box: center xyz, size xyz, rotation wxyz, label
      std::vector<float> rows;
      for (const BoundingBox &b : boxes)
      {
        const math::Quaterniond q = b.Orientation();
        for (double v : {b.Center().X(), b.Center().Y(), b.Center().Z(),
             b.Size().X(), b.Size().Y(), b.Size().Z(), q.W(), q.X(), q.Y(),
             q.Z(), static_cast<double>(b.Label())})
          rows.push_back(static_cast<float>(v));
      }
      Write(dir, "bbox_" + t, rows.data(), 11, boxes.size(), 1, index);
      std::vector<uint8_t> img(320 * 240 * 3, 0);
      for (const BoundingBox &b : boxes)
        bb->DrawBoundingBox(img.data(), math::Color::Green, b);
      WriteBytes(dir, "bboxdraw_" + t, img.data(), 320, 240, index);
    }
    conn.reset();
  }

  for (PixelFormat fmt : {PF_L16, PF_L8})
  {
    const std::string name = fmt == PF_L16 ? "thermal16" : "thermal8";
    ThermalCameraPtr th = scene->CreateThermalCamera(name);
    th->SetLocalPose({0, 0, 1, 0, 0.2, 0});
    th->SetImageWidth(320);
    th->SetImageHeight(240);
    th->SetAspectRatio(320.0 / 240.0);
    th->SetHFOV(GZ_PI / 2);
    th->SetNearClipPlane(0.5);
    th->SetFarClipPlane(10);
    th->SetImageFormat(fmt);
    th->SetAmbientTemperature(296.0f);
    th->SetAmbientTemperatureRange(4.0f);
    th->SetHeatSourceTemperatureRange(2.0f);
    th->SetLinearResolution(fmt == PF_L16 ? 0.01f : 3.0f);
    th->SetMinTemperature(250.0f);
    th->SetMaxTemperature(fmt == PF_L16 ? 400.0f : 600.0f);
    scene->RootVisual()->AddChild(th);
    std::vector<uint16_t> img(320 * 240);
    auto conn = th->ConnectNewThermalFrame(
        [&](const uint16_t *_d, unsigned int, unsigned int, unsigned int,
            const std::string &) {
          std::memcpy(img.data(), _d, img.size() * sizeof(uint16_t)); });
    auto tm = Time([&] { th->Update(); }, frames);
    timing << name << " " << tm.first << " " << tm.second << "\n";
    std::vector<float> f(img.begin(), img.end());
    Write(dir, name, f.data(), 320, 240, 1, index);
    conn.reset();
  }

  for (const auto &c : raysCfgs)
  {
    GpuRaysPtr rays = scene->CreateGpuRays(c.name);
    rays->SetLocalPose(c.pose);
    rays->SetNearClipPlane(c.nearClip);
    rays->SetFarClipPlane(c.farClip);
    rays->SetClamp(c.clamp);
    rays->SetAngleMin(c.hMin);
    rays->SetAngleMax(c.hMax);
    rays->SetRayCount(c.hCount);
    rays->SetVerticalRayCount(c.vCount);
    if (c.vCount > 1)
    {
      rays->SetVerticalAngleMin(c.vMin);
      rays->SetVerticalAngleMax(c.vMax);
    }
    scene->RootVisual()->AddChild(rays);

    const unsigned int ch = rays->Channels();
    std::vector<float> scan(c.hCount * c.vCount * ch);
    auto conn = rays->ConnectNewGpuRaysFrame(
        [&](const float *_d, unsigned int, unsigned int, unsigned int,
            const std::string &) {
          std::memcpy(scan.data(), _d, scan.size() * sizeof(float)); });
    auto t = Time([&] { rays->Update(); }, frames);
    timing << c.name << " " << t.first << " " << t.second << "\n";
    // keep the range channel only
    std::vector<float> range(c.hCount * c.vCount);
    for (size_t i = 0; i < range.size(); ++i)
      range[i] = scan[i * ch];
    Write(dir, c.name, range.data(), c.hCount, c.vCount, 1, index);
    conn.reset();
  }

  if (std::getenv("PARITY_ALPHATEST"))
    AlphaTest(scene, dir);

  engine->DestroyScene(scene);
  std::cout << "wrote " << dir << std::endl;
  return 0;
}
