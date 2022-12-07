/*
 * Copyright (C) 2022 Open Source Robotics Foundation
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
#ifndef GZ_RENDERING_LUXCOREENGINE_LUXCOREENGINESENSOR_HH_
#define GZ_RENDERING_LUXCOREENGINE_LUXCOREENGINESENSOR_HH_

#include "gz/rendering/base/BaseSensor.hh"

#include "LuxCoreEngineNode.hh"
#include "LuxCoreEngineRenderTypes.hh"

namespace gz {
namespace rendering {
inline namespace GZ_RENDERING_VERSION_NAMESPACE {

/// \brief LuxCore implementation of the sensor class
class LuxCoreEngineSensor : public BaseSensor<LuxCoreEngineNode>
{
  /// \brief Constructor
  protected: LuxCoreEngineSensor();

  /// \brief Destructor
  public: virtual ~LuxCoreEngineSensor();
};

} // namespace GZ_RENDERING_VERSION_NAMESPACE
} // namespace rendering
} // namespace gz

#endif
