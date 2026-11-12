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
#ifndef GZ_RENDERING_FILAMENT_FILAMENTCOMVISUAL_HH_
#define GZ_RENDERING_FILAMENT_FILAMENTCOMVISUAL_HH_

#include <memory>

#include "gz/rendering/base/BaseCOMVisual.hh"
#include "gz/rendering/filament/FilamentDynamicMesh.hh"
#include "gz/rendering/filament/FilamentVisual.hh"
#include "gz/rendering/filament/Export.hh"

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    /// \brief Center of mass: a sphere of the equivalent mass and a cross
    class GZ_RENDERING_FILAMENT_VISIBLE FilamentCOMVisual :
      public BaseCOMVisual<FilamentVisual>
    {
      /// \brief Constructor
      protected: FilamentCOMVisual();

      /// \brief Destructor
      public: virtual ~FilamentCOMVisual();

      // Documentation inherited
      public: virtual void PreRender() override;

      // Documentation inherited
      public: virtual void Destroy() override;

      // Documentation inherited
      public: virtual VisualPtr SphereVisual() const override;

      /// \brief Only the cross takes the material, like ogre2.
      /// \param[in] _material Material of the cross lines
      /// \param[in] _unique True to use a clone of _material
      public: virtual void SetMaterial(MaterialPtr _material,
                                       bool _unique) override;

      /// \brief Build the sphere and the cross of lines through the center of
      /// mass. The cross reaches the parent visual's local bounding box.
      private: void CreateVisual();

      /// \brief Dynamic mesh of the cross lines.
      private: std::shared_ptr<FilamentDynamicMesh> crossLines;

      /// \brief Child visual drawing the sphere.
      private: VisualPtr sphereVis;

      /// \brief True when material is a clone this visual owns and destroys.
      private: bool ownsMaterial{false};

      private: friend class FilamentScene;
    };
    }
  }
}
#endif
