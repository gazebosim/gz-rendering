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
    class GZ_RENDERING_FILAMENT_VISIBLE FilamentVisual :
      public BaseVisual<FilamentNode>
    {
      protected: FilamentVisual();

      public: virtual ~FilamentVisual();

      public: virtual void SetVisible(bool _visible) override;

      public: virtual void SetVisibilityFlags(uint32_t _flags) override;

      /// \brief Draw this visual's meshes as edges (filament has no
      /// wireframe raster mode)
      public: virtual void SetWireframe(bool _show) override;

      public: virtual math::AxisAlignedBox LocalBoundingBox() const override;

      public: virtual math::AxisAlignedBox BoundingBox() const override;

      /// \brief Merge the boxes of this visual's geometries and its
      /// descendants', in the frame of _pose (ogre2's BoundsHelper)
      private: void BoundsHelper(math::AxisAlignedBox &_box, bool _local,
          const math::Pose3d &_pose) const;

      protected: virtual GeometryStorePtr Geometries() const override;

      protected: virtual bool AttachGeometry(GeometryPtr _geometry) override;

      protected: virtual bool DetachGeometry(GeometryPtr _geometry) override;

      /// \brief Add (or remove) this visual's and its descendants'
      /// renderables to (from) the filament scene
      /// \param[in] _inScene True to add, false to remove
      protected: virtual void SetInScene(bool _inScene) override;

      protected: virtual void Init() override;

      /// \brief Push the current visibility state to the geometries
      protected: void UpdateLayers();

      protected: FilamentGeometryStorePtr geometries;

      protected: bool visible{true};

      private: FilamentVisualPtr SharedThis();

      private: friend class FilamentScene;

      private: friend class FilamentCapsule;

      private: friend class FilamentMarker;
    };
    }
  }
}
#endif
