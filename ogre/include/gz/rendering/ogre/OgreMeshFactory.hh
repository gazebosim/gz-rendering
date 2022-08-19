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

      protected: virtual Ogre::Entity *OgreEntity(
                     const MeshDescriptor &_desc);

      protected: virtual bool Load(const MeshDescriptor &_desc);

      protected: virtual bool IsLoaded(const MeshDescriptor &_desc);

      protected: virtual bool LoadImpl(const MeshDescriptor &_desc);

      protected: virtual std::string MeshName(const MeshDescriptor &_desc);

      protected: virtual bool Validate(const MeshDescriptor &_desc);

      protected: OgreScenePtr scene;
    };

    class IGNITION_RENDERING_OGRE_VISIBLE OgreSubMeshStoreFactory
    {
      typedef std::vector<std::string> NameList;

      public: OgreSubMeshStoreFactory(OgreScenePtr _scene,
                  Ogre::Entity *_entity);

      public: virtual ~OgreSubMeshStoreFactory();

      public: virtual OgreSubMeshStorePtr Create();

      protected: virtual OgreSubMeshPtr CreateSubMesh(unsigned int _index);

      protected: virtual void CreateNameList();

      protected: virtual void PopulateDefaultNames();

      protected: virtual void PopulateGivenNames();

      protected: OgreScenePtr scene;

      protected: Ogre::Entity *ogreEntity;

      protected: NameList names;
    };
    }
  }
}
#endif
