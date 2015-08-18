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
#ifndef _IGNITION_RENDERING_OGRECONVERSIONS_HH_
#define _IGNITION_RENDERING_OGRECONVERSIONS_HH_

#include "gazebo/common/Color.hh"
#include "gazebo/math/Quaternion.hh"
#include "gazebo/math/Vector3.hh"
#include "ignition/rendering/PixelFormat.hh"
#include "ignition/rendering/ogre/OgreIncludes.hh"
#include "ignition/rendering/Util.hh"

namespace ignition
{
  namespace rendering
  {
    /// \addtogroup gazebo_rendering
    /// \{

    /// \brief Conversions Conversions.hh rendering/Conversions.hh
    /// \brief A set of utility function to convert between Gazebo and Ogre
    /// data types
    class IGNITION_VISIBLE OgreConversions
    {
      /// \brief Return the equivalent ogre color
      /// \param[in] _color Gazebo color to convert
      /// \return Ogre color value
      public: static Ogre::ColourValue Convert(
          const gazebo::common::Color &_color);

      /// \brief Return the equivalent gazebo color
      /// \param[in] _color Ogre color to convert
      /// \return Gazebo color value
      public: static gazebo::common::Color Convert(
          const Ogre::ColourValue &_color);

      /// \brief return Ogre Vector from Gazebo Vector3
      /// \param[in] _vector Gazebo vector
      /// \return Ogre vector
      public: static Ogre::Vector3 Convert(const math::Vector3d &_vector);

      /// \brief return gazebo Vector from ogre Vector3
      /// \param[in] _vector Ogre vector
      /// \return Gazebo vector
      public: static math::Vector3d Convert(const Ogre::Vector3 &_vector);

      /// \brief Gazebo quaternion to Ogre quaternion
      /// \param[in] _quat Gazebo quaternion
      /// \return Ogre quaternion
      public: static Ogre::Quaternion Convert(const math::Quaterniond &_quat);

      /// \brief Ogre quaternion to Gazebo quaternion
      /// \param[in] _quat Ogre quaternion
      /// return Gazebo quaternion
      public: static math::Quaterniond Convert(const Ogre::Quaternion &_quat);

      /// \brief Gazebo angle to Ogre angle
      /// \param[in] _angle Gazebo angle
      /// \return Ogre angle
      public: static Ogre::Radian Convert(const math::Angle &_angle);

      /// \brief Ogre angle to Gazebo angle
      /// \param[in] _angle Ogre angle
      /// return Gazebo angle
      public: static math::Angle Convert(const Ogre::Radian &_angle);

      /// \brief Gazebo PixelFormat to Ogre PixelFormat
      /// \param[in] _v Gazebo PixelFormat
      /// return Ogre PixelFormat
      public: static Ogre::PixelFormat Convert(PixelFormat _format);

      private: static const Ogre::PixelFormat ogrePixelFormats[PF_COUNT];
    };

    /// \}
  }
}
#endif
