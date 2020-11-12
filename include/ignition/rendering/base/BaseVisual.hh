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

#include <map>
#include <string>

#include <ignition/math/AxisAlignedBox.hh>

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

      public: virtual math::Pose3d LocalPose() const override;

      public: virtual void SetLocalPose(const math::Pose3d &_pose) override;

      public: virtual unsigned int GeometryCount() const override;

      public: virtual bool HasGeometry(ConstGeometryPtr _geometry) const
                      override;

      public: virtual GeometryPtr GeometryByIndex(unsigned int _index) const
                      override;

      public: virtual void AddGeometry(GeometryPtr _geometry) override;

      public: virtual GeometryPtr RemoveGeometry(GeometryPtr _geometry)
                      override;

      public: virtual GeometryPtr RemoveGeometryByIndex(unsigned int _index)
                      override;

      public: virtual void RemoveGeometries() override;

      public: virtual void SetMaterial(const std::string &_name,
                  bool _unique = true) override;

      public: virtual void SetMaterial(MaterialPtr _material,
                  bool _unique = true) override;

      public: virtual void SetChildMaterial(MaterialPtr _material,
                  bool _unique = true) override;

      public: virtual void SetGeometryMaterial(MaterialPtr _material,
                  bool _unique = true) override;

      // Documentation inherited.
      public: virtual MaterialPtr Material() override;

      // Documentation inherited.
      public: virtual void SetVisible(bool _visible) override;

      // Documentation inherited.
      public: virtual void SetVisibilityFlags(uint32_t _flags) override;

      // Documentation inherited.
      public: virtual uint32_t VisibilityFlags() const override;

      // Documentation inherited.
      public: virtual void AddVisibilityFlags(uint32_t _flags) override;

      // Documentation inherited.
      public: virtual void RemoveVisibilityFlags(uint32_t _flags) override;

      // Documentation inherited.
      public: virtual void PreRender() override;

      // Documentation inherited
      public: virtual void Destroy() override;

      // Documentation inherited.
      public: virtual void SetUserData(const std::string &_key,
          Variant _value) override;

      // Documentation inherited.
      public: virtual Variant UserData(const std::string &_key) const override;

      // Documentation inherited.
      public: virtual ignition::math::AxisAlignedBox BoundingBox()
              const override;

      // Documentation inherited.
      public: virtual ignition::math::AxisAlignedBox LocalBoundingBox()
              const override;

      protected: virtual void PreRenderChildren() override;

      protected: virtual void PreRenderGeometries();

      protected: virtual GeometryStorePtr Geometries() const = 0;

      protected: virtual bool AttachGeometry(GeometryPtr _geometry) = 0;

      protected: virtual bool DetachGeometry(GeometryPtr _geometry) = 0;

      /// \brief Pointer to material assigned to this visual
      protected: MaterialPtr material;

      /// \brief A map of custom key value data
      protected: std::map<std::string, Variant> userData;

      /// \brief Visual's visibility flags
      protected: uint32_t visibilityFlags = IGN_VISIBILITY_ALL;

      /// \brief The bounding box of the visual
      protected: ignition::math::AxisAlignedBox boundingBox;
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

      if (!rawPose.IsFinite())
      {
        ignerr << "Unable to set pose of a node: "
               << "non-finite (nan, inf) values detected." << std::endl;
        return;
      }

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

    //////////////////////////////////////////////////
    template <class T>
    ignition::math::AxisAlignedBox BaseVisual<T>::LocalBoundingBox() const
    {
      ignition::math::AxisAlignedBox box;

      // Recursively loop through child visuals
      auto childNodes =
          std::dynamic_pointer_cast<BaseStore<ignition::rendering::Node, T>>(
          this->Children());
      if (!childNodes)
      {
        ignerr << "Cast failed in BaseVisual::LocalBoundingBox" << std::endl;
        return box;
      }
      for (auto it = childNodes->Begin(); it != childNodes->End(); ++it)
      {
        NodePtr child = it->second;
        VisualPtr visual = std::dynamic_pointer_cast<Visual>(child);
        if (visual)
        {
          ignition::math::AxisAlignedBox aabb = visual->LocalBoundingBox();
          if (aabb.Min().IsFinite() && aabb.Max().IsFinite())
            box.Merge(aabb);
        }
      }
      return box;
    }

    //////////////////////////////////////////////////
    template <class T>
    ignition::math::AxisAlignedBox BaseVisual<T>::BoundingBox() const
    {
      ignition::math::AxisAlignedBox box;

      // Recursively loop through child visuals
      auto childNodes =
          std::dynamic_pointer_cast<BaseStore<ignition::rendering::Node, T>>(
          this->Children());
      if (!childNodes)
      {
        ignerr << "Cast failed in BaseVisual::BoundingBox" << std::endl;
        return box;
      }
      for (auto it = childNodes->Begin(); it != childNodes->End(); ++it)
      {
        NodePtr child = it->second;
        VisualPtr visual = std::dynamic_pointer_cast<Visual>(child);
        if (visual)
          box.Merge(visual->BoundingBox());
      }
      return box;
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseVisual<T>::AddVisibilityFlags(uint32_t _flags)
    {
      this->SetVisibilityFlags(this->VisibilityFlags() | _flags);
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseVisual<T>::RemoveVisibilityFlags(uint32_t _flags)
    {
      this->SetVisibilityFlags(this->VisibilityFlags() & ~(_flags));
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseVisual<T>::SetVisibilityFlags(uint32_t _flags)
    {
      this->visibilityFlags = _flags;

      // recursively set child visuals' visibility flags
      auto childNodes =
          std::dynamic_pointer_cast<BaseStore<ignition::rendering::Node, T>>(
          this->Children());
      if (!childNodes)
      {
        ignerr << "Cast failed in BaseVisual::SetVisibiltyFlags" << std::endl;
        return;
      }
      for (auto it = childNodes->Begin(); it != childNodes->End(); ++it)
      {
        NodePtr child = it->second;
        VisualPtr visual = std::dynamic_pointer_cast<Visual>(child);
        if (visual)
          visual->SetVisibilityFlags(_flags);
      }
    }

    //////////////////////////////////////////////////
    template <class T>
    uint32_t BaseVisual<T>::VisibilityFlags() const
    {
      return this->visibilityFlags;
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseVisual<T>::SetUserData(const std::string &_key, Variant _value)
    {
      this->userData[_key] = _value;
    }

    //////////////////////////////////////////////////
    template <class T>
    Variant BaseVisual<T>::UserData(const std::string &_key) const
    {
      Variant value;
      auto it = this->userData.find(_key);
      if (it != this->userData.end())
        value = it->second;
      return value;
    }
    }
  }
}
#endif
