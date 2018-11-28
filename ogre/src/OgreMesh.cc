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

#include <ignition/common/Console.hh>

#include "ignition/rendering/ogre/OgreMesh.hh"
#include "ignition/rendering/ogre/OgreIncludes.hh"
#include "ignition/rendering/ogre/OgreMaterial.hh"
#include "ignition/rendering/ogre/OgreStorage.hh"
#include "ignition/rendering/ogre/OgreRTShaderSystem.hh"

using namespace ignition;
using namespace rendering;

//////////////////////////////////////////////////
OgreMesh::OgreMesh()
{
}

//////////////////////////////////////////////////
OgreMesh::~OgreMesh()
{
  this->Destroy();
}

//////////////////////////////////////////////////
void OgreMesh::Destroy()
{
  if (!this->ogreEntity)
    return;

  BaseMesh::Destroy();

  auto ogreScene = std::dynamic_pointer_cast<OgreScene>(this->Scene());

  ogreScene->OgreSceneManager()->destroyEntity(this->ogreEntity);
  this->ogreEntity = nullptr;
}

//////////////////////////////////////////////////
Ogre::MovableObject *OgreMesh::OgreObject() const
{
  return this->ogreEntity;
}

//////////////////////////////////////////////////
SubMeshStorePtr OgreMesh::SubMeshes() const
{
  return this->subMeshes;
}

//////////////////////////////////////////////////
OgreSubMesh::OgreSubMesh()
{
}

//////////////////////////////////////////////////
OgreSubMesh::~OgreSubMesh()
{
  this->Destroy();
}

//////////////////////////////////////////////////
Ogre::SubEntity *OgreSubMesh::OgreSubEntity() const
{
  return this->ogreSubEntity;
}

//////////////////////////////////////////////////
void OgreSubMesh::Destroy()
{
  BaseSubMesh::Destroy();
  OgreRTShaderSystem::Instance()->DetachEntity(this);
}

//////////////////////////////////////////////////
void OgreSubMesh::SetMaterialImpl(MaterialPtr _material)
{
  OgreMaterialPtr derived =
      std::dynamic_pointer_cast<OgreMaterial>(_material);

  if (!derived)
  {
    ignerr << "Cannot assign material created by another render-engine"
        << std::endl;

    return;
  }

  std::string materialName = derived->Name();
  Ogre::MaterialPtr ogreMaterial = derived->Material();
  this->ogreSubEntity->setMaterialName(materialName);
}

//////////////////////////////////////////////////
void OgreSubMesh::Init()
{
  BaseSubMesh::Init();
  OgreRTShaderSystem::Instance()->AttachEntity(this);
}

//////////////////////////////////////////////////
