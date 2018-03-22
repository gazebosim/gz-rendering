/* * Copyright (C) 2017 Open Source Robotics Foundation
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


#include "ignition/rendering/ShaderParam.hh"

using namespace ignition::rendering;


/////////////////////////////////////////////////
TEST(ShaderParam, InitiallyNone)
{
  ShaderParam p;
  EXPECT_EQ(ShaderParam::PARAM_NONE, p.Type());
}

/////////////////////////////////////////////////
TEST(ShaderParam, FloatType)
{
  ShaderParam p;
  p = 10.0f;
  EXPECT_EQ(ShaderParam::PARAM_FLOAT, p.Type());
  float val;
  EXPECT_TRUE(p.Value(&val));
  EXPECT_FLOAT_EQ(10.0f, val);
}

/////////////////////////////////////////////////
TEST(ShaderParam, IntType)
{
  ShaderParam p;
  p = 10;
  EXPECT_EQ(ShaderParam::PARAM_INT, p.Type());
  int val;
  EXPECT_TRUE(p.Value(&val));
  EXPECT_EQ(10, val);
}

//////////////////////////////////////////////////
int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
