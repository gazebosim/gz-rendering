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
#ifndef GZ_RENDERING_FILAMENT_FILAMENTHEIGHTMAP_HH_
#define GZ_RENDERING_FILAMENT_FILAMENTHEIGHTMAP_HH_

#include <memory>

#include "gz/rendering/config.hh"
#include "gz/rendering/base/BaseHeightmap.hh"
#include "gz/rendering/filament/FilamentGeometry.hh"
#include "gz/rendering/filament/FilamentRenderTypes.hh"
#include "gz/rendering/filament/Export.hh"

namespace gz
{
  namespace common
  {
    class Mesh;
  }
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    //
    /// \brief Heightmap as one static triangle grid with the vertices of
    /// ogre2's Terra at full resolution (no LOD, no skirts). It is drawn
    /// through an internal FilamentMesh, so every sensor that walks
    /// FilamentScene::Meshes() sees it.
    class GZ_RENDERING_FILAMENT_VISIBLE FilamentHeightmap :
      public BaseHeightmap<FilamentGeometry>
    {
      protected: explicit FilamentHeightmap(const HeightmapDescriptor &_desc);

      public: virtual ~FilamentHeightmap();

      public: virtual void Destroy() override;

      public: virtual void PreRender() override;

      public: virtual ::utils::Entity Entity() const override;

      public: virtual void SetLayers(uint8_t _layers) override;

      /// \brief No-op, like ogre2: the terrain keeps its own material
      public: virtual void SetMaterial(MaterialPtr _material,
                  bool _unique) override;

      /// \brief nullptr, like ogre2
      public: virtual MaterialPtr Material() const override;

      protected: virtual void Init() override;

      protected: virtual void SetParent(FilamentVisualPtr _parent) override;

      /// \brief Source grid, referenced by the mesh descriptor
      private: std::unique_ptr<common::Mesh> grid;

      /// \brief Renderable built from grid
      private: FilamentMeshPtr mesh;

      private: friend class FilamentScene;
    };
    }
  }
}
#endif
