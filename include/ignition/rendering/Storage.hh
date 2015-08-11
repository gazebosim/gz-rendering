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
#ifndef _IGNITION_RENDERING_STORAGE_HH_
#define _IGNITION_RENDERING_STORAGE_HH_

#include <string>
#include <boost/shared_ptr.hpp>
#include "ignition/rendering/Scene.hh"
#include "ignition/rendering/Node.hh"
#include "ignition/rendering/Light.hh"
#include "ignition/rendering/Sensor.hh"
#include "ignition/rendering/Visual.hh"
#include "ignition/rendering/Geometry.hh"
#include "ignition/rendering/Material.hh"
#include "ignition/rendering/Mesh.hh"
#include "ignition/rendering/Util.hh"

namespace ignition
{
  namespace rendering
  {
    template <class T>
    class IGNITION_VISIBLE Map
    {
      typedef boost::shared_ptr<T> TPtr;

      typedef boost::shared_ptr<const T> ConstTPtr;

      public: virtual ~Map() { }

      public: virtual unsigned int Size() const = 0;

      public: virtual bool ContainsKey(const std::string &_key) const = 0;

      public: virtual bool ContainsValue(ConstTPtr _value) const = 0;

      public: virtual TPtr Get(const std::string &_key) const = 0;

      public: virtual TPtr GetByIndex(unsigned int _index) const = 0;

      public: virtual bool Put(const std::string &_key, TPtr _value) = 0;

      public: virtual void Remove(const std::string &_key) = 0;

      public: virtual void Remove(TPtr _value) = 0;

      public: virtual void RemoveAll() = 0;
    };

    template <class T>
    class IGNITION_VISIBLE Store
    {
      typedef boost::shared_ptr<T> TPtr;

      typedef boost::shared_ptr<const T> ConstTPtr;

      public: virtual ~Store() { }

      public: virtual unsigned int Size() const = 0;

      public: virtual bool Contains(ConstTPtr _object) const = 0;

      public: virtual bool ContainsId(unsigned int _id) const = 0;

      public: virtual bool ContainsName(const std::string &_name) const = 0;

      public: virtual TPtr GetById(unsigned int _id) const = 0;

      public: virtual TPtr GetByName(const std::string &_name) const = 0;

      public: virtual TPtr GetByIndex(unsigned int _index) const = 0;

      public: virtual bool Add(TPtr _object) = 0;

      public: virtual TPtr Remove(TPtr _object) = 0;

      public: virtual TPtr RemoveById(unsigned int _id) = 0;

      public: virtual TPtr RemoveByName(const std::string &_name) = 0;

      public: virtual TPtr RemoveByIndex(unsigned int _index) = 0;

      public: virtual void RemoveAll() = 0;

      public: virtual void Destroy(TPtr _object) = 0;

      public: virtual void DestroyById(unsigned int _id) = 0;

      public: virtual void DestroyByName(const std::string &_name) = 0;

      public: virtual void DestroyByIndex(unsigned int _index) = 0;

      public: virtual void DestroyAll() = 0;
    };

    template <class T>
    class IGNITION_VISIBLE CompositeStore :
      public Store<T>
    {
      typedef boost::shared_ptr<T> TPtr;

      typedef Store<T> TStore;

      typedef boost::shared_ptr<TStore> TStorePtr;

      typedef boost::shared_ptr<const TStore> ConstTStorePtr;

      public: virtual ~CompositeStore() { }

      public: virtual unsigned int GetStoreCount() const = 0;

      public: virtual bool ContainsStore(ConstTStorePtr _store) const = 0;

      public: virtual bool AddStore(TStorePtr _store) = 0;

      public: virtual TStorePtr GetStore(unsigned int _index) const = 0;

      public: virtual TStorePtr RemoveStore(TStorePtr _store) = 0;

      public: virtual TStorePtr RemoveStore(unsigned int _index) = 0;
    };

    template <class T, class U>
    class IGNITION_VISIBLE StoreWrapper :
      public Store<T>
    {
      public: virtual ~StoreWrapper() { }
    };

    template class Store<Scene>;
    template class Store<Node>;
    template class Store<Light>;
    template class Store<Sensor>;
    template class Store<Visual>;
    template class Store<Geometry>;
    template class Store<SubMesh>;
    template class Map<Material>;

    typedef Store<Scene>    SceneStore;
    typedef Store<Node>     NodeStore;
    typedef Store<Light>    LightStore;
    typedef Store<Sensor>   SensorStore;
    typedef Store<Visual>   VisualStore;
    typedef Store<Geometry> GeometryStore;
    typedef Store<SubMesh>  SubMeshStore;
    typedef Map<Material>   MaterialMap;

    typedef boost::shared_ptr<SceneStore>    SceneStorePtr;
    typedef boost::shared_ptr<NodeStore>     NodeStorePtr;
    typedef boost::shared_ptr<LightStore>    LightStorePtr;
    typedef boost::shared_ptr<SensorStore>   SensorStorePtr;
    typedef boost::shared_ptr<VisualStore>   VisualStorePtr;
    typedef boost::shared_ptr<GeometryStore> GeometryStorePtr;
    typedef boost::shared_ptr<SubMeshStore>  SubMeshStorePtr;
    typedef boost::shared_ptr<MaterialMap>   MaterialMapPtr;

    template class CompositeStore<Node>;
    typedef CompositeStore<Node> NodeCompositeStore;
    typedef boost::shared_ptr<NodeCompositeStore> NodeCompositeStorePtr;
  }
}
#endif
