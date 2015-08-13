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
#ifndef _IGNITION_RENDERING_BASEVISUAL_HH_
#define _IGNITION_RENDERING_BASEVISUAL_HH_

#include "ignition/rendering/Visual.hh"
#include "ignition/rendering/Storage.hh"

#include "gazebo/common/Console.hh"

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

      public: virtual unsigned int GetChildCount() const;

      public: virtual gazebo::math::Pose GetLocalPose() const;

      public: virtual void SetLocalPose(const gazebo::math::Pose &_pose);

      public: virtual bool HasChild(ConstNodePtr _child) const;

      public: virtual bool HasChildId(unsigned int _id) const;

      public: virtual bool HasChildName(const std::string &_name) const;

      public: virtual NodePtr GetChildById(unsigned int _id) const;

      public: virtual NodePtr GetChildByName(const std::string &_name) const;

      public: virtual NodePtr GetChildByIndex(unsigned int _index) const;

      public: virtual void AddChild(NodePtr _child);

      public: virtual NodePtr RemoveChild(NodePtr _child);

      public: virtual NodePtr RemoveChildById(unsigned int _id);

      public: virtual NodePtr RemoveChildByName(const std::string &_name);

      public: virtual NodePtr RemoveChildByIndex(unsigned int _index);

      public: virtual void RemoveChildren();

      public: virtual unsigned int GetGeometryCount() const;

      public: virtual bool HasGeometry(ConstGeometryPtr _geometry) const;

      public: virtual GeometryPtr GetGeometryByIndex(unsigned int _index) const;

      public: virtual void AddGeometry(GeometryPtr _geometry);

      public: virtual GeometryPtr RemoveGeometry(GeometryPtr _geometry);

      public: virtual GeometryPtr RemoveGeometryByIndex(unsigned int _index);

      public: virtual void RemoveGeometries();

      public: virtual void SetMaterial(MaterialPtr _material,
                  bool unique = true);

      public: virtual void SetChildMaterial(MaterialPtr _material,
                  bool unique = true);

      public: virtual void SetGeometryMaterial(MaterialPtr _material,
                  bool unique = true);

      public: virtual gazebo::math::Vector3 GetLocalScale() const = 0;

      public: virtual void SetLocalScale(double _scale);

      public: virtual void SetLocalScale(double _x, double _y, double _z);

      public: virtual void SetLocalScale(const gazebo::math::Vector3 &_scale);

      public: virtual gazebo::math::Vector3 GetWorldScale() const;

      public: virtual void SetWorldScale(double _scale);

      public: virtual void SetWorldScale(double _x, double _y, double _z);

      public: virtual void SetWorldScale(const gazebo::math::Vector3 &_scale);

      public: virtual void Scale(double _scale);

      public: virtual void Scale(double _x, double _y, double _z);

      public: virtual void Scale(const gazebo::math::Vector3 &_scale);

      public: virtual bool GetInheritScale() const = 0;

      public: virtual void PreRender();

      protected: virtual void PreRenderChildren();

      protected: virtual void PreRenderGeometries();

      protected: virtual NodeStorePtr GetChildren() const = 0;

      protected: virtual GeometryStorePtr GetGeometries() const = 0;

      protected: virtual bool AttachChild(NodePtr _child) = 0;

      protected: virtual bool DetachChild(NodePtr _child) = 0;

      protected: virtual bool AttachGeometry(GeometryPtr _geometry) = 0;

      protected: virtual bool DetachGeometry(GeometryPtr _geometry) = 0;

      protected: virtual void SetLocalScaleImpl(
                     const gazebo::math::Vector3 &_scale) = 0;
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
    gazebo::math::Pose BaseVisual<T>::GetLocalPose() const
    {
      gazebo::math::Pose rawPose = this->GetRawLocalPose();
      gazebo::math::Vector3 scale = this->GetLocalScale();
      rawPose.pos += rawPose.rot * (scale * this->origin);
      return rawPose;
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseVisual<T>::SetLocalPose(const gazebo::math::Pose &_pose)
    {
      gazebo::math::Pose rawPose = _pose;
      gazebo::math::Vector3 scale = this->GetLocalScale();
      rawPose.pos -= rawPose.rot * (scale * this->origin);
      this->SetRawLocalPose(rawPose);
    }

    //////////////////////////////////////////////////
    template <class T>
    unsigned int BaseVisual<T>::GetChildCount() const
    {
      return this->GetChildren()->Size();
    }

    //////////////////////////////////////////////////
    template <class T>
    bool BaseVisual<T>::HasChild(ConstNodePtr _child) const
    {
      return this->GetChildren()->Contains(_child);
    }

    //////////////////////////////////////////////////
    template <class T>
    bool BaseVisual<T>::HasChildId(unsigned int _id) const
    {
      return this->GetChildren()->ContainsId(_id);
    }

    //////////////////////////////////////////////////
    template <class T>
    bool BaseVisual<T>::HasChildName(const std::string &_name) const
    {
      return this->GetChildren()->ContainsName(_name);
    }

    //////////////////////////////////////////////////
    template <class T>
    NodePtr BaseVisual<T>::GetChildById(unsigned int _id) const
    {
      return this->GetChildren()->GetById(_id);
    }

    //////////////////////////////////////////////////
    template <class T>
    NodePtr BaseVisual<T>::GetChildByName(const std::string &_name) const
    {
      return this->GetChildren()->GetByName(_name);
    }

    //////////////////////////////////////////////////
    template <class T>
    NodePtr BaseVisual<T>::GetChildByIndex(unsigned int _index) const
    {
      return this->GetChildren()->GetByIndex(_index);
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseVisual<T>::AddChild(NodePtr _child)
    {
      if (this->AttachChild(_child))
      {
        this->GetChildren()->Add(_child);
      }
    }

    //////////////////////////////////////////////////
    template <class T>
    NodePtr BaseVisual<T>::RemoveChild(NodePtr _child)
    {
      return this->GetChildren()->Remove(_child);
    }

    //////////////////////////////////////////////////
    template <class T>
    NodePtr BaseVisual<T>::RemoveChildById(unsigned int _id)
    {
      return this->GetChildren()->RemoveById(_id);
    }

    //////////////////////////////////////////////////
    template <class T>
    NodePtr BaseVisual<T>::RemoveChildByName(const std::string &_name)
    {
      return this->GetChildren()->RemoveByName(_name);
    }

    //////////////////////////////////////////////////
    template <class T>
    NodePtr BaseVisual<T>::RemoveChildByIndex(unsigned int _index)
    {
      return this->GetChildren()->RemoveByIndex(_index);
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseVisual<T>::RemoveChildren()
    {
      this->GetChildren()->RemoveAll();
    }

    //////////////////////////////////////////////////
    template <class T>
    unsigned int BaseVisual<T>::GetGeometryCount() const
    {
      return this->GetGeometries()->Size();
    }

    //////////////////////////////////////////////////
    template <class T>
    bool BaseVisual<T>::HasGeometry(ConstGeometryPtr _geometry) const
    {
      return this->GetGeometries()->Contains(_geometry);
    }

    //////////////////////////////////////////////////
    template <class T>
    GeometryPtr BaseVisual<T>::GetGeometryByIndex(unsigned int _index) const
    {
      return this->GetGeometries()->GetByIndex(_index);
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseVisual<T>::AddGeometry(GeometryPtr _geometry)
    {
      if (this->AttachGeometry(_geometry))
      {
        this->GetGeometries()->Add(_geometry);
      }
    }

    //////////////////////////////////////////////////
    template <class T>
    GeometryPtr BaseVisual<T>::RemoveGeometry(GeometryPtr _geometry)
    {
      return this->GetGeometries()->Remove(_geometry);
    }

    //////////////////////////////////////////////////
    template <class T>
    GeometryPtr BaseVisual<T>::RemoveGeometryByIndex(unsigned int _index)
    {
      return this->GetGeometries()->RemoveByIndex(_index);
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseVisual<T>::RemoveGeometries()
    {
      this->GetGeometries()->RemoveAll();
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseVisual<T>::SetMaterial(MaterialPtr _material, bool unique)
    {
      _material = (unique) ? _material->Clone() : _material;
      this->SetChildMaterial(_material, false);
      this->SetGeometryMaterial(_material, false);
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseVisual<T>::SetChildMaterial(MaterialPtr _material, bool unique)
    {
      _material = (unique) ? _material->Clone() : _material;
      unsigned int count = this->GetChildCount();

      for (unsigned int i = 0; i < count; ++i)
      {
        NodePtr child = this->GetChildByIndex(i);
        VisualPtr visual = boost::dynamic_pointer_cast<Visual>(child);
        if (visual) visual->SetMaterial(_material);
      }
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseVisual<T>::SetGeometryMaterial(MaterialPtr _material, bool unique)
    {
      _material = (unique) ? _material->Clone() : _material;
      unsigned int count = this->GetGeometryCount();

      for (unsigned int i = 0; i < count; ++i)
      {
        GeometryPtr geometry = this->GetGeometryByIndex(i);
        geometry->SetMaterial(_material);
      }
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseVisual<T>::SetLocalScale(double _scale)
    {
      this->SetLocalScale(gazebo::math::Vector3(_scale, _scale, _scale));
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseVisual<T>::SetLocalScale(double _x, double _y, double _z)
    {
      this->SetLocalScale(gazebo::math::Vector3(_x, _y, _z));
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseVisual<T>::SetLocalScale(const gazebo::math::Vector3 &_scale)
    {
      gazebo::math::Pose rawPose = this->GetLocalPose();
      this->SetLocalScaleImpl(_scale);
      this->SetLocalPose(rawPose);
    }

    //////////////////////////////////////////////////
    template <class T>
    gazebo::math::Vector3 BaseVisual<T>::GetWorldScale() const
    {
      gazebo::math::Vector3 scale = this->GetLocalScale();

      if (!this->GetInheritScale() || !this->HasParent())
      {
        return scale;
      }

      return scale * this->GetParent()->GetWorldScale();
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseVisual<T>::SetWorldScale(double _scale)
    {
      this->SetWorldScale(gazebo::math::Vector3(_scale, _scale, _scale));
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseVisual<T>::SetWorldScale(double _x, double _y, double _z)
    {
      this->SetWorldScale(gazebo::math::Vector3(_x, _y, _z));
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseVisual<T>::SetWorldScale(const gazebo::math::Vector3 &_scale)
    {
      gazebo::math::Vector3 currentScale = this->GetWorldScale();
      this->SetLocalScale(_scale / currentScale);
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseVisual<T>::Scale(double _scale)
    {
      this->Scale(gazebo::math::Vector3(_scale, _scale, _scale));
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseVisual<T>::Scale(double _x, double _y, double _z)
    {
      this->Scale(gazebo::math::Vector3(_x, _y, _z));
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseVisual<T>::Scale(const gazebo::math::Vector3 &_scale)
    {
      this->SetLocalScale(_scale * this->GetLocalScale());
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
    void BaseVisual<T>::PreRenderChildren()
    {
      unsigned int count = this->GetChildCount();

      for (unsigned int i = 0; i < count; ++i)
      {
        NodePtr child = this->GetChildByIndex(i);
        child->PreRender();
      }
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseVisual<T>::PreRenderGeometries()
    {
      unsigned int count = this->GetGeometryCount();

      for (unsigned int i = 0; i < count; ++i)
      {
        GeometryPtr geometry = this->GetGeometryByIndex(i);
        geometry->PreRender();
      }
    }

  }
}
#endif
