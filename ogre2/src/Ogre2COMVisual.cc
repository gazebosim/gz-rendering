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

#include "ignition/rendering/ogre2/Ogre2COMVisual.hh"
#include "ignition/rendering/ogre2/Ogre2Material.hh"
#include "ignition/rendering/ogre2/Ogre2DynamicRenderable.hh"

#ifdef _MSC_VER
  #pragma warning(push, 0)
#endif
#include <OgreSceneNode.h>
#ifdef _MSC_VER
  #pragma warning(pop)
#endif

class ignition::rendering::Ogre2COMVisualPrivate
{
  /// \brief Grid materal
  public: Ogre2MaterialPtr material = nullptr;

  /// \brief Lines that make the cross marking the center of mass.
  public: std::shared_ptr<Ogre2DynamicRenderable> crossLines = nullptr;

  /// \brief Sphere visual marking the center of mass
  public: VisualPtr sphereVis = nullptr;
};

using namespace ignition;
using namespace rendering;

//////////////////////////////////////////////////
Ogre2COMVisual::Ogre2COMVisual()
  : dataPtr(new Ogre2COMVisualPrivate)
{
}

//////////////////////////////////////////////////
Ogre2COMVisual::~Ogre2COMVisual()
{
}

//////////////////////////////////////////////////
void Ogre2COMVisual::PreRender()
{
  if (this->dirtyCOMVisual)
  {
    this->CreateVisual();
    this->dirtyCOMVisual = false;
  }
}

//////////////////////////////////////////////////
void Ogre2COMVisual::Init()
{
  BaseCOMVisual::Init();
}

//////////////////////////////////////////////////
Ogre::MovableObject *Ogre2COMVisual::OgreObject() const
{
  return this->dataPtr->crossLines->OgreObject();
}

//////////////////////////////////////////////////
void Ogre2COMVisual::CreateVisual()
{
  if (!this->dataPtr->crossLines)
  {
    this->dataPtr->crossLines.reset(
        new Ogre2DynamicRenderable(this->Scene()));
    this->ogreNode->attachObject(this->OgreObject());
  }

  if (!this->dataPtr->sphereVis)
  {
    this->dataPtr->sphereVis = this->Scene()->CreateVisual();
    this->dataPtr->sphereVis->AddGeometry(this->Scene()->CreateSphere());
    this->dataPtr->sphereVis->SetMaterial("Default/CoM");
    this->dataPtr->sphereVis->SetInheritScale(false);
    this->AddChild(this->dataPtr->sphereVis);
  }

  // Compute radius of sphere with density of lead and equivalent mass.
  double sphereRadius;
  double dLead = 11340;
  sphereRadius = cbrt((0.75 * this->Mass()) / (M_PI * dLead));

  this->dataPtr->sphereVis->SetLocalScale(ignition::math::Vector3d(
      sphereRadius*2, sphereRadius*2, sphereRadius*2));
  this->dataPtr->sphereVis->SetLocalPosition(this->InertiaPose().Pos());
  this->dataPtr->sphereVis->SetLocalRotation(this->InertiaPose().Rot());

  // Get the link's bounding box
  if (this->ParentLink().empty() ||
      !this->Scene()->HasVisualName(this->ParentLink()))
    return;

  VisualPtr vis = this->Scene()->VisualByName(this->linkName);

  ignition::math::AxisAlignedBox box;
  if (vis)
    box = vis->LocalBoundingBox();

  // Clear any previous data from the grid and update
  this->dataPtr->crossLines->Clear();
  this->dataPtr->crossLines->Update();

  this->dataPtr->crossLines->SetOperationType(MarkerType::MT_LINE_LIST);
  if (!this->dataPtr->material)
  {
    MaterialPtr COMVisualMaterial =
        this->Scene()->Material("Default/TransGreen");
    this->SetMaterial(COMVisualMaterial, false);
  }

  // CoM position indicator
  ignition::math::Vector3d p1(0, 0,
      box.Min().Z() - this->InertiaPose().Pos().Z());
  ignition::math::Vector3d p2(0, 0,
      box.Max().Z() - this->InertiaPose().Pos().Z());

  ignition::math::Vector3d p3(0,
      box.Min().Y() - this->InertiaPose().Pos().Y(), 0);
  ignition::math::Vector3d p4(0,
      box.Max().Y() - this->InertiaPose().Pos().Y(), 0);

  ignition::math::Vector3d p5(
      box.Min().X() - this->InertiaPose().Pos().X(), 0, 0);
  ignition::math::Vector3d p6(
      box.Max().X() - this->InertiaPose().Pos().X(), 0, 0);

  p1 += this->InertiaPose().Pos();
  p2 += this->InertiaPose().Pos();
  p3 += this->InertiaPose().Pos();
  p4 += this->InertiaPose().Pos();
  p5 += this->InertiaPose().Pos();
  p6 += this->InertiaPose().Pos();

  this->dataPtr->crossLines->AddPoint(p1);
  this->dataPtr->crossLines->AddPoint(p2);
  this->dataPtr->crossLines->AddPoint(p3);
  this->dataPtr->crossLines->AddPoint(p4);
  this->dataPtr->crossLines->AddPoint(p5);
  this->dataPtr->crossLines->AddPoint(p6);

  this->dataPtr->crossLines->Update();
  this->ogreNode->setVisible(true);
}

//////////////////////////////////////////////////
void Ogre2COMVisual::SetMaterial(MaterialPtr _material, bool _unique)
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
  this->dataPtr->crossLines->SetMaterial(_material, false);
  this->SetMaterialImpl(derived);
}

//////////////////////////////////////////////////
void Ogre2COMVisual::SetMaterialImpl(Ogre2MaterialPtr _material)
{
  Ogre::MaterialPtr ogreMaterial = _material->Material();
  this->dataPtr->material = _material;
}

//////////////////////////////////////////////////
MaterialPtr Ogre2COMVisual::Material() const
{
  return this->dataPtr->material;
}

//////////////////////////////////////////////////
VisualPtr Ogre2COMVisual::SphereVisual() const
{
  return this->dataPtr->sphereVis;
}
