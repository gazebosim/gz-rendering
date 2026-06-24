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
#include <cstring>

#include "gz/rendering/CompressedImage.hh"

using namespace gz;
using namespace rendering;

/////////////////////////////////////////////////
TEST(CompressedImageTest, DefaultsAndReserve)
{
  CompressedImage img;
  EXPECT_EQ(IE_NONE, img.Encoding());
  EXPECT_EQ(0u, img.Size());
  EXPECT_EQ(nullptr, img.Data());

  img.SetDimensions(64u, 48u);
  img.SetEncoding(IE_NV12);
  img.SetKeyFrame(true);
  EXPECT_EQ(64u, img.Width());
  EXPECT_EQ(48u, img.Height());
  EXPECT_EQ(IE_NV12, img.Encoding());
  EXPECT_TRUE(img.KeyFrame());

  // Default colorimetry: BT.709 limited, sRGB transfer.
  EXPECT_EQ(1, img.Colorimetry().matrixCoefficients);
  EXPECT_EQ(1, img.Colorimetry().colourPrimaries);
  EXPECT_EQ(13, img.Colorimetry().transferCharacteristics);
  EXPECT_FALSE(img.Colorimetry().fullRange);

  unsigned char *buf = img.Reserve(6u);
  ASSERT_NE(nullptr, buf);
  const unsigned char bytes[6] = {1, 2, 3, 4, 5, 6};
  std::memcpy(buf, bytes, 6u);
  EXPECT_EQ(6u, img.Size());
  EXPECT_EQ(0, std::memcmp(img.Data(), bytes, 6u));

  // Reserve smaller keeps capacity, updates logical size.
  img.Reserve(2u);
  EXPECT_EQ(2u, img.Size());
}

/////////////////////////////////////////////////
TEST(CompressedImageTest, DeepCopyIndependent)
{
  CompressedImage a(8u, 8u, IE_NV12);
  unsigned char *buf = a.Reserve(4u);
  buf[0] = 42u;

  CompressedImage b = a;            // deep copy via ImplPtr
  EXPECT_EQ(4u, b.Size());
  EXPECT_EQ(42u, static_cast<const unsigned char *>(b.Data())[0]);

  a.Reserve(4u)[0] = 7u;            // mutate a
  EXPECT_EQ(42u, static_cast<const unsigned char *>(b.Data())[0]);  // b unaffected
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
