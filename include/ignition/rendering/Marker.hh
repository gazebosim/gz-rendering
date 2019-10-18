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
#include "ignition/rendering/RenderTypes.hh"
#include "ignition/common/Time.hh"

namespace ignition
{
  namespace rendering
  {
    inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {
    //
    /// \class Marker Marker.hh ignition/rendering/Marker
    class IGNITION_RENDERING_VISIBLE Marker :
      public virtual Geometry
    {
      /// \brief Destructor
      public: virtual ~Marker() { }

      public: virtual void SetLifetime(
                  const std::chrono::steady_clock::duration &_lifetime) = 0;

      public: virtual std::chrono::steady_clock::duration Lifetime() const = 0;

      public: virtual void SetLayer(const int32_t _layer) = 0;

      public: virtual int32_t Layer() const = 0;

      public: virtual void SetType(const MarkerType _markerType) = 0;

      public: virtual MarkerType Type() const = 0;

      public: virtual void SetRenderOperation(const MarkerType _markerType) = 0;

      public: virtual MarkerType RenderOperation() const = 0;

      public: virtual void ClearPoints() = 0;

      public: virtual void AddPoint(const double _x,
                  const double _y, const double _z,
                  const ignition::math::Color &_color) = 0;

      public: virtual void AddPoint(const ignition::math::Vector3d &_pt,
                  const ignition::math::Color &_color) = 0;
 
      public: virtual void SetPoint(const unsigned int _index,
                  const ignition::math::Vector3d &_value) = 0;
    };
    }
  }
}

#endif
