/*
 * Copyright (C) 2022 Open Source Robotics Foundation
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
#include "ignition/rendering/ogre2/Ogre2Conversions.hh"

using namespace ignition;
using namespace rendering;

/////////////////////////////////////////////////
TEST(Ogre2ConversionsTest, Vector3)
{
  math::Vector3d gzVec(0.1, 0.2, 0.3);
  auto ogreVec = Ogre2Conversions::Convert(gzVec);
  auto gzVec2 = Ogre2Conversions::Convert(ogreVec);
  EXPECT_EQ(gzVec2, gzVec);
}

/////////////////////////////////////////////////
TEST(Ogre2ConversionsTest, Vector3Inf)
{
  math::Vector3d gzVec(
      std::numeric_limits<float>::infinity(),
      -std::numeric_limits<double>::infinity(),
      std::numeric_limits<float>::max() + 1);

  auto ogreVec = Ogre2Conversions::Convert(gzVec);

  EXPECT_DOUBLE_EQ(std::numeric_limits<float>::max(), ogreVec.x);
  EXPECT_DOUBLE_EQ(-std::numeric_limits<float>::max(), ogreVec.y);
  EXPECT_DOUBLE_EQ(std::numeric_limits<float>::max(), ogreVec.z);
}

/////////////////////////////////////////////////
TEST(Ogre2ConversionsTest, Quaternion)
{
  math::Quaterniond gzQuat(0.1, 0.2, 0.3);
  auto ogreQuat = Ogre2Conversions::Convert(gzQuat);
  auto gzQuat2 = Ogre2Conversions::Convert(ogreQuat);
  EXPECT_EQ(gzQuat2, gzQuat);
}

/////////////////////////////////////////////////
TEST(Ogre2ConversionsTest, QuaternionInf)
{
  math::Quaterniond gzQuat(
      std::numeric_limits<float>::infinity(),
      -std::numeric_limits<double>::infinity(),
      std::numeric_limits<float>::max() + 1,
      -std::numeric_limits<float>::max() - 1);

  auto ogreQuat = Ogre2Conversions::Convert(gzQuat);

  EXPECT_DOUBLE_EQ(std::numeric_limits<float>::max(), ogreQuat.w);
  EXPECT_DOUBLE_EQ(-std::numeric_limits<float>::max(), ogreQuat.x);
  EXPECT_DOUBLE_EQ(std::numeric_limits<float>::max(), ogreQuat.y);
  EXPECT_DOUBLE_EQ(-std::numeric_limits<float>::max(), ogreQuat.z);
}
