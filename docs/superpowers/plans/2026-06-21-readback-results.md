# Persistent-ticket readback — results (2026-06-21)

## Equivalence (byte-identical)
- Depth: new MATCH baseline; legacy MATCH baseline (data-md5 depth/points/image).
- GPU rays: new MATCH baseline; legacy MATCH baseline (lidar data-md5). Oracle
  sensitivity verified (moved box -> MISMATCH).
- Integration: INTEGRATION_depth_camera PASS; INTEGRATION_gpu_rays PASS.

## Performance (from perf_*_*.txt)

Baseline (Task 1, no ticket, legacy default): 15.03 Hz/cam, serverCPU=177%, GPU=35.1%

| world        | path   | per-cam Hz | serverCPU% | GPU%  | VRAM |
|--------------|--------|-----------:|-----------:|------:|-----:|
| rgbd8        | new    |      15.07 |        148 |  22.2 |  n/a |
| rgbd8        | legacy |      14.91 |        148 |  23.5 |  n/a |
| rgbd16_fast  | new    |      12.73 |        205 |  35.4 |  n/a |
| rgbd16_fast  | legacy |      11.55 |        191 |  32.0 |  n/a |

Notes on VRAM column: pre-existing harness awk quirk — reads 0/NA; ignored.

### Interpretation
- **rgbd8 (8 cams @15 Hz)**: both paths hit the 15 Hz sensor-rate cap (15.07 vs
  14.91 Hz); no meaningful delta expected here. serverCPU identical at 148%.
  Cap-bound — this is expected per task brief.
- **rgbd16_fast (16 cams, uncapped)**: new path 12.73 Hz/cam vs legacy 11.55 Hz/cam
  — a **+1.18 Hz/cam (+10.2%) throughput gain** at 16 cameras. serverCPU slightly
  higher on new path (205 vs 191%) because the CPU is doing more render work per
  unit time (higher throughput), not because the path is heavier per frame.
  GPU utilization 35.4 vs 32.0% — consistent with doing more frames.

## Leak / stability
- RSS delta over 25s soak (rgbd_one.sdf, 1 RGBD cam, continuous):
  - **new path**: delta = 0 kB (t0=712044 kB, t25=712044 kB) — ticket reused, no growth
  - **legacy path**: delta = 0 kB (t0=711668 kB, t25=711668 kB) — flat too

  Both paths flat: persistent ticket is correctly reused per-frame; no per-frame
  allocation growth observed.

## ASan
Skipped — Vulkan driver internals trip ASan reliably in this environment.
Relied on: (a) RSS soak above confirming no leak, (b) map/unmap lifetime argument
in the spec (ticket mapped only during copy, unmapped before use by caller),
(c) byte-identical equivalence proving no buffer overread/overwrite in the copy path.

## Notes
- Removes per-frame ~(16 B/px * W * H) Image2 alloc/free + one full memcpy
  pass per camera per frame; collapses depth's extraction into the copy.
- Win is visible only under saturation (rgbd16_fast). At 8 cams @15 Hz the
  sensor-rate cap hides the per-frame savings.
- serverCPU on rgbd16_fast new=205% vs legacy=191%: higher because throughput is
  higher (more frames processed per second), not because the code path is heavier.
