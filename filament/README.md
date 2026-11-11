# Filament render engine (experimental)

`gz-rendering-filament` is a render engine plugin built on
[Filament](https://github.com/google/filament). It renders the scene,
user cameras and the sensors (camera, depth, GPU rays, thermal,
segmentation, bounding box, wide angle). Non-RGB sensor output matches
ogre2. Load it as engine `filament`.

## Building Filament

The plugin needs a Filament install built from commit `0643fd0f` with
patches 0001 (build fixes) and 0004 (gz light semantics) from
[patches/](patches/README.md) applied. 0002 and 0003 are optional: they
make Vulkan readback faster (3-7x per sensor capture on NVIDIA) without
changing its output.

```bash
git clone https://github.com/google/filament.git
cd filament
git checkout 0643fd0f
P=/path/to/gz-rendering/filament/patches
git apply $P/0001-*.patch $P/0004-*.patch
# optional, faster Vulkan readback:
git apply $P/0002-*.patch $P/0003-*.patch

cmake -S . -B build -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=$HOME/filament-install \
  -DFILAMENT_ENABLE_EXPERIMENTAL_GCC_SUPPORT=ON \
  -DFILAMENT_SUPPORTS_EGL_ON_LINUX=ON \
  -DFILAMENT_SUPPORTS_VULKAN=ON \
  -DFILAMENT_BUILD_FILAMAT=ON \
  -DFILAMENT_SKIP_SAMPLES=ON \
  -DFILAMENT_BUILD_TESTING=OFF \
  -DFILAMENT_SKIP_SDL2=ON \
  -DCMAKE_CXX_STANDARD_LIBRARIES=-lGL \
  -DUSE_STATIC_LIBCXX=ON
cmake --build build --target install
```

With EGL, Filament's own executables (e.g. `benchmark_filament`) miss the
GL library at link time, hence `-lGL`. The bundled SDL2 doesn't build with
GCC 15 (`FILAMENT_SKIP_SDL2=ON`); the tests aren't needed.

Then configure gz-rendering with `FILAMENT_ROOT` pointing at the install
prefix, as a CMake variable or an environment variable:

```bash
cmake -DFILAMENT_ROOT=$HOME/filament-install ...
```

The materials in `src/media/materials` are compiled at build time with
that install's `matc`. Filament refuses materials from a different
`MATERIAL_VERSION`, so rebuild gz-rendering after rebuilding Filament.

## Runtime options

- Resources: the compiled materials and textures are installed to
  `<prefix>/share/gz/gz-rendering<N>/filament/media`. Like ogre2, the
  engine looks for them under `$GZ_RENDERING_RESOURCE_PATH/filament/media`
  if that variable is set, else under the install prefix, and falls back
  to the build tree. `GZ_RENDERING_FILAMENT_RESOURCE_PATH` overrides all
  of these with a `media` directory of its own (it holds `materials/` and
  `textures/`).

- Backend: OpenGL (EGL) by default. The engine parameter `vulkan=1` (or
  `backend=vulkan`) selects Vulkan, falling back to OpenGL when no Vulkan
  driver is available. `egl_display` / `egl_context` share an existing EGL
  context (e.g. gz-gui).
- With OpenGL the engine sets `__GL_YIELD=USLEEP` before the driver
  loads, unless `__GL_YIELD` is already set: NVIDIA's driver then sleeps
  instead of spinning while it waits for the GPU, which every sensor
  readback does (6-9% less CPU in gz-sim with an RGB or depth sensor).
  It is a process-wide setting: set `__GL_YIELD` yourself to choose
  another mode. Other drivers ignore it.
- Look: every view uses Filament's lighting model. The GUI camera also
  uses tone mapping, ambient occlusion, anti-aliasing and sky lighting;
  RGB camera sensors use sky lighting only (no post-processing, so they
  cost about the same). Depth, lidar, segmentation, thermal and bounding
  box output is not affected.
