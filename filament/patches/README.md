Patches applied on top of Filament 0643fd0f for this spike:

- 0001: build with GCC (-fstrict-vtable-pointers is clang only) and compile
  the built-in materials for all platforms when EGL is used on Linux (EGL
  selects the GLES backend, which needs the mobile shader variants).
- 0002: Vulkan readPixels reuses staging buffers instead of allocating
  device memory per readback (upstream TODO in VulkanReadPixels.cpp). Cuts
  the Vulkan depth camera from 1.47 to 0.96 ms per frame here.
- 0003 (on top of 0002): Vulkan readPixels also recycles its fences and
  command buffers instead of creating/destroying them per readback (driver
  ioctls on NVIDIA), keeps staging memory mapped for its whole life, and
  DataReshaper memcpys rows when source and destination layouts match (same
  type and channel count, no swizzle, zero offset) for UBYTE/FLOAT/INT/UINT,
  not only RGBA8. Output bytes unchanged.
- 0004 (on top of 0003): lights like ogre2. LightManager::setSpecularScale
  (scales the specular lobe of the standard shading model; ogre2 multiplies
  it by the light's specular colour) and setPolynomialAttenuation (point/spot
  1 / (c + l*d + q*d^2), cut at the falloff radius, optional linear fade to
  it, spot ramp^exponent, as ogre-next's light loops). Carried in the unused
  LightsUib/PerViewUib fields (no UBO size change); MATERIAL_VERSION 80, so
  materials must be rebuilt with this matc (gz-rendering's are, at build).
- 0005 (on top of 0004): ogre2's diffuse lobe. The standard model's diffuse
  is Ogre PBS's Default BRDF diffuse (normalized Disney: energy factor
  1 -> 1/1.51 with perceptual roughness, light/view scatter with
  fd90 = r/2 + 2 r LoH^2) times 1 - max(Schlick Fresnel) with Ogre's
  metallic-workflow F0 = mix(0.03, baseColor, metallic) (PixelParams.diffuseF0),
  instead of roughness-independent Lambert. Shader only: rebuild matc (filamat) and the
  materials; the engine library is unchanged. FilamentLight's light power has
  no 0.8 factor any more, so materials compiled with a matc without 0005 come
  out ~25% too bright in the diffuse.
- 0007 (on top of 0005; 0006 is reserved for the ambient patch): per-view
  switch back to Filament's stock diffuse.
  The standard model's diffuseLobe returns roughness-independent Lambert
  (Filament's own) when material global 3.z is non-zero
  (View::setMaterialGlobal(3, ...); Filament's default is (0, 0, 0, 1), so
  every view keeps 0005's ogre2 diffuse unless asked). gz-rendering sets it
  on its quality-mode views only (GUI and RGB cameras, the default;
  GZ_FILAMENT_QUALITY=0 turns it off). Shader only, like 0005;
  MATERIAL_VERSION 81, so materials from a matc without 0007 are refused
  instead of silently ignoring the switch.
  The switch overwrites Fd after the ogre2 lobe in surfaceShading: an early
  return inside diffuseLobe changed one Vulkan rgb pixel by 1 LSB (Intel
  Mesa), the overwrite keeps every sensor dump byte-identical. If 0006 (the
  ambient patch) also bumps MATERIAL_VERSION, 0007 takes the next number.

Apply with `git apply` in the Filament checkout, then configure with
-DFILAMENT_ENABLE_EXPERIMENTAL_GCC_SUPPORT=ON -DFILAMENT_SUPPORTS_EGL_ON_LINUX=ON
(see ../tools/gzsim/build_in_container.sh for the full command line).
