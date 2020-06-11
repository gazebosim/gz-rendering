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
#ifndef IGNITION_RENDERING_LIDARVISUAL_HH_
#define IGNITION_RENDERING_LIDARVISUAL_HH_

#include <ignition/common/Time.hh>
#include <ignition/math/Color.hh>
#include <ignition/math/Vector3.hh>
#include "ignition/rendering/config.hh"
#include "ignition/rendering/Visual.hh"
#include "ignition/rendering/Object.hh"
#include "ignition/rendering/RenderTypes.hh"
#include "ignition/rendering/Marker.hh"

namespace ignition
{
  namespace rendering
  {
    inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {
    //
    /// \brief Enum for LidarVisual types
	typedef MarkerType LidarVisualType;
  

    /// \class LidarVisual LidarVisual.hh ignition/rendering/LidarVisual
    /// \brief A LidarVisual geometry class. The visual appearance is based
    /// on the type specified.
    class IGNITION_RENDERING_VISIBLE LidarVisual :
      public virtual Visual
    {
      protected: LidarVisual();

      /// \brief Destructor
      public: virtual ~LidarVisual();

      /// \brief Clear the points of the LidarVisual, if applicable
      public: virtual void ClearPoints() = 0;

      /// \brief Update the Visual
      public: virtual void Update() = 0;

      /// \brief Callback when the senspr data is recieved
      /// \param[in] TODO correct message type
      public: virtual void OnMsg(std::vector<double> &msg) = 0;

    };
    }
  }
}

#endif
