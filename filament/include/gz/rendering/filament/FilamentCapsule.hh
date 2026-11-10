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
      protected: FilamentCapsule();

      public: virtual ~FilamentCapsule();

      public: virtual void Init() override;

      public: virtual void Destroy() override;

      public: virtual void PreRender() override;

      public: virtual ::utils::Entity Entity() const override;

      public: virtual void SetLayers(uint8_t _layers) override;

      public: virtual MaterialPtr Material() const override;

      public: virtual void SetMaterial(MaterialPtr _material,
                                       bool _unique) override;

      protected: virtual void SetParent(FilamentVisualPtr _parent) override;

      private: void Update();

      private: FilamentMeshPtr mesh;

      private: MaterialPtr material;

      /// \brief True when material is a clone made by SetMaterial
      private: bool ownsMaterial{false};

      private: friend class FilamentScene;
    };
    }
  }
}
#endif
