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
#ifndef IGNITION_RENDERING_BASE_BASEARROWVISUAL_HH_
#define IGNITION_RENDERING_BASE_BASEARROWVISUAL_HH_

#include "ignition/rendering/ArrowVisual.hh"
#include "ignition/rendering/Scene.hh"

namespace ignition
{
  namespace rendering
  {
    inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {
    //
    template <class T>
    class BaseArrowVisual :
      public virtual ArrowVisual,
      public virtual T
    {
      /// \brief Constructor
      protected: BaseArrowVisual();

      /// \brief Destructor
      public: virtual ~BaseArrowVisual();

      // Documentation inherited.
      public: virtual VisualPtr Head() const override;

      // Documentation inherited.
      public: virtual VisualPtr Shaft() const override;

      // Documentation inherited
      public: virtual void ShowArrowHead(bool _b) override;

      // Documentation inherited.
      protected: virtual void Init() override;
    };

    //////////////////////////////////////////////////
    template <class T>
    BaseArrowVisual<T>::BaseArrowVisual()
    {
    }

    //////////////////////////////////////////////////
    template <class T>
    BaseArrowVisual<T>::~BaseArrowVisual()
    {
    }

    //////////////////////////////////////////////////
    template <class T>
    VisualPtr BaseArrowVisual<T>::Head() const
    {
      return std::dynamic_pointer_cast<Visual>(this->ChildByIndex(1));
    }

    //////////////////////////////////////////////////
    template <class T>
    VisualPtr BaseArrowVisual<T>::Shaft() const
    {
      return std::dynamic_pointer_cast<Visual>(this->ChildByIndex(0));
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseArrowVisual<T>::ShowArrowHead(bool _b)
    {
      NodePtr child = this->ChildByIndex(1);
      VisualPtr visual = std::dynamic_pointer_cast<Visual>(child);
      if (visual)
      {
        visual->SetVisible(_b);
      }
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseArrowVisual<T>::Init()
    {
      T::Init();

      VisualPtr cone = this->Scene()->CreateVisual();
      cone->AddGeometry(this->Scene()->CreateCone());
      cone->SetOrigin(0, 0, -0.5);
      cone->SetLocalPosition(0, 0, 0);
      cone->SetLocalScale(0.1, 0.1, 0.25);
      this->AddChild(cone);

      VisualPtr cylinder = this->Scene()->CreateVisual();
      cylinder->AddGeometry(this->Scene()->CreateCylinder());
      cylinder->SetOrigin(0, 0, 0.5);
      cylinder->SetLocalPosition(0, 0, 0);
      cylinder->SetLocalScale(0.05, 0.05, 0.5);
      this->AddChild(cylinder);

      this->SetOrigin(0, 0, -0.5);
    }
    }
  }
}
#endif
