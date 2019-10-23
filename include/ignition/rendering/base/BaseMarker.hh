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
#ifndef IGNITION_RENDERING_BASEMARKER_HH_
#define IGNITION_RENDERING_BASEMARKER_HH_

#include "ignition/rendering/Marker.hh"
#include "ignition/rendering/base/BaseObject.hh"
#include "ignition/rendering/base/BaseRenderTypes.hh"

namespace ignition
{
  namespace rendering
  {
    inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {
    /// \brief A marker visualization. The MarkerManager class should
    /// instantiate instances of this class.
    /// \sa Base implementation of a Marker geometry
    template <class T>
    class BaseMarker :
      public virtual Marker,
      public virtual T
    {
      /// \brief Constructor.
      protected: BaseMarker();

      /// \brief Destructor.
      public: virtual ~BaseMarker();

      /// \brief PreRender function
      public: virtual void PreRender();

      /// \brief Destroy function
      public: virtual void Destroy();

      // Documentation inherited
      public: virtual void SetLifetime(
                  const std::chrono::steady_clock::duration &_lifetime) override;

      // Documentation inherited
      public: virtual std::chrono::steady_clock::duration Lifetime() const override;

      // Documentation inherited
      public: virtual void SetType(const MarkerType _markerType) override;

      // Documentation inherited
      public: virtual MarkerType Type() const override;

      // Documentation inherited
      public: virtual void SetLayer(int32_t _layer) override;

      // Documentation inherited
      public: virtual int32_t Layer() const override;

      protected: std::chrono::steady_clock::duration lifetime;
      protected: int32_t layer = 0;
      protected: bool markerDirty = false;
      protected: MarkerType markerType = ignition::rendering::MarkerType::NONE;
    };

    /////////////////////////////////////////////////
    // BaseMarker
    /////////////////////////////////////////////////
    template <class T>
    BaseMarker<T>::BaseMarker()
    {
    }

    /////////////////////////////////////////////////
    template <class T>
    BaseMarker<T>::~BaseMarker()
    {
    }

    /////////////////////////////////////////////////
    template <class T>
    void BaseMarker<T>::SetLifetime(
        const std::chrono::steady_clock::duration &_lifetime)
    {
      this->lifetime = _lifetime;
      this->markerDirty = true;
    }

    /////////////////////////////////////////////////
    template <class T>
    std::chrono::steady_clock::duration BaseMarker<T>::Lifetime() const
    {
      return this->lifetime;
    }

    /////////////////////////////////////////////////
    template <class T>
    void BaseMarker<T>::SetLayer(int32_t _layer)
    {
      this->layer = _layer;
      this->markerDirty = true;
    }

    /////////////////////////////////////////////////
    template <class T>
    int32_t BaseMarker<T>::Layer() const
    {
      return this->layer;
    }

    /////////////////////////////////////////////////
    template <class T>
    void BaseMarker<T>::SetType(const MarkerType _markerType)
    {
      this->markerType = _markerType;
      this->markerDirty = true;
    }

    /////////////////////////////////////////////////
    template <class T>
    MarkerType BaseMarker<T>::Type() const
    {
      return this->markerType;
    }

    /////////////////////////////////////////////////
    template <class T>
    void BaseMarker<T>::PreRender()
    {
      T::PreRender();
    }

    /////////////////////////////////////////////////
    template <class T>
    void BaseMarker<T>::Destroy()
    {
      T::Destroy();
    }
    }
  }
}
#endif
