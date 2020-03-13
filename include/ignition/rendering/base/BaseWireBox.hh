/*
 * Copyright (C) 2020 Open Source Robotics Foundation
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
#ifndef IGNITION_RENDERING_BASE_WIREBOX_HH_
#define IGNITION_RENDERING_BASE_WIREBOX_HH_

#include <string>
#include "ignition/rendering/WireBox.hh"

namespace ignition
{
  namespace rendering
  {
    inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {
    //
    /// \brief Base implementation of a wireframe box.
    template <class T>
    class BaseWireBox :
      public virtual WireBox,
      public virtual T
    {
      /// \brief Constructor
      protected: BaseWireBox();

      /// \brief Destructor
      public: virtual ~BaseWireBox();

      // Documentation inherited.
      public: virtual void PreRender();

      // Documentation inherited.
      public: virtual void Destroy();

      public: virtual void SetVisible(bool _visible);

      public: virtual bool Visible() const;

      public: virtual void SetBox(const ignition::math::AxisAlignedBox &_box);

      public: virtual ignition::math::AxisAlignedBox Box() const;

      protected: ignition::math::AxisAlignedBox box;

      protected: bool visible = false;

      /// \brief Flag to indicate WireBox properties have changed
      protected: bool wireBoxDirty = false;
    };

    //////////////////////////////////////////////////
    // BaseWireBox
    //////////////////////////////////////////////////
    template <class T>
    BaseWireBox<T>::BaseWireBox()
    {
    }

    //////////////////////////////////////////////////
    template <class T>
    BaseWireBox<T>::~BaseWireBox()
    {
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseWireBox<T>::PreRender()
    {
      T::PreRender();
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseWireBox<T>::Destroy()
    {
      T::Destroy();
    }

    template <class T>
    void BaseWireBox<T>::SetVisible(bool _visible)
    {
      this->visible = _visible;
      this->wireBoxDirty = true;
    }

    template <class T>
    bool BaseWireBox<T>::Visible()
    {
      return this->visible;
    }

    template <class T>
    void BaseWireBox<T>::SetBox(const ignition::math::AxisAlignedBox &_box)
    {
      this->box = _box;
      this->wireBoxDirty = true;
    }

    template <class T>
    ignition::math::AxisAlignedBox BaseWireBox<T>::Box()
    {
      return this->box;
    }

    }
  }
}
#endif
