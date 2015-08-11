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
#include "gazebo/math/Pose.hh"
#include "ignition/rendering/RenderTypes.hh"
#include "ignition/rendering/Object.hh"
#include "ignition/rendering/Util.hh"

namespace ignition
{
  namespace rendering
  {
    class IGNITION_VISIBLE Node :
      public virtual Object
    {
      public: virtual ~Node() { }

      public: virtual bool HasParent() const = 0;

      public: virtual VisualPtr GetParent() const = 0;

      public: virtual void RemoveParent() = 0;

      public: virtual gazebo::math::Pose GetLocalPose() const = 0;

      public: virtual void SetLocalPose(const gazebo::math::Pose &_pose) = 0;

      public: virtual gazebo::math::Vector3 GetLocalPosition() const = 0;

      public: virtual void SetLocalPosition(double _x, double _y,
                  double _z) = 0;

      public: virtual void SetLocalPosition(const gazebo::math::Vector3 &_position) = 0;

      public: virtual gazebo::math::Quaternion GetLocalRotation() const = 0;

      public: virtual void SetLocalRotation(double _r, double _p,
                  double _y) = 0;

      public: virtual void SetLocalRotation(double _w, double _x, double _y,
                  double _z) = 0;

      public: virtual void SetLocalRotation(
                  const gazebo::math::Quaternion &_rotation) = 0;

      public: virtual gazebo::math::Pose GetWorldPose() const = 0;

      public: virtual void SetWorldPose(const gazebo::math::Pose &_pose) = 0;

      public: virtual gazebo::math::Vector3 GetWorldPosition() const = 0;

      public: virtual void SetWorldPosition(double _x, double _y,
                  double _z) = 0;

      public: virtual void SetWorldPosition(const gazebo::math::Vector3 &_position) = 0;

      public: virtual void SetWorldRotation(double _r, double _p,
                  double _y) = 0;

      public: virtual void SetWorldRotation(double _w, double _x, double _y,
                  double _z) = 0;

      public: virtual gazebo::math::Quaternion GetWorldRotation() const = 0;

      public: virtual void SetWorldRotation(
                  const gazebo::math::Quaternion &_rotation) = 0;

      public: virtual gazebo::math::Pose WorldToLocal(
                  const gazebo::math::Pose &_pose) const = 0;

      public: virtual gazebo::math::Vector3 GetOrigin() const = 0;

      public: virtual void SetOrigin(double _x, double _y, double _z) = 0;

      public: virtual void SetOrigin(const gazebo::math::Vector3 &_origin) = 0;
    };
  }
}
#endif
