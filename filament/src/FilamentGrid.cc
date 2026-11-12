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
#include <gz/math/Helpers.hh>

#include "gz/rendering/filament/FilamentGrid.hh"
#include "gz/rendering/filament/FilamentScene.hh"

using namespace gz;
using namespace rendering;

//////////////////////////////////////////////////
FilamentGrid::FilamentGrid() = default;

//////////////////////////////////////////////////
FilamentGrid::~FilamentGrid() = default;

//////////////////////////////////////////////////
void FilamentGrid::Init()
{
  BaseGrid::Init();
  this->Create();
}

//////////////////////////////////////////////////
void FilamentGrid::PreRender()
{
  if (this->gridDirty)
  {
    this->Create();
    this->gridDirty = false;
  }
  // ogre2 lights the grid's line list (Ogre PBS) with zero normals: no direct
  // light, only the hemisphere ambient term, scene ambient * diffuse / pi
  // (linear). Draw the lines unlit in that color.
  if (this->shaded && this->Material())
  {
    const math::Color a = this->Scene()->AmbientLight();
    const math::Color d = this->Material()->Diffuse();
    const float k = static_cast<float>(1.0 / GZ_PI);
    this->shaded->SetDiffuse(math::Color(a.R() * d.R() * k,
        a.G() * d.G() * k, a.B() * d.B() * k, d.A()));
  }
  BaseGrid::PreRender();
}

//////////////////////////////////////////////////
void FilamentGrid::SetMaterial(MaterialPtr _material, bool _unique)
{
  FilamentDynamicGeometry::SetMaterial(_material, _unique);
  if (!this->Material())
    return;
  MaterialPtr old = this->shaded;
  this->shaded = this->Material()->Clone();
  this->lines->SetLineMaterial(this->shaded);
  if (old)
    this->Scene()->DestroyMaterial(old);
}

//////////////////////////////////////////////////
void FilamentGrid::Destroy()
{
  BaseGrid::Destroy();
  // after the renderable: filament panics on destroying a material in use
  if (this->shaded && this->Scene())
    this->Scene()->DestroyMaterial(this->shaded);
  this->shaded.reset();
}

//////////////////////////////////////////////////
void FilamentGrid::Create()
{
  this->lines->Clear();
  // same layout as Ogre2Grid::Create
  const double baseExtent = (this->cellLength *
      static_cast<double>(this->cellCount - this->cellCount % 2)) / 2;
  for (unsigned int h = 0; h <= this->verticalCellCount; ++h)
  {
    double hReal = this->heightOffset +
        (this->verticalCellCount / 2 - static_cast<double>(h))
        * this->cellLength;

    // If there are odd vertical cells, shift cell planes up
    if (this->verticalCellCount % 2)
      hReal += this->cellLength;

    for (unsigned int i = 0; i <= this->cellCount; i++)
    {
      double extent = baseExtent;

      // If there is an odd cell count, extend a row and column along
      // the positive x and y axes
      if (this->cellCount % 2)
        extent += this->cellLength;

      const double inc = extent - (i * this->cellLength);
      this->lines->AddPoint({inc, -baseExtent, hReal});
      this->lines->AddPoint({inc, extent, hReal});
      this->lines->AddPoint({-baseExtent, inc, hReal});
      this->lines->AddPoint({extent, inc, hReal});
    }
  }
  if (this->verticalCellCount > 0)
  {
    for (unsigned int x = 0; x <= this->cellCount; ++x)
    {
      for (unsigned int y = 0; y <= this->cellCount; ++y)
      {
        double xReal = baseExtent - x * this->cellLength;
        double yReal = baseExtent - y * this->cellLength;

        double zTop = (this->verticalCellCount / 2.0f) * this->cellLength;
        double zBottom = -zTop;

        // If odd vertical cell count, add cell length offset to adjust
        // z min and max
        if (this->verticalCellCount % 2)
        {
          zTop += this->cellLength / 2.0f;
          zBottom += this->cellLength / 2.0f;
        }

        // If odd horizontal cell count, shift vertical lines
        // towards positive x, y axes
        if (this->cellCount % 2)
        {
          xReal += this->cellLength;
          yReal += this->cellLength;
        }

        this->lines->AddPoint({xReal, yReal, zBottom});
        this->lines->AddPoint({xReal, yReal, zTop});
      }
    }
  }
}
