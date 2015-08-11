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

      public: virtual gazebo::math::Pose GetLocalPose() const;

      public: virtual void SetLocalPose(const gazebo::math::Pose &_pose);

      public: virtual gazebo::math::Vector3 GetLocalPosition() const;

      public: virtual void SetLocalPosition(double _x, double _y, double _z);

      public: virtual void SetLocalPosition(const gazebo::math::Vector3 &_position);

      public: virtual gazebo::math::Quaternion GetLocalRotation() const;

      public: virtual void SetLocalRotation(double _r, double _p, double _y);

      public: virtual void SetLocalRotation(double _w, double _x, double _y,
                  double _z);

      public: virtual void SetLocalRotation(const gazebo::math::Quaternion &_rotation);

      public: virtual gazebo::math::Pose GetWorldPose() const;

      public: virtual void SetWorldPose(const gazebo::math::Pose &_pose);

      public: virtual gazebo::math::Vector3 GetWorldPosition() const;

      public: virtual void SetWorldPosition(double _x, double _y, double _z);

      public: virtual void SetWorldPosition(const gazebo::math::Vector3 &_position);

      public: virtual gazebo::math::Quaternion GetWorldRotation() const;

      public: virtual void SetWorldRotation(double _r, double _p, double _y);

      public: virtual void SetWorldRotation(double _w, double _x, double _y,
                  double _z);

      public: virtual void SetWorldRotation(const gazebo::math::Quaternion &_rotation);

      public: virtual gazebo::math::Pose WorldToLocal(const gazebo::math::Pose &_pose) const;

      public: virtual gazebo::math::Vector3 GetOrigin() const;

      public: virtual void SetOrigin(double _x, double _y, double _z);

      public: virtual void SetOrigin(const gazebo::math::Vector3 &_origin);

      public: virtual void Destroy();

      protected: virtual gazebo::math::Pose GetRawLocalPose() const = 0;

      protected: virtual void SetRawLocalPose(const gazebo::math::Pose &_pose) = 0;

      protected: gazebo::math::Vector3 origin;
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
    gazebo::math::Pose BaseNode<T>::GetLocalPose() const
    {
      gazebo::math::Pose pose = this->GetRawLocalPose();
      pose.pos += pose.rot * this->origin;
      return pose;
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseNode<T>::SetLocalPose(const gazebo::math::Pose &_pose)
    {
      gazebo::math::Pose pose = _pose;
      pose.pos -= pose.rot * this->origin;
      this->SetRawLocalPose(pose);
    }

    //////////////////////////////////////////////////
    template <class T>
    gazebo::math::Vector3 BaseNode<T>::GetLocalPosition() const
    {
      return this->GetLocalPose().pos;
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseNode<T>::SetLocalPosition(double _x, double _y, double _z)
    {
      this->SetLocalPosition(gazebo::math::Vector3(_x, _y, _z));
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseNode<T>::SetLocalPosition(const gazebo::math::Vector3 &_position)
    {
      gazebo::math::Pose pose = this->GetLocalPose();
      pose.pos = _position;
      this->SetLocalPose(pose);
    }

    //////////////////////////////////////////////////
    template <class T>
    gazebo::math::Quaternion BaseNode<T>::GetLocalRotation() const
    {
      return this->GetLocalPose().rot;
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseNode<T>::SetLocalRotation(double _r, double _p, double _y)
    {
      this->SetLocalRotation(gazebo::math::Quaternion(_r, _p, _y));
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseNode<T>::SetLocalRotation(double _w, double _x, double _y,
        double _z)
    {
      this->SetLocalRotation(gazebo::math::Quaternion(_w, _x, _y, _z));
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseNode<T>::SetLocalRotation(const gazebo::math::Quaternion &_rotation)
    {
      gazebo::math::Pose pose = this->GetLocalPose();
      pose.rot = _rotation;
      this->SetLocalPose(pose);
    }

    //////////////////////////////////////////////////
    template <class T>
    gazebo::math::Pose BaseNode<T>::GetWorldPose() const
    {
      VisualPtr parent = this->GetParent();
      gazebo::math::Pose pose = this->GetLocalPose();

      if (!parent)
      {
        return pose;
      }

      return pose + parent->GetWorldPose();
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseNode<T>::SetWorldPose(const gazebo::math::Pose &_pose)
    {
      gazebo::math::Pose pose = this->WorldToLocal(_pose);
      this->SetLocalPose(pose);
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseNode<T>::SetWorldPosition(double _x, double _y, double _z)
    {
      this->SetWorldPosition(gazebo::math::Vector3(_x, _y, _z));
    }

    //////////////////////////////////////////////////
    template <class T>
    gazebo::math::Vector3 BaseNode<T>::GetWorldPosition() const
    {
      return this->GetWorldPose().pos;
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseNode<T>::SetWorldPosition(const gazebo::math::Vector3 &_position)
    {
      gazebo::math::Pose pose = this->GetWorldPose();
      pose.pos = _position;
      this->SetWorldPose(pose);
    }

    //////////////////////////////////////////////////
    template <class T>
    gazebo::math::Quaternion BaseNode<T>::GetWorldRotation() const
    {
      return this->GetWorldPose().rot;
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseNode<T>::SetWorldRotation(double _r, double _p, double _y)
    {
      this->SetWorldRotation(gazebo::math::Quaternion(_r, _p, _y));
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseNode<T>::SetWorldRotation(double _w, double _x, double _y,
        double _z)
    {
      this->SetWorldRotation(gazebo::math::Quaternion(_w, _x, _y, _z));
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseNode<T>::SetWorldRotation(const gazebo::math::Quaternion &_rotation)
    {
      gazebo::math::Pose pose = this->GetWorldPose();
      pose.rot = _rotation;
      this->SetWorldPose(pose);
    }

    //////////////////////////////////////////////////
    template <class T>
    gazebo::math::Pose BaseNode<T>::WorldToLocal(const gazebo::math::Pose &_pose) const
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
    gazebo::math::Vector3 BaseNode<T>::GetOrigin() const
    {
      return this->origin;
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseNode<T>::SetOrigin(double _x, double _y, double _z)
    {
      this->SetOrigin(gazebo::math::Vector3(_x, _y, _z));
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseNode<T>::SetOrigin(const gazebo::math::Vector3 &_origin)
    {
      this->origin = _origin;
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseNode<T>::Destroy()
    {
      this->RemoveParent();
      T::Destroy();
    }
  }
}
#endif
