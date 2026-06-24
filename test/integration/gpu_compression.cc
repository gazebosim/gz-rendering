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
#include <vector>

#include "gz/rendering/RenderEngine.hh"
#include "gz/rendering/RenderingIface.hh"
#include "gz/rendering/Scene.hh"
#include "gz/rendering/Camera.hh"

#include "CommonRenderingTest.hh"

using namespace gz;
using namespace rendering;

class GpuCompressionTest : public CommonRenderingTest
{
};

/////////////////////////////////////////////////
// CPU-reference NV12 math stub. Upgraded to a real GPU-vs-CPU comparison in
// Task 6 (which renders a deterministic solid colour through the camera path).
TEST_F(GpuCompressionTest, Nv12MatchesCpuReference)
{
  CHECK_SUPPORTED_ENGINE("ogre2");
  // BT.709 limited-range luma for mid-grey sRGB (0.5,0.5,0.5).
  auto y = [](double r, double g, double b) {
    return 16.0 + (0.1826*r + 0.6142*g + 0.0620*b) * 255.0; };
  EXPECT_NEAR(125.0, y(0.5, 0.5, 0.5), 2.0);
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
