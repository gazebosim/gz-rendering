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
      /// \brief Constructor
      protected: FilamentGrid();

      /// \brief Destructor
      public: virtual ~FilamentGrid();

      // Documentation inherited
      public: virtual void Init() override;

      /// \brief Rebuild the lines if the grid changed, and color them like
      /// ogre2 shades them: the scene's ambient light times the material's
      /// diffuse color, divided by pi.
      public: virtual void PreRender() override;

      // Documentation inherited
      public: virtual void Destroy() override;

      // Documentation inherited
      public: virtual void SetMaterial(MaterialPtr _material,
                  bool _unique = true) override;

      /// \brief Rebuild the lines of the grid from its cell counts and cell
      /// length
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
