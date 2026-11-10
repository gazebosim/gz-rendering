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
- 0005 ogre2's diffuse lobe: the standard model's diffuse becomes Ogre
  PBS's default BRDF diffuse (normalized Disney with roughness-dependent
  retro-reflection) times one minus the Fresnel term, with Ogre's
  metallic-workflow F0, instead of roughness-independent Lambert. Shader
  only; FilamentLight's light power assumes it, so materials compiled
  without it come out about 25% too bright.
- 0007 Per-view switch back to Filament's stock Lambert diffuse, enabled
  by a non-zero material global 3.z (`View::setMaterialGlobal`).
  gz-rendering sets it on the GUI and RGB camera views (the default);
  other views, and every view with `GZ_FILAMENT_QUALITY=0`, keep 0005's
  diffuse. The switch overwrites the diffuse
  after the ogre2 lobe instead of returning early, which keeps sensor
  output bit-identical. Shader only; bumps MATERIAL_VERSION to 81 so
  materials compiled without it are refused rather than ignoring the
  switch.

Rebuild gz-rendering after changing patches: its materials are compiled
with the patched `matc`.
