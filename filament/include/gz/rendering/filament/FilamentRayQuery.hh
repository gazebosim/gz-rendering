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
#ifndef GZ_RENDERING_FILAMENT_FILAMENTRAYQUERY_HH_
#define GZ_RENDERING_FILAMENT_FILAMENTRAYQUERY_HH_

#include "gz/rendering/base/BaseRayQuery.hh"
#include "gz/rendering/filament/FilamentObject.hh"
#include "gz/rendering/filament/Export.hh"

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    /// \brief Ray cast against the triangles of the scene's meshes, like
    /// ogre2's CPU path (front faces only, hidden meshes skipped). The
    /// result is the same one a selection buffer gives, without the depth
    /// buffer's precision.
    class GZ_RENDERING_FILAMENT_VISIBLE FilamentRayQuery :
      public BaseRayQuery<FilamentObject>
    {
      protected: FilamentRayQuery();

      public: virtual ~FilamentRayQuery();

      public: using BaseRayQuery<FilamentObject>::SetFromCamera;

      // Documentation inherited
      public: virtual void SetFromCamera(const WideAngleCameraPtr &_camera,
                                         uint32_t _faceIdx,
                                         const math::Vector2d &_coord)
                                         override;

      // Documentation inherited
      public: virtual RayQueryResult ClosestPoint(
            bool _forceSceneUpdate = true) override;

      private: friend class FilamentScene;
    };
    }
  }
}
#endif
