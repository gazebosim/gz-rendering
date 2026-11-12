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
    /// \brief Base class of the filament geometries: a geometry with a
    /// renderable entity and filament layers, attached to a visual.
    class GZ_RENDERING_FILAMENT_VISIBLE FilamentGeometry :
      public BaseGeometry<FilamentObject>
    {
      /// \brief Constructor
      protected: FilamentGeometry() = default;

      /// \brief Destructor
      public: virtual ~FilamentGeometry() = default;

      // Documentation inherited
      public: virtual bool HasParent() const override;

      // Documentation inherited
      public: virtual VisualPtr Parent() const override;

      /// \brief Get the renderable entity (has transform + renderable
      /// components).
      /// \return The renderable entity, null if there is none.
      public: virtual ::utils::Entity Entity() const = 0;

      /// \brief Set the filament layer mask of the renderable (0 = hidden).
      /// \param[in] _layers Filament layer mask.
      public: virtual void SetLayers(uint8_t _layers) = 0;

      /// \brief Store the parent visual. FilamentVisual calls it when the
      /// geometry is attached or detached.
      /// \param[in] _parent New parent visual, null when detached.
      protected: virtual void SetParent(FilamentVisualPtr _parent);

      /// \brief Visual this geometry is attached to, null if there is none.
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
