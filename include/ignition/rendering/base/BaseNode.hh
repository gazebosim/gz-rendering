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
#ifndef IGNITION_RENDERING_BASE_BASENODE_HH_
#define IGNITION_RENDERING_BASE_BASENODE_HH_

#include <ignition/math/Matrix4.hh>

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

      public: virtual VisualPtr Parent() const = 0;

      public: virtual void RemoveParent();

      public: virtual math::Pose3d LocalPose() const;

      public: virtual void SetLocalPose(const math::Pose3d &_pose);

      public: virtual math::Vector3d LocalPosition() const;

      public: virtual void SetLocalPosition(double _x, double _y, double _z);

      public: virtual void SetLocalPosition(const math::Vector3d &_position);

      public: virtual math::Quaterniond LocalRotation() const;

      public: virtual void SetLocalRotation(double _r, double _p, double _y);

      public: virtual void SetLocalRotation(double _w, double _x, double _y,
                  double _z);

      public: virtual void SetLocalRotation(const math::Quaterniond &_rotation);

      public: virtual math::Pose3d WorldPose() const;

      public: virtual void SetWorldPose(const math::Pose3d &_pose);

      public: virtual math::Vector3d WorldPosition() const;

      public: virtual void SetWorldPosition(double _x, double _y, double _z);

      public: virtual void SetWorldPosition(const math::Vector3d &_position);

      public: virtual math::Quaterniond WorldRotation() const;

      public: virtual void SetWorldRotation(double _r, double _p, double _y);

      public: virtual void SetWorldRotation(double _w, double _x, double _y,
                  double _z);

      public: virtual void SetWorldRotation(const math::Quaterniond &_rotation);

      public: virtual math::Pose3d WorldToLocal(const math::Pose3d &_pose)
          const;

      public: virtual math::Vector3d Origin() const;

      public: virtual void SetOrigin(double _x, double _y, double _z);

      public: virtual void SetOrigin(const math::Vector3d &_origin);

      public: virtual void Destroy();

      // Documentation inherited.
      public: virtual void PreRender();

      // Documentation inherited.
      public: virtual void SetAutoTrack(const bool _enabled,
                  const NodePtr &_target = NodePtr(),
                  const bool _follow = false,
                  const math::Vector3d &_followOffset = math::Vector3d::Zero);

      protected: virtual math::Pose3d RawLocalPose() const = 0;

      protected: virtual void SetRawLocalPose(const math::Pose3d &_pose) = 0;

      protected: math::Vector3d origin;

      /// \brief Target node to track if auto tracking is on.
      protected: NodePtr trackNode;

      /// \brief Enable follow mode with auto tracking.
      protected: bool trackFollow = false;

      /// \brief Offset distance between this node and target node being
      /// followed.
      protected: math::Vector3d trackFollowOffset;
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
      VisualPtr parent = this->Parent();

      if (parent)
      {
        auto baseShared = this->shared_from_this();
        auto thisShared = std::dynamic_pointer_cast<BaseNode<T>>(baseShared);
        parent->RemoveChild(thisShared);
      }
    }

    //////////////////////////////////////////////////
    template <class T>
    math::Pose3d BaseNode<T>::LocalPose() const
    {
      math::Pose3d pose = this->RawLocalPose();
      pose.Pos() += pose.Rot() * this->origin;
      return pose;
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseNode<T>::SetLocalPose(const math::Pose3d &_pose)
    {
      math::Pose3d pose = _pose;
      pose.Pos() = pose.Pos() - pose.Rot() * this->origin;
      this->SetRawLocalPose(pose);
    }

    //////////////////////////////////////////////////
    template <class T>
    math::Vector3d BaseNode<T>::LocalPosition() const
    {
      return this->LocalPose().Pos();
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
      math::Pose3d pose = this->LocalPose();
      pose.Pos() = _position;
      this->SetLocalPose(pose);
    }

    //////////////////////////////////////////////////
    template <class T>
    math::Quaterniond BaseNode<T>::LocalRotation() const
    {
      return this->LocalPose().Rot();
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
      math::Pose3d pose = this->LocalPose();
      pose.Rot() = _rotation;
      this->SetLocalPose(pose);
    }

    //////////////////////////////////////////////////
    template <class T>
    math::Pose3d BaseNode<T>::WorldPose() const
    {
      VisualPtr parent = this->Parent();
      math::Pose3d pose = this->LocalPose();

      if (!parent)
      {
        return pose;
      }

      return pose + parent->WorldPose();
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
    math::Vector3d BaseNode<T>::WorldPosition() const
    {
      return this->WorldPose().Pos();
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseNode<T>::SetWorldPosition(const math::Vector3d &_position)
    {
      math::Pose3d pose = this->WorldPose();
      pose.Pos() =_position;
      this->SetWorldPose(pose);
    }

    //////////////////////////////////////////////////
    template <class T>
    math::Quaterniond BaseNode<T>::WorldRotation() const
    {
      return this->WorldPose().Rot();
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
      math::Pose3d pose = this->WorldPose();
      pose.Rot() = _rotation;
      this->SetWorldPose(pose);
    }

    //////////////////////////////////////////////////
    template <class T>
    math::Pose3d BaseNode<T>::WorldToLocal(const math::Pose3d &_pose) const
    {
      VisualPtr parent = this->Parent();

      if (!parent)
      {
        return _pose;
      }

      return _pose - parent->WorldPose();
    }

    //////////////////////////////////////////////////
    template <class T>
    math::Vector3d BaseNode<T>::Origin() const
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
    //////////////////////////////////////////////////
    template <class T>
    void BaseNode<T>::PreRender()
    {
      T::PreRender();
      if (this->trackNode)
      {
        math::Vector3d eye = this->WorldPosition();
        math::Vector3d target = this->trackNode->WorldPosition();
        if (this->trackFollow)
        {
          eye = target + this->trackFollowOffset;
        }
        math::Pose3d p =
            math::Matrix4d::LookAt(eye, target).Pose();
        this->SetWorldPose(p);
      }
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseNode<T>::SetAutoTrack(const bool _enabled, const NodePtr &_target,
        const bool _follow,
        const math::Vector3d &_followOffset)
    {
      if (!_enabled)
      {
        this->trackNode = NodePtr();
        this->trackFollow = false;
        this->trackFollowOffset = math::Vector3d::Zero;
        return;
      }
      this->trackNode = _target;
      this->trackFollow = _follow;
      this->trackFollowOffset = _followOffset;
    }
  }
}
#endif
