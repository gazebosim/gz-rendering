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

#include <map>
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

      // Documentation inherited.
      public: virtual void Destroy() override;

      public: virtual bool HasSkeleton() const override;

      public: virtual std::map<std::string, math::Matrix4d>
                          SkeletonLocalTransforms() override;

      public: virtual void SetSkeletonLocalTransforms(
              std::map<std::string, math::Matrix4d> _tfs) override;

      public: virtual Ogre::MovableObject *OgreObject() const override;

      protected: virtual SubMeshStorePtr SubMeshes() const override;

      protected: OgreSubMeshStorePtr subMeshes;

      protected: Ogre::Entity *ogreEntity = nullptr;

      private: friend class OgreScene;

      private: friend class OgreMeshFactory;
    };

    class IGNITION_RENDERING_OGRE_VISIBLE OgreSubMesh :
      public BaseSubMesh<OgreObject>
    {
      protected: OgreSubMesh();

      public: virtual ~OgreSubMesh();

      public: virtual Ogre::SubEntity *OgreSubEntity() const;

      public: virtual void Destroy() override;

      // Documentation inherited
      protected: virtual void SetMaterialImpl(MaterialPtr _material) override;

      protected: virtual void Init() override;

      protected: Ogre::SubEntity *ogreSubEntity;

      private: friend class OgreScene;

      private: friend class OgreSubMeshStoreFactory;
    };
    }
  }
}
#endif
