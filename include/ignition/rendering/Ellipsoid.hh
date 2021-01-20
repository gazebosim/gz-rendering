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

#ifndef IGNITION_RENDERING_ELLIPSOID_HH_
#define IGNITION_RENDERING_ELLIPSOID_HH_

#include "ignition/math/Vector3.hh"

#include "ignition/rendering/config.hh"
#include "ignition/rendering/Geometry.hh"
#include "ignition/rendering/Object.hh"

namespace ignition
{
  namespace rendering
  {
    inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {

    /// \class EllipsoidVisual EllipsoidVisual.hh
    /// ignition/rendering/EllipsoidVisual
    /// \brief A EllipsoidVisual geometry class. The visual appearance is based
    /// on the type specified.
    class IGNITION_RENDERING_VISIBLE Ellipsoid :
      public virtual Geometry
    {
      /// \brief Destructor
      public: virtual ~Ellipsoid() { }

      public: virtual void SetRadii(const ignition::math::Vector3d &_radii) = 0;

      public: virtual ignition::math::Vector3d Radii() = 0;
    };
    }
  }
}
#endif
