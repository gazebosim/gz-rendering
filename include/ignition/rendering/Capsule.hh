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

#ifndef IGNITION_RENDERING_CAPSULE_HH_
#define IGNITION_RENDERING_CAPSULE_HH_

#include "ignition/rendering/config.hh"
#include "ignition/rendering/Geometry.hh"
#include "ignition/rendering/Object.hh"

namespace ignition
{
  namespace rendering
  {
    inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {

    /// \class CapsuleVisual CapsuleVisual.hh ignition/rendering/CapsuleVisual
    /// \brief A CapsuleVisual geometry class. The visual appearance is based
    /// on the type specified.
    class IGNITION_RENDERING_VISIBLE Capsule :
      public virtual Geometry
    {
      /// \brief Destructor
      public: virtual ~Capsule() { }

      public: virtual void SetRadius(const double _radius) = 0;

      public: virtual void SetLength(const double _length) = 0;

      public: virtual double Radius() = 0;

      public: virtual double Length() = 0;
    };
    }
  }
}
#endif
