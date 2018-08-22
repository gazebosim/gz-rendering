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
#ifndef IGNITION_RENDERING_OGRE_OGREMESHFACTORY_HH_
#define IGNITION_RENDERING_OGRE_OGREMESHFACTORY_HH_

#include <string>
#include <vector>

#include "ignition/rendering/MeshDescriptor.hh"
#include "ignition/rendering/ogre/OgreRenderTypes.hh"
#include "ignition/rendering/ogre/Export.hh"

namespace Ogre
{
  class Entity;
  class MovableObject;
}

namespace ignition
{
  namespace rendering
  {
    inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {
    //
    class IGNITION_RENDERING_OGRE_VISIBLE OgreMeshFactory
    {
      public: explicit OgreMeshFactory(OgreScenePtr _scene);

      public: virtual ~OgreMeshFactory();

      public: virtual OgreMeshPtr Create(const MeshDescriptor &_desc);

      public: static std::string MeshName(const MeshDescriptor &_desc);

      private: virtual Ogre::MovableObject *OgreEntity(
                     const MeshDescriptor &_desc);

      private: virtual bool Load(const MeshDescriptor &_desc);

      private: virtual bool IsLoaded(const MeshDescriptor &_desc);

      private: virtual bool LoadImpl(const MeshDescriptor &_desc);

      private: virtual bool Validate(const MeshDescriptor &_desc);

      private: OgreScenePtr scene;
    };

    class IGNITION_RENDERING_OGRE_VISIBLE OgreSubMeshStoreFactory
    {
      typedef std::vector<std::string> NameList;

      public: OgreSubMeshStoreFactory(OgreScenePtr _scene,
                  const MeshDescriptor &_desc, Ogre::MovableObject *_entity);

      public: virtual ~OgreSubMeshStoreFactory();

      public: virtual OgreSubMeshStorePtr Create();

      private: virtual OgreSubMeshPtr CreateSubMesh(unsigned int _index);

      private: virtual void CreateNameList();

      private: virtual void PopulateDefaultNames();

      private: virtual void PopulateGivenNames();

      private: OgreScenePtr scene;

      private: const MeshDescriptor &desc;

      private: Ogre::MovableObject *ogreEntity;

      private: NameList names;
    };
    }
  }
}
#endif
