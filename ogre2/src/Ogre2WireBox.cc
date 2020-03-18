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

#include "ignition/rendering/ogre2/Ogre2WireBox.hh"
#include "ignition/rendering/ogre2/Ogre2Material.hh"
#include "ignition/rendering/ogre2/Ogre2Scene.hh"
#include "ignition/rendering/ogre2/Ogre2DynamicRenderable.hh"

using namespace ignition;
using namespace rendering;

class ignition::rendering::Ogre2WireBoxPrivate
{
  /// \brief Grid materal
  public: Ogre2MaterialPtr material;

  /// \brief Ogre renderable used to render the grid.
  public: Ogre2DynamicRenderable *wireBox = nullptr;

  public: ScenePtr scene;
};

//////////////////////////////////////////////////
Ogre2WireBox::Ogre2WireBox()
    : dataPtr(new Ogre2WireBoxPrivate)
{
}

//////////////////////////////////////////////////
Ogre2WireBox::~Ogre2WireBox()
{
  delete this->dataPtr->wireBox;
}

//////////////////////////////////////////////////
void Ogre2WireBox::PreRender()
{
  if (this->wireBoxDirty)
  {
    this->Create();
    this->wireBoxDirty = false;
  }
}

//////////////////////////////////////////////////
Ogre::MovableObject *Ogre2WireBox::OgreObject() const
{
  return this->dataPtr->wireBox->OgreObject();
}

//////////////////////////////////////////////////
void Ogre2WireBox::Init()
{
  this->Create();
}

void Ogre2WireBox::SetSceneNode(const ScenePtr &_scene)
{
  this->dataPtr->scene = _scene;
}

//////////////////////////////////////////////////
void Ogre2WireBox::Create()
{
  ignwarn << "1\n";
  if (!this->dataPtr->wireBox)
  {
    this->dataPtr->wireBox = new Ogre2DynamicRenderable(this->dataPtr->scene);
  }

  ignwarn << "2\n";
  // Clear any previous data from the grid
  this->dataPtr->wireBox->Clear();
  ignwarn << "3\n";

  this->dataPtr->wireBox->SetOperationType(MarkerType::MT_LINE_LIST);
  ignwarn << "4\n";
  
  /*
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
*/

  ignition::math::Vector3d max = this->box.Max();
  ignition::math::Vector3d min = this->box.Min();
  ignwarn << "5\n";

  max = ignition::math::Vector3d(5, 5, 5);

  this->dataPtr->wireBox->AddPoint({min.X(), min.Y(), min.Z()}, {1, 0, 0});
  this->dataPtr->wireBox->AddPoint({max.X(), min.Y(), min.Z()}, {1, 0, 0});

  // line 1
  this->dataPtr->wireBox->AddPoint({min.X(), min.Y(), min.Z()}, {1, 0, 0});
  this->dataPtr->wireBox->AddPoint({min.X(), min.Y(), max.Z()}, {1, 0, 0});

  // line 2
  this->dataPtr->wireBox->AddPoint({min.X(), min.Y(), min.Z()}, {1, 0, 0});
  this->dataPtr->wireBox->AddPoint({min.X(), max.Y(), min.Z()}, {1, 0, 0});

  // line 3
  this->dataPtr->wireBox->AddPoint({min.X(), max.Y(), min.Z()}, {1, 0, 0});
  this->dataPtr->wireBox->AddPoint({min.X(), max.Y(), max.Z()}, {1, 0, 0});

  // line 4
  this->dataPtr->wireBox->AddPoint({min.X(), max.Y(), min.Z()}, {1, 0, 0});
  this->dataPtr->wireBox->AddPoint({max.X(), max.Y(), min.Z()}, {1, 0, 0});

  // line 5
  this->dataPtr->wireBox->AddPoint({max.X(), min.Y(), min.Z()}, {1, 0, 0});
  this->dataPtr->wireBox->AddPoint({max.X(), min.Y(), max.Z()}, {1, 0, 0});

  // line 6
  this->dataPtr->wireBox->AddPoint({max.X(), min.Y(), min.Z()}, {1, 0, 0});
  this->dataPtr->wireBox->AddPoint({max.X(), max.Y(), min.Z()}, {1, 0, 0});

  // line 7
  this->dataPtr->wireBox->AddPoint({min.X(), max.Y(), max.Z()}, {1, 0, 0});
  this->dataPtr->wireBox->AddPoint({max.X(), max.Y(), max.Z()}, {1, 0, 0});

  // line 8
  this->dataPtr->wireBox->AddPoint({min.X(), max.Y(), max.Z()}, {1, 0, 0});
  this->dataPtr->wireBox->AddPoint({min.X(), min.Y(), max.Z()}, {1, 0, 0});

  // line 9
  this->dataPtr->wireBox->AddPoint({max.X(), max.Y(), min.Z()}, {1, 0, 0});
  this->dataPtr->wireBox->AddPoint({max.X(), max.Y(), max.Z()}, {1, 0, 0});

  // line 10
  this->dataPtr->wireBox->AddPoint({max.X(), min.Y(), max.Z()}, {1, 0, 0});
  this->dataPtr->wireBox->AddPoint({max.X(), max.Y(), max.Z()}, {1, 0, 0});

  // line 11
  this->dataPtr->wireBox->AddPoint({min.X(), min.Y(), max.Z()}, {1, 0, 0});
  this->dataPtr->wireBox->AddPoint({max.X(), min.Y(), max.Z()}, {1, 0, 0});

  this->dataPtr->wireBox->Update();
  /*
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
  */
}

//////////////////////////////////////////////////
void Ogre2WireBox::SetMaterial(MaterialPtr _material, bool _unique)
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
  this->dataPtr->wireBox->SetMaterial(_material, _unique);
  this->SetMaterialImpl(derived);
}

//////////////////////////////////////////////////
void Ogre2WireBox::SetMaterialImpl(Ogre2MaterialPtr _material)
{
  Ogre::MaterialPtr ogreMaterial = _material->Material();
  this->dataPtr->material = _material;

  this->dataPtr->material->SetReceiveShadows(false);
  this->dataPtr->material->SetCastShadows(false);
  this->dataPtr->material->SetLightingEnabled(false);
}

//////////////////////////////////////////////////
MaterialPtr Ogre2WireBox::Material() const
{
  return this->dataPtr->material;
}
