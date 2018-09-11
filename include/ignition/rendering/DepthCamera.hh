/*
 * Copyright (C) 2018 Open Source Robotics Foundation
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
#ifndef IGNITION_RENDERING_DEPTHCAMERA_HH_
#define IGNITION_RENDERING_DEPTHCAMERA_HH_

#include <string>

#include <ignition/common/Event.hh>
#include <ignition/math/Matrix4.hh>

#include "ignition/rendering/Image.hh"
#include "ignition/rendering/PixelFormat.hh"
#include "ignition/rendering/Sensor.hh"
#include "ignition/rendering/Scene.hh"
#include "ignition/rendering/Camera.hh"

namespace ignition
{
  namespace rendering
  {
    inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {
    /// \class Camera Camera.hh ignition/rendering/Camera.hh
    /// \brief Poseable depth camera used for rendering the scene graph.
    /// This camera is designed to produced depth data, instead of a 2D
    /// image.
    class IGNITION_RENDERING_VISIBLE DepthCamera :
      public virtual Camera
    {
      /// \brief Callback function for new frame render event listeners
      public: typedef std::function<void(const void*, unsigned int,
          unsigned int, unsigned int, const std::string&)> NewFrameListener;

      /// \brief Deconstructor
      public: virtual ~DepthCamera() { }

      /// \brief Create a texture which will hold the depth data
      public: virtual void CreateDepthTexture() = 0;

      /// \brief All things needed to get back z buffer for depth data
      /// \return The z-buffer as a float array
      public: virtual const float *DepthData() const = 0;

      /// \brief Connect to the new depth image signal
      /// \param[in] _subscriber Subscriber callback function
      /// \return Pointer to the new Connection. This must be kept in scope
      public: virtual ignition::common::ConnectionPtr ConnectNewDepthFrame(
          std::function<void(const float *, unsigned int, unsigned int,
          unsigned int, const std::string &)>  _subscriber) = 0;

      /// \brief Connect to the new rgb point cloud signal.
      /// \param[in] _subscriber Subscriber callback function
      /// \return Pointer to the new Connection. This must be kept in scope
      public: virtual ignition::common::ConnectionPtr ConnectNewRgbPointCloud(
          std::function<void(const float *, unsigned int, unsigned int,
          unsigned int, const std::string &)>  _subscriber) = 0;
    };
  }
  }
}
#endif
