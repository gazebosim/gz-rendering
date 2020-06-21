/*
 * Copyright (C) 2020 Open Source Robotics Foundation
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

#include <vector>
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

      /// \brief Clear the points of the lidar visual
      public: virtual void ClearPoints() = 0;

      /// \brief Update the Visual
      public: virtual void Update() = 0;

      /// \brief Callback when the senspr data is recieved
      /// \param[in] _msg The vector of points representing distance of the ray from the lidar
      public: virtual void SetLidarMessage(std::vector<double> &_msg) = 0;

      /// \brief Set minimum vertical angle
      /// \param[in] _minVerticalAngle Minimum vertical angle
      public: virtual void SetMinVerticalAngle(
                  const double _minVerticalAngle) = 0;

      /// \brief Get minimum vertical angle
      /// \return The minimum vertical angle value of the lidar visual
      public: virtual double MinVerticalAngle() = 0;

      /// \brief Set maximum vertical angle
      /// \param[in] _maxVerticalAngle Maximum vertical angle value
      public: virtual void SetMaxVerticalAngle(
                  const double _maxVerticalAngle) = 0;

      /// \brief Get minimum vertical angle
      /// \return The maximum vertical angle value of the lidar visual
      public: virtual double MaxVerticalAngle() = 0;

      /// \brief Set minimum horizontal angle
      /// \param[in] _minHorizontalAngle Minimum horizontal angle value
      public: virtual void SetMinHorizontalAngle(
                  const double _minHorizontalAngle) = 0;

      /// \brief Get minimum horizontal angle
      /// \return The minimum horizontal angle value of the lidar visual
      public: virtual double MinHorizontalAngle() = 0;

      /// \brief Set maximum horizontal angle
      /// \param[in] _maxHorizontalAngle Maximum horizontal angle value
      public: virtual void SetMaxHorizontalAngle(
                  const double _maxHorizontalAngle) = 0;

      /// \brief Get maximum horizontal angle
      /// \return The maximum horizontal angle value of the lidar visual
      public: virtual double MaxHorizontalAngle() = 0;

      /// \brief Set number of vertical rays
      /// \param[in] _verticalRayCount Number of vertical rays
      public: virtual void SetVerticalRayCount(
                  const unsigned int _verticalRayCount) = 0;

      /// \brief Get number of vertical rays
      /// \return The number of vertical rays
      public: virtual unsigned int VerticalRayCount() = 0;

      /// \brief Set number of horizontal rays
      /// \return _horizontalRayCount Number of vertical rays
      public: virtual void SetHorizontalRayCount(
                  const unsigned int _horizontalRayCount) = 0;

      /// \brief Get number of horizontal rays
      /// \return The number of horizontal rays
      public: virtual unsigned int HorizontalRayCount() = 0;

      /// \brief Set minimum range of lidar rays
      /// \param[in] _minRange The minimum range for lidar rays
      public: virtual void SetMinRange(const double _minRange) = 0;

      /// \brief Get minimum range of lidar rays
      /// \return The minimum range for lidar rays
      public: virtual double MinRange() = 0;

      /// \brief Set maximum range of lidar rays
      /// \param[in] _maxRange The maximum range for lidar rays
      public: virtual void SetMaxRange(const double _maxRange) = 0;

      /// \brief Get maximum range of lidar rays
      /// \return The maximum range for lidar rays
      public: virtual double MaxRange() = 0;

      /// \brief Set vertical step angle
      /// \param[in] _verticalAngleStep Difference in vertical angle between two rays
      public: virtual void SetVerticalAngleStep(
                  const double _verticalAngleStep) = 0;

      /// \brief Get vertical step angle
      /// \return The difference in vertical angle between two rays
      public: virtual double VerticalAngleStep() = 0;

      /// \brief Set horizontal step angle
      /// \param[in] _horizontalAngleStep Difference in horizontal angle between two rays
      public: virtual void SetHorizontalAngleStep(
                  const double _horizontalAngleStep) = 0;

      /// \brief Get horizontal step angle
      /// \return The difference in horizontal angle between two rays
      public: virtual double HorizontalAngleStep() = 0;

      /// \brief Set offset of visual
      /// \param[in] _offset The offset of the lidar rays from the visual origin
      public: virtual void SetOffset(
                  const ignition::math::Pose3d _offset) = 0;

      /// \brief Get offset of visual
      /// \return The offset of the lidar rays from visual origin
      public: virtual ignition::math::Pose3d  Offset() = 0;

      /// \brief Get number of points in laser data
      /// \return The number of points in the laser data
      public: virtual unsigned int GetPointCount() = 0;
    };
    }
  }
}

#endif
