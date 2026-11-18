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
#ifndef GZ_RENDERING_FILAMENT_FILAMENTINERTIAVISUAL_HH_
#define GZ_RENDERING_FILAMENT_FILAMENTINERTIAVISUAL_HH_

#include <memory>

#include "gz/rendering/base/BaseInertiaVisual.hh"
#include "gz/rendering/filament/FilamentDynamicMesh.hh"
#include "gz/rendering/filament/FilamentVisual.hh"
#include "gz/rendering/filament/Export.hh"

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    /// \brief Equivalent inertia box and a cross of lines.
    class GZ_RENDERING_FILAMENT_VISIBLE FilamentInertiaVisual :
      public BaseInertiaVisual<FilamentVisual>
    {
      /// \brief Constructor
      protected: FilamentInertiaVisual();

      /// \brief Destructor
      public: virtual ~FilamentInertiaVisual();

      // Documentation inherited
      public: virtual void Destroy() override;

      /// \brief Build the box and the cross of lines.
      /// \param[in] _pose Pose of the inertia, the cross is centered on it
      /// \param[in] _scale Scale of the box visual, the cross lines reach
      /// twice this along each axis in both directions
      public: virtual void Load(const math::Pose3d &_pose,
                                const math::Vector3d &_scale) override;

      // Documentation inherited
      public: virtual VisualPtr BoxVisual() const override;

      /// \brief Only the cross takes the material, like ogre2.
      /// \param[in] _material Material of the cross lines
      /// \param[in] _unique True to use a clone of _material
      public: virtual void SetMaterial(MaterialPtr _material,
                                       bool _unique) override;

      /// \brief Dynamic mesh of the cross lines.
      private: std::shared_ptr<FilamentDynamicMesh> crossLines;

      /// \brief Child visual drawing the equivalent inertia box.
      private: VisualPtr boxVis;

      /// \brief True when material is a clone this visual owns and destroys.
      private: bool ownsMaterial{false};

      private: friend class FilamentScene;
    };
    }
  }
}
#endif
