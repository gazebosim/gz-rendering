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

  /// \brief Ogre manual object used to render the grid.
  public: Ogre::v1::ManualObject *manualObject = nullptr;

  public: Ogre2DynamicRenderable *grid = nullptr;

  /// \brief Ogre memory manager responsible for the movable object
  public: Ogre::ObjectMemoryManager *memoryManager = nullptr;
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
    this->dataPtr->grid = new Ogre2DynamicRenderable(this->scene);
  }
  if (!this->dataPtr->grid)
  {
    ignwarn << "grid null\n";
  }

  this->dataPtr->grid->SetOperationType(MarkerType::MT_LINE_LIST);
  
  /*
  if (!this->dataPtr->manualObject)
  {
    this->dataPtr->manualObject = new
      Ogre::v1::ManualObject(1, this->dataPtr->memoryManager, this->scene->OgreSceneManager());
    //  this->dataPtr->manualObject =
    //    this->scene->OgreSceneManager()->createManualObject(this->name);
  }

  this->dataPtr->manualObject->clear();

  */
  double extent = (this->cellLength * static_cast<double>(this->cellCount))/2;
/*
  this->dataPtr->manualObject->setCastShadows(false);
  this->dataPtr->manualObject->estimateVertexCount(
      this->cellCount * 4 * this->verticalCellCount +
      ((this->cellCount + 1) * (this->cellCount + 1)));
*/
  std::string materialName = this->dataPtr->material ?
      this->dataPtr->material->Name() : "Default/White";
  //t his->dataPtr->manualObject->begin(materialName,
  //    Ogre::OT_LINE_LIST);
  ignwarn << "Cell length " << this->cellLength << "\n";
  ignwarn << "Vertical cell count " << this->verticalCellCount << "\n";
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

      //this->dataptr->manualobject->position(p1);
      //this->dataptr->manualobject->position(p2);

      //this->dataptr->manualobject->position(p3);
      //this->dataptr->manualobject->position(p4);
    }
  }
  this->dataPtr->grid->Update();
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

        this->dataPtr->manualObject->position(xReal, yReal, zBottom);
        this->dataPtr->manualObject->position(xReal, yReal, zBottom);
      }
    }
  }

  ignwarn << "Current index count " << this->dataPtr->manualObject->getCurrentIndexCount() << "\n";

  if (this->dataPtr->manualObject->getParentSceneNode())
    this->dataPtr->manualObject->setVisible(1);
    
  ignwarn << "parent node " << this->dataPtr->manualObject->getParentSceneNode() << "\n";
  ignwarn << "is visible? " << this->dataPtr->manualObject->isVisible() << "\n";
  this->dataPtr->manualObject->end();
  */
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

  this->SetMaterialImpl(derived);
}

//////////////////////////////////////////////////
void Ogre2Grid::SetMaterialImpl(Ogre2MaterialPtr _material)
{
  std::string materialName = _material->Name();
  Ogre::MaterialPtr ogreMaterial = _material->Material();
  this->dataPtr->manualObject->setMaterialName(0, materialName);
  this->dataPtr->material = _material;

  this->dataPtr->material->SetReceiveShadows(false);
  this->dataPtr->material->SetLightingEnabled(false);
}

//////////////////////////////////////////////////
MaterialPtr Ogre2Grid::Material() const
{
  return this->dataPtr->material;
}
