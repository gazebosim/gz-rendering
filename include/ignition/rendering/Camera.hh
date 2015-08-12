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
#ifndef _IGNITION_RENDERING_CAMERA_HH_
#define _IGNITION_RENDERING_CAMERA_HH_

#include <sdf/sdf.hh>
#include <boost/function.hpp>
#include "gazebo/common/Event.hh"
#include "ignition/rendering/Image.hh"
#include "ignition/rendering/PixelFormat.hh"
#include "ignition/rendering/Sensor.hh"

namespace ignition
{
  namespace rendering
  {
    class IGNITION_VISIBLE Camera :
      public virtual Sensor
    {
      public: typedef boost::function<void(const void*, unsigned int,
          unsigned int, unsigned int, const std::string&)> NewFrameListener;

      public: virtual ~Camera() { }

      public: virtual void Load(sdf::ElementPtr _sdf) = 0;

      public: virtual unsigned int GetImageWidth() const = 0;

      public: virtual void SetImageWidth(unsigned int _width) = 0;

      public: virtual unsigned int GetImageHeight() const = 0;

      public: virtual void SetImageHeight(unsigned int _height) = 0;

      public: virtual PixelFormat GetImageFormat() const = 0;

      public: virtual void SetImageFormat(PixelFormat _format) = 0;

      public: virtual unsigned int GetImageDepth() const = 0;

      public: virtual unsigned int GetImageMemorySize() const = 0;

      public: virtual gazebo::math::Angle GetHFOV() const = 0;

      public: virtual void SetHFOV(const gazebo::math::Angle &_angle) = 0;

      public: virtual double GetAspectRatio() const = 0;

      public: virtual void SetAspectRatio(double _ratio) = 0;

      public: virtual unsigned int GetAntiAliasing() const = 0;

      public: virtual void SetAntiAliasing(unsigned int _aa) = 0;

      public: virtual void Render() = 0;

      public: virtual void PostRender() = 0;

      public: virtual Image CreateImage() const = 0;

      public: virtual void Capture(Image &_image) = 0;

      public: virtual void GetImageData(Image &_image) const = 0;

      public: virtual bool SaveFrame(const std::string &_name) = 0;

      public: virtual gazebo::event::ConnectionPtr ConnectNewImageFrame(
                  NewFrameListener _listener) = 0;

      public: virtual void DisconnectNewImageFrame(
                  gazebo::event::ConnectionPtr &_conn) = 0;
    };
  }
}
#endif
