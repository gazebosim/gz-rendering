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
#ifndef GZ_RENDERING_FILAMENT_FILAMENTCAPSULE_HH_
#define GZ_RENDERING_FILAMENT_FILAMENTCAPSULE_HH_

#include "gz/rendering/base/BaseCapsule.hh"
#include "gz/rendering/filament/FilamentGeometry.hh"
#include "gz/rendering/filament/FilamentRenderTypes.hh"
#include "gz/rendering/filament/Export.hh"

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    /// \brief Capsule: a FilamentMesh built from common::MeshManager's
    /// capsule (same tessellation as ogre2), rebuilt when radius or length
    /// change.
    class GZ_RENDERING_FILAMENT_VISIBLE FilamentCapsule :
      public BaseCapsule<FilamentGeometry>
    {
      /// \brief Constructor
      protected: FilamentCapsule();

      /// \brief Destructor
      public: virtual ~FilamentCapsule();

      /// \brief Initialize the capsule and build its mesh.
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

      /// \brief Set the parent visual of this capsule and of its mesh.
      /// \param[in] _parent Parent visual, nullptr for none
      protected: virtual void SetParent(FilamentVisualPtr _parent) override;

      /// \brief Rebuild the mesh from the radius and length and re-attach
      /// it under the parent visual.
      private: void Update();

      /// \brief Mesh holding the capsule's renderable.
      private: FilamentMeshPtr mesh;

      /// \brief Material of the capsule, kept when the mesh is rebuilt.
      private: MaterialPtr material;

      /// \brief True when material is a clone made by SetMaterial
      private: bool ownsMaterial{false};

      private: friend class FilamentScene;
    };
    }
  }
}
#endif
