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
#ifndef GZ_RENDERING_FILAMENT_FILAMENTVISUAL_HH_
#define GZ_RENDERING_FILAMENT_FILAMENTVISUAL_HH_

#include "gz/rendering/config.hh"
#include "gz/rendering/base/BaseVisual.hh"
#include "gz/rendering/filament/FilamentNode.hh"
#include "gz/rendering/filament/FilamentRenderTypes.hh"
#include "gz/rendering/filament/Export.hh"

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    //
    /// \brief Filament implementation of a Visual: a node that owns the
    /// geometries attached to it.
    class GZ_RENDERING_FILAMENT_VISIBLE FilamentVisual :
      public BaseVisual<FilamentNode>
    {
      /// \brief Constructor
      protected: FilamentVisual() = default;

      /// \brief Destructor
      public: virtual ~FilamentVisual() = default;

      // Documentation inherited
      public: virtual void SetVisible(bool _visible) override;

      // Documentation inherited
      public: virtual void SetVisibilityFlags(uint32_t _flags) override;

      /// \brief Draw this visual's own meshes as edges, not its children's.
      /// Filament has no wireframe raster mode.
      /// \param[in] _show True to draw edges, false for solid surfaces
      public: virtual void SetWireframe(bool _show) override;

      // Documentation inherited
      public: virtual math::AxisAlignedBox LocalBoundingBox() const override;

      // Documentation inherited
      public: virtual math::AxisAlignedBox BoundingBox() const override;

      /// \brief Merge the boxes of this visual's geometries and its
      /// descendants' into _box, like ogre2's BoundsHelper.
      /// \param[in,out] _box Box the boxes are merged into
      /// \param[in] _local True for a box in the frame of _pose, false for
      /// the world frame
      /// \param[in] _pose World pose of the visual the query started from
      private: void BoundsHelper(math::AxisAlignedBox &_box, bool _local,
          const math::Pose3d &_pose) const;

      /// \brief Get the geometries attached to this visual.
      /// \return The geometry store
      protected: virtual GeometryStorePtr Geometries() const override;

      /// \brief Parent a geometry's entity to this visual and add it to the
      /// scene if this visual is in the scene graph.
      /// \param[in] _geometry Geometry to attach
      /// \return True on success. False, with an error logged, for a null
      /// geometry, one of another render engine or one without an entity.
      protected: virtual bool AttachGeometry(GeometryPtr _geometry) override;

      /// \brief Unparent a geometry's entity from this visual and remove it
      /// from the scene.
      /// \param[in] _geometry Geometry to detach
      /// \return True on success, false, with an error logged, for a null
      /// geometry or one of another render engine.
      protected: virtual bool DetachGeometry(GeometryPtr _geometry) override;

      /// \brief Add (or remove) this visual's and its descendants'
      /// renderables to (from) the filament scene
      /// \param[in] _inScene True to add, false to remove
      protected: virtual void SetInScene(bool _inScene) override;

      /// \brief Initialize the visual and create its geometry store.
      protected: virtual void Init() override;

      /// \brief Push the current visibility state to the geometries
      protected: void UpdateLayers();

      /// \brief Geometries attached to this visual.
      protected: FilamentGeometryStorePtr geometries;

      /// \brief False after SetVisible(false): the geometries are then on no
      /// layer.
      protected: bool visible{true};

      /// \brief Get a shared pointer to this.
      /// \return Shared pointer to this
      private: FilamentVisualPtr SharedThis();

      private: friend class FilamentScene;

      private: friend class FilamentCapsule;

      private: friend class FilamentMarker;
    };
    }
  }
}
#endif
