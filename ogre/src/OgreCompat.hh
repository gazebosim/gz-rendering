/*
 * Copyright (C) 2026 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#ifndef GZ_RENDERING_OGRE_OGRECOMPAT_HH_
#define GZ_RENDERING_OGRE_OGRECOMPAT_HH_

// Internal-only header (not installed). Provides forward-compatible aliases
// for OGRE 1.12.9+ APIs so the gz-rendering OGRE-1 backend can keep building
// against the older 1.9.x system package without scattering #if-blocks at
// every call site. The aliases shadow what 1.12.9+ already provides natively.

#include "gz/rendering/ogre/OgreIncludes.hh"

#ifdef OGRE_VERSION_LT_1_12_9
namespace Ogre
{
  // Modern (1.12.9+) free-scope buffer-usage enum names map to the
  // deprecated nested-scope HardwareBuffer::Usage values on older OGRE.
  constexpr auto HBU_GPU_ONLY    = HardwareBuffer::HBU_STATIC_WRITE_ONLY;
  constexpr auto HBU_CPU_TO_GPU  = HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY;
}
#endif

#endif
