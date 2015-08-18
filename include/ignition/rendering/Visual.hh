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
#ifndef _IGNITION_RENDERING_VISUAL_HH_
#define _IGNITION_RENDERING_VISUAL_HH_

#include "ignition/rendering/Node.hh"

namespace ignition
{
  namespace rendering
  {
    class IGNITION_VISIBLE Visual :
      public virtual Node
    {
      public: virtual ~Visual() { }

      public: virtual unsigned int GetChildCount() const = 0;

      public: virtual bool HasChild(ConstNodePtr _child) const = 0;

      public: virtual bool HasChildId(unsigned int _id) const = 0;

      public: virtual bool HasChildName(const std::string &_name) const = 0;

      public: virtual NodePtr GetChildById(unsigned int _id) const = 0;

      public: virtual NodePtr GetChildByName(
                  const std::string &_name) const = 0;

      public: virtual NodePtr GetChildByIndex(unsigned int _index) const = 0;

      public: virtual void AddChild(NodePtr _child) = 0;

      public: virtual NodePtr RemoveChild(NodePtr _child) = 0;

      public: virtual NodePtr RemoveChildById(unsigned int _id) = 0;

      public: virtual NodePtr RemoveChildByName(const std::string &_name) = 0;

      public: virtual NodePtr RemoveChildByIndex(unsigned int _index) = 0;

      public: virtual void RemoveChildren() = 0;

      public: virtual unsigned int GetGeometryCount() const = 0;

      public: virtual bool HasGeometry(ConstGeometryPtr _geometry) const = 0;

      public: virtual GeometryPtr GetGeometryByIndex(
                  unsigned int _index) const = 0;

      public: virtual void AddGeometry(GeometryPtr _geometry) = 0;

      public: virtual GeometryPtr RemoveGeometry(GeometryPtr _geometry) = 0;

      public: virtual GeometryPtr RemoveGeometryByIndex(
                  unsigned int _index) = 0;

      public: virtual void RemoveGeometries() = 0;

      public: virtual void SetMaterial(const std::string &_name,
                  bool unique = true) = 0;

      public: virtual void SetMaterial(MaterialPtr _material,
                  bool unique = true) = 0;

      public: virtual void SetChildMaterial(MaterialPtr _material,
                  bool unique = true) = 0;

      public: virtual void SetGeometryMaterial(MaterialPtr _material,
                  bool unique = true) = 0;

      public: virtual math::Vector3d GetLocalScale() const = 0;

      public: virtual void SetLocalScale(double _scale) = 0;

      public: virtual void SetLocalScale(double _x, double _y, double _z) = 0;

      public: virtual void SetLocalScale(const math::Vector3d &_scale) = 0;

      public: virtual math::Vector3d GetWorldScale() const = 0;

      public: virtual void SetWorldScale(double _scale) = 0;

      public: virtual void SetWorldScale(double _x, double _y, double _z) = 0;

      public: virtual void SetWorldScale(const math::Vector3d &_scale) = 0;

      public: virtual void Scale(double _scale) = 0;

      public: virtual void Scale(double _x, double _y, double _z) = 0;

      public: virtual void Scale(const math::Vector3d &_scale) = 0;

      public: virtual bool GetInheritScale() const = 0;

      public: virtual void SetInheritScale(bool _inherit) = 0;
    };
  }
}
#endif
