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
#ifndef _IGNITION_RENDERING_MESH_HH_
#define _IGNITION_RENDERING_MESH_HH_

#include <string>
#include "ignition/rendering/Geometry.hh"
#include "ignition/rendering/Object.hh"

namespace ignition
{
  namespace rendering
  {
    class IGNITION_VISIBLE Mesh :
      public virtual Geometry
    {
      public: virtual ~Mesh() { }

      public: virtual unsigned int GetSubMeshCount() const = 0;

      public: virtual bool HasSubMesh(ConstSubMeshPtr _subMesh) const = 0;

      public: virtual bool HasSubMeshName(const std::string &_name) const = 0;

      public: virtual SubMeshPtr GetSubMeshByName(
                  const std::string &_name) const = 0;

      public: virtual SubMeshPtr GetSubMeshByIndex(
                  unsigned int _index) const = 0;
    };

    class IGNITION_VISIBLE SubMesh :
      public virtual Object
    {
      public: virtual ~SubMesh() { }

      public: virtual MaterialPtr GetMaterial() const = 0;

      public: virtual void SetMaterial(const std::string &_name,
                  bool unique = true) = 0;

      public: virtual void SetMaterial(MaterialPtr _material,
                  bool unique = true) = 0;
    };
  }
}
#endif
