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

#include <string>

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
    /// \sa MarkerManager
    //  TODO add descriptions to all below
    template <class T>
    class BaseMarker :
      public virtual Marker,
      public virtual T
    {
      /// \brief Constructor.
      protected: BaseMarker();

      /// \brief Destructor.
      public: virtual ~BaseMarker();
      
      public: virtual void PreRender();

      public: virtual void Destroy();

      public: virtual void setLifetime(common::Time _lifetime);
      public: virtual common::Time Lifetime() const;
      public: virtual void SetLoaded(bool _loaded);
      public: virtual bool isLoaded() const;
      public: virtual void SetNS(std::string _ns);
      public: virtual std::string NS() const;
      public: virtual void SetID(uint64_t id);
      public: virtual uint64_t ID() const;
      public: virtual void SetLayer(int32_t _layer);
      public: virtual int32_t Layer() const;
      public: virtual void SetPose(math::Pose3d _pose);
      public: virtual math::Pose3d Pose() const;
      public: virtual void SetScale(math::Vector3d _scale);
      public: virtual math::Vector3d Scale() const;
      public: virtual void SetParent(std::string _parent);
      public: virtual std::string getParent() const;

      //TODO add init vals
      protected: common::Time lifetime;
      protected: bool loaded = false;
      protected: std::string ns;
      protected: uint64_t id = 0;
      protected: int32_t layer = 0;
      protected: math::Pose3d pose;
      protected: math::Vector3d scale;
      protected: std::string parent;
      protected: bool markerDirty = false;
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
    void BaseMarker<T>::setLifetime(common::Time _lifetime)
    {
      this->lifetime = _lifetime;
      this->markerDirty = true;
    }

    /////////////////////////////////////////////////
    template <class T>
    common::Time BaseMarker<T>::Lifetime() const
    {
      return this->lifetime;
    }

    /////////////////////////////////////////////////
    template <class T>
    void BaseMarker<T>::SetLoaded(bool _loaded)
    {
      this->loaded = _loaded;
      this->markerDirty = true;
    }

    /////////////////////////////////////////////////
    template <class T>
    bool BaseMarker<T>::isLoaded() const
    {
      return this->loaded;
    }

    /////////////////////////////////////////////////
    template <class T>
    void BaseMarker<T>::SetNS(std::string _ns)
    {
      this->ns = _ns;
      this->markerDirty = true;
    }

    /////////////////////////////////////////////////
    template <class T>
    std::string BaseMarker<T>::NS() const
    {
      return this->ns;
    }

    /////////////////////////////////////////////////
    template <class T>
    void BaseMarker<T>::SetID(uint64_t _id)
    {
      this->id = _id;
      this->markerDirty = true;
    }

    /////////////////////////////////////////////////
    template <class T>
    uint64_t BaseMarker<T>::ID() const
    {
      return this->id;
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
    void BaseMarker<T>::SetPose(math::Pose3d _pose)
    {
      this->pose = _pose;
      this->markerDirty = true;
    }

    /////////////////////////////////////////////////
    template <class T>
    math::Pose3d BaseMarker<T>::Pose() const
    {
      return this->pose;
    }

    /////////////////////////////////////////////////
    template <class T>
    void BaseMarker<T>::SetScale(math::Vector3d _scale)
    {
      this->scale = _scale;
      this->markerDirty = true;
    }

    /////////////////////////////////////////////////
    template <class T>
    math::Vector3d BaseMarker<T>::Scale() const
    {
      return this->scale;
    }

    /////////////////////////////////////////////////
    template <class T>
    void BaseMarker<T>::SetParent(std::string _parent)
    {
      this->parent = _parent;
      this->markerDirty = true;
    }

    /////////////////////////////////////////////////
    template <class T>
    std::string BaseMarker<T>::getParent() const
    {
      return this->parent;
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
