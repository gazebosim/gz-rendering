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
      /// \brief Constructor
      public: FilamentDynamicMesh() = default;

      /// \brief Destructor
      public: virtual ~FilamentDynamicMesh() override = default;

      // Documentation inherited
      public: virtual void Destroy() override;

      /// \brief Rebuild the GPU buffers if the points changed (see Update),
      /// then do the regular mesh pre-render.
      public: virtual void PreRender() override;

      /// \brief Set the filament layer mask of the renderable. It is applied
      /// again each time Update rebuilds the renderable.
      /// \param[in] _layers Layer mask, 0 hides the mesh.
      public: virtual void SetLayers(uint8_t _layers) override;

      /// \brief Render with _instance, drawing the points as dots.
      /// Sensors draw points as 1 pixel dots (POINTS primitive) with their
      /// material, like ogre2: the squares of gz_marker_points.mat only exist
      /// in the pass of the point material itself.
      /// \param[in] _instance Instance to render with.
      /// \param[in] _alphaTest Whether alpha tested submeshes discard their
      /// cut-out texels in _instance too, see FilamentMesh::OverrideMaterial.
      public: virtual void OverrideMaterial(
          filament::MaterialInstance *_instance, bool _alphaTest = true)
          override;

      // Documentation inherited
      public: virtual void OverrideMaterial(unsigned int _index,
          filament::MaterialInstance *_instance) override;

      // Documentation inherited
      public: virtual void RestoreMaterials() override;

      /// \brief Draw the points as dots (true, what OverrideMaterial does) or
      /// as their squares, which under a sensor's material collapse to
      /// nothing. Does nothing unless the mesh holds points.
      /// \param[in] _dots True for 1 pixel dots, false for the squares.
      public: void UseDots(bool _dots);

      /// \brief Let the owner (marker, wire box...) parent this mesh
      /// \param[in] _parent Visual that parents this mesh, nullptr for none.
      public: void SetOwnerParent(FilamentVisualPtr _parent);

      /// \brief Set how the points are drawn. Types other than MT_POINTS,
      /// MT_LINE_LIST, MT_LINE_STRIP, MT_TRIANGLE_LIST, MT_TRIANGLE_STRIP and
      /// MT_TRIANGLE_FAN are rejected with an error.
      /// \param[in] _type Type of primitive the points make up.
      public: void SetOperationType(MarkerType _type);

      /// \brief Append a point.
      /// \param[in] _pt Position of the point.
      /// \param[in] _color Color of the point.
      public: void AddPoint(const math::Vector3d &_pt,
          const math::Color &_color = math::Color::White);

      /// \brief Move a point. An index out of range is reported and ignored.
      /// \param[in] _index Index of the point.
      /// \param[in] _pt New position of the point.
      public: void SetPoint(unsigned int _index, const math::Vector3d &_pt);

      /// \brief Set the color of a point. An index out of range is reported
      /// and ignored.
      /// \param[in] _index Index of the point.
      /// \param[in] _color New color of the point.
      public: void SetColor(unsigned int _index, const math::Color &_color);

      /// \brief Get the number of points.
      /// \return Number of points added and not cleared.
      public: unsigned int PointCount() const;

      /// \brief Remove all the points.
      public: void Clear();

      /// \brief Set the side of the point squares, in pixels.
      /// \param[in] _size Side of the squares, in pixels.
      public: void SetPointSize(double _size);

      /// \brief Binds the material, except on points, which keep theirs
      /// \param[in] _material Material to bind.
      /// \param[in] _unique True if _material should be cloned.
      public: virtual void SetMaterial(MaterialPtr _material,
                                       bool _unique) override;

      public: using FilamentMesh::SetMaterial;

      /// \brief Set the material of the lines and triangles. It is not owned.
      /// \param[in] _material Material to use, ignored if null.
      public: void SetLineMaterial(MaterialPtr _material);

      /// \brief Rebuild the GPU buffers if the geometry changed
      public: void Update();

      /// \brief Create the one submesh, drawn flat unlit white until a
      /// material is set.
      protected: virtual void Init() override;

      /// \brief Bind to the submesh the point instance (for points), else
      /// the instance of its gz material, else the default gz_unlit one.
      private: void ApplyMaterial();

      /// \brief Destroy the renderable and the vertex and index buffers.
      private: void Release();


      /// \brief Positions of the points
      private: std::vector<math::Vector3d> vertices;

      /// \brief Colors of the points, one per entry of vertices
      private: std::vector<math::Color> colors;

      /// \brief Type of primitive the points make up
      private: MarkerType opType{MT_LINE_STRIP};

      /// \brief Side of the point squares, in pixels
      private: double pointSize{1.0};

      /// \brief True when vertices, colors or opType changed since the
      /// buffers were last built.
      private: bool dirty{false};

      /// \brief Filament layer mask of the renderable, all layers by default
      private: uint8_t layers{0xff};

      /// \brief Vertex buffer, null until built. Owned.
      private: filament::VertexBuffer *vb{nullptr};

      /// \brief Index buffer of the primitives, null until built. Owned.
      private: filament::IndexBuffer *ib{nullptr};

      /// \brief One index per point, the POINTS primitive of UseDots
      private: filament::IndexBuffer *dotIb{nullptr};

      /// \brief Index count of ib
      private: size_t indexCount{0};

      /// \brief Instance of gz_marker_points that draws the point squares.
      /// Created on first use, null until then. Owned.
      private: filament::MaterialInstance *pointInstance{nullptr};

      /// \brief The one submesh of this mesh
      private: std::shared_ptr<FilamentDynamicSubMesh> sub;
    };
    }
  }
}
#endif
