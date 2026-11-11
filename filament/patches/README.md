Patches on top of Filament commit 0643fd0f, applied in order with
`git apply`. See ../README.md for the full build.

Required:

- 0001 Build fixes: builds with GCC (`-fstrict-vtable-pointers` is clang
  only) and compiles the built-in materials for all platforms when EGL is
  used on Linux, since EGL selects the GLES backend, which needs the mobile
  shader variants (without it the OpenGL backend crashes on Filament's
  default material). Upstream main still needs both (2026-11-11).
- 0004 gz light semantics: `LightManager::setSpecularScale` scales the
  specular lobe (gz lights have a specular colour; ogre2 multiplies the
  lobe by it) and `setPolynomialAttenuation` gives point and spot lights
  gz's attenuation, 1 / (c + l*d + q*d^2) cut at the range, and the spot
  falloff exponent. The values travel in unused uniform fields, so the UBO
  size is unchanged. Bumps MATERIAL_VERSION to 80. gz-rendering's CMake
  refuses a Filament install without it. Filament's extension points can't
  replace it: `customSurfaceShading` only sees the light's colour,
  direction, position and Filament's own attenuation (no light identity,
  spot direction or cone, and at mediump precision in the GLES shaders),
  and an inverse-square falloff can't be fitted to gz's polynomial.

Optional (performance only; the output is the same without them). Without
them each Vulkan sensor capture costs about 0.7-1 ms more on an RTX 4070
(3-7x slower per capture, default parity scene) and about 5% more on Intel
Iris Xe; recommended for Vulkan on NVIDIA:

- 0002 Vulkan readPixels reuses its staging buffers instead of allocating
  device memory per readback. Sensors read back every frame, so this is on
  their hot path.
- 0003 (on top of 0002) Vulkan readPixels also recycles its fences and
  command buffers and keeps the staging memory mapped, and DataReshaper
  copies rows with memcpy when source and destination layouts match (same
  type and channel count, no swizzle, zero offset). Output bytes are
  unchanged.

Removed: 0005 (ogre2's diffuse lobe) and 0007 (per-view switch back to
Filament's diffuse); 0006 was never published. Every view uses Filament's
stock diffuse.

Rebuild gz-rendering after changing patches: its materials are compiled
with the patched `matc`.
