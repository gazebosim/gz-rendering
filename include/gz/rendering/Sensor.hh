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
#ifndef IGNITION_RENDERING_SENSOR_HH_
#define IGNITION_RENDERING_SENSOR_HH_

#include "ignition/rendering/config.hh"
#include "ignition/rendering/Node.hh"

namespace ignition
{
  namespace rendering
  {
    inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {
    //
    /// \class Sensor Sensor.hh ignition/rendering/Sensor.hh
    /// \brief Represents a scene sensor. The most obvious example is a camera,
    /// but it can be anything that generates output from the scene.
    class IGNITION_RENDERING_VISIBLE Sensor :
      public virtual Node
    {
      /// \brief Sensor
      public: virtual ~Sensor() { }
    };
    }
  }
}
#endif
