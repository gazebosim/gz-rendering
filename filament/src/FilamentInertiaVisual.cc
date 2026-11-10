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

#include "gz/rendering/filament/FilamentInertiaVisual.hh"
#include "gz/rendering/filament/FilamentMaterial.hh"
#include "gz/rendering/filament/FilamentScene.hh"

using namespace gz;
using namespace rendering;

//////////////////////////////////////////////////
FilamentInertiaVisual::FilamentInertiaVisual() = default;

//////////////////////////////////////////////////
FilamentInertiaVisual::~FilamentInertiaVisual() = default;

//////////////////////////////////////////////////
void FilamentInertiaVisual::Init()
{
  BaseInertiaVisual::Init();
}

//////////////////////////////////////////////////
void FilamentInertiaVisual::Destroy()
{
  if (this->boxVis)
  {
    this->boxVis->Destroy();
    this->boxVis.reset();
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
  BaseInertiaVisual::Destroy();
}

//////////////////////////////////////////////////
void FilamentInertiaVisual::Load(const math::Pose3d &_pose,
    const math::Vector3d &_scale)
{
  if (!this->crossLines)
  {
    this->crossLines = this->scene->CreateDynamicMesh();
    this->crossLines->SetOperationType(MT_LINE_LIST);
    this->AddGeometry(this->crossLines);
  }

  if (!this->boxVis)
  {
    this->boxVis = this->Scene()->CreateVisual();
    this->boxVis->AddGeometry(this->Scene()->CreateBox());
    this->boxVis->SetMaterial("Default/TransPurple");
    this->AddChild(this->boxVis);
  }

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

  // Inertia position indicator
  const math::Vector3d axes[6] = {
      {0, 0, -2 * _scale.Z()}, {0, 0, 2 * _scale.Z()},
      {0, -2 * _scale.Y(), 0}, {0, 2 * _scale.Y(), 0},
      {-2 * _scale.X(), 0, 0}, {2 * _scale.X(), 0, 0}};
  for (const auto &p : axes)
    this->crossLines->AddPoint(_pose.Rot().RotateVector(p) + _pose.Pos());

  this->boxVis->SetLocalScale(_scale);
  this->boxVis->SetLocalPosition(_pose.Pos());
  this->boxVis->SetLocalRotation(_pose.Rot());
}

//////////////////////////////////////////////////
VisualPtr FilamentInertiaVisual::BoxVisual() const
{
  return this->boxVis;
}

//////////////////////////////////////////////////
void FilamentInertiaVisual::SetMaterial(MaterialPtr _material, bool _unique)
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
MaterialPtr FilamentInertiaVisual::Material() const
{
  return this->material;
}
