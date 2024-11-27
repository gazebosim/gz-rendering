/*
 * Copyright (C) 2024 Open Source Robotics Foundation
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

#include <gz/common/Console.hh>

#include "test_config.h"  // NOLINT(build/include)

#include "gz/rendering/RenderingIface.hh"
#include "gz/rendering/RenderEngine.hh"

using namespace gz;
using namespace rendering;

TEST(LoadOgre2Test, LoadOgre2)
{
  // Get engine
  auto engine = rendering::engine("ogre2");
  ASSERT_NE(nullptr, engine) << "Unable to load ogre2 render engine"
                             << std::endl;

  // Clean up
  unloadEngine(engine->Name());
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
