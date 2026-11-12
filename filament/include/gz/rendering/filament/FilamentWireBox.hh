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
#ifndef GZ_RENDERING_FILAMENT_FILAMENTWIREBOX_HH_
#define GZ_RENDERING_FILAMENT_FILAMENTWIREBOX_HH_

#include "gz/rendering/base/BaseWireBox.hh"
#include "gz/rendering/filament/FilamentDynamicGeometry.hh"
#include "gz/rendering/filament/Export.hh"

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    /// \brief Wire frame of an axis aligned box: 12 lines
    class GZ_RENDERING_FILAMENT_VISIBLE FilamentWireBox :
      public BaseWireBox<FilamentDynamicGeometry>
    {
      /// \brief Constructor
      protected: FilamentWireBox();

      /// \brief Destructor
      public: virtual ~FilamentWireBox();

      // Documentation inherited
      public: virtual void Init() override;

      // Documentation inherited
      public: virtual void PreRender() override;

      /// \brief Rebuild the 12 lines of the box from its extents
      private: void Create();

      private: friend class FilamentScene;
    };
    }
  }
}
#endif
