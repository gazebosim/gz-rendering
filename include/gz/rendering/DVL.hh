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
#ifndef GZ_RENDERING_DVL_HH_
#define GZ_RENDERING_DVL_HH_

#include "gz/rendering/Camera.hh"
namespace ignition
{
  namespace rendering
  {
    inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {
    /// \class DVL DVL.hh
    /// gz/rendering/DVL.hh
    /// \brief Poseable Doppler Velocity Log. This class is a bit different from
    /// the other cameras as it does not output a raw image. Rather, its goal is
    /// to output an image that contains both the Depth and the Entities visible
    /// in the current frame. The velocities can be back calculated using
    /// ignition gazebo.
    class IGNITION_RENDERING_VISIBLE DopplerVelocityLog :
      public virtual Camera
    {
      /// \brief Destructor
      public: virtual ~DopplerVelocityLog() {}

      /// \brief Create a texture which will hold the segmentation data
      public: virtual void CreateDopplerVelocityLogTexture() = 0;

      /// \brief Get the visibility and depth map for the different entities.
      public: virtual uint8_t *VisibilityDepthData() const = 0;

      /// \brief Get the visibility and depth map for the different entities.
      public: virtual ignition::common::ConnectionPtr
        ConnectNewDepthVisibilitymapFrame(
          std::function<void(const uint8_t *, unsigned int, unsigned int,
          unsigned int, const std::string &)>  _subscriber) = 0;
    };
    }
  }
}

#endif