# Filament render engine (experimental)

`gz-rendering-filament` is a render engine plugin built on
[Filament](https://github.com/google/filament). It renders the scene,
user cameras and the sensors (camera, depth, GPU rays, thermal,
segmentation, bounding box, wide angle). Non-RGB sensor output matches
ogre2. Load it as engine `filament`.

## Building Filament

The plugin builds against an unpatched Filament. It is tested with commit
`0643fd0f`. Release `v1.80.0` builds and runs with the same recipe and
passes the tests except picking (`Camera::VisualAt` finds no visual). The
patches in [patches/](patches/README.md) are optional:

- 0004 gives lights gz's attenuation and specular colour (see
  [Lighting without patch 0004](#lighting-without-patch-0004)).
- 0002 and 0003 make Vulkan readback faster (3-7x per sensor capture on
  NVIDIA) without changing its output.

```bash
git clone https://github.com/google/filament.git
cd filament
git checkout 0643fd0f
# optional, gz light attenuation and specular colour:
P=/path/to/gz-rendering/filament/patches
git apply $P/0004-*.patch
# optional, faster Vulkan readback:
git apply $P/0002-*.patch $P/0003-*.patch

cmake -S . -B build -G Ninja \
  -DCMAKE_BUILD_TYPE=MinSizeRel \
  -DCMAKE_C_FLAGS_MINSIZEREL="-O3 -DNDEBUG" \
  -DCMAKE_CXX_FLAGS_MINSIZEREL="-O3 -DNDEBUG" \
  -DCMAKE_INSTALL_PREFIX=$HOME/filament-install \
  -DFILAMENT_ENABLE_EXPERIMENTAL_GCC_SUPPORT=ON \
  -DMATC_OPT_FLAGS=--platform=all \
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

- GCC, in a `MinSizeRel` build with `Release`'s `-O3`: in `Release` builds
  `libs/iblprefilter` and `libs/generatePrefilterMipmap` pass the
  clang-only flag `-fstrict-vtable-pointers`, which GCC rejects; no other
  build type adds it. (Filament's own compiler is clang with libc++. Clang
  with libstdc++ stops on Filament's thread-safety annotations, which
  `-Werror` turns into errors, so it isn't an option without libc++.)
- `MATC_OPT_FLAGS=--platform=all`: with EGL on Linux the OpenGL backend
  runs OpenGL ES, which needs the mobile variants of Filament's built-in
  materials. Filament compiles them for desktop only on Linux; the extra
  option (the last `--platform` wins) adds the mobile ones. Without it the
  OpenGL backend crashes at start-up on "Filament Default Material".
  Vulkan doesn't need it.
- `-lGL`: with EGL, Filament's own executables (e.g. `benchmark_filament`)
  miss the GL library at link time. The bundled SDL2 doesn't build with
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
  context.
- GUI: stock gz-gui works as it is, no gz-gui change is needed. gz-gui
  passes `useCurrentGLContext=1`; when Qt's context is an EGL one the
  engine shares it and draws each frame into a texture in it
  (`RenderTextureGLId`), with no copy through the CPU. Otherwise (Qt on
  GLX) the frame is read back and uploaded into that texture. Either way
  the texture holds the image top row first, like ogre2's, so gz-gui must
  not mirror it.
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

## Lighting without patch 0004

gz-rendering's CMake looks for patch 0004 in the Filament install
(`setPolynomialAttenuation` in `LightManager.h`). Without it the plugin
still builds and runs, with a CMake warning and one warning per scene, and
lights use Filament's own model:

- Point and spot lights fall off with the inverse square of the distance,
  windowed to zero at their range, instead of gz's
  1 / (c + l*d + q*d^2) (ogre2 uses c = 0.5). With the same intensity, a
  typical light (l = 0.01, q = 0.001) is 2x brighter at 0.5 m, 2x darker
  at 1 m and 43x darker at 5 m.
- Spot cones use Filament's squared ramp instead of the light's falloff
  exponent.
- Light specular colours are ignored: every light's specular lobe has the
  strength of its diffuse colour (the default gz sun has a specular colour
  of 0.25 of its diffuse colour, so highlights are about 4x stronger).

Only lit output changes (RGB cameras, the GUI camera, wide-angle cameras,
the colour of depth camera point clouds). Depth, lidar, segmentation,
thermal and bounding box output doesn't depend on lights.

