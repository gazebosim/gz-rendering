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
#ifndef GZ_RENDERING_FILAMENT_FILAMENTLIGHTVISUAL_HH_
#define GZ_RENDERING_FILAMENT_FILAMENTLIGHTVISUAL_HH_

#include <memory>

#include "gz/rendering/base/BaseLightVisual.hh"
#include "gz/rendering/filament/FilamentDynamicMesh.hh"
#include "gz/rendering/filament/FilamentVisual.hh"
#include "gz/rendering/filament/Export.hh"

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    /// \brief Lines that show a light's type, range and cone
    class GZ_RENDERING_FILAMENT_VISIBLE FilamentLightVisual :
      public BaseLightVisual<FilamentVisual>
    {
      /// \brief Constructor
      protected: FilamentLightVisual();

      /// \brief Destructor
      public: virtual ~FilamentLightVisual();

      /// \brief Initialize the visual and create its lines.
      public: virtual void Init() override;

      // Documentation inherited
      public: virtual void PreRender() override;

      // Documentation inherited
      public: virtual void Destroy() override;

      // Documentation inherited
      public: virtual void SetMaterial(MaterialPtr _material,
                                       bool _unique) override;

      /// \brief Build the lines from CreateVisualLines(), creating the
      /// dynamic mesh on first use.
      private: void CreateVisual();

      /// \brief Dynamic mesh of the light's lines.
      private: std::shared_ptr<FilamentDynamicMesh> lines;

      /// \brief True when material is a clone this visual owns and destroys.
      private: bool ownsMaterial{false};

      private: friend class FilamentScene;
    };
    }
  }
}
#endif
