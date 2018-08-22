/*
 * Copyright (C) 2017 Open Source Robotics Foundation
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

#include "ignition/rendering/ogre/OgreGrid.hh"
#include "ignition/rendering/ogre/OgreMaterial.hh"
#include "ignition/rendering/ogre/OgreScene.hh"

class ignition::rendering::OgreGridPrivate
{
  /// \brief Grid materal
  public: OgreMaterialPtr material;

  /// \brief Ogre manual object used to render the grid.
  public: Ogre::ManualObject *manualObject = nullptr;
};

using namespace ignition;
using namespace rendering;

//////////////////////////////////////////////////
OgreGrid::OgreGrid()
    : dataPtr(new OgreGridPrivate)
{
}

//////////////////////////////////////////////////
OgreGrid::~OgreGrid()
{
}

//////////////////////////////////////////////////
void OgreGrid::PreRender()
{
  if (this->gridDirty)
  {
    this->Create();
    this->gridDirty = false;
  }
}

//////////////////////////////////////////////////
Ogre::MovableObject *OgreGrid::OgreObject() const
{
  return this->dataPtr->manualObject;
}

//////////////////////////////////////////////////
void OgreGrid::Init()
{
  this->Create();
}

//////////////////////////////////////////////////
void OgreGrid::Create()
{
  if (!this->dataPtr->manualObject)
  {
    this->dataPtr->manualObject =
      this->scene->OgreSceneManager()->createManualObject(this->name);
  }

  this->dataPtr->manualObject->clear();

  double extent = (this->cellLength * static_cast<double>(this->cellCount))/2;

  this->dataPtr->manualObject->setCastShadows(false);
  this->dataPtr->manualObject->estimateVertexCount(
      this->cellCount * 4 * this->verticalCellCount +
      ((this->cellCount + 1) * (this->cellCount + 1)));

  std::string materialName = this->dataPtr->material ?
      this->dataPtr->material->Name() : "Default/White";
  this->dataPtr->manualObject->begin(materialName,
      Ogre::RenderOperation::OT_LINE_LIST);
  for (unsigned int h = 0; h <= this->verticalCellCount; ++h)
  {
    double hReal = this->heightOffset +
        (this->verticalCellCount / 2.0f - static_cast<double>(h))
        * this->cellLength;
    for (unsigned int i = 0; i <= this->cellCount; i++)
    {
      double inc = extent - (i * this->cellLength);

      Ogre::Vector3 p1(inc, -extent, hReal);
      Ogre::Vector3 p2(inc, extent , hReal);
      Ogre::Vector3 p3(-extent, inc, hReal);
      Ogre::Vector3 p4(extent, inc, hReal);

      this->dataPtr->manualObject->position(p1);
      this->dataPtr->manualObject->position(p2);

      this->dataPtr->manualObject->position(p3);
      this->dataPtr->manualObject->position(p4);
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

        this->dataPtr->manualObject->position(xReal, yReal, zBottom);
        this->dataPtr->manualObject->position(xReal, yReal, zBottom);
      }
    }
  }

  this->dataPtr->manualObject->end();
}

//////////////////////////////////////////////////
void OgreGrid::SetMaterial(MaterialPtr _material, bool _unique)
{
  _material = (_unique) ? _material->Clone() : _material;

  OgreMaterialPtr derived =
      std::dynamic_pointer_cast<OgreMaterial>(_material);

  if (!derived)
  {
    ignerr << "Cannot assign material created by another render-engine"
        << std::endl;

    return;
  }

  this->SetMaterialImpl(derived);
}

//////////////////////////////////////////////////
void OgreGrid::SetMaterialImpl(OgreMaterialPtr _material)
{
  std::string materialName = _material->Name();
  Ogre::MaterialPtr ogreMaterial = _material->Material();
  this->dataPtr->manualObject->setMaterialName(0, materialName);
  this->dataPtr->material = _material;

  this->dataPtr->material->SetReceiveShadows(false);
  this->dataPtr->material->SetLightingEnabled(false);
}

//////////////////////////////////////////////////
MaterialPtr OgreGrid::Material() const
{
  return this->dataPtr->material;
}
