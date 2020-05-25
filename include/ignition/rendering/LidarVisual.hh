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
#include "ignition/rendering/Geometry.hh"
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
    // enum IGNITION_RENDERING_VISIBLE LidarVisualType
    // {
    //   /// \brief No type
    //   LT_NONE           = 0,

    //   /// \brief Line strip primitive
    //   LT_LINE_STRIP     = 3,

    //   /// \brief Line list primitive
    //   LT_LINE_LIST      = 4,

    //   /// \brief Points primitive
    //   LT_POINTS         = 5,

    //   /// \brief Triangle fan primitive
    //   LT_TRIANGLE_FAN   = 8,

    //   /// \brief Triangle list primitive
    //   LT_TRIANGLE_LIST  = 9,

    //   /// \brief Triangle strip primitive
    //   LT_TRIANGLE_STRIP = 10
    // };

    /// \class LidarVisual LidarVisual.hh ignition/rendering/LidarVisual
    /// \brief A LidarVisual geometry class. The visual appearance is based
    /// on the type specified.
    class IGNITION_RENDERING_VISIBLE LidarVisual :
      public virtual Geometry
    {
      protected: LidarVisual();

      /// \brief Destructor
      public: virtual ~LidarVisual();

      /// \brief Set the lifetime of this LidarVisual
      /// \param[in] _lifetime The time at which the visual will be removed
      public: virtual void SetLifetime(
                  const std::chrono::steady_clock::duration &_lifetime) = 0;

      /// \brief Get the lifetime of this LidarVisual
      /// \return The time at which the visual will be removed
      public: virtual std::chrono::steady_clock::duration Lifetime() const = 0;

      /// \brief Set the layer of this LidarVisual
      /// \param[in] _layer Layer at which the visual will reside
      public: virtual void SetLayer(int32_t _layer) = 0;

      /// \brief Get the layer of this LidarVisual
      /// \return The layer of the visual
      public: virtual int32_t Layer() const = 0;

      /// \brief Set the render type of this LidarVisual
      /// \param[in] The desired render type
      public: virtual void SetType(
                  const ignition::rendering::LidarVisualType _lidarVisualType) = 0;

      /// \brief Get the render type of this LidarVisual
      /// \return The render type of the visual
      public: virtual ignition::rendering::LidarVisualType Type() const = 0;

      /// \brief Clear the points of the LidarVisual, if applicable
      public: virtual void ClearPoints() = 0;

      /// \brief Add a point with its respective color to the LidarVisual
      /// \param[in] _x X coordinate
      /// \param[in] _y Y coordinate
      /// \param[in] _z Z coordinate
      /// \param[in] _color The color the point is set to
      public: virtual void AddPoint(double _x,
                  double _y, double _z,
                  const ignition::math::Color &_color) = 0;

      /// \brief Add a point with its respective color to the LidarVisual
      /// \param[in] _pt A vector containing the position of the point
      /// \param[in] _color The color the point is set to
      public: virtual void AddPoint(const ignition::math::Vector3d &_pt,
                  const ignition::math::Color &_color) = 0;

      /// \brief Set an existing point's vector
      /// \param[in] _index The index of the point
      /// \param[in] _value The new positional vector of the point
      public: virtual void SetPoint(unsigned int _index,
                  const ignition::math::Vector3d &_value) = 0;
    };
    }
  }
}

#endif
