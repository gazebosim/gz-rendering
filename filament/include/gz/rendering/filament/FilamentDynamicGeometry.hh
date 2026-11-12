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
#ifndef GZ_RENDERING_FILAMENT_FILAMENTDYNAMICGEOMETRY_HH_
#define GZ_RENDERING_FILAMENT_FILAMENTDYNAMICGEOMETRY_HH_

#include "gz/rendering/filament/FilamentDynamicMesh.hh"
#include "gz/rendering/filament/FilamentGeometry.hh"
#include "gz/rendering/filament/FilamentRenderTypes.hh"
#include "gz/rendering/filament/Export.hh"

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    /// \brief A geometry drawn as lines by a FilamentDynamicMesh (wire box,
    /// grid): the geometry side of ogre2's Ogre2DynamicRenderable users.
    class GZ_RENDERING_FILAMENT_VISIBLE FilamentDynamicGeometry :
      public FilamentGeometry
    {
      /// \brief Constructor
      protected: FilamentDynamicGeometry();

      /// \brief Destructor
      public: virtual ~FilamentDynamicGeometry() override;

      /// \brief Create the FilamentDynamicMesh that draws the lines, as a
      /// line list.
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

      /// \brief Set the material of the lines. Lighting and shadows are
      /// turned off on the material that gets assigned.
      /// \param[in] _material Material to assign. A null material, or one
      /// from another render engine, is rejected with an error.
      /// \param[in] _unique True if _material should be cloned. The clone
      /// belongs to this geometry and is destroyed when it is replaced.
      public: virtual void SetMaterial(MaterialPtr _material,
                                       bool _unique) override;

      /// \brief Set the parent visual of this geometry and of its line mesh.
      /// \param[in] _parent New parent, nullptr for none.
      protected: virtual void SetParent(FilamentVisualPtr _parent) override;

      /// \brief Mesh that draws the lines, created by Init
      protected: FilamentDynamicMeshPtr lines;

      /// \brief Material of the lines
      private: MaterialPtr material;

      /// \brief True when material is a clone made by SetMaterial.
      private: bool ownsMaterial{false};
    };
    }
  }
}
#endif
