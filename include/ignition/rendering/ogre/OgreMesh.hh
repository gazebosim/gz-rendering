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
#ifndef _IGNITION_RENDERING_OGREMESH_HH_
#define _IGNITION_RENDERING_OGREMESH_HH_

#include "ignition/rendering/base/BaseMesh.hh"
#include "ignition/rendering/ogre/OgreGeometry.hh"
#include "ignition/rendering/ogre/OgreObject.hh"
#include "ignition/rendering/ogre/OgreRenderTypes.hh"

namespace Ogre
{
  class Entity;
  class SubEntity;
}

namespace ignition
{
  namespace rendering
  {
    class IGNITION_VISIBLE OgreMesh :
      public BaseMesh<OgreGeometry>
    {
      typedef std::vector<std::string> NameList;

      protected: OgreMesh();

      public: virtual ~OgreMesh();

      public: virtual Ogre::MovableObject *OgreObject() const;

      protected: virtual SubMeshStorePtr SubMeshes() const;

      protected: OgreSubMeshStorePtr subMeshes;

      protected: Ogre::Entity *ogreEntity;

      private: friend class OgreScene;

      private: friend class OgreMeshFactory;
    };

    class IGNITION_VISIBLE OgreSubMesh :
      public BaseSubMesh<OgreObject>
    {
      protected: OgreSubMesh();

      public: virtual ~OgreSubMesh();

      public: virtual MaterialPtr Material() const;

      public: virtual void SetMaterial(MaterialPtr _material,
                  bool unique = true);

      public: virtual Ogre::SubEntity *OgreSubEntity() const;

      public: virtual void Destroy();

      protected: virtual void SetMaterialImpl(OgreMaterialPtr _material);

      protected: virtual void Init();

      protected: OgreMaterialPtr material;

      protected: Ogre::SubEntity *ogreSubEntity;

      private: friend class OgreScene;

      private: friend class OgreSubMeshStoreFactory;
    };
  }
}
#endif
