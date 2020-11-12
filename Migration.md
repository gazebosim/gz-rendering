# Note on deprecations
A tick-tock release cycle allows easy migration to new software versions.
Obsolete code is marked as deprecated for one major release.
Deprecated code produces compile-time warnings. These warning serve as
notification to users that their code should be upgraded. The next major
release will remove the deprecated code.


## Ignition Rendering 4.0 to 4.1

## ABI break

1. **ogre2/include/ignition/rendering/ogre2/Ogre2DepthCamera.hh**
    + Medium severity ABI break with the addition of the `AddRenderPass` override.

## Ignition Rendering 3.X to 4.X

### Deprecations

1. **ignition::common::Time** deprecated in favor of **std::chrono::steady_clock::duration**
    + Deprecated: `void BaseScene::SetSimTime(const common::Time &_time)`
    + Replacement: `void BaseScene::SetTime(const std::chrono::steady_clock::duration &_time)`
    + Deprecated: `common::Time SimTime() const`
    + Replacement: `std::chrono::steady_clock::duration Time() const`
    + Deprecated: `common::Time simTime`
    + Replacement: `std::chrono::steady_clock::duration time`

## Ignition Rendering 2.X to 3.X

### Deletions

1. **Ogre2DepthCamera.hh**
    + Removed unused member variables `captureData` and `newData`

## Ignition Rendering 1.X to 2.X

### Modifications

1. **Visual.hh** and **Node.hh**
    + All functions for setting and getting scale properties in the Visual class
      are moved to the base Node class. These functions are now inherited by
      the Visual class.
