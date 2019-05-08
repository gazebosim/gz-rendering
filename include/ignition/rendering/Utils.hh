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
#ifndef IGNITION_RENDERING_UTILS_HH_
#define IGNITION_RENDERING_UTILS_HH_

#include <ignition/math/Helpers.hh>

#include "ignition/rendering/config.hh"
#include "ignition/rendering/Export.hh"

namespace ignition
{
  /// \brief Rendering classes and function useful in robot applications.
  namespace rendering
  {
    // Inline bracket to help doxygen filtering.
    inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {
    //
    /// \brief Get the screen scaling factor.
    /// \return The screen scaling factor.
    IGNITION_RENDERING_VISIBLE
    float screenScalingFactor();
    }
  }
}
#endif
