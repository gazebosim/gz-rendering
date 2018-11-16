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

// Note this include is placed in the src file because
// otherwise ogre produces compile errors
#include <Hlms/Pbs/OgreHlmsPbsDatablock.h>

#include <ignition/common/Console.hh>

#include "ignition/rendering/ogre2/Ogre2Mesh.hh"
#include "ignition/rendering/ogre2/Ogre2Includes.hh"
#include "ignition/rendering/ogre2/Ogre2Material.hh"
#include "ignition/rendering/ogre2/Ogre2Storage.hh"

using namespace ignition;
using namespace rendering;

//////////////////////////////////////////////////
Ogre2Mesh::Ogre2Mesh()
{
}

//////////////////////////////////////////////////
Ogre2Mesh::~Ogre2Mesh()
{
}

//////////////////////////////////////////////////
Ogre::MovableObject *Ogre2Mesh::OgreObject() const
{
  return this->ogreItem;
}

//////////////////////////////////////////////////
SubMeshStorePtr Ogre2Mesh::SubMeshes() const
{
  return this->subMeshes;
}

//////////////////////////////////////////////////
Ogre2SubMesh::Ogre2SubMesh()
{
}

//////////////////////////////////////////////////
Ogre2SubMesh::~Ogre2SubMesh()
{
  this->Destroy();
}

//////////////////////////////////////////////////
MaterialPtr Ogre2SubMesh::Material() const
{
  return this->material;
}

//////////////////////////////////////////////////
void Ogre2SubMesh::SetMaterial(MaterialPtr _material, bool _unique)
{
  _material = (_unique) ? _material->Clone() : _material;

  Ogre2MaterialPtr derived =
      std::dynamic_pointer_cast<Ogre2Material>(_material);

  if (!derived)
  {
    ignerr << "Cannot assign material created by another render-engine"
        << std::endl;

    return;
  }

  this->SetMaterialImpl(derived);
}

//////////////////////////////////////////////////
Ogre::SubItem *Ogre2SubMesh::Ogre2SubItem() const
{
  return this->ogreSubItem;
}

//////////////////////////////////////////////////
void Ogre2SubMesh::Destroy()
{
}

//////////////////////////////////////////////////
void Ogre2SubMesh::SetMaterialImpl(Ogre2MaterialPtr _material)
{
  this->material = _material;
  this->ogreSubItem->setDatablock(
      static_cast<Ogre::HlmsPbsDatablock *>(this->material->Datablock()));
}

//////////////////////////////////////////////////
void Ogre2SubMesh::Init()
{
  BaseSubMesh::Init();
}

//////////////////////////////////////////////////
