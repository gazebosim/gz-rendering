# Persistent-ticket GPU readback for Ogre2 depth camera & GPU rays

- **Date:** 2026-06-21
- **Component:** `gz-rendering` (ogre2), branch `jrivero/remove_dup_image_copy`
- **Status:** Design approved; ready for implementation plan
- **Related:** Stage 2 / "reduce memory traffic" sub-lever of the many-RGBD parallelism work
  (see project memory `project_ogre2_rgbd_sensor_parallelism`). Stacks with — does not
  replace — Stage 1's off-thread publish (gz-sensors `SensorTailPool`).

## 1. Problem

`Ogre2DepthCamera::PostRender()` (`ogre2/src/Ogre2DepthCamera.cc:1162-1260`) and the
structurally identical `Ogre2GpuRays::PostRender()` (`ogre2/src/Ogre2GpuRays.cc:1304-1368`)
read the GPU render target back to the CPU with a wasteful pattern. Per camera, per frame
(640×480 × 16 B/px ≈ 4.9 MB):

1. `Ogre::Image2 image;` + `image.convertFromTexture(tex, 0u, 0u)` — internally allocates a
   fresh ~4.9 MB CPU buffer (`OGRE_MALLOC_SIMD`), creates a throwaway `AsyncTextureTicket`,
   does a blocking GPU→CPU DMA download, `memcpy`s the staging buffer into the Image2's own
   buffer (`TextureBox::copyFrom`), then frees the ticket. (`OgreImage2.cpp:334,356,383-410`)
2. A **second** full-buffer row-by-row `memcpy` from the Image2 into a persistent member
   buffer — `depthBuffer` / `gpuRaysScan`. (`Ogre2DepthCamera.cc:1185-1191`,
   `Ogre2GpuRays.cc:1330-1346`)
3. (depth only) A **third** per-pixel pass extracting channel 0 into `depthImage`.
   (`Ogre2DepthCamera.cc:1199-1207`)

### What is redundant vs unavoidable

- **Unavoidable:** the GPU→CPU DMA (`vkCmdCopyImageToBuffer`) and its fence wait — the bytes
  must leave the GPU once. Stage 0 measured this path is **CPU-bound, not fence-blocked**
  (wait% < 6%); the throughput ceiling is **memory bandwidth**.
- **Redundant:** (a) the per-frame Image2 ~4.9 MB malloc/free; (b) the per-call ticket
  create/destroy; (c) one of the two full-buffer memcpys (step 1's `copyFrom` exists only to
  fill a throwaway buffer that step 2 immediately re-copies); (d) the separate third
  extraction pass (depth), which can be fused into the one necessary copy.

Removing these is a genuine **byte-reducer** (a category-a lever): it deletes a 4.9 MB
allocator round-trip and a full read+write pass over ~4.9 MB per camera per frame. At
8 cameras × ~10 Hz that is on the order of ~400 MB/s of bus traffic removed. It is a
different lever from async/off-thread readback (which only reshuffles the same bytes) and
stacks with it.

### Format note (load-bearing for correctness)

The depth texture `ogreDepthTexture[1]` is created as `PFG_RGBA32_UINT`
(`Ogre2DepthCamera.cc:963-964`), not `PFG_RGBA32_FLOAT`. Both are 16 B/px with identical
component layout; `convertFromTexture` performs **no** numeric conversion — it is a verbatim
bit copy that the CPU reinterprets as `float*`. The GPU-rays second-pass texture is
`PFG_RGBA32_FLOAT` (`Ogre2GpuRays.cc:1121-1122`). In both cases a raw `AsyncTextureTicket`
`map()` returns exactly the same bytes, so replacing `convertFromTexture` is a 1:1 byte
replacement. **No code path may numerically convert these bytes.**

## 2. Goal & non-goals

**Goal.** Per camera per frame, go from *(1 malloc/free + 2 full memcpys + 1 extraction
pass)* to *(0 malloc + 1 fused stride-aware pass)* by introducing a reusable persistent-
`AsyncTextureTicket` readback helper used by `Ogre2DepthCamera` and `Ogre2GpuRays`. Output
**byte-identical**, **+0 frame latency** (synchronous `accurateTracking=true` download +
immediate `map()`). Default on, with an env opt-out for A/B measurement and safety.

**Non-goals (explicitly deferred).**
- Deferred/async non-blocking readback (Option B, +1 frame latency, double-buffered tickets).
- The other readback sites: thermal, segmentation, bounding-box, wide-angle, selection
  buffer (all use `convertFromTexture`), and the regular camera / wide-angle
  `copyContentsToMemory` path (a different shape). The helper is built so these are small
  follow-ups.

## 3. Components

### 3.1 `Ogre2GpuReadbackTicket` (new helper)

New private files `ogre2/src/Ogre2GpuReadbackTicket.{hh,cc}` (not installed public API). A
render-thread-only class owning **one persistent `Ogre::AsyncTextureTicket`** across frames.

```cpp
class Ogre2GpuReadbackTicket
{
  public: ~Ogre2GpuReadbackTicket();        // destroys the ticket via the manager

  /// Lazily creates/recreates a ticket sized to the texture's (w,h,PFG family),
  /// issues a blocking download (accurateTracking=true), and maps it.
  /// Returns the mapped TextureBox (carries .data + .bytesPerRow); .data==nullptr on failure.
  public: Ogre::TextureBox DownloadAndMap(Ogre::TextureGpu *_tex);

  public: void Unmap();

  private: Ogre::AsyncTextureTicket *ticket{nullptr};
  private: unsigned int width{0}, height{0};
  private: Ogre::PixelFormatGpu format{Ogre::PFG_UNKNOWN};
};
```

- Obtains `Ogre::TextureGpuManager*` lazily from
  `Ogre2RenderEngine::Instance()->OgreRoot()->getRenderSystem()->getTextureGpuManager()`.
- On dimension/format change vs the cached values, `destroyAsyncTextureTicket` + recreate
  (ticket size is baked at construction).
- Knows nothing about channels/repack — it returns a **stride-aware view**; each caller does
  its own copy. This is the genuinely-shared part; the divergent post-copy stays caller-side.

Also in the same header, a free function:

```cpp
bool Ogre2UseLegacyReadback();   // reads GZ_RENDERING_OGRE2_LEGACY_READBACK once (cached static)
```

### 3.2 Call-site integration (both cameras, identical structure)

Each `PostRender` branches once on the cached flag. The **new** path acquires bytes via the
helper and does the fused copy; the **legacy** path is the original code, kept verbatim, so it
remains a faithful before/after control:

```cpp
if (Ogre2UseLegacyReadback())
{
  // ── ORIGINAL CODE, VERBATIM (control) ──
  Ogre::Image2 image;
  image.convertFromTexture(tex, 0u, 0u);
  Ogre::TextureBox box = image.getData(0);
  // ... original two copy loops (row memcpy -> depthBuffer; then per-pixel -> depthImage) ...
}
else
{
  // ── NEW PATH: persistent ticket + single fused pass ──
  Ogre::TextureBox box = this->dataPtr->readback.DownloadAndMap(tex);
  if (!box.data) { /* gzerr once; drop frame */ return; }
  this->FillBuffers(box);                  // fused, stride-aware (uses box.bytesPerRow)
  this->dataPtr->readback.Unmap();
}
// fire events from the persistent buffers (valid post-unmap)
```

- **Why legacy is NOT routed through `FillBuffers`:** the original does an internal
  `copyFrom` into the Image2 buffer (inside `convertFromTexture`) **plus** two gz-side passes
  (row memcpy + channel-0 extraction). Reusing `FillBuffers` for legacy would already give it
  the extraction-fusion, so the A/B would no longer reflect the full fix. Keeping legacy
  verbatim makes the perf delta the genuine before/after and makes the legacy branch trivially
  deletable in a later cleanup.
- **Depth `FillBuffers` (new path only):** one row loop copies all 4 channels into
  `depthBuffer` **and** extracts channel-0 into `depthImage` in the same pass (fuses the old
  1185-1191 and 1199-1207 loops). `depthImage` filled unconditionally; `depthBuffer` remains
  the persistent buffer returned by `DepthData()` and passed to `newRgbPointCloud`.
- **GPU-rays `FillScan` (new path only):** one row loop copies RGBA→RGB (drops alpha) into
  `gpuRaysScan` (mirrors current 1330-1346).

### 3.3 Env opt-out

`GZ_RENDERING_OGRE2_LEGACY_READBACK=1` restores the original `Image2::convertFromTexture`
path. Default (unset) = new persistent-ticket path. Read once via `Ogre2UseLegacyReadback()`.

## 4. Invariants the implementation must hold

- **Row stride:** always copy row-by-row honoring `box.bytesPerRow` (4-byte aligned, may
  pad) — never a flat `width*height*bpp` memcpy.
- **Map window:** the fused copy completes **before** `Unmap()`; events fire afterward from
  persistent buffers. Mapped staging memory is never handed to a consumer.
- **No conversion:** treat bytes as a verbatim 16 B/px reinterpret (`float*`); never invoke a
  numeric pixel conversion.
- **Resize:** recreate the ticket on dimension/format change.
- **Teardown:** destroy the ticket in each camera's `Destroy()` (before engine teardown,
  ahead of the manager's shutdown sweep that auto-destroys remaining tickets).
- **Re-download rule:** `download()` throws if the ticket is still `Mapped`; the per-frame
  order must be download → map → (copy) → unmap, with `unmap()` before the next frame's
  download.
- **Concurrency:** none added — single render thread, synchronous events, in-place buffer
  reuse preserved.

## 5. Verification

- **Build (gz-rendering only; `src/ogre-next` is NOT the linked lib — `ogre_vk_prefix/` is):**
  `CMAKE_BUILD_PARALLEL_LEVEL=5 colcon build --packages-select gz-rendering --cmake-args
  -DCMAKE_BUILD_TYPE=RelWithDebInfo -DBUILD_TESTING=OFF`. Add the new `.cc` to ogre2 sources.
- **Equivalence (depth):** `stage1_offthread/capture.sh` → `compare.sh` vs `golden/`, run
  **both** default (new) and `GZ_RENDERING_OGRE2_LEGACY_READBACK=1` (legacy); both must match
  golden `.datamd5` for depth/points/image → proves new path is byte-identical to original.
  (Oracle = header-stripped data-md5 on the static `rgbd_one.sdf` scene; valid for static
  scenes only — sufficient because the fix is a verbatim byte copy.)
- **Equivalence (gpu rays):** add a small `gpu_lidar` capture/compare mirroring `capture.sh`
  for the scan/range topic (no existing ogre2 lidar oracle in the workspace — new sub-task).
- **Perf:** `perf8.sh` new vs legacy on `stage0_rgbd/rgbd8.sdf` (8×640²); plus the saturated
  `stage1_offthread/rgbd16_fast.sdf` (8×640@15 Hz is not render-bound on an idle box — measure
  under saturation). Expect per-cam Hz up / serverCPU down.
- **ASan smoke run** to catch map/unmap-lifetime or ticket double-free regressions (cheap; no
  new threading so TSan is lower-value).
- **Benchmarks must attach subscribers** — gz-sim disables sensors with no subscribers.

## 6. Files touched

- **New:** `ogre2/src/Ogre2GpuReadbackTicket.{hh,cc}` (+ ogre2 CMake sources entry).
- **Edit:** `ogre2/src/Ogre2DepthCamera.cc` (PostRender, `Destroy`, private `FillBuffers`,
  private member) and its private decl; `ogre2/src/Ogre2GpuRays.cc` (PostRender, `Destroy`,
  private `FillScan`, private member) and its private decl.
- **New (verify harness):** small `gpu_lidar` capture/compare scripts under the workspace root.

## 7. Risks

- Padded `bytesPerRow` for non-4-byte-aligned widths — mitigated by mandatory row-by-row copy.
- Reading unmapped/freed staging memory — mitigated by copy-before-unmap and events-after-copy.
- Stale-size ticket after camera resize — mitigated by dimension/format recheck + recreate.
- Ticket leak / double-free at shutdown — mitigated by explicit destroy in `Destroy()`.
- gpu-rays oracle is new; must confirm it reliably detects divergence before trusting it.
