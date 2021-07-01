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

#include "ignition/rendering/base/BaseObject.hh"
#include "ignition/rendering/base/BaseRenderTypes.hh"
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
