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
#ifndef GZ_RENDERING_FILAMENT_FILAMENTFRUSTUMVISUAL_HH_
#define GZ_RENDERING_FILAMENT_FILAMENTFRUSTUMVISUAL_HH_

#include <memory>

#include "gz/rendering/base/BaseFrustumVisual.hh"
#include "gz/rendering/filament/FilamentDynamicMesh.hh"
#include "gz/rendering/filament/FilamentVisual.hh"
#include "gz/rendering/filament/Export.hh"

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    /// \brief The frustum of a camera drawn as lines
    class GZ_RENDERING_FILAMENT_VISIBLE FilamentFrustumVisual :
      public BaseFrustumVisual<FilamentVisual>
    {
      protected: FilamentFrustumVisual();

      public: virtual ~FilamentFrustumVisual();

      public: virtual void Destroy() override;

      public: virtual void Update() override;

      private: void ClearVisualData();

      private: std::shared_ptr<FilamentDynamicMesh> rayLines;

      private: friend class FilamentScene;
    };
    }
  }
}
#endif
