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
#include "ignition/rendering/base/BaseStorage.hh"

namespace ignition
{
  namespace rendering
  {
    inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {
    //
    template <class T>
    class BaseVisual :
      public virtual Visual,
      public virtual T
    {
      protected: BaseVisual();

      public: virtual ~BaseVisual();

      public: virtual math::Pose3d LocalPose() const;

      public: virtual void SetLocalPose(const math::Pose3d &_pose);

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

      // Documentation inherited.
      public: virtual MaterialPtr Material();

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

      // Documentation inherited.
      public: virtual void SetVisible(bool _visible);

      public: virtual void PreRender();

      // Documentation inherited
      public: virtual void Destroy() override;

      protected: virtual void PreRenderChildren();

      protected: virtual void PreRenderGeometries();

      protected: virtual GeometryStorePtr Geometries() const = 0;

      protected: virtual bool AttachGeometry(GeometryPtr _geometry) = 0;

      protected: virtual bool DetachGeometry(GeometryPtr _geometry) = 0;

      protected: virtual void SetLocalScaleImpl(
                     const math::Vector3d &_scale) = 0;

      /// \brief Pointer to material assigned to this visual
      protected: MaterialPtr material;
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
      if (this->DetachGeometry(_geometry))
      {
        this->Geometries()->Remove(_geometry);
      }
      return _geometry;
    }

    //////////////////////////////////////////////////
    template <class T>
    GeometryPtr BaseVisual<T>::RemoveGeometryByIndex(unsigned int _index)
    {
      return this->RemoveGeometry(this->GeometryByIndex(_index));
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseVisual<T>::RemoveGeometries()
    {
      for (unsigned int i = this->GeometryCount(); i > 0; --i)
      {
        this->RemoveGeometryByIndex(i-1);
      }
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseVisual<T>::SetMaterial(const std::string &_name, bool _unique)
    {
      MaterialPtr mat = this->Scene()->Material(_name);
      if (mat) this->SetMaterial(mat, _unique);
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseVisual<T>::SetMaterial(MaterialPtr _material, bool _unique)
    {
      _material = (_unique) ? _material->Clone() : _material;
      this->SetChildMaterial(_material, false);
      this->SetGeometryMaterial(_material, false);
      this->material = _material;
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseVisual<T>::SetChildMaterial(MaterialPtr _material, bool _unique)
    {
      unsigned int count = this->ChildCount();
      _material = (_unique && count > 0) ? _material->Clone() : _material;

      auto children_ =
          std::dynamic_pointer_cast<BaseStore<ignition::rendering::Node, T>>(
          this->Children());
      if (!children_)
      {
        ignerr << "Cast failed in BaseVisual::SetChildMaterial" << std::endl;
        return;
      }
      for (auto it = children_->Begin(); it != children_->End(); ++it)
      {
        NodePtr child = it->second;
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
    MaterialPtr BaseVisual<T>::Material()
    {
      return this->material;
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

      VisualPtr derived = std::dynamic_pointer_cast<Visual>(this->Parent());
      return scale * derived->WorldScale();
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
      this->Geometries()->DestroyAll();
      this->Children()->RemoveAll();
      this->material.reset();
      T::Destroy();
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseVisual<T>::PreRenderChildren()
    {
      auto children_ =
          std::dynamic_pointer_cast<BaseStore<ignition::rendering::Node, T>>(
          this->Children());
      if (!children_)
      {
        ignerr << "Cast failed in BaseVisual::PreRenderChildren" << std::endl;
        return;
      }
      for (auto it = children_->Begin(); it != children_->End(); ++it)
      {
        it->second->PreRender();
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

    //////////////////////////////////////////////////
    template <class T>
    void BaseVisual<T>::SetVisible(bool _visible)
    {
      ignerr << "SetVisible(" << _visible << ") not supported for "
             << "render engine: " << this->Scene()->Engine()->Name()
             << std::endl;
    }
    }
  }
}
#endif
