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
#ifndef IGNITION_RENDERING_BASE_BASECOMVISUAL_HH_
#define IGNITION_RENDERING_BASE_BASECOMVISUAL_HH_

#include "ignition/common/Console.hh"

#include "ignition/rendering/base/BaseObject.hh"
#include "ignition/rendering/base/BaseRenderTypes.hh"
#include "ignition/rendering/COMVisual.hh"
#include "ignition/rendering/Scene.hh"

namespace ignition
{
  namespace rendering
  {
    inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {
    //
    /// \brief Base implementation of an center of mass visual
    template <class T>
    class BaseCOMVisual :
      public virtual COMVisual,
      public virtual T
    {
      /// \brief Constructor
      protected: BaseCOMVisual();

      /// \brief Destructor
      public: virtual ~BaseCOMVisual();

      // Documentation inherited.
      protected: virtual void Init() override;

      // Documentation inherited.
      protected: virtual void PreRender() override;

      // Documentation inherited.
      public: virtual void SetInertial(
                  const ignition::math::Inertiald &_inertial) override;

      // Documentation inherited.
      public: virtual void Load(const ignition::math::Pose3d &,
          const ignition::math::Vector3d &) override;

      // Documentation inherited
      public: ignition::math::Pose3d InertiaPose() const;

      // Documentation inherited
      public: virtual VisualPtr SphereVisual() const override;

      /// \brief Inertia pose in link frame.
      private: ignition::math::Pose3d inertiaPose =
          ignition::math::Pose3d::Zero;
    };

    //////////////////////////////////////////////////
    template <class T>
    BaseCOMVisual<T>::BaseCOMVisual()
    {
    }

    //////////////////////////////////////////////////
    template <class T>
    BaseCOMVisual<T>::~BaseCOMVisual()
    {
    }

    /////////////////////////////////////////////////
    template <class T>
    void BaseCOMVisual<T>::PreRender()
    {
      T::PreRender();
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseCOMVisual<T>::Init()
    {
      T::Init();
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseCOMVisual<T>::SetInertial(
          const ignition::math::Inertiald &_inertial)
    {
      auto xyz = _inertial.Pose().Pos();
      auto q = _inertial.Pose().Rot();

      // Use ignition::math::MassMatrix3 to compute
      // equivalent box size and rotation
      auto m = _inertial.MassMatrix();
      ignition::math::Vector3d boxScale;
      ignition::math::Quaterniond boxRot;
      if (!m.EquivalentBox(boxScale, boxRot))
      {
        // Invalid inertia, load with default scale
        ignlog << "The link is static or has unrealistic "
            << "inertia, so the equivalent inertia box will not be shown.\n";
      }
      else
      {
        // Apply additional rotation by boxRot
        this->Load(ignition::math::Pose3d(xyz, q * boxRot), boxScale);
      }
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseCOMVisual<T>::Load(const ignition::math::Pose3d &,
        const ignition::math::Vector3d &)
    {
      // no op
    }

    //////////////////////////////////////////////////
    template <class T>
    VisualPtr BaseCOMVisual<T>::InertiaPose() const
    {
      return this->inertiaPose;
    }

    //////////////////////////////////////////////////
    template <class T>
    VisualPtr BaseCOMVisual<T>::BoxVisual() const
    {
      return nullptr;
    }
    }
  }
}
#endif
