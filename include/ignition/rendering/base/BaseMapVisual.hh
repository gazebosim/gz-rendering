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
#ifndef IGNITION_RENDERING_BASE_BASEMAPVISUAL_HH_
#define IGNITION_RENDERING_BASE_BASEMAPVISUAL_HH_

#include <string>

#include <ignition/common/MeshManager.hh>

#include "ignition/rendering/MapVisual.hh"
#include "ignition/rendering/Scene.hh"

namespace ignition
{
  namespace rendering
  {
    inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {
    //
    template <class T>
    class BaseMapVisual :
      public virtual MapVisual,
      public virtual T
    {
      /// \brief Constructor
      protected: BaseMapVisual();

      /// \brief Destructor
      public: virtual ~BaseMapVisual();

      // Documentation inherited.
      protected: virtual void Init() override;

      // Documentation inherited.
      protected: virtual void PreRender() override;

      // Documentation inherited.
    };

    //////////////////////////////////////////////////
    template <class T>
    BaseMapVisual<T>::BaseMapVisual()
    {
    }

    //////////////////////////////////////////////////
    template <class T>
    BaseMapVisual<T>::~BaseMapVisual()
    {
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseMapVisual<T>::PreRender()
    {
      T::PreRender();
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseMapVisual<T>::Init()
    {
      T::Init();
    }
    }
  }
}
#endif
