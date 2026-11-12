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
#ifndef GZ_RENDERING_FILAMENT_FILAMENTMARKER_HH_
#define GZ_RENDERING_FILAMENT_FILAMENTMARKER_HH_

#include "gz/rendering/base/BaseMarker.hh"
#include "gz/rendering/filament/FilamentDynamicMesh.hh"
#include "gz/rendering/filament/FilamentGeometry.hh"
#include "gz/rendering/filament/FilamentRenderTypes.hh"
#include "gz/rendering/filament/Export.hh"

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    /// \brief Marker: box, capsule, cone, cylinder and sphere markers are
    /// the scene's geometries; lines, points and triangles are a
    /// FilamentDynamicMesh. The marker material is always flat and
    /// shadowless, like ogre2.
    class GZ_RENDERING_FILAMENT_VISIBLE FilamentMarker :
      public BaseMarker<FilamentGeometry>
    {
      /// \brief Constructor
      protected: FilamentMarker();

      /// \brief Destructor
      public: virtual ~FilamentMarker();

      /// \brief Initialize the marker and create its dynamic mesh.
      public: virtual void Init() override;

      // Documentation inherited
      public: virtual void Destroy() override;

      // Documentation inherited
      public: virtual void PreRender() override;

      // Documentation inherited
      public: virtual ::utils::Entity Entity() const override;

      // Documentation inherited
      public: virtual void SetLayers(uint8_t _layers) override;

      // Documentation inherited
      public: virtual MaterialPtr Material() const override;

      // Documentation inherited
      public: virtual void SetMaterial(MaterialPtr _material,
                                       bool _unique) override;

      // Documentation inherited
      public: virtual void SetType(MarkerType _markerType) override;

      // Documentation inherited
      public: virtual void SetSize(double _size) override;

      public: using BaseMarker<FilamentGeometry>::AddPoint;

      /// \brief Add a point with its color to the dynamic mesh.
      /// \param[in] _pt Position of the point
      /// \param[in] _color Color of the point
      public: virtual void AddPoint(const math::Vector3d &_pt,
                                    const math::Color &_color) override;

      /// \brief Set the position of an existing point of the dynamic mesh.
      /// \param[in] _index Index of the point
      /// \param[in] _value New position of the point
      public: virtual void SetPoint(unsigned int _index,
                                    const math::Vector3d &_value) override;

      // Documentation inherited
      public: virtual void ClearPoints() override;

      /// \brief Set the parent visual of this marker and of the geometry and
      /// dynamic mesh it holds.
      /// \param[in] _parent Parent visual, nullptr for none
      protected: virtual void SetParent(FilamentVisualPtr _parent) override;

      /// \brief Shape of the geometry types, null for the dynamic ones
      private: FilamentGeometryPtr geom;

      /// \brief Lines, points and triangles (always created)
      private: FilamentDynamicMeshPtr dynamic;

      /// \brief Material assigned by SetMaterial, flat and shadowless.
      private: MaterialPtr material;

      /// \brief True when material is a clone made by SetMaterial
      private: bool ownsMaterial{false};

      /// \brief Layer mask last given to SetLayers.
      private: uint8_t layers{0xff};

      private: friend class FilamentScene;
    };
    }
  }
}
#endif
