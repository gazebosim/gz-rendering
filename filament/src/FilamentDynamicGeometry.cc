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
#include <gz/common/Console.hh>

#include "gz/rendering/filament/FilamentDynamicGeometry.hh"
#include "gz/rendering/filament/FilamentMaterial.hh"
#include "gz/rendering/filament/FilamentScene.hh"

using namespace gz;
using namespace rendering;

//////////////////////////////////////////////////
FilamentDynamicGeometry::FilamentDynamicGeometry() = default;

//////////////////////////////////////////////////
FilamentDynamicGeometry::~FilamentDynamicGeometry() = default;

//////////////////////////////////////////////////
void FilamentDynamicGeometry::Init()
{
  FilamentGeometry::Init();
  this->lines = this->scene->CreateDynamicMesh();
  this->lines->SetOperationType(MT_LINE_LIST);
}

//////////////////////////////////////////////////
void FilamentDynamicGeometry::Destroy()
{
  FilamentGeometry::Destroy();
  if (this->lines)
  {
    this->lines->SetOwnerParent(nullptr);
    this->lines->Destroy();
    this->lines.reset();
  }
  // after the renderable: filament panics on destroying a material in use
  if (this->material && this->ownsMaterial && this->Scene())
    this->Scene()->DestroyMaterial(this->material);
  this->material.reset();
}

//////////////////////////////////////////////////
void FilamentDynamicGeometry::PreRender()
{
  FilamentGeometry::PreRender();
  if (this->lines)
    this->lines->PreRender();
}

//////////////////////////////////////////////////
::utils::Entity FilamentDynamicGeometry::Entity() const
{
  return this->lines ? this->lines->Entity() : ::utils::Entity();
}

//////////////////////////////////////////////////
void FilamentDynamicGeometry::SetLayers(uint8_t _layers)
{
  if (this->lines)
    this->lines->SetLayers(_layers);
}

//////////////////////////////////////////////////
MaterialPtr FilamentDynamicGeometry::Material() const
{
  return this->material;
}

//////////////////////////////////////////////////
void FilamentDynamicGeometry::SetMaterial(MaterialPtr _material, bool _unique)
{
  if (!_material)
  {
    gzerr << "Cannot assign null material" << std::endl;
    return;
  }
  _material = _unique ? _material->Clone() : _material;
  if (!std::dynamic_pointer_cast<FilamentMaterial>(_material))
  {
    gzerr << "Cannot assign material created by another render-engine"
          << std::endl;
    return;
  }
  _material->SetReceiveShadows(false);
  _material->SetCastShadows(false);
  _material->SetLightingEnabled(false);
  this->lines->SetLineMaterial(_material);

  MaterialPtr old = this->ownsMaterial ? this->material : nullptr;
  this->material = _material;
  this->ownsMaterial = _unique;
  if (old && old != _material)
    this->Scene()->DestroyMaterial(old);
}

//////////////////////////////////////////////////
void FilamentDynamicGeometry::SetParent(FilamentVisualPtr _parent)
{
  FilamentGeometry::SetParent(_parent);
  if (this->lines)
    this->lines->SetOwnerParent(_parent);
}
