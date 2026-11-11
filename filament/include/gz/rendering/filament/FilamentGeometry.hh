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
#ifndef GZ_RENDERING_FILAMENT_FILAMENTGEOMETRY_HH_
#define GZ_RENDERING_FILAMENT_FILAMENTGEOMETRY_HH_

#include <utils/Entity.h>

#include "gz/rendering/config.hh"
#include "gz/rendering/base/BaseGeometry.hh"
#include "gz/rendering/filament/FilamentObject.hh"
#include "gz/rendering/filament/FilamentRenderTypes.hh"
#include "gz/rendering/filament/Export.hh"

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    //
    class GZ_RENDERING_FILAMENT_VISIBLE FilamentGeometry :
      public BaseGeometry<FilamentObject>
    {
      protected: FilamentGeometry() = default;

      public: virtual ~FilamentGeometry() = default;

      public: virtual bool HasParent() const override;

      public: virtual VisualPtr Parent() const override;

      /// \brief Renderable entity (has transform + renderable components)
      public: virtual ::utils::Entity Entity() const = 0;

      /// \brief Set the filament layer mask of the renderable (0 = hidden)
      public: virtual void SetLayers(uint8_t _layers) = 0;

      protected: virtual void SetParent(FilamentVisualPtr _parent);

      protected: FilamentVisualPtr parent;

      private: friend class FilamentVisual;
      private: friend class FilamentHeightmap;

      private: friend class FilamentCapsule;
      private: friend class FilamentMarker;
    };
    }
  }
}
#endif
