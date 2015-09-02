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
#ifndef _IGNITION_RENDERING_NODE_HH_
#define _IGNITION_RENDERING_NODE_HH_

#include <string>
#include <ignition/math/Pose3.hh>
#include <ignition/math/Quaternion.hh>

#include "ignition/rendering/RenderTypes.hh"
#include "ignition/rendering/Object.hh"
#include "ignition/rendering/Util.hh"

namespace ignition
{
  namespace rendering
  {
    /// \class Node Node.hh ignition/rendering/Node.hh
    /// \brief Represents a single posable node in the scene graph
    class IGNITION_VISIBLE Node :
      public virtual Object
    {
      /// \brief Deconstructor
      public: virtual ~Node() { }

      /// \brief Determine if this Geometry is attached to a Visual
      /// \return True if this Geometry has a parent Visual
      public: virtual bool HasParent() const = 0;

      /// \brief Get the parent Visual
      /// \return the parent Visual
      public: virtual VisualPtr GetParent() const = 0;

      /// \brief Detach this Geometry from its parent Visual. If this
      /// Geometry does not have a parent, no work will be done.
      public: virtual void RemoveParent() = 0;

      /// \brief Get the local pose
      /// \return The local pose
      public: virtual math::Pose3d GetLocalPose() const = 0;

      /// \brief Set the local pose
      /// \param[in] _pose New local pose
      public: virtual void SetLocalPose(const math::Pose3d &_pose) = 0;

      /// \brief Get the local position
      /// \return The local position
      public: virtual math::Vector3d GetLocalPosition() const = 0;

      /// \brief Set the local position
      /// \param[in] _x X-coordinate
      /// \param[in] _y Y-coordinate
      /// \param[in] _z Z-coordinate
      public: virtual void SetLocalPosition(double _x, double _y,
                  double _z) = 0;

      /// \brief Set the local position
      /// \param[in] _position New local position
      public: virtual void SetLocalPosition(
                  const math::Vector3d &_position) = 0;

      /// \brief Get the local rotation
      /// \return The local rotation
      public: virtual math::Quaterniond GetLocalRotation() const = 0;

      /// \brief Set the local rotation
      /// \param[in] _r roll
      /// \param[in] _p pitch
      /// \param[in] _y yaw
      public: virtual void SetLocalRotation(double _r, double _p,
                  double _y) = 0;

      /// \brief Set the local rotation
      /// \param[in] _w W-coordinate
      /// \param[in] _x X-coordinate
      /// \param[in] _y Y-coordinate
      /// \param[in] _z Z-coordinate
      public: virtual void SetLocalRotation(double _w, double _x, double _y,
                  double _z) = 0;

      /// \brief Set the local rotation
      /// \param[in] _rotation New local rotation
      public: virtual void SetLocalRotation(
                  const math::Quaterniond &_rotation) = 0;

      /// \brief Get the world pose
      /// \return The world pose
      public: virtual math::Pose3d GetWorldPose() const = 0;

      /// \brief Set the world pose
      /// \param[in] _pose New world pose
      public: virtual void SetWorldPose(const math::Pose3d &_pose) = 0;

      /// \brief Get the world position
      /// \return The world position
      public: virtual math::Vector3d GetWorldPosition() const = 0;

      /// \brief Set the world position
      /// \param[in] _x X-coordinate
      /// \param[in] _y Y-coordinate
      /// \param[in] _z Z-coordinate
      public: virtual void SetWorldPosition(double _x, double _y,
                  double _z) = 0;

      /// \brief Set the world position
      /// \param[in] _position New world position
      public: virtual void SetWorldPosition(
                  const math::Vector3d &_position) = 0;

      /// \brief Get the world rotation
      /// \return The world rotation
      public: virtual math::Quaterniond GetWorldRotation() const = 0;

      /// \brief Set the world rotation
      /// \param[in] _r roll
      /// \param[in] _p pitch
      /// \param[in] _y yaw
      public: virtual void SetWorldRotation(double _r, double _p,
                  double _y) = 0;

      /// \brief Set the world rotation
      /// \param[in] _w W-coordinate
      /// \param[in] _x X-coordinate
      /// \param[in] _y Y-coordinate
      /// \param[in] _z Z-coordinate
      public: virtual void SetWorldRotation(double _w, double _x, double _y,
                  double _z) = 0;

      /// \brief Set the world rotation
      /// \param[in] _rotation New world rotation
      public: virtual void SetWorldRotation(
                  const math::Quaterniond &_rotation) = 0;

      /// \brief Convert given world pose to local pose
      /// \param[in] _pose World pose to be converted
      public: virtual math::Pose3d WorldToLocal(
                  const math::Pose3d &_pose) const = 0;

      /// \brief Get position of origin
      /// \return The position of the origin
      public: virtual math::Vector3d GetOrigin() const = 0;

      /// \brief Set position of origin. The position should be relative to the
      /// original origin of the geometry.
      /// \param[in] _x X-coordinate
      /// \param[in] _y Y-coordinate
      /// \param[in] _z Z-coordinate
      public: virtual void SetOrigin(double _x, double _y, double _z) = 0;

      /// \brief Set position of origin. The position should be relative to the
      /// original origin of the geometry.
      /// \param[in] _origin New origin position
      public: virtual void SetOrigin(const math::Vector3d &_origin) = 0;
    };
  }
}
#endif
