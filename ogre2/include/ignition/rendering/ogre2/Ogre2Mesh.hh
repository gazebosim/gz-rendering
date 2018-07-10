/*
 * Copyright (C) 2018 Open Source Robotics Foundation
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
#ifndef IGNITION_RENDERING_OGRE2_OGRE2MESH_HH_
#define IGNITION_RENDERING_OGRE2_OGRE2MESH_HH_

#include <string>
#include <vector>
#include "ignition/rendering/base/BaseMesh.hh"
#include "ignition/rendering/ogre2/Ogre2Geometry.hh"
#include "ignition/rendering/ogre2/Ogre2Object.hh"
#include "ignition/rendering/ogre2/Ogre2RenderTypes.hh"

namespace Ogre
{
  namespace v1 
  {
    class Entity;
    class SubEntity;
  }
}

namespace ignition
{
  namespace rendering
  {
    class IGNITION_RENDERING_OGRE2_VISIBLE Ogre2Mesh :
      public BaseMesh<Ogre2Geometry>
    {
      typedef std::vector<std::string> NameList;

      protected: Ogre2Mesh();

      public: virtual ~Ogre2Mesh();

      public: virtual Ogre::MovableObject *OgreObject() const;

      protected: virtual SubMeshStorePtr SubMeshes() const;

      protected: Ogre2SubMeshStorePtr subMeshes;

      protected: Ogre::v1::Entity *ogreEntity = nullptr;

      private: friend class Ogre2Scene;

      private: friend class Ogre2MeshFactory;
    };

    class IGNITION_RENDERING_OGRE2_VISIBLE Ogre2SubMesh :
      public BaseSubMesh<Ogre2Object>
    {
      protected: Ogre2SubMesh();

      public: virtual ~Ogre2SubMesh();

      public: virtual MaterialPtr Material() const;

      public: virtual void SetMaterial(MaterialPtr _material,
                  bool _unique = true);

      public: virtual Ogre::v1::SubEntity *Ogre2SubEntity() const;

      public: virtual void Destroy();

      protected: virtual void SetMaterialImpl(Ogre2MaterialPtr _material);

      protected: virtual void Init();

      protected: Ogre2MaterialPtr material;

      protected: Ogre::v1::SubEntity *ogreSubEntity = nullptr;

      private: friend class Ogre2Scene;

      private: friend class Ogre2SubMeshStoreFactory;
    };
  }
}
#endif
