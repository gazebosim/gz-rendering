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
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef GZ_RENDERING_OGRE2_OGRE2SHAREDFULLSCREENRECT_HH_
#define GZ_RENDERING_OGRE2_OGRE2SHAREDFULLSCREENRECT_HH_

#include "gz/rendering/config.hh"
#include "gz/rendering/ogre2/Export.hh"

#ifdef _MSC_VER
  #pragma warning(push, 0)
#endif
#include <OgreMaterial.h>
#ifdef _MSC_VER
  #pragma warning(pop)
#endif

namespace gz
{
namespace rendering
{
inline namespace GZ_RENDERING_VERSION_NAMESPACE {

/// \brief Make Ogre's shared fullscreen rectangles stop using a material,
/// so that the material can be destroyed.
///
/// Every compositor quad pass draws with one of two rectangles shared by
/// all workspaces, and leaves its material set on it. After a sensor
/// removes its workspace and its cloned material, the rectangle still
/// holds the clone. The next quad pass then sets its own material, which
/// destroys the clone while its datablock is still linked to the rectangle:
/// Ogre asserts ("This Datablock is still being used by some Renderables").
///
/// Call it before removing a material a quad pass used.
/// \param[in] _material The material that is about to be destroyed
/// \param[in] _replacement Material for the rectangles to use instead. It
/// must outlive them, e.g. the material _material was cloned from.
GZ_RENDERING_OGRE2_HIDDEN
void ReleaseFromSharedFullscreenRects(const Ogre::MaterialPtr &_material,
    const Ogre::MaterialPtr &_replacement);
}
}  // namespace rendering
}  // namespace gz

#endif  // GZ_RENDERING_OGRE2_OGRE2SHAREDFULLSCREENRECT_HH_
