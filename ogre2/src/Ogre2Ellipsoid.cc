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

#include "ignition/rendering/ogre2/Ogre2Ellipsoid.hh"
#include "ignition/rendering/ogre2/Ogre2Material.hh"
#include "ignition/rendering/ogre2/Ogre2Scene.hh"
#include "ignition/rendering/ogre2/Ogre2Mesh.hh"
#include "ignition/rendering/ogre2/Ogre2Visual.hh"

#include <ignition/math/Vector3.hh>

class ignition::rendering::Ogre2EllipsoidPrivate
{
  /// \brief Ellipsoid materal
  public: Ogre2MaterialPtr material;

  /// \brief Mesh Object for ellipsoid shape
  public: Ogre2MeshPtr ogreMesh = nullptr;
};

using namespace ignition;
using namespace rendering;

//////////////////////////////////////////////////
Ogre2Ellipsoid::Ogre2Ellipsoid()
 : dataPtr(new Ogre2EllipsoidPrivate)
{
}

//////////////////////////////////////////////////
Ogre2Ellipsoid::~Ogre2Ellipsoid()
{
  // no ops
}

//////////////////////////////////////////////////
Ogre::MovableObject *Ogre2Ellipsoid::OgreObject() const
{
  return this->dataPtr->ogreMesh->OgreObject();
}

//////////////////////////////////////////////////
void Ogre2Ellipsoid::PreRender()
{
  if (this->ellipsoidDirty)
  {
    this->Create();
    this->ellipsoidDirty = false;
  }
}

//////////////////////////////////////////////////
void Ogre2Ellipsoid::Init()
{
  this->Create();
}

//////////////////////////////////////////////////
void Ogre2Ellipsoid::Destroy()
{
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

////////////////////////////////////////////////
void Ogre2Ellipsoid::Create()
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
      auto visual = std::dynamic_pointer_cast<Ogre2Visual>(this->Parent());

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
        std::dynamic_pointer_cast<Ogre2Mesh>(
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
void Ogre2Ellipsoid::SetMaterial(MaterialPtr _material, bool _unique)
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

  // Set material for the underlying dynamic renderable
  this->dataPtr->ogreMesh->SetMaterial(_material, false);
  this->SetMaterialImpl(derived);
}

//////////////////////////////////////////////////
void Ogre2Ellipsoid::SetMaterialImpl(Ogre2MaterialPtr _material)
{
  Ogre::MaterialPtr ogreMaterial = _material->Material();
  this->dataPtr->material = _material;
}

//////////////////////////////////////////////////
MaterialPtr Ogre2Ellipsoid::Material() const
{
  return this->dataPtr->material;
}
