# Filament render engine (experimental)

`gz-rendering-filament` is a render engine plugin built on
[Filament](https://github.com/google/filament). It renders the scene,
user cameras and the sensors (camera, depth, GPU rays, thermal,
segmentation, bounding box, wide angle). Non-RGB sensor output matches
ogre2. Load it as engine `filament`.

## Building Filament

The plugin needs a Filament install built from commit `0643fd0f` with the
patches in [patches/](patches/README.md) applied:

```bash
git clone https://github.com/google/filament.git
cd filament
git checkout 0643fd0f
git apply /path/to/gz-rendering/filament/patches/*.patch

cmake -S . -B build -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=$HOME/filament-install \
  -DFILAMENT_ENABLE_EXPERIMENTAL_GCC_SUPPORT=ON \
  -DFILAMENT_SUPPORTS_EGL_ON_LINUX=ON \
  -DFILAMENT_SUPPORTS_VULKAN=ON \
  -DFILAMENT_BUILD_FILAMAT=ON \
  -DFILAMENT_SKIP_SAMPLES=ON \
  -DUSE_STATIC_LIBCXX=ON
cmake --build build --target install
```

Then configure gz-rendering with `FILAMENT_ROOT` pointing at the install
prefix, as a CMake variable or an environment variable:

```bash
cmake -DFILAMENT_ROOT=$HOME/filament-install ...
```

The materials in `src/media/materials` are compiled at build time with
that install's `matc`. Filament refuses materials from a different
`MATERIAL_VERSION`, so rebuild gz-rendering after rebuilding Filament.

`-DGZ_FILAMENT_DEV_TOOLS=ON` also builds `filament_smoke` and
`filament_parity` (renders a scene with any engine and dumps every sensor;
`tools/parity_compare.py` compares two dumps).

## Runtime options

- Backend: OpenGL (EGL) by default. The engine parameter `vulkan=1` (or
  `backend=vulkan`) selects Vulkan, falling back to OpenGL when no Vulkan
  driver is available. `egl_display` / `egl_context` share an existing EGL
  context (e.g. gz-gui).
- Look: every view uses Filament's lighting model. The GUI camera also
  uses tone mapping, ambient occlusion, anti-aliasing and sky lighting;
  RGB camera sensors use sky lighting only (no post-processing, so they
  cost about the same). Depth, lidar, segmentation, thermal and bounding
  box output is not affected.
