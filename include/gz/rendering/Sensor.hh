/*
 * Copyright (C) 2015 Open Source Robotics Foundation
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
#ifndef GZ_RENDERING_SENSOR_HH_
#define GZ_RENDERING_SENSOR_HH_

#include "gz/rendering/config.hh"
#include "gz/rendering/Node.hh"

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    //
    /// \class Sensor Sensor.hh gz/rendering/Sensor.hh
    /// \brief Represents a scene sensor. The most obvious example is a camera,
    /// but it can be anything that generates output from the scene.
    class GZ_RENDERING_VISIBLE Sensor :
      public virtual Node
    {
      /// \brief Sensor
      public: virtual ~Sensor() { }

      /// \brief Set visibility mask
      /// \param[in] _mask Visibility mask
      public: virtual void SetVisibilityMask(uint32_t _mask) = 0;

      /// \brief Get visibility mask
      /// \return visibility mask
      public: virtual uint32_t VisibilityMask() const = 0;
    };
    }
  }
}
#endif
