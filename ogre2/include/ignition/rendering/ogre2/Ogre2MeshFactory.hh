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
#ifndef IGNITION_RENDERING_OGRE2_OGRE2MESHFACTORY_HH_
#define IGNITION_RENDERING_OGRE2_OGRE2MESHFACTORY_HH_

#include <string>
#include <vector>

#include "ignition/rendering/MeshDescriptor.hh"
#include "ignition/rendering/ogre2/Ogre2RenderTypes.hh"
#include "ignition/rendering/ogre2/Export.hh"

namespace Ogre
{
  namespace v1
  {
    class Entity;
  }
}

namespace ignition
{
  namespace rendering
  {
    class IGNITION_RENDERING_OGRE2_VISIBLE Ogre2MeshFactory
    {
      public: explicit Ogre2MeshFactory(Ogre2ScenePtr _scene);

      public: virtual ~Ogre2MeshFactory();

      public: virtual Ogre2MeshPtr Create(const MeshDescriptor &_desc);

//      protected: virtual Ogre::v1::Entity *OgreEntity(
//                     const MeshDescriptor &_desc);

      protected: virtual Ogre::Item *OgreItem(
                     const MeshDescriptor &_desc);

      protected: virtual bool Load(const MeshDescriptor &_desc);

      protected: virtual bool IsLoaded(const MeshDescriptor &_desc);

      protected: virtual bool LoadImpl(const MeshDescriptor &_desc);

      protected: virtual std::string MeshName(const MeshDescriptor &_desc);

      protected: virtual bool Validate(const MeshDescriptor &_desc);

      protected: Ogre2ScenePtr scene;
    };

    class IGNITION_RENDERING_OGRE2_VISIBLE Ogre2SubMeshStoreFactory
    {
      typedef std::vector<std::string> NameList;

      public: Ogre2SubMeshStoreFactory(Ogre2ScenePtr _scene,
                  Ogre::Item *_item);

      public: virtual ~Ogre2SubMeshStoreFactory();

      public: virtual Ogre2SubMeshStorePtr Create();

      protected: virtual Ogre2SubMeshPtr CreateSubMesh(unsigned int _index);

      protected: virtual void CreateNameList();

      protected: virtual void PopulateDefaultNames();

      protected: virtual void PopulateGivenNames();

      protected: Ogre2ScenePtr scene;

      protected: Ogre::Item *ogreItem = nullptr;

      protected: NameList names;
    };
  }
}
#endif
