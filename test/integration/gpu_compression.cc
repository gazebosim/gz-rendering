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
#include <cmath>
#include <vector>

#include "gz/rendering/Camera.hh"
#include "gz/rendering/CompressedImage.hh"
#include "gz/rendering/RenderEngine.hh"
#include "gz/rendering/RenderingIface.hh"
#include "gz/rendering/Scene.hh"

#include "CommonRenderingTest.hh"

using namespace gz;
using namespace rendering;

class GpuCompressionTest : public CommonRenderingTest
{
};

/////////////////////////////////////////////////
// End-to-end: selecting IE_NV12 on a Vulkan backend delivers a CompressedImage
// of the right size and colorimetry via the compressed-frame signal; on a
// non-Vulkan backend it falls back to raw and reports unsupported.
TEST_F(GpuCompressionTest, Nv12FrameDelivered)
{
  CHECK_SUPPORTED_ENGINE("ogre2");
  ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);
  scene->SetBackgroundColor(0.2, 0.6, 0.9);
  CameraPtr camera = scene->CreateCamera("camera");
  camera->SetImageWidth(64u);
  camera->SetImageHeight(48u);
  camera->SetImageFormat(PF_R8G8B8A8);
  scene->RootVisual()->AddChild(camera);

  camera->SetImageEncoding(IE_NV12);
  if (!camera->IsEncodingSupported(IE_NV12))
  {
    // Non-Vulkan backend: must fall back to raw and report unsupported.
    EXPECT_EQ(IE_NONE, camera->Encoding());
    engine->DestroyScene(scene);
    GTEST_SKIP() << "IE_NV12 unsupported on this backend (expected on OpenGL)";
  }
  EXPECT_EQ(IE_NV12, camera->Encoding());

  CompressedImage received;
  bool got = false;
  auto conn = camera->ConnectNewCompressedImageFrame(
      [&](const CompressedImage &_img)
      {
        received = _img;             // deep copy out of the transient buffer
        got = true;
      });
  ASSERT_NE(nullptr, conn);

  // Pump frames; with a 1-frame pipeline the first compressed frame arrives
  // within a few Updates.
  for (int i = 0; i < 8 && !got; ++i)
    camera->Update();

  ASSERT_TRUE(got);
  EXPECT_EQ(IE_NV12, received.Encoding());
  EXPECT_EQ(64u * 48u * 3u / 2u, received.Size());
  EXPECT_EQ(1, received.Colorimetry().matrixCoefficients);  // BT.709
  EXPECT_FALSE(received.Colorimetry().fullRange);            // limited
  engine->DestroyScene(scene);
}

/////////////////////////////////////////////////
// GPU NV12 vs CPU reference: render a solid-colour scene through the camera,
// capture the raw RGBA8 via the blocking path (these are the sRGB-encoded
// stored bytes), compute a CPU NV12 using the same BT.709 limited formulas and
// NV12 layout as the shader, then compare GPU vs CPU NV12 per-byte.
TEST_F(GpuCompressionTest, Nv12MatchesCpuReference)
{
  CHECK_SUPPORTED_ENGINE("ogre2");

  ScenePtr scene = engine->CreateScene("scene2");
  ASSERT_NE(nullptr, scene);
  scene->SetBackgroundColor(0.5, 0.5, 0.5);
  CameraPtr camera = scene->CreateCamera("camera2");
  camera->SetImageWidth(64u);
  camera->SetImageHeight(48u);
  camera->SetImageFormat(PF_R8G8B8A8);
  scene->RootVisual()->AddChild(camera);

  camera->SetImageEncoding(IE_NV12);
  if (!camera->IsEncodingSupported(IE_NV12))
  {
    engine->DestroyScene(scene);
    GTEST_SKIP() << "IE_NV12 unsupported on this backend";
  }

  // --- Capture raw RGBA8 from the GPU (these are the sRGB-encoded stored bytes
  //     that Capture() returns, and which texelFetch also reads without decode).
  Image rawImg = camera->CreateImage();
  camera->Capture(rawImg);  // Update() + Copy()

  const unsigned int w = 64u;
  const unsigned int h = 48u;
  const unsigned char *rgba = static_cast<const unsigned char *>(rawImg.Data());

  // --- Compute CPU reference NV12 using the same formulas as the shader.
  // BT.709 limited range; input in [0, 1] (divide stored byte by 255).
  auto rgb2y = [](double r, double g, double b) -> double {
    return 16.0 + (0.1826 * r + 0.6142 * g + 0.0620 * b) * 255.0;
  };
  auto rgb2cb = [](double r, double g, double b) -> double {
    return 128.0 + (-0.1006 * r - 0.3386 * g + 0.4392 * b) * 255.0;
  };
  auto rgb2cr = [](double r, double g, double b) -> double {
    return 128.0 + (0.4392 * r - 0.3989 * g - 0.0403 * b) * 255.0;
  };

  const size_t nv12Size = w * h * 3u / 2u;
  std::vector<unsigned char> cpuNv12(nv12Size, 0);

  // Y plane: one luma byte per pixel.
  for (unsigned int y = 0; y < h; ++y)
  {
    for (unsigned int x = 0; x < w; ++x)
    {
      const unsigned int pixOff = (y * w + x) * 4u;
      double r = rgba[pixOff + 0] / 255.0;
      double g = rgba[pixOff + 1] / 255.0;
      double b = rgba[pixOff + 2] / 255.0;
      double luma = std::clamp(rgb2y(r, g, b), 0.0, 255.0);
      cpuNv12[y * w + x] = static_cast<unsigned char>(luma);
    }
  }

  // CbCr plane: chroma averaged over each 2x2 block.
  for (unsigned int by = 0; by < h / 2u; ++by)
  {
    for (unsigned int bx = 0; bx < w / 2u; ++bx)
    {
      double rSum = 0, gSum = 0, bSum = 0;
      for (unsigned int dy = 0; dy < 2u; ++dy)
      {
        for (unsigned int dx = 0; dx < 2u; ++dx)
        {
          unsigned int px = bx * 2u + dx;
          unsigned int py = by * 2u + dy;
          unsigned int off = (py * w + px) * 4u;
          rSum += rgba[off + 0] / 255.0;
          gSum += rgba[off + 1] / 255.0;
          bSum += rgba[off + 2] / 255.0;
        }
      }
      double ra = rSum * 0.25, ga = gSum * 0.25, ba = bSum * 0.25;
      unsigned char cb = static_cast<unsigned char>(
          std::clamp(rgb2cb(ra, ga, ba), 0.0, 255.0));
      unsigned char cr = static_cast<unsigned char>(
          std::clamp(rgb2cr(ra, ga, ba), 0.0, 255.0));
      unsigned int cBase = w * h + (by * (w / 2u) + bx) * 2u;
      cpuNv12[cBase]     = cb;
      cpuNv12[cBase + 1] = cr;
    }
  }

  // --- Get GPU NV12 via the compressed-frame signal.
  CompressedImage gpuImg;
  bool got = false;
  // Re-set encoding (Capture() above called Update() which called PostRender()
  // once, but with no connection yet; we need another frame).
  auto conn = camera->ConnectNewCompressedImageFrame(
      [&](const CompressedImage &_i) { gpuImg = _i; got = true; });
  ASSERT_NE(nullptr, conn);

  for (int i = 0; i < 8 && !got; ++i)
    camera->Update();
  ASSERT_TRUE(got) << "GPU NV12 frame not received within 8 updates";

  ASSERT_EQ(nv12Size, gpuImg.Size());
  const unsigned char *gpuNv12 =
      static_cast<const unsigned char *>(gpuImg.Data());

  // Compare per byte; allow a small tolerance for integer rounding.
  int maxDiff = 0;
  for (size_t i = 0; i < nv12Size; ++i)
  {
    int diff = std::abs(static_cast<int>(gpuNv12[i]) -
                        static_cast<int>(cpuNv12[i]));
    if (diff > maxDiff)
      maxDiff = diff;
  }

  // Report the max diff for diagnostic purposes.
  std::cout << "[Nv12MatchesCpuReference] GPU-vs-CPU max byte diff: "
            << maxDiff << std::endl;

  // Allow up to 3 LSBs of rounding difference.
  EXPECT_LE(maxDiff, 3)
      << "GPU NV12 differs from CPU reference by more than 3 LSBs. "
      << "If diff ~10-30+, this is the sRGB-linearization issue (texelFetch "
      << "on sRGB texture). Fix: set texSlot.pixelFormat = PFG_RGBA8_UNORM "
      << "in ConvertToNv12().";

  engine->DestroyScene(scene);
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
