/*
 * Copyright (C) 2021 Open Source Robotics Foundation
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

#include <cmath>

#include <ignition/common/Mesh.hh>
#include <ignition/common/MeshManager.hh>

#include "ignition/rendering/ogre/OgreEllipsoid.hh"
#include "ignition/rendering/ogre/OgreMaterial.hh"
#include "ignition/rendering/ogre/OgreScene.hh"
#include "ignition/rendering/ogre/OgreMesh.hh"
#include "ignition/rendering/ogre/OgreVisual.hh"

#include <ignition/math/Vector3.hh>
#include <ignition/math/Vector2.hh>

class ignition::rendering::OgreEllipsoidPrivate
{
  /// \brief Grid materal
  public: OgreMaterialPtr material;

  /// \brief Mesh Object for primitive shapes
  public: OgreMeshPtr ogreMesh = nullptr;
};

using namespace ignition;
using namespace rendering;

//////////////////////////////////////////////////
OgreEllipsoid::OgreEllipsoid()
 : dataPtr(new OgreEllipsoidPrivate)
{
}

//////////////////////////////////////////////////
OgreEllipsoid::~OgreEllipsoid()
{
  // no ops
}

//////////////////////////////////////////////////
void OgreEllipsoid::PreRender()
{
  if (this->ellipsoidDirty)
  {
    this->Create();
    this->ellipsoidDirty = false;
  }
}

//////////////////////////////////////////////////
Ogre::MovableObject *OgreEllipsoid::OgreObject() const
{
  return this->dataPtr->ogreMesh->OgreObject();;
}

//////////////////////////////////////////////////
void OgreEllipsoid::Destroy()
{
  if (!this->Scene())
    return;

  if (this->dataPtr->ogreMesh)
  {
    this->dataPtr->ogreMesh->Destroy();
    this->dataPtr->ogreMesh.reset();
  }

  if (this->dataPtr->material && this->Scene())
  {
    this->Scene()->DestroyMaterial(this->dataPtr->material);
    this->dataPtr->material.reset();
  }
}

//////////////////////////////////////////////////
void OgreEllipsoid::Init()
{
  this->Create();
}

//////////////////////////////////////////////////
void OgreEllipsoidEllipsoid()
{
  // no ops
}

//////////////////////////////////////////////////
void OgreEllipsoid::Create()
{
  common::MeshManager *meshMgr = common::MeshManager::Instance();
  std::string ellipsoidMeshName = this->Name() + "_ellipsoid_mesh"
    + "_" + std::to_string(this->radii.X())
    + "_" + std::to_string(this->radii.Y())
    + "_" + std::to_string(this->radii.Z());
  if (!meshMgr->HasMesh(ellipsoidMeshName))
  {
    meshMgr->CreateEllipsoid(ellipsoidMeshName, this->radii, 32, 32);
    MeshDescriptor meshDescriptor;
    meshDescriptor.mesh = meshMgr->MeshByName(ellipsoidMeshName);
    if (meshDescriptor.mesh != nullptr)
    {
      auto visual = std::dynamic_pointer_cast<OgreVisual>(this->Parent());
      // clear geom if needed
      if (this->dataPtr->ogreMesh)
      {
        if (visual)
        {
          visual->RemoveGeometry(
              std::dynamic_pointer_cast<Geometry>(shared_from_this()));
        }
        this->dataPtr->ogreMesh->Destroy();
      }
      this->dataPtr->ogreMesh =
        std::dynamic_pointer_cast<OgreMesh>(
          this->Scene()->CreateMesh(meshDescriptor));
      if (this->dataPtr->material != nullptr)
      {
        this->dataPtr->ogreMesh->SetMaterial(this->dataPtr->material, false);
      }
      if (visual)
      {
        visual->AddGeometry(
            std::dynamic_pointer_cast<Geometry>(shared_from_this()));
      }
    }
  }
}

//////////////////////////////////////////////////
void OgreEllipsoid::SetMaterial(MaterialPtr _material, bool _unique)
{
  _material = (_unique) ? _material->Clone() : _material;

  OgreMaterialPtr derived =
      std::dynamic_pointer_cast<OgreMaterial>(_material);

  if (!derived)
  {
    ignerr << "Cannot assign material created by another render-engine"
        << std::endl;

    return;
  }

  this->SetMaterialImpl(derived);
}

//////////////////////////////////////////////////
void OgreEllipsoid::SetMaterialImpl(OgreMaterialPtr _material)
{
  std::string materialName = _material->Name();
  Ogre::MaterialPtr ogreMaterial = _material->Material();
  this->dataPtr->ogreMesh->SetMaterial(_material, false);
  this->dataPtr->material = _material;
}

//////////////////////////////////////////////////
MaterialPtr OgreEllipsoid::Material() const
{
  return this->dataPtr->material;
}
