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
#ifndef IGNITION_RENDERING_BASE_BASEMESH_HH_
#define IGNITION_RENDERING_BASE_BASEMESH_HH_

#include <string>
#include "ignition/rendering/Mesh.hh"
#include "ignition/rendering/Storage.hh"
#include "ignition/rendering/base/BaseObject.hh"

namespace ignition
{
  namespace rendering
  {
    inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {
    //
    //////////////////////////////////////////////////
    template <class T>
    class BaseMesh :
      public virtual Mesh,
      public virtual T
    {
      protected: BaseMesh();

      public: virtual ~BaseMesh();

      public: virtual unsigned int SubMeshCount() const;

      public: virtual bool HasSubMesh(ConstSubMeshPtr _subMesh) const;

      public: virtual bool HasSubMeshName(const std::string &_name) const;

      public: virtual SubMeshPtr SubMeshByName(
                  const std::string &_name) const;

      public: virtual SubMeshPtr SubMeshByIndex(unsigned int _index) const;

      // Documentation inherited.
      public: virtual MaterialPtr Material() const;

      public: virtual void SetMaterial(const std::string &_name,
                  bool _unique = true);

      public: virtual void SetMaterial(MaterialPtr _material,
                  bool _unique = true);

      public: virtual void PreRender();

      // Documentation inherited
      public: virtual void Destroy() override;

      protected: virtual SubMeshStorePtr SubMeshes() const = 0;
    };

    //////////////////////////////////////////////////
    template <class T>
    class BaseSubMesh :
      public virtual SubMesh,
      public virtual T
    {
      protected: BaseSubMesh();

      public: virtual ~BaseSubMesh();

      public: virtual MaterialPtr Material() const = 0;

      public: virtual void SetMaterial(const std::string &_name,
                  bool _unique = true);

      public: virtual void SetMaterial(MaterialPtr _material,
                  bool _unique = true) = 0;

      public: virtual void PreRender();
    };

    //////////////////////////////////////////////////
    // BaseMesh
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
    unsigned int BaseMesh<T>::SubMeshCount() const
    {
      return this->SubMeshes()->Size();
    }

    //////////////////////////////////////////////////
    template <class T>
    bool BaseMesh<T>::HasSubMesh(ConstSubMeshPtr _subMesh) const
    {
      return this->SubMeshes()->Contains(_subMesh);
    }

    //////////////////////////////////////////////////
    template <class T>
    bool BaseMesh<T>::HasSubMeshName(const std::string &_name) const
    {
      return this->SubMeshes()->ContainsName(_name);
    }

    //////////////////////////////////////////////////
    template <class T>
    SubMeshPtr BaseMesh<T>::SubMeshByName(const std::string &_name) const
    {
      return this->SubMeshes()->GetByName(_name);
    }

    //////////////////////////////////////////////////
    template <class T>
    SubMeshPtr BaseMesh<T>::SubMeshByIndex(unsigned int _index) const
    {
      return this->SubMeshes()->GetByIndex(_index);
    }

    //////////////////////////////////////////////////
    template <class T>
    MaterialPtr BaseMesh<T>::Material() const
    {
      unsigned int count = this->SubMeshCount();
      return (count > 0) ? this->SubMeshByIndex(0)->Material() :
          MaterialPtr();
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseMesh<T>::SetMaterial(const std::string &_name, bool _unique)
    {
      MaterialPtr material = this->Scene()->Material(_name);
      if (material) this->SetMaterial(material, _unique);
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseMesh<T>::SetMaterial(MaterialPtr _material, bool _unique)
    {
      unsigned int count = this->SubMeshCount();
      _material = (_unique && count > 0) ? _material->Clone() : _material;

      for (unsigned int i = 0; i < count; ++i)
      {
        SubMeshPtr subMesh = this->SubMeshByIndex(i);
        subMesh->SetMaterial(_material, false);
      }
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseMesh<T>::PreRender()
    {
      unsigned int count = this->SubMeshCount();

      for (unsigned int i = 0; i < count; ++i)
      {
        SubMeshPtr subMesh =  this->SubMeshByIndex(i);
        subMesh->PreRender();
      }

      T::PreRender();
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseMesh<T>::Destroy()
    {
      T::Destroy();
      this->SubMeshes()->DestroyAll();
    }

    //////////////////////////////////////////////////
    // BaseSubMesh
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
    void BaseSubMesh<T>::SetMaterial(const std::string &_name, bool _unique)
    {
      MaterialPtr material = this->Scene()->Material(_name);
      if (material) this->SetMaterial(material, _unique);
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseSubMesh<T>::PreRender()
    {
      T::PreRender();
      if (this->Material())
        this->Material()->PreRender();
    }
    }
  }
}
#endif
