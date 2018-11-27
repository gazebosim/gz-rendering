/*
 * Copyright (C) 2018 Open Source Robotics Foundation
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


#include <atomic>
#include <iostream>
#include <sstream>
#include <thread>
#include <chrono>
#include <unordered_map>

#include "ignition/rendering/RenderEngine.hh"
#include "ignition/rendering/RenderTarget.hh"
#include "ignition/rendering/RenderingIface.hh"
#include "ignition/rendering/Scene.hh"

using namespace ignition;
using namespace rendering;

/////////////////////////////////////////////////
void RepeatCreateDestroy(const std::string &_renderEngine)
{
  auto engine = rendering::engine(_renderEngine);
  if (!engine)
  {
    std::cout << "Engine '" << _renderEngine << "' is not supported"
              << std::endl;
    return;
  }

  auto scene = engine->CreateScene("scene");
  auto root = scene->RootVisual();

  const size_t numCycles = 4;
  for (size_t j = 0; j < numCycles; ++j) {
    std::cout << j << std::endl;

    // parent visual
    auto parent = scene->CreateVisual("parent");
    // Create a N visuals
    const size_t numVisuals = 5000;
    for (size_t i = 0; i < numVisuals; ++i) {
      std::stringstream ss;
      ss << "child" << i;
      auto child = scene->CreateVisual(ss.str());
      // todo: create different types of geometry
      auto box = scene->CreateBox();
      child->AddGeometry(box);
      parent->AddChild(child);
    }

    // Recursive destroy - all child visuals should also be destroyed
    scene->DestroyVisual(parent, true);
  }

  engine->DestroyScene(scene);
}

int main()
{
  RepeatCreateDestroy("ogre");
}
