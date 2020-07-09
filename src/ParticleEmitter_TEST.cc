/*
 * Copyright (C) 2020 Open Source Robotics Foundation
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
#include <string>

#include <ignition/common/Console.hh>
#include <ignition/math/Color.hh>

#include "test_config.h"  // NOLINT(build/include)

#include "ignition/rendering/ParticleEmitter.hh"
#include "ignition/rendering/RenderEngine.hh"
#include "ignition/rendering/RenderingIface.hh"
#include "ignition/rendering/Scene.hh"

using namespace ignition;
using namespace rendering;

class ParticleEmitterTest : public testing::Test,
                            public testing::WithParamInterface<const char *>
{
  /// \brief Test basic API
  public: void ParticleEmitter(const std::string &_renderEngine);
};

/////////////////////////////////////////////////
void ParticleEmitterTest::ParticleEmitter(const std::string &_renderEngine)
{
  RenderEngine *engine = rendering::engine(_renderEngine);
  if (!engine)
  {
    igndbg << "Engine '" << _renderEngine << "' is not supported" << std::endl;
    return;
  }

  ScenePtr scene = engine->CreateScene("scene");

  // Create particle emitter.
  ParticleEmitterPtr particleEmitter = scene->CreateParticleEmitter();
  particleEmitter->SetType(EmitterType::EM_BOX);
  particleEmitter->SetEmitterSize({0.2, 0.2, 0.2});
  particleEmitter->SetRate(5.0);
  particleEmitter->SetLocalPose({1, 2, 3, 0, 0, 0});
  particleEmitter->SetDuration(30);
  particleEmitter->SetParticleSize({10, 10, 10});
  particleEmitter->SetLifetime(4);
  particleEmitter->SetMaterial(nullptr);
  particleEmitter->SetVelocityRange(1, 2);
  particleEmitter->SetColorRange(
    ignition::math::Color::Black, ignition::math::Color::White);
  particleEmitter->SetEmitting(true);

  // Clean up
  engine->DestroyScene(scene);
  rendering::unloadEngine(engine->Name());
}

/////////////////////////////////////////////////
TEST_P(ParticleEmitterTest, ParticleEmitter)
{
  ParticleEmitter(GetParam());
}

INSTANTIATE_TEST_CASE_P(ParticleEmitter, ParticleEmitterTest,
    RENDER_ENGINE_VALUES,
    ignition::rendering::PrintToStringParam());

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
