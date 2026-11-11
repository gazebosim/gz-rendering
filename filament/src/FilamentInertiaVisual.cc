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
#include "FilamentUtils.hh"

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
  filament_utils::ReplaceMaterial(this->Scene(), this->material,
      this->ownsMaterial, nullptr, false);
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
  _material = filament_utils::MaterialToAssign(_material, _unique);
  if (!_material)
    return;
  if (this->crossLines)
    this->crossLines->SetLineMaterial(_material);
  filament_utils::ReplaceMaterial(this->Scene(), this->material,
      this->ownsMaterial, _material, _unique);
}
