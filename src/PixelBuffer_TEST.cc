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

#include <type_traits>
#include <vector>

#include "gz/rendering/Image.hh"
#include "gz/rendering/PixelBuffer.hh"

using namespace gz;
using namespace rendering;

// The whole point of the type: it can be copied but never rebound.
static_assert(std::is_copy_constructible_v<PixelBuffer>);
static_assert(!std::is_copy_assignable_v<PixelBuffer>);
static_assert(!std::is_move_assignable_v<PixelBuffer>);

/////////////////////////////////////////////////
TEST(PixelBufferTest, CallerOwnedBuffer)
{
  std::vector<unsigned char> buffer(36, 0);
  PixelBuffer view(4, 3, PF_R8G8B8, buffer.data(), buffer.size());

  EXPECT_EQ(4u, view.Width());
  EXPECT_EQ(3u, view.Height());
  EXPECT_EQ(PF_R8G8B8, view.Format());
  EXPECT_EQ(36u, view.Size());
  EXPECT_EQ(36u, view.MemorySize());
  EXPECT_EQ(buffer.data(), view.Data());
  EXPECT_TRUE(view.Valid());

  // Writing through the view lands in the caller's buffer.
  view.Data()[5] = 42;
  EXPECT_EQ(42, buffer[5]);

  // A copy of the view shares the same buffer.
  PixelBuffer copy = view;
  EXPECT_EQ(buffer.data(), copy.Data());
  copy.Data()[6] = 43;
  EXPECT_EQ(43, buffer[6]);

  // A larger buffer than needed is fine.
  std::vector<unsigned char> larger(64, 0);
  EXPECT_TRUE(PixelBuffer(4, 3, PF_R8G8B8, larger.data(),
      larger.size()).Valid());
}

/////////////////////////////////////////////////
TEST(PixelBufferTest, Invalid)
{
  std::vector<unsigned char> buffer(36, 0);

  // One byte short
  EXPECT_FALSE(PixelBuffer(4, 3, PF_R8G8B8, buffer.data(), 35).Valid());

  // Null pointer
  EXPECT_FALSE(PixelBuffer(4, 3, PF_R8G8B8, nullptr, 36).Valid());

  // Empty buffer with empty dimensions is not usable either
  EXPECT_FALSE(PixelBuffer(0, 0, PF_R8G8B8, nullptr, 0).Valid());
}

/////////////////////////////////////////////////
TEST(PixelBufferTest, FromImage)
{
  Image image(4, 3, PF_R8G8B8);
  PixelBuffer view(image);

  EXPECT_EQ(image.Width(), view.Width());
  EXPECT_EQ(image.Height(), view.Height());
  EXPECT_EQ(image.Format(), view.Format());
  EXPECT_EQ(image.MemorySize(), view.Size());
  EXPECT_EQ(image.Data(), view.Data());
  EXPECT_TRUE(view.Valid());

  view.Data()[0] = 7;
  EXPECT_EQ(7, image.Data<unsigned char>()[0]);
}
