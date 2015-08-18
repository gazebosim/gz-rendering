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
#ifndef _IGNITION_RENDERING_BASENODE_HH_
#define _IGNITION_RENDERING_BASENODE_HH_

#include "ignition/rendering/Node.hh"
#include "ignition/rendering/Visual.hh"

#include "gazebo/common/Console.hh"

namespace ignition
{
  namespace rendering
  {
    template <class T>
    class IGNITION_VISIBLE BaseNode :
      public virtual Node,
      public virtual T
    {
      protected: BaseNode();

      public: virtual ~BaseNode();

      public: virtual VisualPtr GetParent() const = 0;

      public: virtual void RemoveParent();

      public: virtual math::Pose3d GetLocalPose() const;

      public: virtual void SetLocalPose(const math::Pose3d &_pose);

      public: virtual math::Vector3d GetLocalPosition() const;

      public: virtual void SetLocalPosition(double _x, double _y, double _z);

      public: virtual void SetLocalPosition(const math::Vector3d &_position);

      public: virtual math::Quaterniond GetLocalRotation() const;

      public: virtual void SetLocalRotation(double _r, double _p, double _y);

      public: virtual void SetLocalRotation(double _w, double _x, double _y,
                  double _z);

      public: virtual void SetLocalRotation(const math::Quaterniond &_rotation);

      public: virtual math::Pose3d GetWorldPose() const;

      public: virtual void SetWorldPose(const math::Pose3d &_pose);

      public: virtual math::Vector3d GetWorldPosition() const;

      public: virtual void SetWorldPosition(double _x, double _y, double _z);

      public: virtual void SetWorldPosition(const math::Vector3d &_position);

      public: virtual math::Quaterniond GetWorldRotation() const;

      public: virtual void SetWorldRotation(double _r, double _p, double _y);

      public: virtual void SetWorldRotation(double _w, double _x, double _y,
                  double _z);

      public: virtual void SetWorldRotation(const math::Quaterniond &_rotation);

      public: virtual math::Pose3d WorldToLocal(const math::Pose3d &_pose) const;

      public: virtual math::Vector3d GetOrigin() const;

      public: virtual void SetOrigin(double _x, double _y, double _z);

      public: virtual void SetOrigin(const math::Vector3d &_origin);

      public: virtual void Destroy();

      protected: virtual math::Pose3d GetRawLocalPose() const = 0;

      protected: virtual void SetRawLocalPose(const math::Pose3d &_pose) = 0;

      protected: math::Vector3d origin;
    };

    //////////////////////////////////////////////////
    template <class T>
    BaseNode<T>::BaseNode()
    {
    }

    //////////////////////////////////////////////////
    template <class T>
    BaseNode<T>::~BaseNode()
    {
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseNode<T>::RemoveParent()
    {
      VisualPtr parent = this->GetParent();

      if (parent)
      {
        auto baseShared = this->shared_from_this();
        auto thisShared = boost::dynamic_pointer_cast<BaseNode<T>>(baseShared);
        parent->RemoveChild(thisShared);
      }
    }

    //////////////////////////////////////////////////
    template <class T>
    math::Pose3d BaseNode<T>::GetLocalPose() const
    {
      math::Pose3d pose = this->GetRawLocalPose();
      pose.Pos() += pose.Rot() * this->origin;
      return pose;
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseNode<T>::SetLocalPose(const math::Pose3d &_pose)
    {
      math::Pose3d pose = _pose;
      pose.Pos() -= pose.Pos() - pose.Rot() * this->origin;
      this->SetRawLocalPose(pose);
    }

    //////////////////////////////////////////////////
    template <class T>
    math::Vector3d BaseNode<T>::GetLocalPosition() const
    {
      return this->GetLocalPose().Pos();
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseNode<T>::SetLocalPosition(double _x, double _y, double _z)
    {
      this->SetLocalPosition(math::Vector3d(_x, _y, _z));
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseNode<T>::SetLocalPosition(const math::Vector3d &_position)
    {
      math::Pose3d pose = this->GetLocalPose();
      pose.Pos() = _position;
      this->SetLocalPose(pose);
    }

    //////////////////////////////////////////////////
    template <class T>
    math::Quaterniond BaseNode<T>::GetLocalRotation() const
    {
      return this->GetLocalPose().Rot();
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseNode<T>::SetLocalRotation(double _r, double _p, double _y)
    {
      this->SetLocalRotation(math::Quaterniond(_r, _p, _y));
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseNode<T>::SetLocalRotation(double _w, double _x, double _y,
        double _z)
    {
      this->SetLocalRotation(math::Quaterniond(_w, _x, _y, _z));
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseNode<T>::SetLocalRotation(const math::Quaterniond &_rotation)
    {
      math::Pose3d pose = this->GetLocalPose();
      pose.Rot() = _rotation;
      this->SetLocalPose(pose);
    }

    //////////////////////////////////////////////////
    template <class T>
    math::Pose3d BaseNode<T>::GetWorldPose() const
    {
      VisualPtr parent = this->GetParent();
      math::Pose3d pose = this->GetLocalPose();

      if (!parent)
      {
        return pose;
      }

      return pose + parent->GetWorldPose();
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseNode<T>::SetWorldPose(const math::Pose3d &_pose)
    {
      math::Pose3d pose = this->WorldToLocal(_pose);
      this->SetLocalPose(pose);
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseNode<T>::SetWorldPosition(double _x, double _y, double _z)
    {
      this->SetWorldPosition(math::Vector3d(_x, _y, _z));
    }

    //////////////////////////////////////////////////
    template <class T>
    math::Vector3d BaseNode<T>::GetWorldPosition() const
    {
      return this->GetWorldPose().Pos();
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseNode<T>::SetWorldPosition(const math::Vector3d &_position)
    {
      math::Pose3d pose = this->GetWorldPose();
      pose.Pos() =_position;
      this->SetWorldPose(pose);
    }

    //////////////////////////////////////////////////
    template <class T>
    math::Quaterniond BaseNode<T>::GetWorldRotation() const
    {
      return this->GetWorldPose().Rot();
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseNode<T>::SetWorldRotation(double _r, double _p, double _y)
    {
      this->SetWorldRotation(math::Quaterniond(_r, _p, _y));
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseNode<T>::SetWorldRotation(double _w, double _x, double _y,
        double _z)
    {
      this->SetWorldRotation(math::Quaterniond(_w, _x, _y, _z));
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseNode<T>::SetWorldRotation(const math::Quaterniond &_rotation)
    {
      math::Pose3d pose = this->GetWorldPose();
      pose.Rot() = _rotation;
      this->SetWorldPose(pose);
    }

    //////////////////////////////////////////////////
    template <class T>
    math::Pose3d BaseNode<T>::WorldToLocal(const math::Pose3d &_pose) const
    {
      VisualPtr parent = this->GetParent();

      if (!parent)
      {
        return _pose;
      }

      return _pose - parent->GetWorldPose();
    }

    //////////////////////////////////////////////////
    template <class T>
    math::Vector3d BaseNode<T>::GetOrigin() const
    {
      return this->origin;
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseNode<T>::SetOrigin(double _x, double _y, double _z)
    {
      this->SetOrigin(math::Vector3d(_x, _y, _z));
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseNode<T>::SetOrigin(const math::Vector3d &_origin)
    {
      this->origin = _origin;
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseNode<T>::Destroy()
    {
      T::Destroy();
      this->RemoveParent();
    }
  }
}
#endif
