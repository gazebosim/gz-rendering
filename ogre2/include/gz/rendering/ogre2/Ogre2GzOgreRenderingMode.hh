/*
 * Copyright (C) 2021 Open Source Robotics Foundation
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
#ifndef GZ_RENDERING_OGRE2_OGRE2GZOGRERENDERINGMODE_HH_
#define GZ_RENDERING_OGRE2_OGRE2GZOGRERENDERINGMODE_HH_

#include "gz/rendering/config.hh"
#include "gz/rendering/ogre2/Export.hh"

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE
    {
      /// \brief Rendering modes so that GzHlms implementations
      /// follow alternate code paths or extra customizations
      /// when they're enabled
      enum IgnOgreRenderingMode
      {
        /// \brief Normal rendering
        IORM_NORMAL,

        /// \brief Render a solid color explicit per object.
        /// Used by e.g. Segmentation camera mode
        IORM_SOLID_COLOR,

        /// \brief Like IORM_SOLID_COLOR, but if CustomParameter 2u
        /// is present, raw diffuse texture will be multiplied against
        /// the solid colour.
        ///
        /// Also Unlit will behave as if IORM_NORMAL
        ///
        /// Used by thermal camera
        IORM_SOLID_THERMAL_COLOR_TEXTURED,

        /// \brief Total number of rendering modes
        IORM_COUNT,
      };
    }  // namespace GZ_RENDERING_VERSION_NAMESPACE
  }    // namespace rendering
}  // namespace gz

#endif
