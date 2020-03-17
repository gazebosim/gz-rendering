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
// TODO convert below create function to use dynamic renderable
#include <ignition/common/Console.hh>

#include "ignition/rendering/ogre/OgreWireBox.hh"
#include "ignition/rendering/ogre/OgreMaterial.hh"
#include "ignition/rendering/ogre/OgreScene.hh"
#include "ignition/rendering/ogre/OgreDynamicLines.hh"

class ignition::rendering::OgreWireBoxPrivate
{
  /// \brief WireBox materal
  public: OgreMaterialPtr material;

  /// \brief Ogre manual object used to render the wire box.
  public: Ogre::ManualObject *manualObject = nullptr;

  public: std::shared_ptr<OgreDynamicLines> boundingBox;
};

using namespace ignition;
using namespace rendering;

//////////////////////////////////////////////////
OgreWireBox::OgreWireBox()
    : dataPtr(new OgreWireBoxPrivate)
{
}

//////////////////////////////////////////////////
OgreWireBox::~OgreWireBox()
{
}

//////////////////////////////////////////////////
void OgreWireBox::PreRender()
{
  if (this->wireBoxDirty)
  {
    this->Create();
    this->wireBoxDirty = false;
  }

  if (this->dataPtr->boundingBox)
    this->dataPtr->boundingBox->Update();
}

//////////////////////////////////////////////////
Ogre::MovableObject *OgreWireBox::OgreObject() const
{
  return std::dynamic_pointer_cast<Ogre::MovableObject>(this->dataPtr->boundingBox).get();
}

//////////////////////////////////////////////////
void OgreWireBox::Init()
{
  this->Create();
}

//////////////////////////////////////////////////
void OgreWireBox::Create()
{
  if (!this->visual)
  {
    ignerr << "Wire Box has no attached visual.\n";
    return;
  }

  if (!this->dataPtr->boundingBox)
  {
    this->dataPtr->boundingBox.reset(new OgreDynamicLines(MarkerType::MT_LINE_LIST));
  }

  this->dataPtr->boundingBox->Clear();

/*
  if (!this->dataPtr->manualObject)
  {
    this->dataPtr->manualObject =
      this->scene->OgreSceneManager()->createManualObject(this->name);
  }

  this->dataPtr->manualObject->clear();
  this->dataPtr->manualObject->setCastShadows(false);
  
  // TODO below estimate vertex count
  this->dataPtr->manualObject->estimateVertexCount(12);
  this->dataPtr->manualObject->begin(materialName,
      Ogre::RenderOperation::OT_LINE_LIST);
*/

  std::string materialName = this->dataPtr->material ?
      this->dataPtr->material->Name() : "Default/Blue";
  ignition::math::Vector3d max = this->box.Max();
  ignition::math::Vector3d min = this->box.Min();

  max = ignition::math::Vector3d(1, 1, 1);
  ignwarn << "Max " << max << "\n";
  ignwarn << "Min " << min << "\n";

  // line 0
  this->dataPtr->boundingBox->AddPoint({min.X(), min.Y(), min.Z()}, {1, 0, 0});
  this->dataPtr->boundingBox->AddPoint({max.X(), min.Y(), min.Z()}, {1, 0, 0});

  // line 1
  this->dataPtr->boundingBox->AddPoint({min.X(), min.Y(), min.Z()}, {1, 0, 0});
  this->dataPtr->boundingBox->AddPoint({min.X(), min.Y(), max.Z()}, {1, 0, 0});

  // line 2
  this->dataPtr->boundingBox->AddPoint({min.X(), min.Y(), min.Z()}, {1, 0, 0});
  this->dataPtr->boundingBox->AddPoint({min.X(), max.Y(), min.Z()}, {1, 0, 0});

  // line 3
  this->dataPtr->boundingBox->AddPoint({min.X(), max.Y(), min.Z()}, {1, 0, 0});
  this->dataPtr->boundingBox->AddPoint({min.X(), max.Y(), max.Z()}, {1, 0, 0});

  // line 4
  this->dataPtr->boundingBox->AddPoint({min.X(), max.Y(), min.Z()}, {1, 0, 0});
  this->dataPtr->boundingBox->AddPoint({max.X(), max.Y(), min.Z()}, {1, 0, 0});

  // line 5
  this->dataPtr->boundingBox->AddPoint({max.X(), min.Y(), min.Z()}, {1, 0, 0});
  this->dataPtr->boundingBox->AddPoint({max.X(), min.Y(), max.Z()}, {1, 0, 0});

  // line 6
  this->dataPtr->boundingBox->AddPoint({max.X(), min.Y(), min.Z()}, {1, 0, 0});
  this->dataPtr->boundingBox->AddPoint({max.X(), max.Y(), min.Z()}, {1, 0, 0});

  // line 7
  this->dataPtr->boundingBox->AddPoint({min.X(), max.Y(), max.Z()}, {1, 0, 0});
  this->dataPtr->boundingBox->AddPoint({max.X(), max.Y(), max.Z()}, {1, 0, 0});

  // line 8
  this->dataPtr->boundingBox->AddPoint({min.X(), max.Y(), max.Z()}, {1, 0, 0});
  this->dataPtr->boundingBox->AddPoint({min.X(), min.Y(), max.Z()}, {1, 0, 0});

  // line 9
  this->dataPtr->boundingBox->AddPoint({max.X(), max.Y(), min.Z()}, {1, 0, 0});
  this->dataPtr->boundingBox->AddPoint({max.X(), max.Y(), max.Z()}, {1, 0, 0});

  // line 10
  this->dataPtr->boundingBox->AddPoint({max.X(), min.Y(), max.Z()}, {1, 0, 0});
  this->dataPtr->boundingBox->AddPoint({max.X(), max.Y(), max.Z()}, {1, 0, 0});

  // line 11
  this->dataPtr->boundingBox->AddPoint({min.X(), min.Y(), max.Z()}, {1, 0, 0});
  this->dataPtr->boundingBox->AddPoint({max.X(), min.Y(), max.Z()}, {1, 0, 0});


 // this->dataPtr->manualObject->end();
}

//////////////////////////////////////////////////
void OgreWireBox::SetMaterial(MaterialPtr _material, bool _unique)
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
void OgreWireBox::SetMaterialImpl(OgreMaterialPtr _material)
{
  std::string materialName = _material->Name();
  Ogre::MaterialPtr ogreMaterial = _material->Material();
  this->dataPtr->manualObject->setMaterialName(0, materialName);
  this->dataPtr->material = _material;

  this->dataPtr->material->SetReceiveShadows(false);
  this->dataPtr->material->SetLightingEnabled(false);
}

//////////////////////////////////////////////////
MaterialPtr OgreWireBox::Material() const
{
  ignwarn << "Returning material: " << this->dataPtr->material << "\n";
  return this->dataPtr->material;
}
