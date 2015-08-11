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
#ifndef _IGNITION_RENDERING_BASEMESH_HH_
#define _IGNITION_RENDERING_BASEMESH_HH_

#include "ignition/rendering/Mesh.hh"
#include "ignition/rendering/Storage.hh"
#include "ignition/rendering/base/BaseObject.hh"

namespace ignition
{
  namespace rendering
  {
    //////////////////////////////////////////////////
    template <class T>
    class IGNITION_VISIBLE BaseMesh :
      public virtual Mesh,
      public virtual T
    {
      protected: BaseMesh();

      public: virtual ~BaseMesh();

      public: virtual unsigned int GetSubMeshCount() const;

      public: virtual bool HasSubMesh(ConstSubMeshPtr _subMesh) const;

      public: virtual bool HasSubMeshName(const std::string &_name) const;

      public: virtual SubMeshPtr GetSubMeshByName(
                  const std::string &_name) const;

      public: virtual SubMeshPtr GetSubMeshByIndex(unsigned int _index) const;

      public: virtual void SetMaterial(MaterialPtr _material,
                  bool unique = true);

      public: virtual void PreRender();

      protected: virtual SubMeshStorePtr GetSubMeshes() const = 0;
    };

    //////////////////////////////////////////////////
    template <class T>
    class IGNITION_VISIBLE BaseSubMesh :
      public virtual SubMesh,
      public virtual T
    {
      protected: BaseSubMesh();

      public: virtual ~BaseSubMesh();

      public: virtual void PreRender();

      public: virtual MaterialPtr GetMaterial() const = 0;
    };

    //////////////////////////////////////////////////
    template <class T>
    BaseMesh<T>::BaseMesh()
    {
    }

    //////////////////////////////////////////////////
    template <class T>
    BaseMesh<T>::~BaseMesh()
    {
    }

    //////////////////////////////////////////////////
    template <class T>
    unsigned int BaseMesh<T>::GetSubMeshCount() const
    {
      return this->GetSubMeshes()->Size();
    }

    //////////////////////////////////////////////////
    template <class T>
    bool BaseMesh<T>::HasSubMesh(ConstSubMeshPtr _subMesh) const
    {
      return this->GetSubMeshes()->Contains(_subMesh);
    }

    //////////////////////////////////////////////////
    template <class T>
    bool BaseMesh<T>::HasSubMeshName(const std::string &_name) const
    {
      return this->GetSubMeshes()->ContainsName(_name);
    }

    //////////////////////////////////////////////////
    template <class T>
    SubMeshPtr BaseMesh<T>::GetSubMeshByName(const std::string &_name) const
    {
      return this->GetSubMeshes()->GetByName(_name);
    }

    //////////////////////////////////////////////////
    template <class T>
    SubMeshPtr BaseMesh<T>::GetSubMeshByIndex(unsigned int _index) const
    {
      return this->GetSubMeshes()->GetByIndex(_index);
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseMesh<T>::SetMaterial(MaterialPtr _material, bool unique)
    {
      _material = (unique) ? _material->Clone() : _material;
      unsigned int count = this->GetSubMeshCount();

      for (unsigned int i = 0; i < count; ++i)
      {
        SubMeshPtr subMesh = this->GetSubMeshByIndex(i);
        subMesh->SetMaterial(_material);
      }
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseMesh<T>::PreRender()
    {
      unsigned int count = this->GetSubMeshCount();

      for (unsigned int i = 0; i < count; ++i)
      {
        SubMeshPtr subMesh =  this->GetSubMeshByIndex(i);
        subMesh->PreRender();
      }

      T::PreRender();
    }

    //////////////////////////////////////////////////
    template <class T>
    BaseSubMesh<T>::BaseSubMesh()
    {
    }

    //////////////////////////////////////////////////
    template <class T>
    BaseSubMesh<T>::~BaseSubMesh()
    {
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseSubMesh<T>::PreRender()
    {
      this->GetMaterial()->PreRender();
      T::PreRender();
    }
  }
}
#endif
