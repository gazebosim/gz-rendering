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
#ifndef IGNITION_RENDERING_OGRE_OGREMESH_HH_
#define IGNITION_RENDERING_OGRE_OGREMESH_HH_

#include <string>
#include <vector>
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
    inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {
    //
    class IGNITION_RENDERING_OGRE_VISIBLE OgreMesh :
      public BaseMesh<OgreGeometry>
    {
      typedef std::vector<std::string> NameList;

      protected: OgreMesh();

      public: virtual ~OgreMesh();

      /// \brief Get the master movable object, which is equivalent to the
      /// first submesh.
      /// \return A pointer to the master movable object, which is the first
      /// submesh.
      public: virtual Ogre::MovableObject *OgreObject() const override final;

      public: virtual SubMeshStorePtr SubMeshes() const override final;

      protected: OgreSubMeshStorePtr subMeshes;

      protected: Ogre::MovableObject *ogreEntity = nullptr;

      private: friend class OgreScene;

      private: friend class OgreMeshFactory;
    };

    class IGNITION_RENDERING_OGRE_VISIBLE OgreSubMesh :
      public BaseSubMesh<OgreObject>
    {
      protected: OgreSubMesh();

      public: virtual ~OgreSubMesh();

      public: virtual MaterialPtr Material() const;

      public: virtual void SetMaterial(MaterialPtr _material,
                  bool _unique = true);

      public: Ogre::SubEntity *OgreSubEntity() const;

      public: Ogre::MovableObject *OgreObject() const;

      public: virtual void Destroy();

      protected: virtual void SetMaterialImpl(OgreMaterialPtr _material);

      protected: virtual void Init();

      protected: OgreMaterialPtr material;

      protected: Ogre::SubEntity *ogreSubEntity = nullptr;

      protected: Ogre::MovableObject *ogreObject = nullptr;

      private: friend class OgreScene;

      private: friend class OgreSubMeshStoreFactory;
    };
    }
  }
}
#endif
