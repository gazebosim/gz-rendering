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
#ifndef GZ_RENDERING_FILAMENT_FILAMENTGRID_HH_
#define GZ_RENDERING_FILAMENT_FILAMENTGRID_HH_

#include "gz/rendering/base/BaseGrid.hh"
#include "gz/rendering/filament/FilamentDynamicGeometry.hh"
#include "gz/rendering/filament/Export.hh"

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    /// \brief Grid of lines, same layout as ogre2's grid
    class GZ_RENDERING_FILAMENT_VISIBLE FilamentGrid :
      public BaseGrid<FilamentDynamicGeometry>
    {
      protected: FilamentGrid();

      public: virtual ~FilamentGrid();

      public: virtual void Init() override;

      public: virtual void PreRender() override;

      public: virtual void Destroy() override;

      public: virtual void SetMaterial(MaterialPtr _material,
                  bool _unique = true) override;

      private: void Create();

      /// \brief Copy of the material the lines are drawn with, colored like
      /// ogre2 shades them (see PreRender)
      private: MaterialPtr shaded;

      private: friend class FilamentScene;
    };
    }
  }
}
#endif
