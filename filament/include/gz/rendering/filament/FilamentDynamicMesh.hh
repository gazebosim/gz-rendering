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
#ifndef GZ_RENDERING_FILAMENT_FILAMENTDYNAMICMESH_HH_
#define GZ_RENDERING_FILAMENT_FILAMENTDYNAMICMESH_HH_

#include <cstdint>
#include <memory>
#include <vector>

#include <gz/math/Color.hh>
#include <gz/math/Vector3.hh>

#include "gz/rendering/Marker.hh"
#include "gz/rendering/filament/FilamentMesh.hh"
#include "gz/rendering/filament/FilamentRenderTypes.hh"
#include "gz/rendering/filament/Export.hh"

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    class FilamentDynamicMesh;
    using FilamentDynamicMeshPtr = std::shared_ptr<FilamentDynamicMesh>;

    /// \brief The one primitive of a FilamentDynamicMesh.
    class GZ_RENDERING_FILAMENT_VISIBLE FilamentDynamicSubMesh :
      public FilamentSubMesh
    {
      /// \brief Points keep their own material instance, so skip the
      /// lit / unlit rebinding of a gz material
      public: virtual void PreRender() override;

      /// \brief Material instance that replaces the gz material (points)
      public: filament::MaterialInstance *custom{nullptr};
    };

    /// \brief A mesh of one primitive whose vertices are rebuilt whenever they
    /// change: lines, triangles and points, the counterpart of ogre2's
    /// Ogre2DynamicRenderable. Being a FilamentMesh, it takes part in the
    /// scene's mesh registry, so sensors override its material like any
    /// other mesh and picking finds its parent visual.
    ///
    /// Lines and triangles use the gz material (flat color). Points are
    /// camera-facing squares of Size() pixels drawn with the vertex colors,
    /// like ogre2's PointCloudPoint material.
    class GZ_RENDERING_FILAMENT_VISIBLE FilamentDynamicMesh :
      public FilamentMesh
    {
      public: FilamentDynamicMesh() = default;

      public: virtual ~FilamentDynamicMesh() = default;

      public: virtual void Destroy() override;

      public: virtual void PreRender() override;

      public: virtual void SetLayers(uint8_t _layers) override;

      /// \brief Sensors draw points as 1 pixel dots (POINTS primitive) with
      /// their material, like ogre2: the squares of gz_marker_points.mat
      /// only exist in the pass of the point material itself
      public: virtual void OverrideMaterial(
          filament::MaterialInstance *_instance, bool _alphaTest = true)
          override;

      public: virtual void OverrideMaterial(unsigned int _index,
          filament::MaterialInstance *_instance) override;

      public: virtual void RestoreMaterials() override;

      /// \brief Draw the points as dots (true, what OverrideMaterial does) or
      /// as their squares, which under a sensor's material collapse to nothing
      public: void UseDots(bool _dots);

      /// \brief Let the owner (marker, wire box...) parent this mesh
      public: void SetOwnerParent(FilamentVisualPtr _parent);

      /// \brief MT_POINTS, MT_LINE_*, MT_TRIANGLE_*
      public: void SetOperationType(MarkerType _type);

      public: void AddPoint(const math::Vector3d &_pt,
          const math::Color &_color = math::Color::White);

      public: void SetPoint(unsigned int _index, const math::Vector3d &_pt);

      public: void SetColor(unsigned int _index, const math::Color &_color);

      public: unsigned int PointCount() const;

      public: void Clear();

      /// \brief Side of the point squares, in pixels
      public: void SetPointSize(double _size);

      /// \brief Binds the material, except on points, which keep theirs
      public: virtual void SetMaterial(MaterialPtr _material,
                                       bool _unique) override;

      public: using FilamentMesh::SetMaterial;

      /// \brief Material of the lines and triangles. It is not owned.
      public: void SetLineMaterial(MaterialPtr _material);

      /// \brief Rebuild the GPU buffers if the geometry changed
      public: void Update();

      protected: virtual void Init() override;

      private: void ApplyMaterial();

      private: void Release();


      private: std::vector<math::Vector3d> vertices;

      private: std::vector<math::Color> colors;

      private: MarkerType opType{MT_LINE_STRIP};

      private: double pointSize{1.0};

      private: bool dirty{false};

      private: uint8_t layers{0xff};

      private: filament::VertexBuffer *vb{nullptr};

      private: filament::IndexBuffer *ib{nullptr};

      /// \brief One index per point, the POINTS primitive of UseDots
      private: filament::IndexBuffer *dotIb{nullptr};

      /// \brief Index count of ib
      private: size_t indexCount{0};

      private: filament::MaterialInstance *pointInstance{nullptr};

      private: std::shared_ptr<FilamentDynamicSubMesh> sub;
    };
    }
  }
}
#endif
