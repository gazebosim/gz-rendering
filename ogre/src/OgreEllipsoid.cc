/*
 * Copyright (C) 2021 Open Source Robotics Foundation
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

#include <cmath>

#include "ignition/rendering/ogre/OgreEllipsoid.hh"
#include "ignition/rendering/ogre/OgreMaterial.hh"
#include "ignition/rendering/ogre/OgreScene.hh"

#include <ignition/math/Vector3.hh>
#include <ignition/math/Vector2.hh>

class ignition::rendering::OgreEllipsoidPrivate
{
  /// \brief Grid materal
  public: OgreMaterialPtr material;

  public: Ogre::ManualObject *manualObject = nullptr;
};

using namespace ignition;
using namespace rendering;

//////////////////////////////////////////////////
OgreEllipsoid::OgreEllipsoid()
 : dataPtr(new OgreEllipsoidPrivate)
{
}

//////////////////////////////////////////////////
OgreEllipsoid::~OgreEllipsoid()
{
  // no ops
}

//////////////////////////////////////////////////
void OgreEllipsoid::PreRender()
{
  if (this->ellipsoidDirty)
  {
    this->Create();
    this->ellipsoidDirty = false;
  }
}

//////////////////////////////////////////////////
Ogre::MovableObject *OgreEllipsoid::OgreObject() const
{
  return this->dataPtr->manualObject;
}

//////////////////////////////////////////////////
void OgreEllipsoid::Init()
{
  this->Create();
}

//////////////////////////////////////////////////
void OgreEllipsoidEllipsoid()
{
  // no ops
}

//////////////////////////////////////////////////
void OgreEllipsoid::Create()
{
  if (!this->dataPtr->manualObject)
  {
    this->dataPtr->manualObject =
      this->scene->OgreSceneManager()->createManualObject(this->name);
  }
  this->dataPtr->manualObject->clear();

  this->dataPtr->manualObject->setCastShadows(false);

  std::string materialName = this->dataPtr->material ?
      this->dataPtr->material->Name() : "Default/White";
  this->dataPtr->manualObject->begin(materialName,
      Ogre::RenderOperation::OT_TRIANGLE_LIST);

  std::vector<ignition::math::Vector3d> positions;
  std::vector<int> indexes;
  std::vector<ignition::math::Vector2d> uvs;

  this->EllipsoidMesh(positions, indexes, uvs);

  for (unsigned int i = 0; i < positions.size(); i++)
  {
    this->dataPtr->manualObject->position(
      Ogre::Vector3(positions[i].X(), positions[i].Y(), positions[i].Z()));
    if (i < uvs.size())
      this->dataPtr->manualObject->textureCoord(
        Ogre::Vector2(uvs[i].X(), uvs[i].Y()));
    this->dataPtr->manualObject->normal(
      positions[i].Normalize().X(),
      positions[i].Normalize().Y(),
      positions[i].Normalize().Z());
  }

  for (auto index : indexes)
  {
    this->dataPtr->manualObject->index(index);
  }
  this->dataPtr->manualObject->end();
}

//////////////////////////////////////////////////
void OgreEllipsoid::SetMaterial(MaterialPtr _material, bool _unique)
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
void OgreEllipsoid::SetMaterialImpl(OgreMaterialPtr _material)
{
  std::string materialName = _material->Name();
  Ogre::MaterialPtr ogreMaterial = _material->Material();
  this->dataPtr->manualObject->setMaterialName(0, materialName);
  this->dataPtr->material = _material;
}

//////////////////////////////////////////////////
MaterialPtr OgreEllipsoid::Material() const
{
  return this->dataPtr->material;
}
