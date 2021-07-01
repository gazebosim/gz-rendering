/*
 * Copyright (C) 2021 Open Source Robotics Foundation
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
#ifndef IGNITION_RENDERING_BASE_BASEJOINTVISUAL_HH_
#define IGNITION_RENDERING_BASE_BASEJOINTVISUAL_HH_

#include <string>
#include <sstream>

#include "ignition/rendering/base/BaseObject.hh"
#include "ignition/rendering/base/BaseRenderTypes.hh"
#include "ignition/rendering/ArrowVisual.hh"
#include "ignition/rendering/AxisVisual.hh"
#include "ignition/rendering/JointVisual.hh"
#include "ignition/rendering/Scene.hh"

namespace ignition
{
  namespace rendering
  {
    inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {
    //
    /// \brief Base implementation of a joint visual
    template <class T>
    class BaseJointVisual :
      public virtual JointVisual,
      public virtual T
    {
      /// \brief Constructor
      protected: BaseJointVisual();

      /// \brief Destructor
      public: virtual ~BaseJointVisual();

      // Documentation inherited.
      protected: virtual void Init() override;

      // Documentation inherited.
      protected: virtual void PreRender() override;

      // Documentation inherited
      public: virtual void CreateAxis(const ignition::math::Vector3d &_axis,
          const std::string &_xyzExpressedIn) override;

      // Documentation inherited
      public: virtual void CreateParentAxis(
          const ignition::math::Vector3d &_axis,
          const std::string &_xyzExpressedIn) override;

      // Documentation inherited
      public: virtual void UpdateAxis(const ignition::math::Vector3d &_axis,
          const std::string &_xyzExpressedIn) override;

      // Documentation inherited
      public: virtual void UpdateParentAxis(
          const ignition::math::Vector3d &_axis,
          const std::string &_xyzExpressedIn) override;

      // Documentation inherited
      public: virtual void SetType(const JointVisualType _type) override;

      // Documentation inherited
      public: virtual JointVisualPtr ParentAxisVisual() const override;

      // Documentation inherited
      public: virtual ArrowVisualPtr ArrowVisual() const override;

      /// \brief Implementation for updating an axis' arrow visual.
      /// \param[in] _arrowVisual Arrow visual to be updated.
      /// \param[in] _axis Axis vector.
      /// \param[in] _xyzExpressedIn Frame in which the axis vector is
      /// expressed.
      protected: void UpdateAxisImpl(ArrowVisualPtr _arrowVisual,
          const ignition::math::Vector3d &_axis,
          const std::string &_xyzExpressedIn);

      /// \brief Type of joint visualization
      protected: JointVisualType jointVisualType =
          JointVisualType::JVT_NONE;

      /// \brief The joint's XYZ frame visual.
      protected: AxisVisualPtr axisVisual = nullptr;

      /// \brief The visual representing the one joint axis. There can be only
      /// one axis visual per joint visual, so joints with two axes have a 2nd
      /// JointVisual with its own arrowVisual.
      protected: ArrowVisualPtr arrowVisual = nullptr;

      /// \brief Second joint visual for hinge2 and universal joints. It is a
      /// simplified visual without an XYZ frame.
      protected: JointVisualPtr parentAxisVis = nullptr;

      /// \brief Scale based on the size of the joint's child.
      protected: ignition::math::Vector3d scaleToChild =
          ignition::math::Vector3d::One;
    };

    //////////////////////////////////////////////////
    template <class T>
    BaseJointVisual<T>::BaseJointVisual()
    {
    }

    //////////////////////////////////////////////////
    template <class T>
    BaseJointVisual<T>::~BaseJointVisual()
    {
    }

    /////////////////////////////////////////////////
    template <class T>
    void BaseJointVisual<T>::PreRender()
    {
      T::PreRender();
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseJointVisual<T>::Init()
    {
      T::Init();

      this->axisVisual = this->Scene()->CreateAxisVisual();
      this->AddChild(this->axisVisual);
    }

    /////////////////////////////////////////////////
    template <class T>
    void BaseJointVisual<T>::CreateAxis(
          const ignition::math::Vector3d &_axis,
          const std::string &_xyzExpressedIn)
    {
      if (this->arrowVisual)
      {
        this->arrowVisual->Destroy();
        this->arrowVisual.reset();
      }

      this->arrowVisual = this->Scene()->CreateArrowVisual();
      this->arrowVisual->SetMaterial("Default/TransYellow");
      this->UpdateAxis(_axis, _xyzExpressedIn);
      this->AddChild(this->arrowVisual);
    }

    /////////////////////////////////////////////////
    template <class T>
    void BaseJointVisual<T>::CreateParentAxis(
          const ignition::math::Vector3d &_axis,
          const std::string &_xyzExpressedIn)
    {
      if (this->parentAxisVis)
      {
        this->parentAxisVis->Destroy();
        this->parentAxisVis.reset();
      }

      this->parentAxisVis = this->Scene()->CreateJointVisual();
      this->parentAxisVis->SetType(this->jointVisualType);
      this->parentAxisVis->CreateAxis(_axis, _xyzExpressedIn);
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseJointVisual<T>::UpdateAxis(const ignition::math::Vector3d &_axis,
          const std::string &_xyzExpressedIn)
    {
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseJointVisual<T>::UpdateParentAxis(
          const ignition::math::Vector3d &_axis,
          const std::string &_xyzExpressedIn)
    {
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseJointVisual<T>::UpdateAxisImpl(ArrowVisualPtr _arrowVisual,
          const ignition::math::Vector3d &_axis,
          const std::string &_xyzExpressedIn)
    {
      // Get rotation to axis vector
      ignition::math::Vector3d axisDir = _axis;
      ignition::math::Vector3d u = axisDir.Normalize();
      ignition::math::Vector3d v = ignition::math::Vector3d::UnitZ;
      double cosTheta = v.Dot(u);
      double angle = acos(cosTheta);
      ignition::math::Quaterniond quat;
      // check the parallel case
      if (ignition::math::equal(angle, M_PI))
        quat.Axis(u.Perpendicular(), angle);
      else
        quat.Axis((v.Cross(u)).Normalize(), angle);
      _arrowVisual->SetLocalRotation(quat);

      if (_xyzExpressedIn == "__model__")
      {
        VisualPtr parentVis = this->Parent();
        ignition::math::Pose3d parentInitPose = parentVis->InitialLocalPose();

        // get rotation of joint visual in model frame
        ignition::math::Quaterniond quatFromModel =
            (this->Pose() + parentInitPose).Rot();

        // rotate arrow visual so that the axis vector applies to the model
        // frame.
        _arrowVisual->SetLocalRotation(quatFromModel.Inverse() *
            _arrowVisual->LocalRotation());
      }

      // _arrowVisual->ShowRotation(_type == msgs::Joint::REVOLUTE ||
      //                        _type == msgs::Joint::REVOLUTE2 ||
      //                        _type == msgs::Joint::UNIVERSAL ||
      //                        _type == msgs::Joint::GEARBOX);

      if (this->axisVisual)
        _arrowVisual->SetVisible(true);
      else
        return;

       // Hide existing arrow head if it overlaps with the axis
      auto axisWorldRotation = _arrowVisual->WorldPose().Rot();
      auto jointWorldRotation = this->WorldPose().Rot();

      this->axisVisual->ShowAxisHead(true);
      _arrowVisual->ShowArrowShaft(true);

      auto axisWorld = axisWorldRotation * ignition::math::Vector3d::UnitZ;
      if (axisWorld == jointWorldRotation * ignition::math::Vector3d::UnitX)
      {
        this->axisVisual->ShowAxisHead(0, false);
        _arrowVisual->ShowArrowShaft(false);
      }
      else if (axisWorld ==
          jointWorldRotation * ignition::math::Vector3d::UnitY)
      {
        this->axisVisual->ShowAxisHead(1, false);
        _arrowVisual->ShowArrowShaft(false);
      }
      else if (axisWorld ==
          jointWorldRotation * ignition::math::Vector3d::UnitZ)
      {
        this->axisVisual->ShowAxisHead(2, false);
        _arrowVisual->ShowArrowShaft(false);
      }
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseJointVisual<T>::SetType(const JointVisualType _type)
    {
      this->jointVisualType = _type;
    }

    //////////////////////////////////////////////////
    template <class T>
    JointVisualPtr BaseJointVisual<T>::ParentAxisVisual() const
    {
      return nullptr;
    }

    //////////////////////////////////////////////////
    template <class T>
    ArrowVisualPtr BaseJointVisual<T>::ArrowVisual() const
    {
      return nullptr;
    }
    }
  }
}
#endif
