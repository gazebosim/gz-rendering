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

#include "gz/rendering/filament/FilamentCOMVisual.hh"
#include "gz/rendering/filament/FilamentMaterial.hh"
#include "gz/rendering/filament/FilamentScene.hh"

using namespace gz;
using namespace rendering;

//////////////////////////////////////////////////
FilamentCOMVisual::FilamentCOMVisual() = default;

//////////////////////////////////////////////////
FilamentCOMVisual::~FilamentCOMVisual() = default;

//////////////////////////////////////////////////
void FilamentCOMVisual::Init()
{
  BaseCOMVisual::Init();
}

//////////////////////////////////////////////////
void FilamentCOMVisual::PreRender()
{
  if (this->HasParent() && this->parentName.empty())
    this->parentName = this->Parent()->Name();

  if (this->dirtyCOMVisual && !this->parentName.empty())
  {
    this->parentName = this->Parent()->Name();
    this->CreateVisual();
    this->dirtyCOMVisual = false;
  }
  // the new geometry builds in this frame
  BaseCOMVisual::PreRender();
}

//////////////////////////////////////////////////
void FilamentCOMVisual::Destroy()
{
  if (this->sphereVis)
  {
    this->sphereVis->Destroy();
    this->sphereVis.reset();
  }
  if (this->crossLines)
  {
    this->crossLines->Destroy();
    this->crossLines.reset();
  }
  // after the renderable: filament panics on destroying a material in use
  if (this->material && this->ownsMaterial && this->Scene())
    this->Scene()->DestroyMaterial(this->material);
  this->material.reset();
  BaseCOMVisual::Destroy();
}

//////////////////////////////////////////////////
void FilamentCOMVisual::CreateVisual()
{
  if (!this->crossLines)
  {
    this->crossLines = this->scene->CreateDynamicMesh();
    this->crossLines->SetOperationType(MT_LINE_LIST);
    this->AddGeometry(this->crossLines);
  }

  if (!this->sphereVis)
  {
    this->sphereVis = this->Scene()->CreateVisual();
    this->sphereVis->AddGeometry(this->Scene()->CreateSphere());
    this->sphereVis->SetMaterial("Default/CoM");
    this->sphereVis->SetInheritScale(false);
    this->AddChild(this->sphereVis);
  }

  const double sphereRadius = this->SphereRadius();
  this->sphereVis->SetLocalScale(math::Vector3d(
      sphereRadius * 2, sphereRadius * 2, sphereRadius * 2));
  this->sphereVis->SetLocalPosition(this->InertiaPose().Pos());
  this->sphereVis->SetLocalRotation(this->InertiaPose().Rot());

  // Get the bounding box of the parent visual
  VisualPtr vis = this->Scene()->VisualByName(this->parentName);
  math::AxisAlignedBox box;
  if (vis)
    box = vis->LocalBoundingBox();

  this->crossLines->Clear();
  if (!this->material)
  {
    this->SetMaterial(this->Scene()->Material("Default/TransGreen")->Clone(),
        false);
    this->ownsMaterial = true;
  }
  else
  {
    this->crossLines->SetLineMaterial(this->material);
  }

  // CoM position indicator
  const math::Vector3d c = this->InertiaPose().Pos();
  this->crossLines->AddPoint(
      math::Vector3d(0, 0, box.Min().Z() - c.Z()) + c);
  this->crossLines->AddPoint(
      math::Vector3d(0, 0, box.Max().Z() - c.Z()) + c);
  this->crossLines->AddPoint(
      math::Vector3d(0, box.Min().Y() - c.Y(), 0) + c);
  this->crossLines->AddPoint(
      math::Vector3d(0, box.Max().Y() - c.Y(), 0) + c);
  this->crossLines->AddPoint(
      math::Vector3d(box.Min().X() - c.X(), 0, 0) + c);
  this->crossLines->AddPoint(
      math::Vector3d(box.Max().X() - c.X(), 0, 0) + c);
}

//////////////////////////////////////////////////
void FilamentCOMVisual::SetMaterial(MaterialPtr _material, bool _unique)
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
  if (this->crossLines)
    this->crossLines->SetLineMaterial(_material);
  MaterialPtr old = this->ownsMaterial ? this->material : nullptr;
  this->material = _material;
  this->ownsMaterial = _unique;
  if (old && old != _material)
    this->Scene()->DestroyMaterial(old);
}

//////////////////////////////////////////////////
MaterialPtr FilamentCOMVisual::Material() const
{
  return this->material;
}

//////////////////////////////////////////////////
VisualPtr FilamentCOMVisual::SphereVisual() const
{
  return this->sphereVis;
}
