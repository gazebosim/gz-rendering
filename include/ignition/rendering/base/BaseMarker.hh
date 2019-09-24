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
#ifndef IGNITION_RENDERING_MARKER_HH_
#define IGNITION_RENDERING_MARKER_HH_

#include <string>

#include "ignition/rendering/ogre/Export.hh"
#include "ignition/rendering/Visual.hh"

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
    class IGNITION_RENDERING_OGRE_VISIBLE BaseMarker :
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
      public: virtual void SetPose(math::Pose _pose);
      public: virtual math::Pose Pose() const;
      public: virtual void SetScale(math::Vector3d _scale);
      public: virtual math::Vector3d Scale() const;
      public: virtual void SetParent(std::string _parent);
      public: virtual std::string Parent() const;

      //TODO add init vals
      protected: common::Time lifetime;
      protected: bool loaded;
      protected: std::string ns;
      protected: uint64_t id;
      protected: int32_t layer;
      protected: math::Pose pose;
      protected: math::Vector3d scale;
      protected: std::string parent;
      protected: bool markerDirty = false;
    };
    
    /////////////////////////////////////////////////
    // BaseMarkerVisual
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
    void setLifetime(common::Time _lifetime)
    {
      this->lifetime = _lifetime;
      this->markerDirty = true;
    }

    /////////////////////////////////////////////////
    template <class T>
    common::Time Lifetime() const
    {
      return this->lifetime;
    }

    /////////////////////////////////////////////////
    template <class T>
    void SetLoaded(bool _loaded)
    {
      this->loaded = _loaded;
      this->markerDirty = true;
    }

    /////////////////////////////////////////////////
    template <class T>
    bool isLoaded() const
    {
      return this->loaded;
    }

    /////////////////////////////////////////////////
    template <class T>
    void SetNS(std::string _ns)
    {
      this->ns = _ns;
      this->markerDirty = true;
    }

    /////////////////////////////////////////////////
    template <class T>
    std::string NS() const
    {
      return this->ns;
    }

    /////////////////////////////////////////////////
    template <class T>
    void SetID(uint64_t _id)
    {
      this->id = _id;
      this->markerDirty = true;
    }

    /////////////////////////////////////////////////
    template <class T>
    uint64_t ID() const
    {
      return this->id;
    }

    /////////////////////////////////////////////////
    template <class T>
    void SetLayer(int32_t _layer)
    {
      this->layer = _layer;
      this->markerDirty = true;
    }

    /////////////////////////////////////////////////
    template <class T>
    int32_t Layer() const
    {
      return this->layer;
    }

    /////////////////////////////////////////////////
    template <class T>
    void SetPose(math::Pose _pose)
    {
      this->pose = _pose;
      this->markerDirty = true;
    }

    /////////////////////////////////////////////////
    template <class T>
    math::Pose Pose() const
    {
      return this->pose;
    }

    /////////////////////////////////////////////////
    template <class T>
    void SetScale(math::Vector3d _scale)
    {
      this->scale = _scale;
      this->markerDirty = true;
    }

    /////////////////////////////////////////////////
    template <class T>
    math::Vector3d Scale() const
    {
      return this->scale;
    }

    /////////////////////////////////////////////////
    template <class T>
    void SetParent(std::string _parent)
    {
      this->parent = _parent;
      this->markerDirty = true;
    }

    /////////////////////////////////////////////////
    template <class T>
    std::string Parent() const
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
