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

#include <gz/common/Mesh.hh>
#include <gz/common/MeshManager.hh>
#include <gz/common/Profiler.hh>

#include "gz/rendering/ogre_next/OgreNextCapsule.hh"
#include "gz/rendering/ogre_next/OgreNextMaterial.hh"
#include "gz/rendering/ogre_next/OgreNextScene.hh"
#include "gz/rendering/ogre_next/OgreNextMesh.hh"
#include "gz/rendering/ogre_next/OgreNextVisual.hh"

class gz::rendering::OgreNextCapsulePrivate
{
  /// \brief Capsule materal
  public: OgreNextMaterialPtr material{nullptr};

  /// \brief Mesh Object for capsule shape
  public: OgreNextMeshPtr ogreMesh{nullptr};
};

using namespace gz;
using namespace rendering;

//////////////////////////////////////////////////
OgreNextCapsule::OgreNextCapsule()
  : dataPtr(new OgreNextCapsulePrivate)
{
}

//////////////////////////////////////////////////
OgreNextCapsule::~OgreNextCapsule() = default;

//////////////////////////////////////////////////
Ogre::MovableObject *OgreNextCapsule::OgreObject() const
{
  if (this->dataPtr->ogreMesh)
    return this->dataPtr->ogreMesh->OgreObject();
  else
    return nullptr;
}

//////////////////////////////////////////////////
void OgreNextCapsule::PreRender()
{
  GZ_PROFILE("Ogre2Capsule::PreRender");
  if (this->capsuleDirty)
  {
    this->Update();
    this->capsuleDirty = false;
  }
}

//////////////////////////////////////////////////
void OgreNextCapsule::Init()
{
  this->Update();
}

//////////////////////////////////////////////////
void OgreNextCapsule::Destroy()
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

//////////////////////////////////////////////////
void OgreNextCapsule::Update()
{
  GZ_PROFILE("Ogre2Capsule::Update");
  common::MeshManager *meshMgr = common::MeshManager::Instance();
  std::string capsuleMeshName = "capsule_mesh";
  capsuleMeshName += "_" + std::to_string(this->radius)
      + "_" + std::to_string(this->length);

  // Create new mesh if needed
  if (!meshMgr->HasMesh(capsuleMeshName))
  {
    meshMgr->CreateCapsule(capsuleMeshName, this->radius, this->length, 32, 32);
  }

  MeshDescriptor meshDescriptor;
  meshDescriptor.mesh = meshMgr->MeshByName(capsuleMeshName);
  if (meshDescriptor.mesh == nullptr)
  {
    gzerr << "Capsule mesh is unavailable in the Mesh Manager" << std::endl;
    return;
  }

  auto visual = std::dynamic_pointer_cast<OgreNextVisual>(this->Parent());

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
  this->dataPtr->ogreMesh = std::dynamic_pointer_cast<OgreNextMesh>(
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

//////////////////////////////////////////////////
void OgreNextCapsule::SetMaterial(MaterialPtr _material, bool _unique)
{
  _material = (_unique) ? _material->Clone() : _material;

  OgreNextMaterialPtr derived =
      std::dynamic_pointer_cast<OgreNextMaterial>(_material);

  if (!derived)
  {
    gzerr << "Cannot assign material created by another render-engine"
        << std::endl;

    return;
  }

  // Set material for the underlying dynamic renderable
  this->dataPtr->ogreMesh->SetMaterial(derived, false);
  this->dataPtr->material = derived;
}

//////////////////////////////////////////////////
MaterialPtr OgreNextCapsule::Material() const
{
  return this->dataPtr->material;
}
