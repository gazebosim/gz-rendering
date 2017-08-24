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
#ifndef IGNITION_RENDERING_BASE_BASEVISUAL_HH_
#define IGNITION_RENDERING_BASE_BASEVISUAL_HH_

#include <string>
#include "ignition/rendering/Visual.hh"
#include "ignition/rendering/Storage.hh"

#include "ignition/rendering/RenderEngine.hh"

namespace ignition
{
  namespace rendering
  {
    template <class T>
    class IGNITION_VISIBLE BaseVisual :
      public virtual Visual,
      public virtual T
    {
      protected: BaseVisual();

      public: virtual ~BaseVisual();

      public: virtual unsigned int ChildCount() const;

      public: virtual math::Pose3d LocalPose() const;

      public: virtual void SetLocalPose(const math::Pose3d &_pose);

      public: virtual bool HasChild(ConstNodePtr _child) const;

      public: virtual bool HasChildId(unsigned int _id) const;

      public: virtual bool HasChildName(const std::string &_name) const;

      public: virtual NodePtr ChildById(unsigned int _id) const;

      public: virtual NodePtr ChildByName(const std::string &_name) const;

      public: virtual NodePtr ChildByIndex(unsigned int _index) const;

      public: virtual void AddChild(NodePtr _child);

      public: virtual NodePtr RemoveChild(NodePtr _child);

      public: virtual NodePtr RemoveChildById(unsigned int _id);

      public: virtual NodePtr RemoveChildByName(const std::string &_name);

      public: virtual NodePtr RemoveChildByIndex(unsigned int _index);

      public: virtual void RemoveChildren();

      public: virtual unsigned int GeometryCount() const;

      public: virtual bool HasGeometry(ConstGeometryPtr _geometry) const;

      public: virtual GeometryPtr GeometryByIndex(unsigned int _index) const;

      public: virtual void AddGeometry(GeometryPtr _geometry);

      public: virtual GeometryPtr RemoveGeometry(GeometryPtr _geometry);

      public: virtual GeometryPtr RemoveGeometryByIndex(unsigned int _index);

      public: virtual void RemoveGeometries();

      public: virtual void SetMaterial(const std::string &_name,
                  bool _unique = true);

      public: virtual void SetMaterial(MaterialPtr _material,
                  bool _unique = true);

      public: virtual void SetChildMaterial(MaterialPtr _material,
                  bool _unique = true);

      public: virtual void SetGeometryMaterial(MaterialPtr _material,
                  bool _unique = true);

      public: virtual math::Vector3d LocalScale() const = 0;

      public: virtual void SetLocalScale(double _scale);

      public: virtual void SetLocalScale(double _x, double _y, double _z);

      public: virtual void SetLocalScale(const math::Vector3d &_scale);

      public: virtual math::Vector3d WorldScale() const;

      public: virtual void SetWorldScale(double _scale);

      public: virtual void SetWorldScale(double _x, double _y, double _z);

      public: virtual void SetWorldScale(const math::Vector3d &_scale);

      public: virtual void Scale(double _scale);

      public: virtual void Scale(double _x, double _y, double _z);

      public: virtual void Scale(const math::Vector3d &_scale);

      public: virtual bool InheritScale() const = 0;

      public: virtual void PreRender();

      public: virtual void Destroy();

      protected: virtual void PreRenderChildren();

      protected: virtual void PreRenderGeometries();

      protected: virtual NodeStorePtr Children() const = 0;

      protected: virtual GeometryStorePtr Geometries() const = 0;

      protected: virtual bool AttachChild(NodePtr _child) = 0;

      protected: virtual bool DetachChild(NodePtr _child) = 0;

      protected: virtual bool AttachGeometry(GeometryPtr _geometry) = 0;

      protected: virtual bool DetachGeometry(GeometryPtr _geometry) = 0;

      protected: virtual void SetLocalScaleImpl(
                     const math::Vector3d &_scale) = 0;
    };

    //////////////////////////////////////////////////
    template <class T>
    BaseVisual<T>::BaseVisual()
    {
    }

    //////////////////////////////////////////////////
    template <class T>
    BaseVisual<T>::~BaseVisual()
    {
    }

    //////////////////////////////////////////////////
    template <class T>
    math::Pose3d BaseVisual<T>::LocalPose() const
    {
      math::Pose3d rawPose = this->RawLocalPose();
      math::Vector3d scale = this->LocalScale();
      rawPose.Pos() += rawPose.Rot() * (scale * this->origin);
      return rawPose;
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseVisual<T>::SetLocalPose(const math::Pose3d &_pose)
    {
      math::Pose3d rawPose = _pose;
      math::Vector3d scale = this->LocalScale();
      rawPose.Pos() -= rawPose.Rot() * (scale * this->origin);
      this->SetRawLocalPose(rawPose);
    }

    //////////////////////////////////////////////////
    template <class T>
    unsigned int BaseVisual<T>::ChildCount() const
    {
      return this->Children()->Size();
    }

    //////////////////////////////////////////////////
    template <class T>
    bool BaseVisual<T>::HasChild(ConstNodePtr _child) const
    {
      return this->Children()->Contains(_child);
    }

    //////////////////////////////////////////////////
    template <class T>
    bool BaseVisual<T>::HasChildId(unsigned int _id) const
    {
      return this->Children()->ContainsId(_id);
    }

    //////////////////////////////////////////////////
    template <class T>
    bool BaseVisual<T>::HasChildName(const std::string &_name) const
    {
      return this->Children()->ContainsName(_name);
    }

    //////////////////////////////////////////////////
    template <class T>
    NodePtr BaseVisual<T>::ChildById(unsigned int _id) const
    {
      return this->Children()->GetById(_id);
    }

    //////////////////////////////////////////////////
    template <class T>
    NodePtr BaseVisual<T>::ChildByName(const std::string &_name) const
    {
      return this->Children()->GetByName(_name);
    }

    //////////////////////////////////////////////////
    template <class T>
    NodePtr BaseVisual<T>::ChildByIndex(unsigned int _index) const
    {
      return this->Children()->GetByIndex(_index);
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseVisual<T>::AddChild(NodePtr _child)
    {
      if (this->AttachChild(_child))
      {
        this->Children()->Add(_child);
      }
    }

    //////////////////////////////////////////////////
    template <class T>
    NodePtr BaseVisual<T>::RemoveChild(NodePtr _child)
    {
      NodePtr child = this->Children()->Remove(_child);
      if (child) this->DetachChild(child);
      return child;
    }

    //////////////////////////////////////////////////
    template <class T>
    NodePtr BaseVisual<T>::RemoveChildById(unsigned int _id)
    {
      NodePtr child = this->Children()->RemoveById(_id);
      if (child) this->DetachChild(child);
      return child;
    }

    //////////////////////////////////////////////////
    template <class T>
    NodePtr BaseVisual<T>::RemoveChildByName(const std::string &_name)
    {
      NodePtr child = this->Children()->RemoveByName(_name);
      if (child) this->DetachChild(child);
      return child;
    }

    //////////////////////////////////////////////////
    template <class T>
    NodePtr BaseVisual<T>::RemoveChildByIndex(unsigned int _index)
    {
      NodePtr child = this->Children()->RemoveByIndex(_index);
      if (child) this->DetachChild(child);
      return child;
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseVisual<T>::RemoveChildren()
    {
      unsigned int count = this->Children()->Size();

      for (unsigned int i = 0; i < count; ++i)
      {
        this->RemoveChildByIndex(i);
      }
    }

    //////////////////////////////////////////////////
    template <class T>
    unsigned int BaseVisual<T>::GeometryCount() const
    {
      return this->Geometries()->Size();
    }

    //////////////////////////////////////////////////
    template <class T>
    bool BaseVisual<T>::HasGeometry(ConstGeometryPtr _geometry) const
    {
      return this->Geometries()->Contains(_geometry);
    }

    //////////////////////////////////////////////////
    template <class T>
    GeometryPtr BaseVisual<T>::GeometryByIndex(unsigned int _index) const
    {
      return this->Geometries()->GetByIndex(_index);
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseVisual<T>::AddGeometry(GeometryPtr _geometry)
    {
      if (this->AttachGeometry(_geometry))
      {
        this->Geometries()->Add(_geometry);
      }
    }

    //////////////////////////////////////////////////
    template <class T>
    GeometryPtr BaseVisual<T>::RemoveGeometry(GeometryPtr _geometry)
    {
      return this->Geometries()->Remove(_geometry);
    }

    //////////////////////////////////////////////////
    template <class T>
    GeometryPtr BaseVisual<T>::RemoveGeometryByIndex(unsigned int _index)
    {
      return this->Geometries()->RemoveByIndex(_index);
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseVisual<T>::RemoveGeometries()
    {
      this->Geometries()->RemoveAll();
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseVisual<T>::SetMaterial(const std::string &_name, bool _unique)
    {
      MaterialPtr material = this->Scene()->Material(_name);
      if (material) this->SetMaterial(material, _unique);
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseVisual<T>::SetMaterial(MaterialPtr _material, bool _unique)
    {
      _material = (_unique) ? _material->Clone() : _material;
      this->SetChildMaterial(_material, false);
      this->SetGeometryMaterial(_material, false);
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseVisual<T>::SetChildMaterial(MaterialPtr _material, bool _unique)
    {
      unsigned int count = this->ChildCount();
      _material = (_unique && count > 0) ? _material->Clone() : _material;

      for (unsigned int i = 0; i < count; ++i)
      {
        NodePtr child = this->ChildByIndex(i);
        VisualPtr visual = std::dynamic_pointer_cast<Visual>(child);
        if (visual) visual->SetMaterial(_material, false);
      }
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseVisual<T>::SetGeometryMaterial(MaterialPtr _material, bool _unique)
    {
      unsigned int count = this->GeometryCount();
      _material = (_unique && count > 0) ? _material->Clone() : _material;

      for (unsigned int i = 0; i < count; ++i)
      {
        GeometryPtr geometry = this->GeometryByIndex(i);
        geometry->SetMaterial(_material, false);
      }
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseVisual<T>::SetLocalScale(double _scale)
    {
      this->SetLocalScale(math::Vector3d(_scale, _scale, _scale));
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseVisual<T>::SetLocalScale(double _x, double _y, double _z)
    {
      this->SetLocalScale(math::Vector3d(_x, _y, _z));
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseVisual<T>::SetLocalScale(const math::Vector3d &_scale)
    {
      math::Pose3d rawPose = this->LocalPose();
      this->SetLocalScaleImpl(_scale);
      this->SetLocalPose(rawPose);
    }

    //////////////////////////////////////////////////
    template <class T>
    math::Vector3d BaseVisual<T>::WorldScale() const
    {
      math::Vector3d scale = this->LocalScale();

      if (!this->InheritScale() || !this->HasParent())
      {
        return scale;
      }

      return scale * this->Parent()->WorldScale();
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseVisual<T>::SetWorldScale(double _scale)
    {
      this->SetWorldScale(math::Vector3d(_scale, _scale, _scale));
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseVisual<T>::SetWorldScale(double _x, double _y, double _z)
    {
      this->SetWorldScale(math::Vector3d(_x, _y, _z));
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseVisual<T>::SetWorldScale(const math::Vector3d &_scale)
    {
      math::Vector3d currentScale = this->WorldScale();
      this->SetLocalScale(_scale / currentScale);
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseVisual<T>::Scale(double _scale)
    {
      this->Scale(math::Vector3d(_scale, _scale, _scale));
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseVisual<T>::Scale(double _x, double _y, double _z)
    {
      this->Scale(math::Vector3d(_x, _y, _z));
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseVisual<T>::Scale(const math::Vector3d &_scale)
    {
      this->SetLocalScale(_scale * this->LocalScale());
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseVisual<T>::PreRender()
    {
      T::PreRender();
      this->PreRenderChildren();
      this->PreRenderGeometries();
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseVisual<T>::Destroy()
    {
      T::Destroy();
      this->Geometries()->DestroyAll();
      this->Children()->RemoveAll();
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseVisual<T>::PreRenderChildren()
    {
      unsigned int count = this->ChildCount();

      for (unsigned int i = 0; i < count; ++i)
      {
        NodePtr child = this->ChildByIndex(i);
        child->PreRender();
      }
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseVisual<T>::PreRenderGeometries()
    {
      unsigned int count = this->GeometryCount();

      for (unsigned int i = 0; i < count; ++i)
      {
        GeometryPtr geometry = this->GeometryByIndex(i);
        geometry->PreRender();
      }
    }

  }
}
#endif
