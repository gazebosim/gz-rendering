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

#include "ignition/rendering/ogre2/Ogre2Capsule.hh"
#include "ignition/rendering/ogre2/Ogre2Material.hh"
#include "ignition/rendering/ogre2/Ogre2Scene.hh"
#include "ignition/rendering/ogre2/Ogre2DynamicRenderable.hh"

#include <ignition/math/Vector3.hh>

class ignition::rendering::Ogre2CapsulePrivate
{
  /// \brief Capsule materal
  public: Ogre2MaterialPtr material;

  public: std::shared_ptr<Ogre2DynamicRenderable> capsule = nullptr;
};

using namespace ignition;
using namespace rendering;

//////////////////////////////////////////////////
Ogre2Capsule::Ogre2Capsule()
 : dataPtr(new Ogre2CapsulePrivate)
{
}

//////////////////////////////////////////////////
Ogre2Capsule::~Ogre2Capsule()
{
  // no ops
}

//////////////////////////////////////////////////
Ogre::MovableObject *Ogre2Capsule::OgreObject() const
{
  return this->dataPtr->capsule->OgreObject();
}

//////////////////////////////////////////////////
void Ogre2Capsule::PreRender()
{
  if (this->capsuleDirty)
  {
    this->Create();
    this->capsuleDirty = false;
  }
}

//////////////////////////////////////////////////
void Ogre2Capsule::Init()
{
  this->Create();
}

////////////////////////////////////////////////
void Ogre2Capsule::Create()
{
  if (!this->dataPtr->capsule)
  {
    this->dataPtr->capsule.reset(new Ogre2DynamicRenderable(this->Scene()));
  }

  this->dataPtr->capsule->Clear();
  this->dataPtr->capsule->Update();

  this->dataPtr->capsule->SetOperationType(MT_TRIANGLE_LIST);

  std::vector<ignition::math::Vector3d> positions;
  std::vector<int> indexes;
  std::vector<ignition::math::Vector2d> uvs;

  this->CapsuleMesh(positions, indexes, uvs);

  for (auto index : indexes)
  {
    this->dataPtr->capsule->AddPoint(positions[index]);
  }

  this->dataPtr->capsule->Update();
}

//////////////////////////////////////////////////
void Ogre2Capsule::SetMaterial(MaterialPtr _material, bool _unique)
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
  this->dataPtr->capsule->SetMaterial(_material, false);
  this->SetMaterialImpl(derived);
}

//////////////////////////////////////////////////
void Ogre2Capsule::SetMaterialImpl(Ogre2MaterialPtr _material)
{
  Ogre::MaterialPtr ogreMaterial = _material->Material();
  this->dataPtr->material = _material;
}

//////////////////////////////////////////////////
MaterialPtr Ogre2Capsule::Material() const
{
  return this->dataPtr->material;
}
