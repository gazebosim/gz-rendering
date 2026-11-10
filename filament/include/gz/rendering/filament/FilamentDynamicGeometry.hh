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
      protected: FilamentDynamicGeometry();

      public: virtual ~FilamentDynamicGeometry();

      public: virtual void Init() override;

      public: virtual void Destroy() override;

      public: virtual void PreRender() override;

      public: virtual ::utils::Entity Entity() const override;

      public: virtual void SetLayers(uint8_t _layers) override;

      public: virtual MaterialPtr Material() const override;

      public: virtual void SetMaterial(MaterialPtr _material,
                                       bool _unique) override;

      protected: virtual void SetParent(FilamentVisualPtr _parent) override;

      protected: FilamentDynamicMeshPtr lines;

      private: MaterialPtr material;

      /// \brief True when material is a clone made by SetMaterial
      private: bool ownsMaterial{false};
    };
    }
  }
}
#endif
