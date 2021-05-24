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
#ifndef IGNITION_RENDERING_BASE_BASEINERTIAVISUAL_HH_
#define IGNITION_RENDERING_BASE_BASEINERTIAVISUAL_HH_

#include <vector>

#include "ignition/rendering/base/BaseObject.hh"
#include "ignition/rendering/base/BaseRenderTypes.hh"
#include "ignition/rendering/InertiaVisual.hh"
#include "ignition/rendering/Scene.hh"

namespace ignition
{
  namespace rendering
  {
    inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {
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
      public: virtual void Load(const ignition::math::Pose3d &,
          const ignition::math::Vector3d &) override;

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
    void BaseInertiaVisual<T>::Load(const ignition::math::Pose3d &,
        const ignition::math::Vector3d &)
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
