/*
 * Copyright (C) 2020 Open Source Robotics Foundation
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

#include "ignition/rendering/ogre2/Ogre2Grid.hh"
#include "ignition/rendering/ogre2/Ogre2Material.hh"
#include "ignition/rendering/ogre2/Ogre2Scene.hh"
#include "ignition/rendering/ogre2/Ogre2DynamicRenderable.hh"

using namespace ignition;
using namespace rendering;

class ignition::rendering::Ogre2GridPrivate
{
  /// \brief Grid materal
  public: Ogre2MaterialPtr material;

  /// \brief Ogre renderable used to render the grid.
  public: std::shared_ptr<Ogre2DynamicRenderable> grid = nullptr;
};

//////////////////////////////////////////////////
Ogre2Grid::Ogre2Grid()
    : dataPtr(new Ogre2GridPrivate)
{
}

//////////////////////////////////////////////////
Ogre2Grid::~Ogre2Grid()
{
}

//////////////////////////////////////////////////
void Ogre2Grid::PreRender()
{
  if (this->gridDirty)
  {
    this->Create();
    this->gridDirty = false;
  }
}

//////////////////////////////////////////////////
Ogre::MovableObject *Ogre2Grid::OgreObject() const
{
  return this->dataPtr->grid->OgreObject();
}

//////////////////////////////////////////////////
void Ogre2Grid::Init()
{
  this->Create();
}

//////////////////////////////////////////////////
void Ogre2Grid::Create()
{
  if (!this->dataPtr->grid)
  {
    this->dataPtr->grid.reset(new Ogre2DynamicRenderable(this->scene));
  }

  // Clear any previous data from the grid and update
  this->dataPtr->grid->Clear();
  this->dataPtr->grid->Update();

  this->dataPtr->grid->SetOperationType(MarkerType::MT_LINE_LIST);
  double extent = (this->cellLength * static_cast<double>(this->cellCount))/2;
  for (unsigned int h = 0; h <= this->verticalCellCount; ++h)
  {
    double hReal = this->heightOffset +
        (this->verticalCellCount / 2.0f - static_cast<double>(h))
        * this->cellLength;
    for (unsigned int i = 0; i <= this->cellCount; i++)
    {
      double inc = extent - (i * this->cellLength);

      math::Vector3d p1{inc, -extent, hReal};
      math::Vector3d p2{inc, extent , hReal};
      math::Vector3d p3{-extent, inc, hReal};
      math::Vector3d p4{extent, inc, hReal};

      this->dataPtr->grid->AddPoint(p1);
      this->dataPtr->grid->AddPoint(p2);
      this->dataPtr->grid->AddPoint(p3);
      this->dataPtr->grid->AddPoint(p4);
    }
  }

  if (this->verticalCellCount > 0)
  {
    for (unsigned int x = 0; x <= this->cellCount; ++x)
    {
      for (unsigned int y = 0; y <= this->cellCount; ++y)
      {
        double xReal = extent - x * this->cellLength;
        double yReal = extent - y * this->cellLength;

        double zTop = (this->verticalCellCount / 2.0f) * this->cellLength;
        double zBottom = -zTop;

        this->dataPtr->grid->AddPoint(xReal, yReal, zBottom);
      }
    }
  }
  this->dataPtr->grid->Update();
}

//////////////////////////////////////////////////
void Ogre2Grid::SetMaterial(MaterialPtr _material, bool _unique)
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
  this->dataPtr->grid->SetMaterial(_material, false);
  this->SetMaterialImpl(derived);
}

//////////////////////////////////////////////////
void Ogre2Grid::SetMaterialImpl(Ogre2MaterialPtr _material)
{
  Ogre::MaterialPtr ogreMaterial = _material->Material();
  this->dataPtr->material = _material;

  this->dataPtr->material->SetReceiveShadows(false);
  this->dataPtr->material->SetCastShadows(false);
  this->dataPtr->material->SetLightingEnabled(false);
}

//////////////////////////////////////////////////
MaterialPtr Ogre2Grid::Material() const
{
  return this->dataPtr->material;
}
