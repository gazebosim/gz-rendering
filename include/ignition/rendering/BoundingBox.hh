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
#ifndef IGNITION_RENDERING_BOUNDINGBOX_HH_
#define IGNITION_RENDERING_BOUNDINGBOX_HH_

#include "ignition/rendering/config.hh"
#include "ignition/rendering/Export.hh"

namespace ignition
{
namespace rendering
{
inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {
  class BoundingBoxPrivate;

  /// \brief 2D or 3D Bounding box. It stores the
  /// position / orientation / size info of the box and its label
  class IGNITION_RENDERING_VISIBLE BoundingBox
  {
    /// \internal
    /// \brief Private data
    IGN_COMMON_WARN_IGNORE__DLL_INTERFACE_MISSING
    private: std::unique_ptr<BoundingBoxPrivate> dataPtr;
    IGN_COMMON_WARN_RESUME__DLL_INTERFACE_MISSING
  };
}
}
}
#endif
