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

#include <memory>
#include <vector>

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

/////////////////////////////////////////////////
TEST(ImageTest, ExternalBuffer)
{
  std::vector<unsigned char> buffer(36, 0);

  {
    // The caller keeps ownership: the deleter does nothing.
    Image image(4, 3, PF_R8G8B8,
        Image::DataPtr(buffer.data(), [](unsigned char *) {}));
    EXPECT_EQ(4u, image.Width());
    EXPECT_EQ(3u, image.Height());
    EXPECT_EQ(PF_R8G8B8, image.Format());
    EXPECT_EQ(36u, image.MemorySize());

    // The image uses the caller's buffer instead of allocating its own.
    EXPECT_EQ(buffer.data(), image.Data<unsigned char>());

    // Writing through the image lands in the caller's buffer.
    image.Data<unsigned char>()[5] = 42;
    EXPECT_EQ(42, buffer[5]);

    // A copy of the image shares the same external buffer.
    Image copy = image;
    EXPECT_EQ(buffer.data(), copy.Data<unsigned char>());
    copy.Data<unsigned char>()[6] = 43;
    EXPECT_EQ(43, buffer[6]);
  }

  // The image went out of scope; the caller still owns the buffer and its
  // contents are intact.
  EXPECT_EQ(42, buffer[5]);
  EXPECT_EQ(43, buffer[6]);
}

/////////////////////////////////////////////////
TEST(ImageTest, SharedOwner)
{
  // The buffer belongs to an owner object; the image keeps that owner alive
  // through an aliased shared pointer.
  auto owner = std::make_shared<std::vector<unsigned char>>(36, 7);
  std::weak_ptr<std::vector<unsigned char>> watch = owner;

  Image image(4, 3, PF_R8G8B8, Image::DataPtr(owner, owner->data()));
  EXPECT_EQ(owner->data(), image.Data<unsigned char>());
  EXPECT_EQ(2, owner.use_count());

  // Dropping the caller's reference does not free the buffer.
  owner.reset();
  EXPECT_FALSE(watch.expired());
  EXPECT_EQ(7, image.Data<unsigned char>()[0]);

  // A copy shares the owner too; the buffer lives until the last image goes.
  {
    Image copy = image;
    EXPECT_EQ(image.Data<unsigned char>(), copy.Data<unsigned char>());
  }
  EXPECT_FALSE(watch.expired());
  image = Image(1, 1, PF_R8G8B8);
  EXPECT_TRUE(watch.expired());
}
