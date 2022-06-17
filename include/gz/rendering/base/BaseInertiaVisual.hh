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
#ifndef GZ_RENDERING_BASE_BASEINERTIAVISUAL_HH_
#define GZ_RENDERING_BASE_BASEINERTIAVISUAL_HH_

#include "gz/common/Console.hh"

#include "gz/rendering/base/BaseObject.hh"
#include "gz/rendering/base/BaseRenderTypes.hh"
#include "gz/rendering/InertiaVisual.hh"
#include "gz/rendering/Scene.hh"

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    //
    /// \brief Base implementation of an inertia visual
    template <class T>
    class BaseInertiaVisual :
      public virtual InertiaVisual,
      public virtual T
    {
      /// \brief Constructor
      protected: BaseInertiaVisual();

      /// \brief Destructor
      public: virtual ~BaseInertiaVisual();

      // Documentation inherited.
      protected: virtual void Init() override;

      // Documentation inherited.
      protected: virtual void PreRender() override;

      // Documentation inherited.
      public: virtual void SetInertial(
                  const gz::math::Inertiald &_inertial) override;

      // Documentation inherited.
      public: virtual void Load(const gz::math::Pose3d &,
          const gz::math::Vector3d &) override;

      // Documentation inherited
      public: virtual VisualPtr BoxVisual() const override;
    };

    //////////////////////////////////////////////////
    template <class T>
    BaseInertiaVisual<T>::BaseInertiaVisual()
    {
    }

    //////////////////////////////////////////////////
    template <class T>
    BaseInertiaVisual<T>::~BaseInertiaVisual()
    {
    }

    /////////////////////////////////////////////////
    template <class T>
    void BaseInertiaVisual<T>::PreRender()
    {
      T::PreRender();
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseInertiaVisual<T>::Init()
    {
      T::Init();
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseInertiaVisual<T>::SetInertial(
          const gz::math::Inertiald &_inertial)
    {
      auto xyz = _inertial.Pose().Pos();
      auto q = _inertial.Pose().Rot();

      // Use gz::math::MassMatrix3 to compute
      // equivalent box size and rotation
      auto m = _inertial.MassMatrix();
      gz::math::Vector3d boxScale;
      gz::math::Quaterniond boxRot;
      if (!m.EquivalentBox(boxScale, boxRot))
      {
        // Invalid inertia, load with default scale
        gzlog << "The link is static or has unrealistic "
            << "inertia, so the equivalent inertia box will not be shown.\n";
      }
      else
      {
        // Apply additional rotation by boxRot
        this->Load(gz::math::Pose3d(xyz, q * boxRot), boxScale);
      }
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseInertiaVisual<T>::Load(const gz::math::Pose3d &,
        const gz::math::Vector3d &)
    {
      // no op
    }

    //////////////////////////////////////////////////
    template <class T>
    VisualPtr BaseInertiaVisual<T>::BoxVisual() const
    {
      return nullptr;
    }
    }
  }
}
#endif
