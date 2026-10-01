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


#include "gz/rendering/Image.hh"
#include "gz/rendering/PixelFormat.hh"

using namespace gz;
using namespace rendering;

/////////////////////////////////////////////////
TEST(ImageTest, OwnedBuffer)
{
  Image image(4, 3, PF_R8G8B8);
  EXPECT_EQ(4u, image.Width());
  EXPECT_EQ(3u, image.Height());
  EXPECT_EQ(PF_R8G8B8, image.Format());
  EXPECT_EQ(36u, image.MemorySize());
  ASSERT_NE(nullptr, image.Data());

  // The buffer is writable and readable through the typed accessors.
  image.Data<unsigned char>()[0] = 7;
  EXPECT_EQ(7, image.Data<unsigned char>()[0]);
}
