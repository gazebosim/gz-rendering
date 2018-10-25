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
#ifndef IGNITION_RENDERING_GPURAYS_HH_
#define IGNITION_RENDERING_GPURAYS_HH_

#include <string>

#include <ignition/common/Event.hh>

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
    /// \class GpuRays GpuRays.hh ignition/rendering/GpuRays.hh
    /// \brief Generate depth ray data.
    class IGNITION_RENDERING_VISIBLE GpuRays :
      public virtual Camera
    {
      /// \brief Callback function for new frame render event listeners
      public: typedef std::function<void(const void*, unsigned int,
          unsigned int, unsigned int, const std::string&)> NewFrameListener;

      /// \brief Deconstructor
      public: virtual ~GpuRays() { }

      /// \brief Create the texture which is used to render laser data.
      public: virtual void CreateLaserTexture() = 0;

      /// \brief All things needed to get back z buffer for laser data.
      /// \return Array of laser data.
      /// \deprecated use LaserDataBegin() and LaserDataEnd() instead
      public: virtual const float *RaysData() const = 0;

      /// \brief Connect to a laser frame signal
      /// \param[in] _subscriber Callback that is called when a new image is
      /// generated
      /// \return A pointer to the connection. This must be kept in scope.
      public: virtual common::ConnectionPtr ConnectNewLaserFrame(
                  std::function<void(const float *_frame, unsigned int _width,
                  unsigned int _height, unsigned int _depth,
                  PixelFormat)> _subscriber) = 0;

      /// \brief Get (horizontal_max_angle + horizontal_min_angle) * 0.5
      /// \return (horizontal_max_angle + horizontal_min_angle) * 0.5
      public: virtual double HorzHalfAngle() const = 0;

      /// \brief Get (vertical_max_angle + vertical_min_angle) * 0.5
      /// \return (vertical_max_angle + vertical_min_angle) * 0.5
      public: virtual double VertHalfAngle() const = 0;

      /// \brief Set the horizontal half angle
      /// \param[in] _angle horizontal half angle
      public: virtual void SetHorzHalfAngle(const double _angle) = 0;

      /// \brief Set the vertical half angle
      /// \param[in] _angle vertical half angle
      public: virtual void SetVertHalfAngle(const double _angle) = 0;

      /// \brief Set sensor horizontal or vertical
      /// \param[in] _horizontal True if horizontal, false if not
      public: virtual void SetIsHorizontal(const bool _horizontal) = 0;

      /// \brief Gets if sensor is horizontal
      /// \return True if horizontal, false if not
      public: virtual bool IsHorizontal() const = 0;

      /// \brief Get the horizontal field of view of the laser sensor.
      /// \return The horizontal field of view of the laser sensor.
      public: virtual double HorzFOV() const = 0;

      /// \brief Get Cos Horz field-of-view
      /// \return 2 * atan(tan(this->hfov/2) / cos(this->vfov/2))
      public: virtual double CosHorzFOV() const = 0;

      /// \brief Set the Cos Horz FOV
      /// \param[in] _chfov Cos Horz FOV
      public: virtual void SetCosHorzFOV(const double _chfov) = 0;

      /// \brief Get the vertical field-of-view.
      /// \return The vertical field of view of the laser sensor.
      public: virtual double VertFOV() const = 0;

      /// \brief Get Cos Vert field-of-view
      /// \return 2 * atan(tan(this->vfov/2) / cos(this->hfov/2))
      public: virtual double CosVertFOV() const = 0;

      /// \brief Set the Cos Horz FOV
      /// \param[in] _cvfov Cos Horz FOV
      public: virtual void SetCosVertFOV(const double _cvfov) = 0;

      /// \brief Set the horizontal fov
      /// \param[in] _hfov horizontal fov
      public: virtual void SetHorzFOV(const double _hfov) = 0;

      /// \brief Set the vertical fov
      /// \param[in] _vfov vertical fov
      public: virtual void SetVertFOV(const double _vfov) = 0;

      /// \brief Get the number of cameras required
      /// \return Number of cameras needed to generate the rays
      public: virtual unsigned int CameraCount() const = 0;

      /// \brief Set the number of cameras required
      /// \param[in] _cameraCount The number of cameras required to generate
      /// the rays
      public: virtual void SetCameraCount(const unsigned int _cameraCount) = 0;

      /// \brief Get the ray count ratio (equivalent to aspect ratio)
      /// \return The ray count ratio (equivalent to aspect ratio)
      public: virtual double RayCountRatio() const = 0;

      /// \brief Get the ray count ratio (equivalent to aspect ratio)
      /// \return The ray count ratio (equivalent to aspect ratio)
      public: virtual double RangeCountRatio() const = 0;

      /// \brief Sets the ray count ratio (equivalent to aspect ratio)
      /// \param[in] _rayCountRatio ray count ratio (equivalent to aspect ratio)
      public: virtual void SetRayCountRatio(const double _rayCountRatio) = 0;

      /// \brief Get minimal horizontal angle value
      // \return minimal horizontal angle value
      public: virtual ignition::math::Angle AngleMin() const = 0;

      /// \brief Set minimal horizontal angle value
      public: virtual void SetAngleMin(double _angle) = 0;

      /// \brief Get maximal horizontal angle value
      // \return maximal horizontal angle value
      public: virtual ignition::math::Angle AngleMax() const = 0;

      /// \brief Set maximal horizontal angle value
      public: virtual void SetAngleMax(double _angle) = 0;

      /// \brief Get horizontal quantity of rays
      // \return horizontal quantity of rays
      public: virtual int RayCount() const = 0;

      /// \brief Set horizontal quantity of rays
      public: virtual void SetRayCount(int _samples) = 0;

      /// \brief Get hoizontal range count, i.e. ray count * horz resolution
      // \return horizontal range count
      public: virtual int RangeCount() const = 0;

      /// \brief Get vertical quantity of rays
      // \return vertical quantity of rays
      public: virtual int VerticalRayCount() const = 0;

      /// \brief Set vertical quantity of rays
      public: virtual void SetVerticalRayCount(int _samples) = 0;

      /// \brief Get vertical range count, i.e. ray count * vert resolution
      // \return Vertical range count
      public: virtual int VerticalRangeCount() const = 0;

      /// \brief Get minimal vertical angle value
      // \return minimal vertical angle value
      public: virtual ignition::math::Angle VerticalAngleMin() const = 0;

      /// \brief Set minimal vertical angle value
      public: virtual void SetVerticalAngleMin(const double _angle) = 0;

      /// \brief Get maximal vertical angle value
      // \return maximal vertical angle value
      public: virtual ignition::math::Angle VerticalAngleMax() const = 0;

      /// \brief Set maximal vertical angle value
      // \return minimal vertical angle value
      public: virtual void SetVerticalAngleMax(const double _angle) = 0;
    };
  }
  }
}
#endif
