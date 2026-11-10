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
#ifndef GZ_RENDERING_FILAMENT_FILAMENTGIZMOVISUAL_HH_
#define GZ_RENDERING_FILAMENT_FILAMENTGIZMOVISUAL_HH_

#include <memory>
#include <vector>

#include "gz/rendering/base/BaseGizmoVisual.hh"
#include "gz/rendering/filament/FilamentVisual.hh"
#include "gz/rendering/filament/Export.hh"

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    /// \brief Translation, rotation and scale handles, built by BaseGizmoVisual
    class GZ_RENDERING_FILAMENT_VISIBLE FilamentGizmoVisual :
      public BaseGizmoVisual<FilamentVisual>
    {
      protected: FilamentGizmoVisual();

      public: virtual ~FilamentGizmoVisual();

      /// \brief BaseGizmoVisual::PreRender does not recurse: filament
      /// materials only reach the GPU when their meshes are prerendered.
      public: virtual void PreRender() override;

      private: friend class FilamentScene;
    };
    }
  }
}
#endif
