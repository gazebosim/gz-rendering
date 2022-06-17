/*
 * Copyright (C) 2021 Open Source Robotics Foundation
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
#ifndef GZ_RENDERING_INERTIAVISUAL_HH_
#define GZ_RENDERING_INERTIAVISUAL_HH_

#include <gz/math/Inertial.hh>
#include "gz/rendering/config.hh"
#include "gz/rendering/Object.hh"
#include "gz/rendering/RenderTypes.hh"
#include "gz/rendering/Visual.hh"

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {

    /// \class InertiaVisual InertiaVisual.hh
    /// gz/rendering/InertiaVisual.hh
    /// \brief Represents a inertia visual
    class GZ_RENDERING_VISIBLE InertiaVisual :
      public virtual Visual
    {
      /// \brief Destructor
      public: virtual ~InertiaVisual() {}

      /// \brief Set the inertial component of the visual
      /// \param[in] _inertial Inertial component of the visual
      public: virtual void SetInertial(
                  const gz::math::Inertiald &_inertial) = 0;

      /// \brief Load the Inertia visual from its pose and scale
      /// \param[in] _pose Pose of the Inertia visual
      /// \param[in] _scale Scale factor of the box visual
      public: virtual void Load(const gz::math::Pose3d &_pose,
          const gz::math::Vector3d &_scale) = 0;

      /// \brief Get the box visual
      /// \return Pointer to the box visual
      public: virtual VisualPtr BoxVisual() const = 0;
    };
    }
  }
}
#endif
