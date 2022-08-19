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
#ifndef IGNITION_RENDERING_BASE_BASEAXISVISUAL_HH_
#define IGNITION_RENDERING_BASE_BASEAXISVISUAL_HH_

#include "ignition/rendering/AxisVisual.hh"
#include "ignition/rendering/ArrowVisual.hh"
#include "ignition/rendering/Scene.hh"

namespace ignition
{
  namespace rendering
  {
    inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {
    //
    template <class T>
    class BaseAxisVisual :
      public virtual AxisVisual,
      public virtual T
    {
      protected: BaseAxisVisual();

      public: virtual ~BaseAxisVisual();

      public: virtual void Init();
    };

    //////////////////////////////////////////////////
    template <class T>
    BaseAxisVisual<T>::BaseAxisVisual()
    {
    }

    //////////////////////////////////////////////////
    template <class T>
    BaseAxisVisual<T>::~BaseAxisVisual()
    {
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseAxisVisual<T>::Init()
    {
      T::Init();

      ArrowVisualPtr xArrow = this->Scene()->CreateArrowVisual();
      xArrow->SetLocalPosition(0, 0, 0);
      xArrow->SetLocalRotation(0, IGN_PI / 2, 0);
      xArrow->SetMaterial("Default/TransRed");
      this->AddChild(xArrow);

      ArrowVisualPtr yArrow = this->Scene()->CreateArrowVisual();
      yArrow->SetLocalPosition(0, 0, 0);
      yArrow->SetLocalRotation(-IGN_PI / 2, 0, 0);
      yArrow->SetMaterial("Default/TransGreen");
      this->AddChild(yArrow);

      ArrowVisualPtr zArrow = this->Scene()->CreateArrowVisual();
      zArrow->SetLocalPosition(0, 0, 0);
      zArrow->SetLocalRotation(0, 0, 0);
      zArrow->SetMaterial("Default/TransBlue");
      this->AddChild(zArrow);
    }
    }
  }
}
#endif
