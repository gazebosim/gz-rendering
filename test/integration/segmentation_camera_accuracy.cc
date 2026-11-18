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

// Checks the images of the segmentation camera against the scene: labels and
// instances at known pixels, colored maps against label maps, and that
// rendering doesn't change the scene for other cameras.
//
// The camera is at the origin looking along +X (Z up), with a 90 degree
// horizontal field of view, so a point (x, y, z) lands on pixel
//   u = W/2 * (1 - y/x),  v = H/2 * (1 - z / (x * tan(vfov/2))).

#include <gtest/gtest.h>

#include <array>
#include <cmath>
#include <cstring>
#include <memory>
#include <optional>
#include <random>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include "CommonRenderingTest.hh"

#include <gz/common/Filesystem.hh>
#include <gz/common/geospatial/ImageHeightmap.hh>
#include <gz/math/Color.hh>
#include <gz/math/Pose3.hh>
#include <gz/math/Vector3.hh>

#include "gz/rendering/Camera.hh"
#include "gz/rendering/Heightmap.hh"
#include "gz/rendering/Image.hh"
#include "gz/rendering/Marker.hh"
#include "gz/rendering/Material.hh"
#include "gz/rendering/Scene.hh"
#include "gz/rendering/SegmentationCamera.hh"
#include "gz/rendering/Visual.hh"

using namespace gz;
using namespace rendering;

namespace
{
constexpr unsigned int kWidth = 320;
constexpr unsigned int kHeight = 240;
constexpr int kBackground = 23;

using Pixel = std::array<uint8_t, 3>;

/// \brief An image produced by the camera, 3 bytes per pixel.
struct Frame
{
  std::vector<uint8_t> data;

  Pixel At(int _u, int _v) const
  {
    const size_t i = (static_cast<size_t>(_v) * kWidth + _u) * 3;
    return {this->data[i], this->data[i + 1], this->data[i + 2]};
  }

  /// \brief Pixel where a point of the world lands.
  Pixel At(const math::Vector3d &_p) const
  {
    int u, v;
    Project(_p, u, v);
    return this->At(u, v);
  }

  /// \brief Whether a pixel next to where a point lands (3 x 3 pixels) has
  /// this value. For things drawn one pixel wide.
  bool Near(const math::Vector3d &_p, const Pixel &_value) const
  {
    int u, v;
    Project(_p, u, v);
    for (int dv = -1; dv <= 1; ++dv)
    {
      for (int du = -1; du <= 1; ++du)
      {
        if (this->At(u + du, v + dv) == _value)
          return true;
      }
    }
    return false;
  }

  static void Project(const math::Vector3d &_p, int &_u, int &_v)
  {
    const double tanH = std::tan(GZ_PI / 4.0);
    const double tanV = tanH * kHeight / kWidth;
    _u = static_cast<int>(kWidth * 0.5 * (1.0 - _p.Y() / _p.X() / tanH));
    _v = static_cast<int>(kHeight * 0.5 * (1.0 - _p.Z() / _p.X() / tanV));
  }
};

/// \brief Pixel of a label map: label in every channel (semantic).
Pixel Semantic(int _label)
{
  const auto l = static_cast<uint8_t>(_label);
  return {l, l, l};
}

/// \brief Instance id stored in a panoptic label map pixel.
int Instance(const Pixel &_p)
{
  return _p[1] * 256 + _p[0];
}

/// \brief The color the camera tries first for a label in a semantic
/// colored map: random, seeded with the label.
Pixel SeededColor(int64_t _label)
{
  std::default_random_engine generator;
  generator.seed(_label);
  std::uniform_int_distribution<int> distribution(0, 255);
  Pixel color;
  for (auto &c : color)
    c = static_cast<uint8_t>(distribution(generator));
  return color;
}
}  // namespace

//////////////////////////////////////////////////
class SegmentationCameraAccuracyTest : public CommonRenderingTest
{
  /// \brief Camera at the origin looking along +X.
  protected: SegmentationCameraPtr CreateCamera(ScenePtr _scene,
      SegmentationType _type, bool _colored)
  {
    auto camera = _scene->CreateSegmentationCamera();
    if (!camera)
      return camera;
    camera->SetLocalPosition(0, 0, 0);
    camera->SetLocalRotation(0, 0, 0);
    camera->SetImageWidth(kWidth);
    camera->SetImageHeight(kHeight);
    camera->SetAspectRatio(static_cast<double>(kWidth) / kHeight);
    camera->SetHFOV(GZ_PI / 2.0);
    camera->SetNearClipPlane(0.1);
    camera->SetFarClipPlane(100);
    camera->SetBackgroundLabel(kBackground);
    camera->SetSegmentationType(_type);
    camera->EnableColoredMap(_colored);
    _scene->RootVisual()->AddChild(camera);
    return camera;
  }

  /// \brief Render one frame.
  protected: Frame Render(SegmentationCameraPtr _camera)
  {
    Frame frame;
    auto connection = _camera->ConnectNewSegmentationFrame(
        [&frame](const uint8_t *_data, unsigned int _w, unsigned int _h,
            unsigned int _channels, const std::string &)
        {
          frame.data.assign(_data, _data + _w * _h * _channels);
        });
    _camera->Update();
    EXPECT_EQ(kWidth * kHeight * 3, frame.data.size());
    return frame;
  }

  /// \brief Render one frame of a colored map and convert it to labels.
  protected: Frame RenderLabelsFromColors(SegmentationCameraPtr _camera)
  {
    EXPECT_TRUE(_camera->IsColoredMap());
    this->Render(_camera);
    Frame labels;
    labels.data.resize(kWidth * kHeight * 3);
    _camera->LabelMapFromColoredBuffer(labels.data.data());
    return labels;
  }

  /// \brief Add a box visual.
  protected: static VisualPtr AddBox(ScenePtr _scene, VisualPtr _parent,
      const std::string &_name, const math::Vector3d &_position,
      const math::Vector3d &_size, std::optional<int> _label)
  {
    VisualPtr vis = _scene->CreateVisual(_name);
    vis->AddGeometry(_scene->CreateBox());
    vis->SetLocalPosition(_position);
    vis->SetLocalScale(_size);
    if (_label)
      vis->SetUserData("label", *_label);
    _parent->AddChild(vis);
    return vis;
  }
};

//////////////////////////////////////////////////
// Semantic label map: each object's pixels hold its label; the background
// and unlabeled objects hold the background label; the front object wins.
TEST_F(SegmentationCameraAccuracyTest, SemanticLabelMap)
{
  CHECK_SUPPORTED_ENGINE("ogre2");
  ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);
  auto camera = this->CreateCamera(scene, SegmentationType::ST_SEMANTIC,
      false);
  ASSERT_NE(nullptr, camera);

  VisualPtr root = scene->RootVisual();
  AddBox(scene, root, "left", {5, 1.5, 0}, {1, 1, 1}, 1);
  AddBox(scene, root, "middle", {5, 0, 0}, {1, 1, 1}, 2);
  AddBox(scene, root, "right", {5, -1.5, 0}, {1, 1, 1}, 1);
  AddBox(scene, root, "unlabeled", {5, 0, 1.5}, {1, 1, 1}, std::nullopt);
  // a small box in front of the bottom of the middle one
  AddBox(scene, root, "front", {3, 0, -0.3}, {0.2, 0.2, 0.2}, 9);

  const Frame frame = this->Render(camera);
  EXPECT_EQ(Semantic(1), frame.At({5, 1.5, 0}));
  EXPECT_EQ(Semantic(2), frame.At({5, 0, 0}));
  EXPECT_EQ(Semantic(1), frame.At({5, -1.5, 0}));
  EXPECT_EQ(Semantic(kBackground), frame.At({5, 0, 1.5}));
  EXPECT_EQ(Semantic(9), frame.At({3, 0, -0.3}));
  EXPECT_EQ(Semantic(kBackground), frame.At(0, 0));

  engine->DestroyScene(scene);
}

//////////////////////////////////////////////////
// Semantic colored map: one color per label, the background color for the
// background and unlabeled objects, and the label map computed from the
// colors matches the label map rendered directly.
TEST_F(SegmentationCameraAccuracyTest, SemanticColoredMap)
{
  CHECK_SUPPORTED_ENGINE("ogre2");
  ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);
  auto colored = this->CreateCamera(scene, SegmentationType::ST_SEMANTIC,
      true);
  ASSERT_NE(nullptr, colored);
  auto plain = this->CreateCamera(scene, SegmentationType::ST_SEMANTIC,
      false);
  ASSERT_NE(nullptr, plain);

  VisualPtr root = scene->RootVisual();
  AddBox(scene, root, "left", {5, 1.5, 0}, {1, 1, 1}, 1);
  AddBox(scene, root, "middle", {5, 0, 0}, {1, 1, 1}, 2);
  AddBox(scene, root, "right", {5, -1.5, 0}, {1, 1, 1}, 1);
  AddBox(scene, root, "unlabeled", {5, 0, 1.5}, {1, 1, 1}, std::nullopt);

  const Frame frame = this->Render(colored);
  const Pixel background = Semantic(kBackground);
  EXPECT_EQ(frame.At({5, 1.5, 0}), frame.At({5, -1.5, 0}));
  EXPECT_NE(frame.At({5, 1.5, 0}), frame.At({5, 0, 0}));
  EXPECT_NE(background, frame.At({5, 1.5, 0}));
  EXPECT_NE(background, frame.At({5, 0, 0}));
  EXPECT_EQ(background, frame.At({5, 0, 1.5}));
  EXPECT_EQ(background, frame.At(0, 0));

  const Frame labels = this->RenderLabelsFromColors(colored);
  EXPECT_EQ(Semantic(1), labels.At({5, 1.5, 0}));
  EXPECT_EQ(Semantic(2), labels.At({5, 0, 0}));
  EXPECT_EQ(Semantic(kBackground), labels.At({5, 0, 1.5}));
  EXPECT_TRUE(labels.data == this->Render(plain).data)
      << "label map from colors differs from the rendered label map";

  engine->DestroyScene(scene);
}

//////////////////////////////////////////////////
// Panoptic label map: label in the third channel, instance in the first two.
// Models with the same label get different instances, numbered in
// descending name order; the links of a model share one instance.
TEST_F(SegmentationCameraAccuracyTest, PanopticLabelMap)
{
  CHECK_SUPPORTED_ENGINE("ogre2");
  ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);
  auto camera = this->CreateCamera(scene, SegmentationType::ST_PANOPTIC,
      false);
  ASSERT_NE(nullptr, camera);

  VisualPtr root = scene->RootVisual();
  AddBox(scene, root, "box_left", {5, 1.5, 0}, {1, 1, 1}, 1);
  AddBox(scene, root, "box_mid", {5, 0, 0}, {1, 1, 1}, 2);
  AddBox(scene, root, "box_right", {5, -1.5, 0}, {1, 1, 1}, 1);
  VisualPtr robot = scene->CreateVisual("robot");
  robot->SetLocalPosition(5, 0, 1.5);
  root->AddChild(robot);
  AddBox(scene, robot, "robot::link_a", {0, 0.6, 0}, {1, 1, 1}, 3);
  AddBox(scene, robot, "robot::link_b", {0, -0.6, 0}, {1, 1, 1}, 3);

  const Frame frame = this->Render(camera);
  const Pixel left = frame.At({5, 1.5, 0});
  const Pixel mid = frame.At({5, 0, 0});
  const Pixel right = frame.At({5, -1.5, 0});
  const Pixel linkA = frame.At({5, 0.6, 1.5});
  const Pixel linkB = frame.At({5, -0.6, 1.5});
  EXPECT_EQ(1, left[2]);
  EXPECT_EQ(2, mid[2]);
  EXPECT_EQ(1, right[2]);
  EXPECT_EQ(3, linkA[2]);
  EXPECT_EQ(3, linkB[2]);
  EXPECT_EQ(2, Instance(left));
  EXPECT_EQ(1, Instance(mid));
  EXPECT_EQ(1, Instance(right));
  EXPECT_EQ(1, Instance(linkA));
  EXPECT_EQ(1, Instance(linkB));
  EXPECT_EQ(Semantic(kBackground), frame.At(0, 0));

  engine->DestroyScene(scene);
}

//////////////////////////////////////////////////
// Panoptic colored map: one color per instance, and the label map computed
// from the colors matches the label map rendered directly.
TEST_F(SegmentationCameraAccuracyTest, PanopticColoredMap)
{
  CHECK_SUPPORTED_ENGINE("ogre2");
  ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);
  auto colored = this->CreateCamera(scene, SegmentationType::ST_PANOPTIC,
      true);
  ASSERT_NE(nullptr, colored);
  auto plain = this->CreateCamera(scene, SegmentationType::ST_PANOPTIC,
      false);
  ASSERT_NE(nullptr, plain);

  VisualPtr root = scene->RootVisual();
  AddBox(scene, root, "box_left", {5, 1.5, 0}, {1, 1, 1}, 1);
  AddBox(scene, root, "box_mid", {5, 0, 0}, {1, 1, 1}, 2);
  AddBox(scene, root, "box_right", {5, -1.5, 0}, {1, 1, 1}, 1);
  VisualPtr robot = scene->CreateVisual("robot");
  robot->SetLocalPosition(5, 0, 1.5);
  root->AddChild(robot);
  AddBox(scene, robot, "robot::link_a", {0, 0.6, 0}, {1, 1, 1}, 3);
  AddBox(scene, robot, "robot::link_b", {0, -0.6, 0}, {1, 1, 1}, 3);

  const Frame frame = this->Render(colored);
  std::set<Pixel> colors = {frame.At({5, 1.5, 0}), frame.At({5, 0, 0}),
      frame.At({5, -1.5, 0}), frame.At({5, 0.6, 1.5}),
      Semantic(kBackground)};
  EXPECT_EQ(5u, colors.size()) << "instances must have distinct colors";
  EXPECT_EQ(frame.At({5, 0.6, 1.5}), frame.At({5, -0.6, 1.5}));

  const Frame labels = this->RenderLabelsFromColors(colored);
  EXPECT_TRUE(labels.data == this->Render(plain).data)
      << "label map from colors differs from the rendered label map";

  engine->DestroyScene(scene);
}

//////////////////////////////////////////////////
// The links of a model are one instance, whatever their names: here the
// name of another model with the same label sorts between them.
TEST_F(SegmentationCameraAccuracyTest, MultiLinkModelNames)
{
  CHECK_SUPPORTED_ENGINE("ogre2");
  ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);

  VisualPtr root = scene->RootVisual();
  VisualPtr robot = scene->CreateVisual("robot");
  robot->SetLocalPosition(5, 0, 0);
  root->AddChild(robot);
  AddBox(scene, robot, "zz_link", {0, 1, 0}, {1, 1, 1}, 3);
  AddBox(scene, robot, "aa_link", {0, -1, 0}, {1, 1, 1}, 3);
  AddBox(scene, root, "mm", {5, 0, 1.5}, {1, 1, 1}, 3);

  for (bool coloredMap : {false, true})
  {
    SCOPED_TRACE(coloredMap ? "colored" : "label map");
    auto camera = this->CreateCamera(scene, SegmentationType::ST_PANOPTIC,
        coloredMap);
    ASSERT_NE(nullptr, camera);
    const Frame frame = coloredMap ? this->RenderLabelsFromColors(camera) :
        this->Render(camera);
    const Pixel link1 = frame.At({5, 1, 0});
    const Pixel link2 = frame.At({5, -1, 0});
    const Pixel other = frame.At({5, 0, 1.5});
    EXPECT_EQ(3, link1[2]);
    EXPECT_EQ(3, other[2]);
    EXPECT_EQ(link1, link2);
    // models in descending name order: robot, then mm
    EXPECT_EQ(1, Instance(link1));
    EXPECT_EQ(2, Instance(other));
    scene->DestroySensor(camera);
  }

  engine->DestroyScene(scene);
}

//////////////////////////////////////////////////
// Label maps hold 8 bit labels: labels outside 0-255 are clamped, the same
// way whether the map is rendered directly or computed from colors. The
// colored map still tells them apart.
TEST_F(SegmentationCameraAccuracyTest, LabelsOutsideLabelMapRange)
{
  CHECK_SUPPORTED_ENGINE("ogre2");
  ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);

  VisualPtr root = scene->RootVisual();
  AddBox(scene, root, "high", {5, 1.5, 0}, {1, 1, 1}, 300);
  AddBox(scene, root, "higher", {5, 0, 0}, {1, 1, 1}, 70000);
  AddBox(scene, root, "negative", {5, -1.5, 0}, {1, 1, 1}, -5);

  for (auto type : {SegmentationType::ST_SEMANTIC,
      SegmentationType::ST_PANOPTIC})
  {
    for (bool coloredMap : {false, true})
    {
      SCOPED_TRACE(std::to_string(static_cast<int>(type)) + " " +
          std::to_string(coloredMap));
      auto camera = this->CreateCamera(scene, type, coloredMap);
      ASSERT_NE(nullptr, camera);
      if (coloredMap)
      {
        const Frame colors = this->Render(camera);
        EXPECT_NE(colors.At({5, 1.5, 0}), colors.At({5, 0, 0}));
      }
      const Frame frame = coloredMap ? this->RenderLabelsFromColors(camera) :
          this->Render(camera);
      EXPECT_EQ(255, frame.At({5, 1.5, 0})[2]);
      EXPECT_EQ(255, frame.At({5, 0, 0})[2]);
      EXPECT_EQ(0, frame.At({5, -1.5, 0})[2]);
      if (type == SegmentationType::ST_PANOPTIC)
      {
        EXPECT_EQ(1, Instance(frame.At({5, 1.5, 0})));
        EXPECT_EQ(1, Instance(frame.At({5, 0, 0})));
        EXPECT_EQ(1, Instance(frame.At({5, -1.5, 0})));
      }
      scene->DestroySensor(camera);
    }
  }

  engine->DestroyScene(scene);
}

//////////////////////////////////////////////////
// Two labels whose random colors are the same still get different colors.
// With the standard library's default engine, labels 0 and 1 are such a
// pair (seeds 0 and 1 give the same sequence).
TEST_F(SegmentationCameraAccuracyTest, ColorCollision)
{
  CHECK_SUPPORTED_ENGINE("ogre2");

  // find two labels with the same first color
  std::unordered_map<int, int> labelOfColor;
  int labelA = -1;
  int labelB = -1;
  for (int label = 0; label < 1000000 && labelA < 0; ++label)
  {
    const Pixel c = SeededColor(label);
    const int id = c[0] * 65536 + c[1] * 256 + c[2];
    auto [it, inserted] = labelOfColor.emplace(id, label);
    if (!inserted)
    {
      labelA = it->second;
      labelB = label;
    }
  }
  ASSERT_GE(labelA, 0);
  ASSERT_EQ(SeededColor(labelA), SeededColor(labelB));

  ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);
  auto camera = this->CreateCamera(scene, SegmentationType::ST_SEMANTIC,
      true);
  ASSERT_NE(nullptr, camera);
  VisualPtr root = scene->RootVisual();
  AddBox(scene, root, "a", {5, 1, 0}, {1, 1, 1}, labelA);
  AddBox(scene, root, "b", {5, -1, 0}, {1, 1, 1}, labelB);

  const Frame frame = this->Render(camera);
  EXPECT_NE(frame.At({5, 1, 0}), frame.At({5, -1, 0}));
  EXPECT_NE(Semantic(kBackground), frame.At({5, 1, 0}));
  EXPECT_NE(Semantic(kBackground), frame.At({5, -1, 0}));

  if (labelB <= 255)
  {
    const Frame labels = this->RenderLabelsFromColors(camera);
    EXPECT_EQ(Semantic(labelA), labels.At({5, 1, 0}));
    EXPECT_EQ(Semantic(labelB), labels.At({5, -1, 0}));
  }

  engine->DestroyScene(scene);
}

//////////////////////////////////////////////////
// A label whose random color is the background color gets another color,
// so its pixels are not taken for background (and the background for it).
TEST_F(SegmentationCameraAccuracyTest, BackgroundColorCollision)
{
  CHECK_SUPPORTED_ENGINE("ogre2");

  // find a label whose first color is gray, the color of a background label
  int label = -1;
  for (int l = 256; l < 10000000 && label < 0; ++l)
  {
    const Pixel c = SeededColor(l);
    if (c[0] == c[1] && c[1] == c[2])
      label = l;
  }
  ASSERT_GT(label, 0);
  const int background = SeededColor(label)[0];

  ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);
  auto camera = this->CreateCamera(scene, SegmentationType::ST_SEMANTIC,
      true);
  ASSERT_NE(nullptr, camera);
  camera->SetBackgroundLabel(background);
  AddBox(scene, scene->RootVisual(), "box", {5, 0, 0}, {1, 1, 1}, label);

  const Frame frame = this->Render(camera);
  EXPECT_EQ(Semantic(background), frame.At(0, 0));
  EXPECT_NE(Semantic(background), frame.At({5, 0, 0}));

  const Frame labels = this->RenderLabelsFromColors(camera);
  EXPECT_EQ(Semantic(background), labels.At(0, 0));
  EXPECT_EQ(Semantic(255), labels.At({5, 0, 0}));

  engine->DestroyScene(scene);
}

//////////////////////////////////////////////////
// Many labels and instances: every color is unique, so the label maps
// computed from the colors match the rendered ones.
TEST_F(SegmentationCameraAccuracyTest, ManyObjects)
{
  CHECK_SUPPORTED_ENGINE("ogre2");
  ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);

  // 25 x 10 small boxes filling the view, with labels 1-251 except the
  // background label
  auto Label = [](int _i) { return _i + 1 < kBackground ? _i + 1 : _i + 2; };
  VisualPtr root = scene->RootVisual();
  for (int i = 0; i < 250; ++i)
  {
    const double y = 4.0 - (i % 25) * (8.0 / 24.0);
    const double z = 2.5 - (i / 25) * (5.0 / 9.0);
    char name[32];
    std::snprintf(name, sizeof(name), "box_%03d", i);
    AddBox(scene, root, name, {5, y, z}, {0.2, 0.2, 0.2}, Label(i));
  }

  for (auto type : {SegmentationType::ST_SEMANTIC,
      SegmentationType::ST_PANOPTIC})
  {
    SCOPED_TRACE(type == SegmentationType::ST_SEMANTIC ? "semantic" :
        "panoptic");
    auto colored = this->CreateCamera(scene, type, true);
    auto plain = this->CreateCamera(scene, type, false);
    ASSERT_NE(nullptr, colored);
    ASSERT_NE(nullptr, plain);

    const Frame frame = this->Render(plain);
    for (int i = 0; i < 250; ++i)
    {
      const double y = 4.0 - (i % 25) * (8.0 / 24.0);
      const double z = 2.5 - (i / 25) * (5.0 / 9.0);
      const Pixel p = frame.At({5, y, z});
      if (type == SegmentationType::ST_SEMANTIC)
      {
        EXPECT_EQ(Semantic(Label(i)), p) << i;
      }
      else
      {
        EXPECT_EQ(Label(i), p[2]) << i;
        EXPECT_EQ(1, Instance(p)) << i;
      }
    }

    const Frame labels = this->RenderLabelsFromColors(colored);
    EXPECT_TRUE(labels.data == frame.data)
        << "label map from colors differs from the rendered label map";
    scene->DestroySensor(colored);
    scene->DestroySensor(plain);
  }

  engine->DestroyScene(scene);
}

//////////////////////////////////////////////////
// The same scene gives the same images, frame after frame, in every mode.
TEST_F(SegmentationCameraAccuracyTest, RepeatedFramesIdentical)
{
  CHECK_SUPPORTED_ENGINE("ogre2");
  ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);

  VisualPtr root = scene->RootVisual();
  for (int i = 0; i < 40; ++i)
  {
    VisualPtr model = scene->CreateVisual("model_" + std::to_string(i));
    model->SetLocalPosition(6 + (i % 3), 3.0 - (i % 8) * 0.8,
        1.2 - (i / 8) * 0.6);
    root->AddChild(model);
    for (int l = 0; l < 2; ++l)
    {
      AddBox(scene, model, "model_" + std::to_string(i) + "::link_" +
          std::to_string(l), {0, l * 0.3, 0}, {0.3, 0.3, 0.3},
          i % 7 == 6 ? std::nullopt : std::optional<int>(1 + i % 5));
    }
  }

  for (auto type : {SegmentationType::ST_SEMANTIC,
      SegmentationType::ST_PANOPTIC})
  {
    for (bool coloredMap : {false, true})
    {
      SCOPED_TRACE(std::to_string(static_cast<int>(type)) + " " +
          std::to_string(coloredMap));
      auto camera = this->CreateCamera(scene, type, coloredMap);
      ASSERT_NE(nullptr, camera);
      const Frame first = this->Render(camera);
      Frame firstLabels;
      if (coloredMap)
        firstLabels = this->RenderLabelsFromColors(camera);
      for (int f = 0; f < 3; ++f)
      {
        EXPECT_TRUE(first.data == this->Render(camera).data) << f;
        if (coloredMap)
        {
          EXPECT_TRUE(firstLabels.data ==
              this->RenderLabelsFromColors(camera).data) << f;
        }
      }
      scene->DestroySensor(camera);
    }
  }

  engine->DestroyScene(scene);
}

//////////////////////////////////////////////////
// Instance ids depend on the scene, not on what is in view: a model with
// the same label moving in and out of view doesn't change them.
TEST_F(SegmentationCameraAccuracyTest, InstancesDontDependOnView)
{
  CHECK_SUPPORTED_ENGINE("ogre2");
  ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);
  auto camera = this->CreateCamera(scene, SegmentationType::ST_PANOPTIC,
      false);
  ASSERT_NE(nullptr, camera);

  VisualPtr root = scene->RootVisual();
  AddBox(scene, root, "m_a", {5, 0, 0}, {1, 1, 1}, 1);
  // behind the camera
  VisualPtr other = AddBox(scene, root, "m_z", {-5, 0, 0}, {1, 1, 1}, 1);

  const Pixel hidden = this->Render(camera).At({5, 0, 0});
  EXPECT_EQ(1, hidden[2]);

  other->SetLocalPosition(5, 2, 0);
  const Frame frame = this->Render(camera);
  EXPECT_EQ(hidden, frame.At({5, 0, 0}));
  EXPECT_EQ(1, frame.At({5, 2, 0})[2]);
  EXPECT_NE(Instance(hidden), Instance(frame.At({5, 2, 0})));

  engine->DestroyScene(scene);
}

//////////////////////////////////////////////////
// Objects all around the camera, some cut by the image borders: those
// partly in view keep their label up to the border.
TEST_F(SegmentationCameraAccuracyTest, ObjectsAllAroundCamera)
{
  CHECK_SUPPORTED_ENGINE("ogre2");
  ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);

  VisualPtr root = scene->RootVisual();
  // a ring of boxes around the camera, at several heights
  for (int i = 0; i < 72; ++i)
  {
    const double angle = i * GZ_PI / 36.0;
    for (int h = -1; h <= 1; ++h)
    {
      AddBox(scene, root, "ring_" + std::to_string(i) + "_" +
          std::to_string(h + 1), {8 * std::cos(angle), 8 * std::sin(angle),
          h * 4.0}, {0.5, 0.5, 0.5}, 100 + i % 50);
    }
  }
  // boxes cut by the left, right, top and bottom borders, with their
  // centers just outside the view: the borders are at y = +-x and
  // z = +-0.75 x
  AddBox(scene, root, "left", {4, 4.3, 0}, {0.5, 1, 0.5}, 1);
  AddBox(scene, root, "right", {4, -4.3, 0}, {0.5, 1, 0.5}, 2);
  AddBox(scene, root, "top", {4, 0, 3.25}, {0.5, 0.5, 1}, 3);
  AddBox(scene, root, "bottom", {4, 0, -3.25}, {0.5, 0.5, 1}, 4);

  for (auto type : {SegmentationType::ST_SEMANTIC,
      SegmentationType::ST_PANOPTIC})
  {
    auto camera = this->CreateCamera(scene, type, false);
    ASSERT_NE(nullptr, camera);
    const Frame frame = this->Render(camera);
    EXPECT_EQ(1, frame.At(0, kHeight / 2)[2]);
    EXPECT_EQ(2, frame.At(kWidth - 1, kHeight / 2)[2]);
    EXPECT_EQ(3, frame.At(kWidth / 2, 0)[2]);
    EXPECT_EQ(4, frame.At(kWidth / 2, kHeight - 1)[2]);
    scene->DestroySensor(camera);
  }

  engine->DestroyScene(scene);
}

//////////////////////////////////////////////////
// Changes between frames are seen in the next frame: a new label, and a
// geometry moved to another visual.
TEST_F(SegmentationCameraAccuracyTest, RelabelAndMoveGeometry)
{
  CHECK_SUPPORTED_ENGINE("ogre2");
  ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);
  auto camera = this->CreateCamera(scene, SegmentationType::ST_SEMANTIC,
      false);
  ASSERT_NE(nullptr, camera);

  VisualPtr root = scene->RootVisual();
  VisualPtr box = AddBox(scene, root, "box", {5, 0, 0}, {1, 1, 1}, 1);
  EXPECT_EQ(Semantic(1), this->Render(camera).At({5, 0, 0}));

  box->SetUserData("label", 2);
  EXPECT_EQ(Semantic(2), this->Render(camera).At({5, 0, 0}));

  VisualPtr other = scene->CreateVisual("other");
  other->SetLocalPosition(5, 0, 0);
  other->SetUserData("label", 3);
  root->AddChild(other);
  GeometryPtr geometry = box->GeometryByIndex(0);
  box->RemoveGeometry(geometry);
  other->AddGeometry(geometry);
  EXPECT_EQ(Semantic(3), this->Render(camera).At({5, 0, 0}));

  engine->DestroyScene(scene);
}

//////////////////////////////////////////////////
// A transparent object is drawn with its exact label: no blending with the
// object behind it.
TEST_F(SegmentationCameraAccuracyTest, TransparentObject)
{
  CHECK_SUPPORTED_ENGINE("ogre2");
  ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);
  auto plain = this->CreateCamera(scene, SegmentationType::ST_SEMANTIC,
      false);
  auto colored = this->CreateCamera(scene, SegmentationType::ST_SEMANTIC,
      true);
  ASSERT_NE(nullptr, plain);
  ASSERT_NE(nullptr, colored);

  MaterialPtr glass = scene->CreateMaterial();
  glass->SetDiffuse(0.2, 0.8, 0.2);
  glass->SetTransparency(0.6);
  VisualPtr root = scene->RootVisual();
  VisualPtr front = AddBox(scene, root, "glass", {4, 0, 0}, {0.8, 0.8, 0.8},
      7);
  front->SetMaterial(glass);
  AddBox(scene, root, "wall", {7, 0, 0}, {0.2, 4, 4}, 9);

  EXPECT_EQ(Semantic(7), this->Render(plain).At({4, 0, 0}));
  EXPECT_EQ(Semantic(9), this->Render(plain).At({7, 1.5, 1}));
  EXPECT_EQ(Semantic(7), this->RenderLabelsFromColors(colored).At({4, 0, 0}));

  // the material is still transparent for other cameras
  EXPECT_DOUBLE_EQ(0.6, glass->Transparency());

  engine->DestroyScene(scene);
}

//////////////////////////////////////////////////
// A point cloud marker (low level material) gets its label.
TEST_F(SegmentationCameraAccuracyTest, PointsMarker)
{
  CHECK_SUPPORTED_ENGINE("ogre2");
  ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);
  auto camera = this->CreateCamera(scene, SegmentationType::ST_SEMANTIC,
      false);
  ASSERT_NE(nullptr, camera);

  MarkerPtr marker = scene->CreateMarker();
  ASSERT_NE(nullptr, marker);
  marker->SetType(MarkerType::MT_POINTS);
  for (int y = 0; y <= 10; ++y)
    marker->AddPoint({5, -1 + y * 0.2, 0}, math::Color::Red);
  VisualPtr vis = scene->CreateVisual("points");
  vis->AddGeometry(marker);
  vis->SetUserData("label", 4);
  scene->RootVisual()->AddChild(vis);

  // points are drawn one pixel wide
  const Frame frame = this->Render(camera);
  EXPECT_TRUE(frame.Near({5, 0, 0}, Semantic(4)));
  EXPECT_TRUE(frame.Near({5, 0.6, 0}, Semantic(4)));
  EXPECT_EQ(Semantic(kBackground), frame.At({5, 0, 1}));

  engine->DestroyScene(scene);
}

//////////////////////////////////////////////////
// A heightmap gets the label of its visual.
TEST_F(SegmentationCameraAccuracyTest, Heightmap)
{
  CHECK_SUPPORTED_ENGINE("ogre2");
  engine->AddResourcePath(
      common::joinPaths(std::string(PROJECT_BUILD_PATH), "src"));
  ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);
  auto camera = this->CreateCamera(scene, SegmentationType::ST_SEMANTIC,
      false);
  ASSERT_NE(nullptr, camera);

  const std::string media = common::joinPaths(
      std::string(PROJECT_SOURCE_PATH), "test", "media");
  auto data = std::make_shared<common::ImageHeightmap>();
  ASSERT_EQ(0, data->Load(common::joinPaths(media, "heightmap_bowl.png")));
  HeightmapDescriptor desc;
  desc.SetName("segmentation_heightmap");
  desc.SetData(data);
  desc.SetSize({40, 40, 6});
  desc.SetPosition({20, 0, -9});
  desc.SetSampling(2u);
  desc.SetUseTerrainPaging(false);
  HeightmapTexture texture;
  texture.SetSize(1.0);
  texture.SetDiffuse(common::joinPaths(media, "materials", "textures",
      "texture.png"));
  texture.SetNormal(common::joinPaths(media, "materials", "textures",
      "flat_normal.png"));
  desc.AddTexture(texture);
  auto heightmap = scene->CreateHeightmap(desc);
  ASSERT_NE(nullptr, heightmap);
  VisualPtr terrain = scene->CreateVisual("terrain");
  terrain->AddGeometry(heightmap);
  terrain->SetUserData("label", 5);
  scene->RootVisual()->AddChild(terrain);

  // a box above the terrain
  AddBox(scene, scene->RootVisual(), "box", {6, 0, 1.5}, {0.5, 0.5, 0.5}, 1);

  const Frame frame = this->Render(camera);
  // the bottom of the image sees the terrain
  EXPECT_EQ(Semantic(5), frame.At(kWidth / 2, kHeight - 5));
  EXPECT_EQ(Semantic(1), frame.At({6, 0, 1.5}));

  engine->DestroyScene(scene);
}

//////////////////////////////////////////////////
// A heightmap that isn't attached to a visual (so it has no label) doesn't
// break the camera: it is background.
TEST_F(SegmentationCameraAccuracyTest, HeightmapWithoutVisual)
{
  CHECK_SUPPORTED_ENGINE("ogre2");
  engine->AddResourcePath(
      common::joinPaths(std::string(PROJECT_BUILD_PATH), "src"));
  ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);

  const std::string media = common::joinPaths(
      std::string(PROJECT_SOURCE_PATH), "test", "media");
  auto data = std::make_shared<common::ImageHeightmap>();
  ASSERT_EQ(0, data->Load(common::joinPaths(media, "heightmap_bowl.png")));
  HeightmapDescriptor desc;
  desc.SetName("detached_heightmap");
  desc.SetData(data);
  desc.SetSize({40, 40, 6});
  desc.SetPosition({20, 0, -9});
  desc.SetSampling(2u);
  desc.SetUseTerrainPaging(false);
  auto heightmap = scene->CreateHeightmap(desc);
  ASSERT_NE(nullptr, heightmap);

  AddBox(scene, scene->RootVisual(), "box", {6, 0, 1.5}, {0.5, 0.5, 0.5}, 1);

  for (auto type : {SegmentationType::ST_SEMANTIC,
      SegmentationType::ST_PANOPTIC})
  {
    for (bool coloredMap : {false, true})
    {
      auto camera = this->CreateCamera(scene, type, coloredMap);
      ASSERT_NE(nullptr, camera);
      const Frame frame = coloredMap ? this->RenderLabelsFromColors(camera) :
          this->Render(camera);
      EXPECT_EQ(1, frame.At({6, 0, 1.5})[2]);
      scene->DestroySensor(camera);
    }
  }

  engine->DestroyScene(scene);
}

//////////////////////////////////////////////////
// Rendering the segmentation camera leaves the scene as it was: a regular
// camera sees exactly the same image before and after.
TEST_F(SegmentationCameraAccuracyTest, OtherCamerasUnchanged)
{
  CHECK_SUPPORTED_ENGINE("ogre2");
  ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);
  scene->SetAmbientLight(0.4, 0.4, 0.4);
  DirectionalLightPtr light = scene->CreateDirectionalLight();
  light->SetDirection(0.5, 0.3, -1);
  scene->RootVisual()->AddChild(light);

  MaterialPtr glass = scene->CreateMaterial();
  glass->SetDiffuse(0.2, 0.2, 0.9);
  glass->SetTransparency(0.5);
  MaterialPtr red = scene->CreateMaterial();
  red->SetDiffuse(0.9, 0.1, 0.1);
  VisualPtr root = scene->RootVisual();
  VisualPtr a = AddBox(scene, root, "a", {4, 0.5, 0}, {1, 1, 1}, 1);
  a->SetMaterial(glass);
  VisualPtr b = AddBox(scene, root, "b", {6, -0.5, 0}, {1.5, 1.5, 1.5}, 2);
  b->SetMaterial(red);
  AddBox(scene, root, "c", {5, -2, 0.5}, {1, 1, 1}, std::nullopt);

  CameraPtr rgb = scene->CreateCamera();
  ASSERT_NE(nullptr, rgb);
  rgb->SetImageWidth(kWidth);
  rgb->SetImageHeight(kHeight);
  rgb->SetAspectRatio(static_cast<double>(kWidth) / kHeight);
  rgb->SetHFOV(GZ_PI / 2.0);
  root->AddChild(rgb);

  Image before = rgb->CreateImage();
  rgb->Capture(before);
  rgb->Capture(before);

  for (auto type : {SegmentationType::ST_SEMANTIC,
      SegmentationType::ST_PANOPTIC})
  {
    for (bool coloredMap : {false, true})
    {
      auto camera = this->CreateCamera(scene, type, coloredMap);
      ASSERT_NE(nullptr, camera);
      this->Render(camera);
      scene->DestroySensor(camera);
    }
  }

  Image after = rgb->CreateImage();
  rgb->Capture(after);
  const size_t bytes = before.MemorySize();
  ASSERT_EQ(bytes, after.MemorySize());
  EXPECT_EQ(0, std::memcmp(before.Data<unsigned char>(),
      after.Data<unsigned char>(), bytes))
      << "the regular camera's image changed";
  EXPECT_DOUBLE_EQ(0.5, glass->Transparency());

  engine->DestroyScene(scene);
}
