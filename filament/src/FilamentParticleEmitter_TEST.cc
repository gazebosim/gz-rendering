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

#include <gtest/gtest.h>

#include <chrono>
#include <cstdlib>
#include <map>
#include <memory>
#include <string>

#include <gz/common/Filesystem.hh>
#include <gz/common/Image.hh>

#include "gz/rendering/RenderEngine.hh"
#include "gz/rendering/RenderingIface.hh"
#include "gz/rendering/Scene.hh"
#include "gz/rendering/filament/FilamentParticleEmitter.hh"

using namespace gz;
using namespace rendering;

/////////////////////////////////////////////////
// The colour image drives the particle colour and alpha over the lifetime
// like ogre2's ColourImage affector: first row, left to right, linear
TEST(FilamentParticleEmitterTest, ColorRangeImage)
{
  // 3x1 image: red, half transparent green, blue
  const unsigned char px[] = {255, 0, 0, 255, 0, 255, 0, 102, 0, 0, 255, 255};
  const std::string path =
      common::joinPaths(::testing::TempDir(), "particle_colors.png");
  common::Image png;
  png.SetFromData(px, 3, 1, common::Image::RGBA_INT8);
  png.SavePNG(path);

  // through the plugin loader: the engine singleton of the library
  std::map<std::string, std::string> params;
  if (const char *backend = std::getenv("GZ_ENGINE_BACKEND"))
    params["backend"] = backend;
  RenderEngine *engine = rendering::engine("filament", params);
  ASSERT_NE(nullptr, engine);
  {
    ScenePtr scene = engine->CreateScene("scene");
    ASSERT_NE(nullptr, scene);
    auto make = [&](bool _image)
    {
      auto e = std::dynamic_pointer_cast<FilamentParticleEmitter>(
          scene->CreateParticleEmitter());
      EXPECT_NE(nullptr, e);
      e->SetType(EM_POINT);
      e->SetRate(1);
      e->SetLifetime(4);
      e->SetVelocityRange(0, 0);
      e->SetEmitting(true);
      if (_image)
        e->SetColorRangeImage(path);
      return e;
    };
    auto colored = make(true);
    auto plain = make(false);
    EXPECT_EQ(path, colored->ColorRangeImage());

    // one particle per second, lifetime 4 s; the first step only sets the
    // clock. The oldest particle's colour, by age 0..4 s: pixel 0, halfway
    // to pixel 1, pixel 1, halfway to pixel 2, pixel 2
    auto step = [&]
    {
      scene->SetTime(scene->Time() + std::chrono::seconds(1));
      colored->PreRender();
      plain->PreRender();
    };
    step();
    const math::Color expected[] = {math::Color(1, 0, 0, 1),
        math::Color(0.5, 0.5, 0, 0.7), math::Color(0, 1, 0, 0.4),
        math::Color(0, 0.5, 0.5, 0.7), math::Color(0, 0, 1, 1)};
    for (const math::Color &c : expected)
    {
      step();
      ASSERT_FALSE(colored->Particles().empty());
      const math::Color got = colored->Particles()[0].color;
      EXPECT_NEAR(c.R(), got.R(), 1e-5);
      EXPECT_NEAR(c.G(), got.G(), 1e-5);
      EXPECT_NEAR(c.B(), got.B(), 1e-5);
      EXPECT_NEAR(c.A(), got.A(), 1e-5);
    }
    // the first one expired; the next one is now at the end of its life
    step();
    EXPECT_EQ(5u, colored->Particles().size());
    EXPECT_EQ(math::Color::Blue, colored->Particles()[0].color);

    // without an image: white
    for (const auto &p : plain->Particles())
      EXPECT_EQ(math::Color::White, p.color);
    EXPECT_FALSE(plain->Particles().empty());
    colored.reset();
    plain.reset();
    engine->DestroyScene(scene);
  }
  rendering::unloadEngine(engine->Name());
}
