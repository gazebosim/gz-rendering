/*
 * Copyright (C) 2019 Open Source Robotics Foundation
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
#ifndef IGNITION_RENDERING_MARKER_HH_
#define IGNITION_RENDERING_MARKER_HH_

#include <string>
#include "ignition/rendering/config.hh"
#include "ignition/rendering/Geometry.hh"
#include "ignition/rendering/Object.hh"
#include "ignition/common/Time.hh"

namespace ignition
{
  namespace rendering
  {
    inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {
    //
    /// \class Marker Marker.hh ignition/rendering/Marker
    /// \brief TODO(jshep1): update description of this and all below funcs.
    class IGNITION_RENDERING_VISIBLE Marker :
      public virtual Geometry
    {
      /// \brief Destructor
      public: virtual ~Marker() { }

      public: virtual void setLifetime(common::Time _lifetime) = 0;

      public: virtual common::Time Lifetime() const = 0;

      public: virtual void SetLayer(int32_t _layer) = 0;

      public: virtual int32_t Layer() const = 0;
    };
    }
  }
}

#endif
