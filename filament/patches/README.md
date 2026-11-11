Patches on top of Filament commit 0643fd0f, applied in order with
`git apply`. See ../README.md for the full build.

- 0001 Build fixes: builds with GCC (`-fstrict-vtable-pointers` is clang
  only) and compiles the built-in materials for all platforms when EGL is
  used on Linux, since EGL selects the GLES backend, which needs the mobile
  shader variants.
- 0002 Vulkan readPixels reuses its staging buffers instead of allocating
  device memory per readback. Sensors read back every frame, so this is on
  their hot path.
- 0003 Vulkan readPixels also recycles its fences and command buffers and
  keeps the staging memory mapped, and DataReshaper copies rows with memcpy
  when source and destination layouts match (same type and channel count,
  no swizzle, zero offset). Output bytes are unchanged.
- 0004 ogre2-style lights: `LightManager::setSpecularScale` scales the
  specular lobe (ogre2 multiplies it by the light's specular colour) and
  `setPolynomialAttenuation` gives point and spot lights ogre2's
  attenuation, 1 / (c + l*d + q*d^2) cut at the range, and the spot
  falloff exponent. The values travel in unused uniform fields, so the UBO
  size is unchanged. Bumps MATERIAL_VERSION to 80.

0005 (ogre2's diffuse lobe) and 0007 (per-view switch back to Filament's
diffuse) are gone: every view uses Filament's stock diffuse.

Rebuild gz-rendering after changing patches: its materials are compiled
with the patched `matc`.
