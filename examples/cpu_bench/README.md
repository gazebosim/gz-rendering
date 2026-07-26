# Headless CPU Benchmark for Ogre (and Ogre2) Render Path

This example is a headless CPU benchmark designed to isolate where CPU cycles are spent per-frame during rendering.
It is particularly useful for debugging performance regressions or bottleneck areas, such as when a large number of static or dynamic objects exist in a scene.

## How it works

The benchmark sets up $N$ box visuals in front of a camera.
It runs a timed rendering loop for a configured number of frames, measuring both Wall time and CPU time (User + System time) to evaluate overhead.

Key capabilities of this benchmark include:
* **Isolating Pose Updates:** Re-applying poses every frame via `BENCH_MOVE_POSES` helps mimic the unconditional pose dirtying/updating behaviors of `gz-sim`'s `RenderUtil`.
* **Frustum Culling Cost:** Setting `BENCH_OFFSCREEN=1` positions objects behind the camera to measure the scene graph traversal and frustum culling overhead while skipping actual rasterization.
* **Shadow Overhead:** Benchmarking with and without shadows helps evaluate the cost of shadow map updates (e.g., Ogre2 PSSM splits).

## Building the Example

1. **Build and install gz-rendering:**
   Ensure `gz-rendering` is built and installed (or sourced via workspace overlay).

2. **Build the benchmark:**
   ```bash
   cd examples/cpu_bench
   mkdir build
   cd build
   cmake ..
   make
   ```

## Running the Example

Run the benchmark executable by specifying the rendering engine as an argument:

```bash
# Default to ogre2
./cpu_bench ogre2

# Run with ogre1 (ogre)
./cpu_bench ogre
```

### Configuration (Environment Variables)

All options are controlled via environment variables to allow running quick matrices of A/B testing without rebuilding:

| Variable | Description | Default |
|----------|-------------|---------|
| `BENCH_N` | Number of boxes to render | `400` |
| `BENCH_FRAMES` | Number of timed frames | `300` |
| `BENCH_WARMUP` | Number of untimed warmup frames (for shader compile, first-frame allocs) | `30` |
| `BENCH_SHADOWS` | `1` to enable shadow casting, `0` to disable | `1` |
| `BENCH_MOVE_POSES` | `1` to re-apply box poses every frame (forces dirty updates), `0` to leave static | `1` |
| `BENCH_OFFSCREEN` | `1` to position boxes behind the camera (frustum-culled), `0` to render in front | `0` |
| `BENCH_W` | Render target width in pixels | `800` |
| `BENCH_H` | Render target height in pixels | `600` |
| `BENCH_FPS` | Target frame rate cap (`0` for uncapped, `>0` to sleep-limit) | `0` |
| `BENCH_VERBOSE` | `1` to set Ogre console verbosity to maximum (`4`) | `0` |
| `BENCH_CAPTURE` | `1` to capture a single frame image and log non-background pixel count | `0` |

### Benchmark Output

The executable outputs human-readable timing stats and a stable, grep-friendly CSV line:

```text
BENCH engine=ogre2 N=400 shadows=1 poses=1 offscreen=0 capFps=0 800x600 frames=300
  achieved fps:   135.21
  wall/frame ms:  min=6.82  median=7.15  mean=7.39
  cpu/frame  ms:  mean=7.32
  CORES BUSY:     0.99  (total CPU 2.196s / wall 2.218s)
CSV,ogre2,400,1,1,0,800x600,6.82,7.15,7.39,7.32
```
