/*
 * Copyright (C) 2017 Open Source Robotics Foundation
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

#include "test_config.h"  // NOLINT(build/include)

#include "ignition/rendering/config.hh"
#include "ignition/rendering/RenderEngine.hh"
#include "ignition/rendering/RenderingIface.hh"

using namespace ignition;
using namespace rendering;

/////////////////////////////////////////////////
// get the number of default engines available
unsigned int defaultEnginesForTest()
{
  unsigned int count = 0u;
#if HAVE_OGRE
  count++;
#endif
#if HAVE_OGRE2
  count++;
#endif
#if HAVE_OPTIX
  count++;
#endif
  return count;
}

/////////////////////////////////////////////////
TEST(RenderingIfaceTest, GetEngine)
{
  unsigned int count = defaultEnginesForTest();

  EXPECT_EQ(count, engineCount());

  // check get engine
  for (unsigned int i = 0; i < count; ++i)
  {
    RenderEngine *eng = engine(i, std::map<std::string, std::string>(),
        IGN_RENDERING_TEST_PLUGIN_PATH);
    EXPECT_NE(nullptr, eng);
    EXPECT_TRUE(hasEngine(eng->Name()));
    EXPECT_EQ(eng, engine(eng->Name()));
#if HAVE_OGRE && HAVE_OGRE2
    // TODO(anyone): ogre and ogre2 cannot be loaded at the same time
    // so for now only test rendering engine API with one ogre version
    if (eng->Name() == "ogre" || eng->Name() == "ogre2")
      ++i;
#endif
    rendering::unloadEngine(eng->Name());
  }

  // non-existent engine
  EXPECT_EQ(nullptr, engine("no_such_engine"));
  EXPECT_EQ(nullptr, engine(1000000));
}

/////////////////////////////////////////////////
TEST(RenderingIfaceTest, RegisterEngine)
{
  unsigned int count = defaultEnginesForTest();

  if (count == 0)
    return;

  // unregister existing engine by index
  RenderEngine *eng = engine(0u);
  EXPECT_TRUE(hasEngine(eng->Name()));
  EXPECT_NO_THROW(unregisterEngine(0u));
  EXPECT_FALSE(hasEngine(eng->Name()));

  // register engine back with a different name
  EXPECT_NO_THROW(registerEngine("my_new_engine", eng));
  EXPECT_TRUE(hasEngine("my_new_engine"));

  // unregister engine by name
  EXPECT_NO_THROW(unregisterEngine("my_new_engine"));
  EXPECT_FALSE(hasEngine("my_new_engine"));

  // unregister non-existent engine
  EXPECT_NO_THROW(unregisterEngine("no_such_engine"));
  EXPECT_NO_THROW(unregisterEngine(1000000u));
  EXPECT_NO_THROW(unregisterEngine(nullptr));
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
