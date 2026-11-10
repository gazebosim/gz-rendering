/*
 * Copyright (C) 2026 Open Source Robotics Foundation
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

#include <string>

#include <gz/common/Console.hh>
#include <gz/common/MeshManager.hh>

#include "gz/rendering/filament/FilamentCapsule.hh"
#include "gz/rendering/filament/FilamentMesh.hh"
#include "gz/rendering/filament/FilamentScene.hh"
#include "gz/rendering/filament/FilamentVisual.hh"

using namespace gz;
using namespace rendering;

//////////////////////////////////////////////////
FilamentCapsule::FilamentCapsule() = default;

//////////////////////////////////////////////////
FilamentCapsule::~FilamentCapsule() = default;

//////////////////////////////////////////////////
void FilamentCapsule::Init()
{
  FilamentGeometry::Init();
  this->Update();
}

//////////////////////////////////////////////////
void FilamentCapsule::Destroy()
{
  if (this->mesh)
  {
    this->mesh->Destroy();
    this->mesh.reset();
  }
  // after the mesh: filament panics on destroying a material in use
  if (this->material && this->ownsMaterial && this->Scene())
    this->Scene()->DestroyMaterial(this->material);
  this->material.reset();
  FilamentGeometry::Destroy();
}

//////////////////////////////////////////////////
void FilamentCapsule::PreRender()
{
  if (this->capsuleDirty)
  {
    this->Update();
    this->capsuleDirty = false;
  }
  if (this->mesh)
    this->mesh->PreRender();
}

//////////////////////////////////////////////////
::utils::Entity FilamentCapsule::Entity() const
{
  return this->mesh ? this->mesh->Entity() : ::utils::Entity();
}

//////////////////////////////////////////////////
void FilamentCapsule::SetLayers(uint8_t _layers)
{
  if (this->mesh)
    this->mesh->SetLayers(_layers);
}

//////////////////////////////////////////////////
MaterialPtr FilamentCapsule::Material() const
{
  return this->material;
}

//////////////////////////////////////////////////
void FilamentCapsule::SetMaterial(MaterialPtr _material, bool _unique)
{
  // the capsule owns the clone (not the inner mesh) so it survives mesh
  // rebuilds along with any edit made through Material()
  if (!_material)
    return;
  MaterialPtr old = this->ownsMaterial ? this->material : nullptr;
  this->material = _unique ? _material->Clone() : _material;
  this->ownsMaterial = _unique;
  if (this->mesh)
    this->mesh->SetMaterial(this->material, false);
  if (old && old != this->material && this->Scene())
    this->Scene()->DestroyMaterial(old);
}

//////////////////////////////////////////////////
void FilamentCapsule::SetParent(FilamentVisualPtr _parent)
{
  // sensors find visuals (labels, temperature) through the inner mesh
  FilamentGeometry::SetParent(_parent);
  if (this->mesh)
    this->mesh->SetParent(_parent);
}

//////////////////////////////////////////////////
void FilamentCapsule::Update()
{
  // same mesh name and tessellation as ogre2, so both engines share it
  common::MeshManager *meshMgr = common::MeshManager::Instance();
  const std::string name = "capsule_mesh_" + std::to_string(this->radius) +
      "_" + std::to_string(this->length);
  if (!meshMgr->HasMesh(name))
    meshMgr->CreateCapsule(name, this->radius, this->length, 32, 32);

  MeshDescriptor desc;
  desc.mesh = meshMgr->MeshByName(name);
  if (!desc.mesh)
  {
    gzerr << "Capsule mesh is unavailable in the Mesh Manager" << std::endl;
    return;
  }

  // swap the renderable under the parent visual without touching its
  // geometry store (PreRender runs while the visual iterates over it)
  FilamentVisualPtr visual = this->parent;
  auto self = std::dynamic_pointer_cast<Geometry>(this->shared_from_this());
  if (visual && this->mesh)
    visual->DetachGeometry(self);
  if (this->mesh)
    this->mesh->Destroy();

  this->mesh = std::dynamic_pointer_cast<FilamentMesh>(
      this->Scene()->CreateMesh(desc));
  if (this->mesh && this->material)
    this->mesh->SetMaterial(this->material, false);

  if (visual && this->mesh)
    visual->AttachGeometry(self);
}
