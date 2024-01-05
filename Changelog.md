## Ignition Rendering

### Gazebo Rendering 3.7.2 (2024-01-05)

1. Update github action workflows
    * [Pull request #940](https://github.com/gazebosim/gz-rendering/pull/940)

1. Add minor comments to BaseGizmoVisual
    * [Pull request #881](https://github.com/gazebosim/gz-rendering/pull/881)

1. CI workflow: use checkout v3
    * [Pull request #834](https://github.com/gazebosim/gz-rendering/pull/834)

1. Rename COPYING to LICENSE
    * [Pull request #833](https://github.com/gazebosim/gz-rendering/pull/833)

### Ignition Rendering 3.7.1 (2023-02-03)

1. Remove fini to resolve segfaault at shutdown.
    * [Pull request #813](https://github.com/gazebosim/gz-rendering/pull/813)

### Ignition Rendering 3.7.0 (2022-11-29)

1. Migrate ignition to gazebo headers.
    * [Pull request #705](https://github.com/gazebosim/gz-rendering/pull/705)

### Ignition Rendering 3.6.1 (2022-11-08)

1. Improved coverage: MeshDescriptor, Mesh, MoveToHelper, OrbitViewController, PixelFormat and ShadersParams
    * [Pull request #748](https://github.com/gazebosim/gz-rendering/pull/748)

1. Suppress Windows warnings
    * [Pull request #749](https://github.com/gazebosim/gz-rendering/pull/749)

1. Improved RenderEngine and Mesh coverage
    * [Pull request #739](https://github.com/gazebosim/gz-rendering/pull/739)

1. Increase TransformControl test converage (#668)
    * [Pull request #741](https://github.com/gazebosim/gz-rendering/pull/741)

1. Increased Marker coverage
    * [Pull request #738](https://github.com/gazebosim/gz-rendering/pull/738)

1. Added shaderType converage
    * [Pull request #740](https://github.com/gazebosim/gz-rendering/pull/740)

1. Improved grid coverage
    * [Pull request #737](https://github.com/gazebosim/gz-rendering/pull/737)

1. Improved coverage Arrow and Axis visuals and fixed some warnings
    * [Pull request #736](https://github.com/gazebosim/gz-rendering/pull/736)

1. Backport from 6 to 3. Disable C4275 warnings on Windows from OgreUTFString (#621)
    * [Pull request #707](https://github.com/gazebosim/gz-rendering/pull/707)

1. Remove redundant namespace references
    * [Pull request #701](https://github.com/gazebosim/gz-rendering/pull/701)

1. Disabling flaky depth and thermal camera tests on mac (#405)
    * [Pull request #696](https://github.com/gazebosim/gz-rendering/pull/696)

1. Change IGN_DESIGNATION to GZ_DESIGNATION (#677)
    * [Pull request #691](https://github.com/gazebosim/gz-rendering/pull/691)

1. Add coverage ignore file
    * [Pull request #689](https://github.com/gazebosim/gz-rendering/pull/689)

1. Ignition -> Gazebo
    * [Pull request #674](https://github.com/gazebosim/gz-rendering/pull/674)

1. fix checking unofficial ogre version when ogre 1.x is not installed
    * [Pull request #648](https://github.com/gazebosim/gz-rendering/pull/648)

1. Updated CODEOWNERS
    * [Pull request #612](https://github.com/gazebosim/gz-rendering/pull/612)
    * [Pull request #613](https://github.com/gazebosim/gz-rendering/pull/613)
    * [Pull request #634](https://github.com/gazebosim/gz-rendering/pull/634)

1. Silence CMake policy CMP0072
    * [Pull request #528](https://github.com/gazebosim/gz-rendering/pull/528)

### Ignition Rendering 3.6.0 (2021-10-15)

1. Avoid symbol redefition to fix armel builds
    * [Pull request 457](https://github.com/ignitionrobotics/ign-rendering/pull/457)

1. Fix floating point precision bug handling alpha channel
    * [Pull request 322](https://github.com/ignitionrobotics/ign-rendering/pull/322)
    * [Pull request 333](https://github.com/ignitionrobotics/ign-rendering/pull/333)

1. Ogre: Do not assume that ogre plugins have lib prefix on macOS
    * [Pull request 454](https://github.com/ignitionrobotics/ign-rendering/pull/454)

1. Moved ScreenToPlane and ScreenToScene from ign-gui to ign-rendering
    * [Pull request 363](https://github.com/ignitionrobotics/ign-rendering/pull/363)
    * [Pull request 368](https://github.com/ignitionrobotics/ign-rendering/pull/368)

1. Relax Gaussian noise test tolerance to fix flaky test
    * [Pull request 344](https://github.com/ignitionrobotics/ign-rendering/pull/344)

1. Fix ogre2 render pass high GPU usage
    * [Pull request 342](https://github.com/ignitionrobotics/ign-rendering/pull/342)

1. Backport memory fixes found by ASAN
    * [Pull request 340](https://github.com/ignitionrobotics/ign-rendering/pull/340)

1. CMake warning on Ogre versions that are not officially supported.
   To disable the new warning, it is enough to enable the cmake option USE_UNOFFICIAL_OGRE_VERSIONS
    * [Pull request 376](https://github.com/ignitionrobotics/ign-rendering/pull/376)

### Ignition Rendering 3.5.0 (2021-05-25)

1. Include MoveTo Helper class to ign-rendering
    * [Pull request 311](https://github.com/ignitionrobotics/ign-rendering/pull/311)

1. Remove tools/code_check and update codecov
    * [Pull request 321](https://github.com/ignitionrobotics/ign-rendering/pull/321)

1. Helper function to get a scene (#320)
    * [Pull request 320](https://github.com/ignitionrobotics/ign-rendering/pull/320)

1. Fix DepthGaussianNoise test (#271)
    * [Pull request 271](https://github.com/ignitionrobotics/ign-rendering/pull/271)

1. Master branch updates (#268)
    * [Pull request 268](https://github.com/ignitionrobotics/ign-rendering/pull/268)

1. 👩🌾 Make GitHub actions tests that are flaky due to display more verbose information (#255)
    * [Pull request 255](https://github.com/ignitionrobotics/ign-rendering/pull/255)

1. Fixed OBJ textures with the same name (#239)
    * [Pull request 239](https://github.com/ignitionrobotics/ign-rendering/pull/239)

1. More verbose messages when failing to load render engine (#236)
    * [Pull request 236](https://github.com/ignitionrobotics/ign-rendering/pull/236)

### Ignition Rendering 3.4.0 (2021-02-09)

1. Clear all Windows warnings
    * [Pull request 206](https://github.com/ignitionrobotics/ign-rendering/pull/206)
    * [Pull request 215](https://github.com/ignitionrobotics/ign-rendering/pull/215)
    * [Pull request 225](https://github.com/ignitionrobotics/ign-rendering/pull/225)

1. Add laser_retro support
    * [Pull request 194](https://github.com/ignitionrobotics/ign-rendering/pull/194)

### Ignition Rendering 3.3.0 (2021-01-22)

1. Add Windows installation.
    * [Pull request 196](https://github.com/ignitionrobotics/ign-rendering/pull/196)

1. Make flaky VisualAt test more verbose.
    * [Pull request 174](https://github.com/ignitionrobotics/ign-rendering/pull/174)

1. Resolve updated codecheck issues.
    * [Pull request 173](https://github.com/ignitionrobotics/ign-rendering/pull/173)

1. Fix crash due to NaN pose values.
    * [Pull request 169](https://github.com/ignitionrobotics/ign-rendering/pull/169)

1. Improve fork experience.
    * [Pull request 165](https://github.com/ignitionrobotics/ign-rendering/pull/165)

### Ignition Rendering 3.2.0 (2020-10-13)

1. Add Custom Render Engine support
    * [Pull request 161](https://github.com/ignitionrobotics/ign-rendering/pull/161)
    * [Pull request 154](https://github.com/ignitionrobotics/ign-rendering/pull/154)
    * [Pull request 142](https://github.com/ignitionrobotics/ign-rendering/pull/142)
    * [Pull request 141](https://github.com/ignitionrobotics/ign-rendering/pull/141)

1. Add getter for Ogre2Camera
    * [Pull request 97](https://github.com/ignitionrobotics/ign-rendering/pull/97)

1. Update test configuration to use ogre2
    * [Pull request 83](https://github.com/ignitionrobotics/ign-rendering/pull/83)

1. Ogre2 GPU Ray: Cleanup all resources on destruction
    * [BitBucket pull request 258](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/258)

1. Ogre2: Add Grid
    * [BitBucket pull request 246](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/246)

1. Includes all changes up to and including those in version 2.5.1

### Ignition Rendering 3.1.0 (2020-03-05)

1. Use default material properties from ign-common PBR material when loading a mesh
    * [BitBucket pull request 242](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/242)

1. Fix build for ogre version >= 1.12.0
    * [BitBucket pull request 241](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/241)

1. Add Ogre2 DynamicRenderable class
    * [BitBucket pull request 226](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/226)

1. Skeletons for Ogre2
    * [BitBucket pull request 224](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/224)

1. Fixed gazebo scene viewer example
    * [BitBucket pull request 228](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/228)

1. Ogre2: Fix shadows by dynamically generating shadow maps
    * [BitBucket pull request 222](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/222)

1. Ogre2: Destroy resources before recreating shadows
    * [BitBucket pull request 223](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/223)

1. Fix custom shaders example
    * [BitBucket pull request 230](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/230)

1. Add API to check which engines are loaded
    * [BitBucket pull request 233](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/233)

### Ignition Rendering 3.0.0 (2019-12-10)

1. Support setting cast shadows
    * [BitBucket pull request 209](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/209)

1. Add Thermal Camera
    * [BitBucket pull request 202](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/202)

1. Add support for emissive map
    * [BitBucket pull request 207](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/207)

1. Add Marker class and port ogre dynamic lines and renderable
    * [BitBucket pull request 195](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/195)
    * [BitBucket pull request 208](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/208)

1. Support storing user data in Visual
    * [BitBucket pull request 200](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/200)

1. Add 16 bit format
    * [BitBucket pull request 201](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/201)

1. Destroy camera when the scene is destroyed
    * [BitBucket pull request 175](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/175)

1. Added function to get the number of channels from a GpuRay.
    * [BitBucket pull request 178](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/178)

1. Ogre 1.x skeletal animation changes
    * [BitBucket pull request 184](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/184)
    * [BitBucket pull request 199](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/199)
    * [BitBucket pull request 204](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/204)

### Ignition Rendering 2.5.1 (2020-10-05)

1. Limit number of shadow casting lights in ogre2
    * [Pull Request 155](https://github.com/ignitionrobotics/ign-rendering/pull/155)

1. Backport ogre2 depth camera fix
    * [Pull Request 138](https://github.com/ignitionrobotics/ign-rendering/pull/138)

### Ignition Rendering 2.5.0 (2020-08-07)

1. Add support for Gaussian noise render pass in Ogre2DepthCamera
    * [Pull Request 122](https://github.com/ignitionrobotics/ign-rendering/pull/122)

### Ignition Rendering 2.4.0 (2020-04-17)

1. Fix flaky VisualAt test
    * [BitBucket pull request 248](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/248)

1. Port windows fixes
    * [BitBucket pull request 253](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/253)

1. Backport ogre2 mesh vertices precision change.
    * [BitBucket pull request 255](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/255)

1. Backport transparency setting based on diffuse alpha
    * [BitBucket pull request 247](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/247)

### Ignition Rendering 2.3.0 (2020-02-19)

1. Add API to check which engines are loaded
    * [BitBucket pull request 233](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/233)

1. Fix custom shaders example
    * [BitBucket pull request 230](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/230)

1. Ogre2: Fix shadows by dynamically generating shadow maps
    * [BitBucket pull request 222](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/222)
    * [BitBucket pull request 223](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/223)

### Ignition Rendering 2.2.0 (2019-11-20)

1. Backport setting cast shadows
    * [BitBucket pull request 210](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/210)

1. Ogre2: Add more shadow maps
    * [BitBucket pull request 203](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/203)

### Ignition Rendering 2.1.2 (2019-09-10)

1. Enable gamma write on color texture in depth camera
    * [BitBucket pull request 193](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/193)

### Ignition Rendering 2.1.1 (2019-09-06)

1. Add checks on mouse selection
    * [BitBucket pull request 191](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/191)

### Ignition Rendering 2.1.0 (2019-08-27)

1. Support generating point cloud data in DepthCamera
    * [BitBucket pull request 187](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/187)

1. Fix GizmoVisual material memory leak
    * [BitBucket pull request 185](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/185)

1. Fix PixelFormat FLOAT32 byte and channel count
    * [BitBucket pull request 186](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/186)

### Ignition Rendering 2.0.1 (2019-07-16)

1. Clean up camera objects when destroying scene
    * [BitBucket pull request 174](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/174)
    * [BitBucket pull request 180](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/180)

### Ignition Rendering 2.0.0 (2019-05-20)

1. More ogre2 error messages for debugging mesh loading.
    * [BitBucket pull request 170](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/170)

1. Support for ogre 1.11 on windows
    * [BitBucket pull request 167](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/167)

1. Fix ogre mouse picking after window resize
    * [BitBucket pull request 168](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/168)

1. Ogre2: Support mouse picking for objects with overlay material
    * [BitBucket pull request 165](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/165)
    * [BitBucket pull request 166](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/166)

1. Add GizmoVisual class
    * [BitBucket pull request 163](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/163)

1. Add Transform Controller for translating, rotating, and scaling nodes.
    * [BitBucket pull request 162](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/162)

1. Add depth check and depth write properties to Material
    * [BitBucket pull request 160](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/160)

1. Move scale functions from Visual to Node class.
    * [BitBucket pull request 159](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/159)

1. Add support for Render Passes and implement Gaussian Noise pass
    * [BitBucket pull request 154](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/154)
    * [BitBucket pull request 155](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/155)

### Ignition Rendering 1.X

1. Ogre2: Fix loading mesh with large index count
    * [BitBucket pull request 156](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/156)

1. Move OgreDepthCameraPrivate class to src file so it is not installed.
    * [BitBucket pull request 150](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-rendering/pull-requests/150)

### Ignition Rendering 1.0.0 (2019-02-28)
