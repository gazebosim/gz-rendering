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
#ifndef GZ_RENDERING_FILAMENT_FILAMENTJOINTVISUAL_HH_
#define GZ_RENDERING_FILAMENT_FILAMENTJOINTVISUAL_HH_

#include <memory>
#include <vector>

#include "gz/rendering/base/BaseJointVisual.hh"
#include "gz/rendering/filament/FilamentDynamicMesh.hh"
#include "gz/rendering/filament/FilamentVisual.hh"
#include "gz/rendering/filament/Export.hh"

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    /// \brief Joint axes and parent axes, built by BaseJointVisual
    class GZ_RENDERING_FILAMENT_VISIBLE FilamentJointVisual :
      public BaseJointVisual<FilamentVisual>
    {
      protected: FilamentJointVisual();

      public: virtual ~FilamentJointVisual();


      private: friend class FilamentScene;
    };
    }
  }
}
#endif
