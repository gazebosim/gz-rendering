# Persistent-ticket GPU readback (depth + gpu rays) Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Replace the per-frame `Ogre::Image2::convertFromTexture` GPU→CPU readback in `Ogre2DepthCamera` and `Ogre2GpuRays` with a reusable persistent-`AsyncTextureTicket` helper that maps the staging buffer once and copies in a single stride-aware pass — removing a ~4.9 MB/frame alloc and a redundant full-buffer memcpy, byte-identical and +0 latency.

**Architecture:** A new render-thread-only helper class `Ogre2GpuReadbackTicket` owns one persistent ticket per consumer, recreated only on resolution/format change. Each camera's `PostRender` branches once on an env flag: the **new** path uses the helper + a file-local fused copy; the **legacy** path is the original code kept verbatim as an A/B control. The change lives entirely in `gz-rendering` (the workspace links OGRE-Next from `ogre_vk_prefix/`, not `src/ogre-next`).

**Tech Stack:** C++17, OGRE-Next 2.3.3 (Vulkan), gz-rendering ogre2 component, colcon build, bash equivalence/perf harness under the workspace root.

## Global Constraints

- **gz-rendering only.** `src/ogre-next` is NOT the linked lib (`ogre_vk_prefix/`, built from `~/code/gz/ogre-2.3-release`, is). Do not edit ogre-next.
- **Byte-identical output.** The header-stripped data-md5 oracle MUST match the Task-1 baseline for BOTH the default (new) path and `GZ_RENDERING_OGRE2_LEGACY_READBACK=1`.
- **+0 frame latency.** Synchronous download: `ticket->download(tex, 0u, /*accurateTracking=*/true)` then immediate `map()`. No deferred/async readback in this plan.
- **No numeric pixel conversion.** The bytes are a verbatim 16 B/px reinterpret (`float*`). Never invoke a pixel-format conversion.
- **Row-stride safety.** Always copy row-by-row honoring `box.bytesPerRow` (4-byte aligned, may pad). Never a flat `width*height*bpp` memcpy.
- **Env opt-out:** `GZ_RENDERING_OGRE2_LEGACY_READBACK` (any value) selects legacy; unset = new path (default on). Read once.
- **Style:** Apache-2.0 license header on new files; gz style (2-space indent, per-member `public:`/`private:`, `/// \brief` docs).
- **Build:** `cd /home/jrivero/code/gz/ws_ogre2_cpu_issues && source install/setup.bash && CMAKE_BUILD_PARALLEL_LEVEL=5 colcon build --packages-select gz-rendering --cmake-args -DCMAKE_BUILD_TYPE=RelWithDebInfo -DBUILD_TESTING=ON`
- **Benchmarks must attach subscribers** (gz-sim disables sub-less sensors).
- **Commits:** `git ci` on branch `jrivero/remove_dup_image_copy`; end messages with `Co-Authored-By: Claude Opus 4.8 <noreply@anthropic.com>`. Do NOT push (ask first).

---

## Task 1: Establish a green baseline (control)

No production code change. Capture the unmodified build's depth output as the authoritative baseline (avoids cross-branch drift vs the stale `stage1_offthread/golden`, which was generated on a gz-sensors branch).

**Files:**
- Create: `/home/jrivero/code/gz/ws_ogre2_cpu_issues/gpu_readback_bench/baseline_depth/` (capture output)

**Interfaces:**
- Consumes: existing `stage1_offthread/capture.sh`, `compare.sh`, `perf8.sh`, `stage0_rgbd/rgbd8.sdf`.
- Produces: `gpu_readback_bench/baseline_depth/{depth,points,image}.datamd5` — the equivalence target for Tasks 3.

- [ ] **Step 1: Build the current (unmodified) gz-rendering**

```bash
cd /home/jrivero/code/gz/ws_ogre2_cpu_issues
source install/setup.bash
CMAKE_BUILD_PARALLEL_LEVEL=5 colcon build --packages-select gz-rendering \
  --cmake-args -DCMAKE_BUILD_TYPE=RelWithDebInfo -DBUILD_TESTING=ON
```
Expected: `Finished <<< gz-rendering` with no errors.

- [ ] **Step 2: Capture the depth baseline from unmodified code**

```bash
cd /home/jrivero/code/gz/ws_ogre2_cpu_issues
mkdir -p gpu_readback_bench
bash stage1_offthread/capture.sh gpu_readback_bench/baseline_depth
```
Expected: prints `depth: data=<hash> ...`, `points: data=<hash> ...`, `image: data=<hash> ...` and the three `.datamd5` files exist and are non-empty:
```bash
for f in depth points image; do
  test -s gpu_readback_bench/baseline_depth/$f.datamd5 && echo "$f OK: $(cat gpu_readback_bench/baseline_depth/$f.datamd5)"
done
```
Expected: three `... OK: <32-hex>` lines.

- [ ] **Step 3: Confirm the oracle is self-consistent (re-capture matches)**

```bash
cd /home/jrivero/code/gz/ws_ogre2_cpu_issues
bash stage1_offthread/capture.sh gpu_readback_bench/baseline_depth_recheck
bash stage1_offthread/compare.sh gpu_readback_bench/baseline_depth_recheck gpu_readback_bench/baseline_depth
```
Expected: `depth: MATCH`, `points: MATCH`, `image: MATCH` and exit code 0 (`echo $?` → 0). This proves the static-scene oracle is deterministic before we change anything. If any line is MISMATCH, stop — the harness is non-deterministic and must be fixed first.

- [ ] **Step 4: Record the before-perf number**

```bash
cd /home/jrivero/code/gz/ws_ogre2_cpu_issues
bash stage1_offthread/perf8.sh gpu_readback_bench/perf_before_legacy_default 2>&1 | tee gpu_readback_bench/perf_before.txt
```
Expected: a `RESULT` line with per-cam avg Hz / aggregate / serverCPU / GPU / VRAM. Keep this file; it is the "before" for Task 5.

- [ ] **Step 5: Commit the baseline metadata (not the large .txt dumps)**

```bash
cd /home/jrivero/code/gz/ws_ogre2_cpu_issues/src/gz-rendering
# baseline lives outside the repo (workspace root); record its hashes in-repo for provenance
mkdir -p docs/superpowers/plans
{ echo "# Readback baseline $(date -u +%FT%TZ)"; echo;
  echo "Depth oracle (gpu_readback_bench/baseline_depth):";
  for f in depth points image; do echo "- $f: $(cat /home/jrivero/code/gz/ws_ogre2_cpu_issues/gpu_readback_bench/baseline_depth/$f.datamd5)"; done; } \
  > docs/superpowers/plans/2026-06-21-readback-baseline.md
git add docs/superpowers/plans/2026-06-21-readback-baseline.md
git ci -q -m "test: record unmodified depth-readback baseline hashes

Co-Authored-By: Claude Opus 4.8 <noreply@anthropic.com>"
```
Expected: one commit created (`git log --oneline -1`).

---

## Task 2: The `Ogre2GpuReadbackTicket` helper

A persistent-ticket readback wrapper + the env-flag accessor. Built but not yet wired, so the only gate is that gz-rendering still compiles and links with the new translation unit (auto-collected by `gz_get_libsources_and_unittests`, which globs `ogre2/src/*.cc` — no CMake edit).

**Files:**
- Create: `ogre2/src/Ogre2GpuReadbackTicket.hh`
- Create: `ogre2/src/Ogre2GpuReadbackTicket.cc`

**Interfaces:**
- Consumes: `Ogre2RenderEngine::Instance()->OgreRoot()->getRenderSystem()->getTextureGpuManager()`; OGRE `AsyncTextureTicket`, `TextureGpuManager`, `TextureGpu`, `TextureBox`.
- Produces (used by Tasks 3 & 4):
  - `class gz::rendering::Ogre2GpuReadbackTicket` with `Ogre::TextureBox DownloadAndMap(Ogre::TextureGpu *)`, `void Unmap()`, `void Destroy()`, destructor.
  - free function `bool gz::rendering::Ogre2UseLegacyReadback()`.

- [ ] **Step 1: Create the header `ogre2/src/Ogre2GpuReadbackTicket.hh`**

```cpp
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
#ifndef GZ_RENDERING_OGRE2_OGRE2GPUREADBACKTICKET_HH_
#define GZ_RENDERING_OGRE2_OGRE2GPUREADBACKTICKET_HH_

#include "gz/rendering/config.hh"

#ifdef _MSC_VER
  #pragma warning(push, 0)
#endif
#include <OgrePixelFormatGpu.h>
#include <OgreTextureBox.h>
#ifdef _MSC_VER
  #pragma warning(pop)
#endif

namespace Ogre
{
  class AsyncTextureTicket;
  class TextureGpu;
}

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    //
    /// \brief Reusable GPU->CPU texture readback backed by a single, persistent
    /// Ogre::AsyncTextureTicket. Unlike Ogre::Image2::convertFromTexture (which
    /// allocates a fresh staging buffer + ticket and an extra copy every call),
    /// this keeps one ticket and maps its staging memory directly, so callers
    /// copy once straight into their own persistent buffer.
    /// Render-thread use only; NOT thread-safe.
    class Ogre2GpuReadbackTicket
    {
      /// \brief Destructor; destroys the persistent ticket if still alive.
      public: ~Ogre2GpuReadbackTicket();

      /// \brief Issue a blocking GPU->CPU download of \p _texture (mip 0,
      /// slice 0) into the persistent ticket and map it for reading. The
      /// ticket is lazily created and recreated whenever the texture's
      /// width/height/pixel-format changes. The caller MUST read the data out
      /// honoring TextureBox::bytesPerRow and then call Unmap() before the next
      /// frame's DownloadAndMap().
      /// \param[in] _texture Source GPU texture.
      /// \return Mapped TextureBox; box.data is nullptr on failure.
      public: Ogre::TextureBox DownloadAndMap(Ogre::TextureGpu *_texture);

      /// \brief Unmap the ticket previously mapped by DownloadAndMap().
      /// Safe to call when nothing is mapped.
      public: void Unmap();

      /// \brief Destroy the persistent ticket now. Call from the owner's
      /// Destroy() while the render engine (and its texture manager) is still
      /// alive, ahead of engine teardown.
      public: void Destroy();

      /// \brief The persistent ticket, or nullptr before first use / after
      /// Destroy().
      private: Ogre::AsyncTextureTicket *ticket{nullptr};

      /// \brief Cached ticket geometry; a change forces recreation.
      private: unsigned int width{0u};

      /// \brief Cached ticket height; see width.
      private: unsigned int height{0u};

      /// \brief Cached ticket pixel format; see width.
      private: Ogre::PixelFormatGpu format{Ogre::PFG_UNKNOWN};

      /// \brief True while the ticket is currently mapped.
      private: bool mapped{false};
    };

    /// \brief Whether to use the legacy Ogre::Image2::convertFromTexture
    /// readback path instead of the persistent-ticket path. True iff the env
    /// var GZ_RENDERING_OGRE2_LEGACY_READBACK is set. Evaluated once.
    /// \return true to use the legacy path.
    bool Ogre2UseLegacyReadback();
    }
  }
}
#endif
```

- [ ] **Step 2: Create the implementation `ogre2/src/Ogre2GpuReadbackTicket.cc`**

```cpp
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
#include "Ogre2GpuReadbackTicket.hh"

#include <cstdlib>

#include "gz/rendering/ogre2/Ogre2RenderEngine.hh"

#ifdef _MSC_VER
  #pragma warning(push, 0)
#endif
#include <OgreRoot.h>
#include <OgreRenderSystem.h>
#include <OgreTextureGpu.h>
#include <OgreTextureGpuManager.h>
#include <OgreAsyncTextureTicket.h>
#ifdef _MSC_VER
  #pragma warning(pop)
#endif

using namespace gz;
using namespace rendering;

//////////////////////////////////////////////////
/// \brief Fetch the active OGRE texture manager, or nullptr if unavailable.
static Ogre::TextureGpuManager *ReadbackTextureManager()
{
  auto engine = Ogre2RenderEngine::Instance();
  if (!engine)
    return nullptr;
  auto ogreRoot = engine->OgreRoot();
  if (!ogreRoot || !ogreRoot->getRenderSystem())
    return nullptr;
  return ogreRoot->getRenderSystem()->getTextureGpuManager();
}

//////////////////////////////////////////////////
Ogre2GpuReadbackTicket::~Ogre2GpuReadbackTicket()
{
  this->Destroy();
}

//////////////////////////////////////////////////
void Ogre2GpuReadbackTicket::Destroy()
{
  if (!this->ticket)
    return;

  if (this->mapped)
  {
    this->ticket->unmap();
    this->mapped = false;
  }

  auto textureMgr = ReadbackTextureManager();
  if (textureMgr)
    textureMgr->destroyAsyncTextureTicket(this->ticket);

  this->ticket = nullptr;
  this->width = 0u;
  this->height = 0u;
  this->format = Ogre::PFG_UNKNOWN;
}

//////////////////////////////////////////////////
Ogre::TextureBox Ogre2GpuReadbackTicket::DownloadAndMap(
    Ogre::TextureGpu *_texture)
{
  Ogre::TextureBox box;  // default-constructed: data == nullptr
  if (!_texture)
    return box;

  auto textureMgr = ReadbackTextureManager();
  if (!textureMgr)
    return box;

  const unsigned int w = _texture->getWidth();
  const unsigned int h = _texture->getHeight();
  const Ogre::PixelFormatGpu fmt = _texture->getPixelFormat();

  // (Re)create the ticket on first use or when the geometry/format changed.
  if (!this->ticket || this->width != w || this->height != h ||
      this->format != fmt)
  {
    if (this->ticket)
    {
      if (this->mapped)
      {
        this->ticket->unmap();
        this->mapped = false;
      }
      textureMgr->destroyAsyncTextureTicket(this->ticket);
      this->ticket = nullptr;
    }
    this->ticket = textureMgr->createAsyncTextureTicket(
        w, h, 1u, Ogre::TextureTypes::Type2D, fmt);
    this->width = w;
    this->height = h;
    this->format = fmt;
  }

  if (!this->ticket)
    return box;

  // Blocking download (accurateTracking=true): map() waits on the fence.
  this->ticket->download(_texture, 0u, true);
  box = this->ticket->map(0u);
  this->mapped = true;
  return box;
}

//////////////////////////////////////////////////
void Ogre2GpuReadbackTicket::Unmap()
{
  if (this->ticket && this->mapped)
  {
    this->ticket->unmap();
    this->mapped = false;
  }
}

//////////////////////////////////////////////////
bool gz::rendering::Ogre2UseLegacyReadback()
{
  static const bool legacy =
      (std::getenv("GZ_RENDERING_OGRE2_LEGACY_READBACK") != nullptr);
  return legacy;
}
```

- [ ] **Step 3: Build to confirm the helper compiles & links**

```bash
cd /home/jrivero/code/gz/ws_ogre2_cpu_issues && source install/setup.bash
CMAKE_BUILD_PARALLEL_LEVEL=5 colcon build --packages-select gz-rendering \
  --cmake-args -DCMAKE_BUILD_TYPE=RelWithDebInfo -DBUILD_TESTING=ON
```
Expected: `Finished <<< gz-rendering`. Confirm the TU was compiled:
```bash
find build/gz-rendering -name 'Ogre2GpuReadbackTicket*.o' | head
```
Expected: one `.o` path printed (proves the glob picked it up — no CMake edit needed).

- [ ] **Step 4: Commit**

```bash
cd /home/jrivero/code/gz/ws_ogre2_cpu_issues/src/gz-rendering
git add ogre2/src/Ogre2GpuReadbackTicket.hh ogre2/src/Ogre2GpuReadbackTicket.cc
git ci -q -m "ogre2: add persistent-ticket GPU readback helper

Ogre2GpuReadbackTicket keeps one AsyncTextureTicket and maps its staging
buffer directly, replacing the per-frame Image2 alloc + extra copy of
Ogre::Image2::convertFromTexture. Gated by Ogre2UseLegacyReadback()
(GZ_RENDERING_OGRE2_LEGACY_READBACK). Not yet wired.

Co-Authored-By: Claude Opus 4.8 <noreply@anthropic.com>"
```
Expected: one commit.

---

## Task 3: Wire `Ogre2DepthCamera` (new default + legacy control)

Replace the readback in `PostRender` with a branch, add a fused single-pass copy, store the helper in the private data, and destroy it in `Destroy()`. Gate: depth oracle MATCHES the Task-1 baseline for BOTH the new and legacy paths, and the depth integration test passes.

**Files:**
- Modify: `ogre2/src/Ogre2DepthCamera.cc` (include ~line 44; private member ~line 99; `Destroy()` ~line 304; `PostRender()` 1162-1260)
- Test (existing, runnable): `test/integration/depth_camera.cc` via CTest

**Interfaces:**
- Consumes: `Ogre2GpuReadbackTicket`, `Ogre2UseLegacyReadback()` (Task 2).
- Produces: unchanged public behavior of `Ogre2DepthCamera::PostRender` / `DepthData()`.

- [ ] **Step 1: Add the helper include**

In `ogre2/src/Ogre2DepthCamera.cc`, next to the existing sibling-header include `#include "Ogre2ParticleNoiseListener.hh"` (line 44), add:
```cpp
#include "Ogre2GpuReadbackTicket.hh"
```

- [ ] **Step 2: Add the persistent-ticket member to the private data**

In `Ogre2DepthCameraPrivate` (after the `depthImage` member at line 102), add:
```cpp
  /// \brief Persistent GPU->CPU readback ticket used by the non-legacy
  /// PostRender path.
  public: Ogre2GpuReadbackTicket depthReadback;
```

- [ ] **Step 3: Add the file-local fused-copy helper**

Immediately above `void Ogre2DepthCamera::PostRender()` (line 1162), add an anonymous-namespace function:
```cpp
namespace
{
  /// \brief Copy one mapped RGBA32 readback box into the persistent depth
  /// buffers in a single stride-aware pass: the full RGBA row into
  /// _depthBuffer (point cloud + DepthData()) and channel 0 into _depthImage
  /// (depth frame). Honors _box.bytesPerRow as the source stride.
  void FillDepthBuffers(const Ogre::TextureBox &_box,
      float *_depthBuffer, float *_depthImage,
      unsigned int _width, unsigned int _height,
      unsigned int _channelCount, unsigned int _bytesPerChannel)
  {
    const float *src = static_cast<const float *>(_box.data);
    for (unsigned int i = 0; i < _height; ++i)
    {
      const unsigned int rawRowIdx = i * _box.bytesPerRow / _bytesPerChannel;
      const unsigned int rowIdx = i * _width * _channelCount;
      memcpy(&_depthBuffer[rowIdx], &src[rawRowIdx],
          _width * _channelCount * _bytesPerChannel);
      for (unsigned int j = 0; j < _width; ++j)
        _depthImage[i * _width + j] = src[rawRowIdx + j * _channelCount];
    }
  }
}  // namespace
```

- [ ] **Step 4: Replace the body of `PostRender()` (lines 1162-1260)**

Replace the entire existing function with:
```cpp
//////////////////////////////////////////////////
void Ogre2DepthCamera::PostRender()
{
  GZ_PROFILE("Ogre2DepthCamera::PostRender");
  unsigned int width = this->ImageWidth();
  unsigned int height = this->ImageHeight();

  PixelFormat format = PF_FLOAT32_RGBA;

  int len = width * height;
  unsigned int channelCount = PixelUtil::ChannelCount(format);
  unsigned int bytesPerChannel = PixelUtil::BytesPerChannel(format);

  if (!this->dataPtr->depthBuffer)
    this->dataPtr->depthBuffer = new float[len * channelCount];
  if (!this->dataPtr->depthImage)
    this->dataPtr->depthImage = new float[len];

  if (Ogre2UseLegacyReadback())
  {
    // Legacy A/B control: original Ogre::Image2 path, kept verbatim.
    Ogre::Image2 image;
    image.convertFromTexture(this->dataPtr->ogreDepthTexture[1], 0u, 0u);
    Ogre::TextureBox box = image.getData(0);
    float *depthBufferTmp = static_cast<float *>(box.data);

    // copy data row by row. The texture box may not be a contiguous region of
    // a texture
    for (unsigned int i = 0; i < height; ++i)
    {
      unsigned int rawDataRowIdx = i * box.bytesPerRow / bytesPerChannel;
      unsigned int rowIdx = i * width * channelCount;
      memcpy(&this->dataPtr->depthBuffer[rowIdx],
          &depthBufferTmp[rawDataRowIdx],
          width * channelCount * bytesPerChannel);
    }

    // fill depth data
    for (unsigned int i = 0; i < height; ++i)
    {
      unsigned int step = i*width*channelCount;
      for (unsigned int j = 0; j < width; ++j)
      {
        float x = this->dataPtr->depthBuffer[step + j*channelCount];
        this->dataPtr->depthImage[i*width + j] = x;
      }
    }
  }
  else
  {
    // Persistent-ticket path: map the staging buffer once and copy straight
    // into the persistent buffers in a single fused pass.
    Ogre::TextureBox box = this->dataPtr->depthReadback.DownloadAndMap(
        this->dataPtr->ogreDepthTexture[1]);
    if (!box.data)
    {
      gzerr << "Ogre2DepthCamera: GPU readback failed; dropping frame"
            << std::endl;
      return;
    }
    FillDepthBuffers(box, this->dataPtr->depthBuffer,
        this->dataPtr->depthImage, width, height, channelCount,
        bytesPerChannel);
    this->dataPtr->depthReadback.Unmap();
  }

  this->dataPtr->newDepthFrame(
        this->dataPtr->depthImage, width, height, 1, "FLOAT32");

  // point cloud data
  if (this->dataPtr->newRgbPointCloud.ConnectionCount() > 0u)
  {
    this->dataPtr->newRgbPointCloud(
        this->dataPtr->depthBuffer, width, height, channelCount,
        "PF_FLOAT32_RGBA");
  }
}
```

- [ ] **Step 5: Destroy the ticket in `Destroy()`**

In `Ogre2DepthCamera::Destroy()` (line 304), add the ticket teardown right after `this->RemoveAllRenderPasses();` (line 306) — while the engine is still alive:
```cpp
  this->dataPtr->depthReadback.Destroy();
```

- [ ] **Step 6: Build**

```bash
cd /home/jrivero/code/gz/ws_ogre2_cpu_issues && source install/setup.bash
CMAKE_BUILD_PARALLEL_LEVEL=5 colcon build --packages-select gz-rendering \
  --cmake-args -DCMAKE_BUILD_TYPE=RelWithDebInfo -DBUILD_TESTING=ON
```
Expected: `Finished <<< gz-rendering`.

- [ ] **Step 7: Equivalence — NEW path matches baseline (this is the test that can fail)**

```bash
cd /home/jrivero/code/gz/ws_ogre2_cpu_issues
bash stage1_offthread/capture.sh gpu_readback_bench/new_depth
bash stage1_offthread/compare.sh gpu_readback_bench/new_depth gpu_readback_bench/baseline_depth; echo "exit=$?"
```
Expected: `depth: MATCH`, `points: MATCH`, `image: MATCH`, `exit=0`. A MISMATCH means a stride/format/unmap bug — debug before proceeding (most likely `bytesPerRow` handling or reading after `Unmap()`).

- [ ] **Step 8: Equivalence — LEGACY path also matches baseline**

```bash
cd /home/jrivero/code/gz/ws_ogre2_cpu_issues
GZ_RENDERING_OGRE2_LEGACY_READBACK=1 bash stage1_offthread/capture.sh gpu_readback_bench/legacy_depth
bash stage1_offthread/compare.sh gpu_readback_bench/legacy_depth gpu_readback_bench/baseline_depth; echo "exit=$?"
```
Expected: all `MATCH`, `exit=0` (proves the legacy branch is a faithful verbatim control).

- [ ] **Step 9: Run the existing depth-camera integration test (regression backstop)**

```bash
cd /home/jrivero/code/gz/ws_ogre2_cpu_issues
source install/setup.bash
ctest --test-dir build/gz-rendering -R "INTEGRATION_depth_camera" --output-on-failure
```
Expected: `100% tests passed`. (If the test runs all engines, the ogre2 cases exercise the new default path.)

- [ ] **Step 10: Commit**

```bash
cd /home/jrivero/code/gz/ws_ogre2_cpu_issues/src/gz-rendering
git add ogre2/src/Ogre2DepthCamera.cc
git ci -q -m "ogre2: use persistent-ticket readback in Ogre2DepthCamera

PostRender now maps a persistent AsyncTextureTicket and fills depthBuffer +
depthImage in one stride-aware pass, removing the per-frame Image2 alloc and
a redundant full-buffer memcpy. GZ_RENDERING_OGRE2_LEGACY_READBACK restores
the original path. Byte-identical (data-md5 oracle MATCH, new + legacy).

Co-Authored-By: Claude Opus 4.8 <noreply@anthropic.com>"
```
Expected: one commit.

---

## Task 4: Wire `Ogre2GpuRays` + create the gpu-rays equivalence oracle

No ogre2 lidar data-md5 oracle exists yet, so build one (and prove it detects divergence) before trusting the gpu-rays equivalence result. Then apply the same persistent-ticket pattern to `Ogre2GpuRays::PostRender`.

**Files:**
- Create: `/home/jrivero/code/gz/ws_ogre2_cpu_issues/gpu_readback_bench/gpulidar_one.sdf`
- Create: `/home/jrivero/code/gz/ws_ogre2_cpu_issues/gpu_readback_bench/capture_lidar.sh`
- Create: `/home/jrivero/code/gz/ws_ogre2_cpu_issues/gpu_readback_bench/compare_lidar.sh`
- Modify: `ogre2/src/Ogre2GpuRays.cc` (include ~line 58; private member ~line 134; `Destroy()` ~line 587; `PostRender()` 1304-1349)
- Test (existing, runnable): `test/integration/gpu_rays.cc` via CTest

**Interfaces:**
- Consumes: `Ogre2GpuReadbackTicket`, `Ogre2UseLegacyReadback()` (Task 2).
- Produces: unchanged public behavior of `Ogre2GpuRays::PostRender` / `Data()` / `Copy()`.

- [ ] **Step 1: Create the gpu_lidar world `gpu_readback_bench/gpulidar_one.sdf`**

```xml
<?xml version="1.0"?>
<sdf version="1.9">
  <world name="gpulidar_one">
    <plugin filename="gz-sim-physics-system" name="gz::sim::systems::Physics"/>
    <plugin filename="gz-sim-sensors-system" name="gz::sim::systems::Sensors">
      <render_engine>ogre2</render_engine>
    </plugin>
    <plugin filename="gz-sim-scene-broadcaster-system" name="gz::sim::systems::SceneBroadcaster"/>
    <light type="directional" name="sun">
      <cast_shadows>false</cast_shadows>
      <pose>0 0 10 0 0 0</pose>
      <diffuse>1 1 1 1</diffuse>
      <direction>-0.5 0.1 -0.9</direction>
    </light>
    <model name="ground">
      <static>true</static>
      <link name="l">
        <visual name="v"><geometry><plane><normal>0 0 1</normal><size>40 40</size></plane></geometry></visual>
      </link>
    </model>
    <model name="boxA"><static>true</static><pose>4 0 0.5 0 0 0</pose>
      <link name="l"><visual name="v"><geometry><box><size>1 1 1</size></box></geometry></visual></link>
    </model>
    <model name="boxB"><static>true</static><pose>2 2 0.5 0 0 0.6</pose>
      <link name="l"><visual name="v"><geometry><box><size>1 1 1</size></box></geometry></visual></link>
    </model>
    <model name="boxC"><static>true</static><pose>3 -2.5 0.75 0 0 0</pose>
      <link name="l"><visual name="v"><geometry><box><size>0.5 0.5 1.5</size></box></geometry></visual></link>
    </model>
    <model name="lidar_model">
      <static>true</static>
      <pose>0 0 0.6 0 0 0</pose>
      <link name="l">
        <sensor name="gpu_lidar" type="gpu_lidar">
          <topic>lidar</topic>
          <update_rate>30</update_rate>
          <always_on>1</always_on>
          <lidar>
            <scan>
              <horizontal><samples>320</samples><resolution>1</resolution><min_angle>-1.396</min_angle><max_angle>1.396</max_angle></horizontal>
              <vertical><samples>16</samples><resolution>1</resolution><min_angle>-0.26</min_angle><max_angle>0.26</max_angle></vertical>
            </scan>
            <range><min>0.1</min><max>30</max><resolution>0.01</resolution></range>
          </lidar>
        </sensor>
      </link>
    </model>
  </world>
</sdf>
```

- [ ] **Step 2: Create `gpu_readback_bench/capture_lidar.sh`** (mirrors `stage1_offthread/capture.sh`)

```bash
#!/usr/bin/env bash
# capture_lidar.sh OUTDIR — runs gpulidar_one.sdf headless, captures N /lidar
# messages, writes a header-stripped data-md5 oracle. Honors
# GZ_RENDERING_OGRE2_LEGACY_READBACK from the caller's env.
set -uo pipefail
WS=/home/jrivero/code/gz/ws_ogre2_cpu_issues
cd "$WS"; set +u; source install/setup.bash; set -u
OUT="${1:?usage: capture_lidar.sh OUTDIR}"; mkdir -p "$OUT"
N="${CAP_N:-20}"
export GZ_PARTITION="lidarcap$$"
timeout 40 gz topic -e -t /lidar -n "$N" > "$OUT/lidar.txt" 2>/dev/null &
P1=$!
sleep 1
gz sim -v1 -s -r --render-engine ogre2 "$WS/gpu_readback_bench/gpulidar_one.sdf" \
  > "$OUT/server.log" 2>&1 &
SIM=$!
wait "$P1"
kill "$SIM" 2>/dev/null
pkill -x gz-sim-main 2>/dev/null
sleep 1
# Drop the top-level header{...} block (stamp/seq vary); keep ranges/intensities.
norm() { awk '/^header \{/{skip=1} skip&&/^\}/{skip=0; next} skip{next} {print}' "$1"; }
lines=$(wc -l < "$OUT/lidar.txt")
md5sum "$OUT/lidar.txt" | awk '{print $1}' > "$OUT/lidar.md5"
norm "$OUT/lidar.txt" | md5sum | awk '{print $1}' > "$OUT/lidar.datamd5"
echo "lidar: data=$(cat "$OUT/lidar.datamd5")  raw=$(cat "$OUT/lidar.md5")  ($lines lines)"
```
Then `chmod +x gpu_readback_bench/capture_lidar.sh`.

- [ ] **Step 3: Create `gpu_readback_bench/compare_lidar.sh`** (mirrors `compare.sh`)

```bash
#!/usr/bin/env bash
# compare_lidar.sh DIR_A DIR_B — exit 0 iff /lidar data payloads match.
set -uo pipefail
A="${1:?usage: compare_lidar.sh DIR_A DIR_B}"; B="${2:?usage: compare_lidar.sh DIR_A DIR_B}"
a=$(cat "$A/lidar.datamd5" 2>/dev/null); b=$(cat "$B/lidar.datamd5" 2>/dev/null)
if [ -n "$a" ] && [ "$a" = "$b" ]; then echo "lidar: MATCH ($a)"; exit 0; fi
echo "lidar: MISMATCH (A=$a B=$b)"; exit 1
```
Then `chmod +x gpu_readback_bench/compare_lidar.sh`.

- [ ] **Step 4: Capture the gpu-rays baseline from the CURRENT (unmodified) build**

(gz-rendering currently has the depth change from Task 3 but `Ogre2GpuRays` is still original — a valid baseline for the gpu-rays change.)
```bash
cd /home/jrivero/code/gz/ws_ogre2_cpu_issues
bash gpu_readback_bench/capture_lidar.sh gpu_readback_bench/baseline_lidar
test -s gpu_readback_bench/baseline_lidar/lidar.datamd5 && echo "baseline: $(cat gpu_readback_bench/baseline_lidar/lidar.datamd5)"
```
Expected: a non-empty `lidar.datamd5` and a `baseline: <hash>` line.

- [ ] **Step 5: Prove the oracle DETECTS divergence (sensitivity check)**

```bash
cd /home/jrivero/code/gz/ws_ogre2_cpu_issues
sed 's/<pose>4 0 0.5 0 0 0<\/pose>/<pose>6 0 0.5 0 0 0<\/pose>/' \
  gpu_readback_bench/gpulidar_one.sdf > /tmp/gpulidar_moved.sdf
cp gpu_readback_bench/gpulidar_one.sdf /tmp/gpulidar_orig.sdf
# capture with the moved box by temporarily swapping the world file
cp /tmp/gpulidar_moved.sdf gpu_readback_bench/gpulidar_one.sdf
bash gpu_readback_bench/capture_lidar.sh gpu_readback_bench/moved_lidar
cp /tmp/gpulidar_orig.sdf gpu_readback_bench/gpulidar_one.sdf   # restore
bash gpu_readback_bench/compare_lidar.sh gpu_readback_bench/moved_lidar gpu_readback_bench/baseline_lidar; echo "exit=$?"
```
Expected: `lidar: MISMATCH ...`, `exit=1`. This confirms the oracle is sensitive to range changes (a moved box changes ranges). If it reports MATCH, the oracle is broken — fix before trusting it.

- [ ] **Step 6: Add the helper include to `Ogre2GpuRays.cc`**

After the OGRE includes block (e.g. after line 58 `#include <OgreTechnique.h>`), add:
```cpp
#include "Ogre2GpuReadbackTicket.hh"
```

- [ ] **Step 7: Add the persistent-ticket member**

In `Ogre2GpuRaysPrivate` (after the `gpuRaysScan` member at line 134), add:
```cpp
  /// \brief Persistent GPU->CPU readback ticket used by the non-legacy
  /// PostRender path.
  public: Ogre2GpuReadbackTicket gpuRaysReadback;
```

- [ ] **Step 8: Add the file-local fused-copy helper**

Immediately above `void Ogre2GpuRays::PostRender()` (line 1304), add:
```cpp
namespace
{
  /// \brief Copy one mapped RGBA32 readback box into the RGB scan buffer
  /// (drops alpha) in a single stride-aware pass. Honors _box.bytesPerRow.
  void FillGpuRaysScan(const Ogre::TextureBox &_box, float *_scan,
      unsigned int _width, unsigned int _height,
      unsigned int _channels, unsigned int _rawChannelCount,
      unsigned int _bytesPerChannel)
  {
    const float *src = static_cast<const float *>(_box.data);
    for (unsigned int row = 0; row < _height; ++row)
    {
      const unsigned int rawDataRowIdx =
          row * _box.bytesPerRow / _bytesPerChannel;
      const unsigned int rowIdx = row * _width * _channels;
      for (unsigned int column = 0; column < _width; ++column)
      {
        const unsigned int idx = rowIdx + column * _channels;
        const unsigned int rawIdx = rawDataRowIdx + column * _rawChannelCount;
        _scan[idx] = src[rawIdx];
        _scan[idx + 1] = src[rawIdx + 1];
        _scan[idx + 2] = src[rawIdx + 2];
      }
    }
  }
}  // namespace
```

- [ ] **Step 9: Replace the readback section of `PostRender()` (lines 1314-1346)**

Replace from the comment `// blit data from gpu to cpu` (line 1314) through the end of the RGBA→RGB copy loop (line 1346) with:
```cpp
  int outputLen = width * height * this->Channels();
  if (!this->dataPtr->gpuRaysScan)
  {
    this->dataPtr->gpuRaysScan = new float[outputLen];
  }

  if (Ogre2UseLegacyReadback())
  {
    // Legacy A/B control: original Ogre::Image2 path, kept verbatim.
    Ogre::Image2 image;
    image.convertFromTexture(this->dataPtr->secondPassTexture, 0u, 0u);
    Ogre::TextureBox box = image.getData(0u);
    float *bufferTmp = static_cast<float *>(box.data);

    // copy data from RGBA buffer to RGB buffer
    for (unsigned int row = 0; row < height; ++row)
    {
      unsigned int rawDataRowIdx = row * box.bytesPerRow / bytesPerChannel;
      unsigned int rowIdx = row * width * this->Channels();
      for (unsigned int column = 0; column < width; ++column)
      {
        unsigned int idx = rowIdx + column * this->Channels();
        unsigned int rawIdx = rawDataRowIdx + column * rawChannelCount;
        this->dataPtr->gpuRaysScan[idx] = bufferTmp[rawIdx];
        this->dataPtr->gpuRaysScan[idx + 1] = bufferTmp[rawIdx + 1];
        this->dataPtr->gpuRaysScan[idx + 2] = bufferTmp[rawIdx + 2];
      }
    }
  }
  else
  {
    // Persistent-ticket path: map the staging buffer once and repack RGBA->RGB
    // in a single fused pass.
    Ogre::TextureBox box = this->dataPtr->gpuRaysReadback.DownloadAndMap(
        this->dataPtr->secondPassTexture);
    if (!box.data)
    {
      gzerr << "Ogre2GpuRays: GPU readback failed; dropping frame" << std::endl;
      return;
    }
    FillGpuRaysScan(box, this->dataPtr->gpuRaysScan, width, height,
        this->Channels(), rawChannelCount, bytesPerChannel);
    this->dataPtr->gpuRaysReadback.Unmap();
  }
```
(The `// Metal does not support RGB32_FLOAT ...` comment that preceded `outputLen` moves up with `outputLen`; keep it. Leave the `newGpuRaysFrame` call at line 1348 and everything after unchanged.)

- [ ] **Step 10: Destroy the ticket in `Destroy()`**

In `Ogre2GpuRays::Destroy()` (line 582), after the early `if (!this->dataPtr->ogreCamera) return;` guard and next to the `gpuRaysScan` free (line 587), add:
```cpp
  this->dataPtr->gpuRaysReadback.Destroy();
```

- [ ] **Step 11: Build**

```bash
cd /home/jrivero/code/gz/ws_ogre2_cpu_issues && source install/setup.bash
CMAKE_BUILD_PARALLEL_LEVEL=5 colcon build --packages-select gz-rendering \
  --cmake-args -DCMAKE_BUILD_TYPE=RelWithDebInfo -DBUILD_TESTING=ON
```
Expected: `Finished <<< gz-rendering`.

- [ ] **Step 12: Equivalence — NEW and LEGACY gpu-rays paths match baseline**

```bash
cd /home/jrivero/code/gz/ws_ogre2_cpu_issues
bash gpu_readback_bench/capture_lidar.sh gpu_readback_bench/new_lidar
bash gpu_readback_bench/compare_lidar.sh gpu_readback_bench/new_lidar gpu_readback_bench/baseline_lidar; echo "new_exit=$?"
GZ_RENDERING_OGRE2_LEGACY_READBACK=1 bash gpu_readback_bench/capture_lidar.sh gpu_readback_bench/legacy_lidar
bash gpu_readback_bench/compare_lidar.sh gpu_readback_bench/legacy_lidar gpu_readback_bench/baseline_lidar; echo "legacy_exit=$?"
```
Expected: `lidar: MATCH` twice, `new_exit=0`, `legacy_exit=0`.

- [ ] **Step 13: Run the existing gpu_rays integration test**

```bash
cd /home/jrivero/code/gz/ws_ogre2_cpu_issues && source install/setup.bash
ctest --test-dir build/gz-rendering -R "INTEGRATION_gpu_rays" --output-on-failure
```
Expected: `100% tests passed`.

- [ ] **Step 14: Commit (code + harness together)**

```bash
cd /home/jrivero/code/gz/ws_ogre2_cpu_issues/src/gz-rendering
git add ogre2/src/Ogre2GpuRays.cc
git ci -q -m "ogre2: use persistent-ticket readback in Ogre2GpuRays

Same persistent-AsyncTextureTicket + single fused RGBA->RGB pass as the depth
camera, removing the per-frame Image2 alloc and redundant copy.
GZ_RENDERING_OGRE2_LEGACY_READBACK restores the original path. Byte-identical
range data (new gpu_lidar data-md5 oracle MATCH, new + legacy).

Co-Authored-By: Claude Opus 4.8 <noreply@anthropic.com>"
# harness scripts live at the workspace root (outside this repo); they are
# captured in the plan doc and kept locally — not committed here.
```
Expected: one commit.

---

## Task 5: Measure the win + leak/safety check + results writeup

Quantify the byte-reducer and prove no per-frame leak. This is the "prove the lever" deliverable.

**Files:**
- Create: `/home/jrivero/code/gz/ws_ogre2_cpu_issues/gpu_readback_bench/RESULTS.md`
- Create (in-repo provenance): `docs/superpowers/plans/2026-06-21-readback-results.md`

**Interfaces:**
- Consumes: `stage1_offthread/perf8.sh`, `stage0_rgbd/rgbd8.sdf`, `stage1_offthread/rgbd16_fast.sdf`, the Task-1 `perf_before.txt`.

- [ ] **Step 1: Depth perf — new vs legacy on the 8-cam world**

```bash
cd /home/jrivero/code/gz/ws_ogre2_cpu_issues
bash stage1_offthread/perf8.sh gpu_readback_bench/perf_new_rgbd8 2>&1 | tee gpu_readback_bench/perf_new_rgbd8.txt
GZ_RENDERING_OGRE2_LEGACY_READBACK=1 bash stage1_offthread/perf8.sh gpu_readback_bench/perf_legacy_rgbd8 2>&1 | tee gpu_readback_bench/perf_legacy_rgbd8.txt
```
Expected: two `RESULT` lines. Note per-cam Hz and serverCPU for each.

- [ ] **Step 2: Depth perf under saturation (16-cam uncapped)**

```bash
cd /home/jrivero/code/gz/ws_ogre2_cpu_issues
WORLD=stage1_offthread/rgbd16_fast.sdf NCAM=16 bash stage1_offthread/perf8.sh gpu_readback_bench/perf_new_rgbd16 2>&1 | tee gpu_readback_bench/perf_new_rgbd16.txt
WORLD=stage1_offthread/rgbd16_fast.sdf NCAM=16 GZ_RENDERING_OGRE2_LEGACY_READBACK=1 bash stage1_offthread/perf8.sh gpu_readback_bench/perf_legacy_rgbd16 2>&1 | tee gpu_readback_bench/perf_legacy_rgbd16.txt
```
Expected: two `RESULT` lines. The saturated world is where the byte-reduction should show as higher per-cam Hz / lower serverCPU (the idle 8-cam@15 world may be cap-bound — note that if so).

- [ ] **Step 3: Leak / ticket-stability check (RSS flat over a soak)**

Confirms the persistent ticket is reused (no per-frame alloc growth, no ticket leak):
```bash
cd /home/jrivero/code/gz/ws_ogre2_cpu_issues
export GZ_PARTITION="leakchk$$"
timeout 60 gz topic -e -t /rgbd/points -n 100000 >/dev/null 2>&1 &
sleep 1
gz sim -v1 -s -r --render-engine ogre2 stage1_offthread/rgbd_one.sdf >/tmp/leak.log 2>&1 &
sleep 8
PID=$(pgrep -x gz-sim-main); R1=$(awk '/VmRSS/{print $2}' /proc/$PID/status)
sleep 25
R2=$(awk '/VmRSS/{print $2}' /proc/$PID/status)
kill %1 %2 2>/dev/null; pkill -x gz-sim-main 2>/dev/null
echo "RSS kB: t0=$R1 t25=$R2 delta=$((R2-R1))"
```
Expected: `delta` near zero / not growing per-frame (a small one-time settle is fine; a steady multi-MB-per-second climb indicates a ticket or buffer leak — investigate). Run the same with `GZ_RENDERING_OGRE2_LEGACY_READBACK=1` for comparison.

- [ ] **Step 4: (Best-effort) ASan smoke of the readback path**

ASan over the full ogre/vulkan stack can trip on driver internals; treat as best-effort. If a clean ASan toolchain build is available, build gz-rendering with `-DCMAKE_CXX_FLAGS="-fsanitize=address -g"` and run one short `capture.sh` under it; otherwise rely on Step 3 (leak) + the map/unmap-lifetime argument in the spec. Record whichever was done.

- [ ] **Step 5: Write `gpu_readback_bench/RESULTS.md` and in-repo provenance**

```bash
cd /home/jrivero/code/gz/ws_ogre2_cpu_issues
cat > gpu_readback_bench/RESULTS.md <<'EOF'
# Persistent-ticket readback — results (2026-06-21)

## Equivalence (byte-identical)
- Depth: new MATCH baseline; legacy MATCH baseline (data-md5 depth/points/image).
- GPU rays: new MATCH baseline; legacy MATCH baseline (lidar data-md5). Oracle
  sensitivity verified (moved box -> MISMATCH).
- Integration: INTEGRATION_depth_camera PASS; INTEGRATION_gpu_rays PASS.

## Performance (fill in from perf_*_*.txt)
| world        | path   | per-cam Hz | serverCPU% | GPU% | VRAM |
|--------------|--------|-----------:|-----------:|-----:|-----:|
| rgbd8        | new    |            |            |      |      |
| rgbd8        | legacy |            |            |      |      |
| rgbd16_fast  | new    |            |            |      |      |
| rgbd16_fast  | legacy |            |            |      |      |

## Leak / stability
- RSS delta over 25s soak: new=__ kB, legacy=__ kB (expect ~flat).

## Notes
- Removes per-frame ~ (16 B/px * W * H) Image2 alloc/free + one full memcpy
  pass per camera per frame; collapses depth's extraction into the copy.
EOF
echo "wrote gpu_readback_bench/RESULTS.md (fill in the table from the perf_*.txt files)"
```
Fill the table from the `perf_*` files, then record provenance in-repo:
```bash
cp gpu_readback_bench/RESULTS.md src/gz-rendering/docs/superpowers/plans/2026-06-21-readback-results.md
cd src/gz-rendering
git add docs/superpowers/plans/2026-06-21-readback-results.md
git ci -q -m "docs: persistent-ticket readback equivalence + perf results

Co-Authored-By: Claude Opus 4.8 <noreply@anthropic.com>"
```
Expected: one commit. **Do not push** — ask the user first.

---

## Self-Review

**Spec coverage:**
- Helper `Ogre2GpuReadbackTicket` (spec §3.1) → Task 2. ✓
- Env opt-out `GZ_RENDERING_OGRE2_LEGACY_READBACK` (§3.3) → Task 2 (`Ogre2UseLegacyReadback`), used in Tasks 3/4. ✓
- Depth call-site, fused copy, legacy verbatim control (§3.2, §2) → Task 3. ✓
- GPU-rays call-site + repack (§3.2) → Task 4. ✓
- Invariants: row stride, copy-before-unmap, no conversion, resize recreate, teardown destroy (§4) → encoded in helper (Task 2) + FillDepth/FillScan + `Destroy()` calls (Tasks 3/4). ✓
- Verification: depth oracle, gpu-rays oracle (new), perf, leak/ASan, subscribers (§5) → Tasks 1, 3, 4, 5. ✓
- Build reality: gz-rendering only, auto-glob, ogre_vk_prefix (§5, §6) → Global Constraints + Task 2 Step 3. ✓
- Non-goals (async, other sensors) → out of scope; not in any task. ✓

**Placeholder scan:** No TBD/TODO; every code step shows complete code; the RESULTS.md table is intentionally a fill-in *artifact* (data produced at run time), not a plan placeholder.

**Type consistency:** `Ogre2GpuReadbackTicket::DownloadAndMap/Unmap/Destroy`, `Ogre2UseLegacyReadback()`, `FillDepthBuffers(...)`, `FillGpuRaysScan(...)` names/signatures are used identically wherever referenced across Tasks 2–4. Members `depthReadback` / `gpuRaysReadback` consistent.

---

## Execution Handoff

(Provided after the user reviews this plan.)
