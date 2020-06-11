/*
 * Copyright (C) 2019 Open Source Robotics Foundation
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
#ifndef IGNITION_RENDERING_BASELIDARVISUAL_HH_
#define IGNITION_RENDERING_BASELIDARVISUAL_HH_

#include "ignition/rendering/LidarVisual.hh"
#include "ignition/rendering/base/BaseObject.hh"
#include "ignition/rendering/base/BaseRenderTypes.hh"

namespace ignition
{
  namespace rendering
  {
    inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {
    /// \brief Base implementation of a LidarVisual geometry
    template <class T>
    class BaseLidarVisual :
      public virtual LidarVisual,
      public virtual T
    {
      /// \brief Constructor.
      protected: BaseLidarVisual();

      /// \brief Destructor.
      public: virtual ~BaseLidarVisual();

      /// \brief PreRender function
      public: virtual void PreRender() override;

      /// \brief Destroy function
      public: virtual void Destroy() override;

      // Documentation inherited
      public: virtual void ClearPoints() override;

      // Documentation inherited
      public: virtual void OnMsg(std::vector<double> &msg);

      //Documentation Inherited
      public: virtual void Update();

      //Documentation Inherited
      public: virtual void Init();
    };

    /////////////////////////////////////////////////
    // BaseLidarVisual
    /////////////////////////////////////////////////
    template <class T>
    BaseLidarVisual<T>::BaseLidarVisual()
    {
    }

    /////////////////////////////////////////////////
    template <class T>
    BaseLidarVisual<T>::~BaseLidarVisual()
    {
    }

    /////////////////////////////////////////////////
    template <class T>
    void BaseLidarVisual<T>::PreRender()
    {
      T::PreRender();
    }

    /////////////////////////////////////////////////
    template <class T>
    void BaseLidarVisual<T>::Destroy()
    {
      T::Destroy();
    }

    /////////////////////////////////////////////////
    template <class T>
    void BaseLidarVisual<T>::ClearPoints()
    {
        // no op
    }

    /////////////////////////////////////////////////
    template <class T>
    void BaseLidarVisual<T>::Update()
    {
      // no op
    }

    /////////////////////////////////////////////////
    template <class T>
    void BaseLidarVisual<T>::OnMsg(std::vector<double> &)
    {
      // no op
    }

    /////////////////////////////////////////////////
    template <class T>
    void BaseLidarVisual<T>::Init()
    {
      T::Init();
    }

    }
  }
}
#endif
