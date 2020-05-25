/*
 * Copyright (C) 2019 Open Source Robotics Foundation
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

#include "ignition/rendering/ogre/OgreDynamicLines.hh"
#include "ignition/rendering/ogre/OgreLidarVisual.hh"
#include "ignition/rendering/ogre/OgreMaterial.hh"
#include "ignition/rendering/ogre/OgreMesh.hh"
#include "ignition/rendering/ogre/OgreScene.hh"


class ignition::rendering::OgreLidarVisualPrivate
{
  /// \brief LidarVisual material
  public: OgreMaterialPtr material = nullptr;

  /// \brief DynamicLines Object to display
  public: std::shared_ptr<OgreDynamicLines> dynamicRenderable;

  /// \brief Mesh Object for primitive shapes
  public: OgreMeshPtr mesh = nullptr;
};

using namespace ignition;
using namespace rendering;

//////////////////////////////////////////////////
OgreLidarVisual::OgreLidarVisual()
  : dataPtr(new OgreLidarVisualPrivate)
{
}

//////////////////////////////////////////////////
OgreLidarVisual::~OgreLidarVisual()
{
  this->Destroy();
}

//////////////////////////////////////////////////
void OgreLidarVisual::PreRender()
{
  this->dataPtr->dynamicRenderable->Update();
}

//////////////////////////////////////////////////
void OgreLidarVisual::Destroy()
{
  if (this->dataPtr->mesh)
  {
    this->dataPtr->mesh->Destroy();
    this->dataPtr->mesh.reset();
  }

  if (this->dataPtr->material && this->Scene())
  {
    this->Scene()->DestroyMaterial(this->dataPtr->material);
    this->dataPtr->material.reset();
  }
  this->dataPtr->dynamicRenderable.reset();
}

//////////////////////////////////////////////////
Ogre::MovableObject *OgreLidarVisual::OgreObject() const
{
  switch (lidarVisualType)
  {
    case MT_NONE:
      return nullptr;
    case MT_LINE_STRIP:
    case MT_LINE_LIST:
    case MT_POINTS:
    case MT_TRIANGLE_FAN:
    case MT_TRIANGLE_LIST:
    case MT_TRIANGLE_STRIP:
      return std::dynamic_pointer_cast<Ogre::MovableObject>
        (this->dataPtr->dynamicRenderable).get();
    default:
      ignerr << "Invalid Lidar Visual type " << lidarVisualType << "\n";
      return nullptr;
  }
}

//////////////////////////////////////////////////
void OgreLidarVisual::Init()
{
  this->Create();
}

//////////////////////////////////////////////////
void OgreLidarVisual::Create()
{
  this->lidarVisualType = MT_NONE;
  this->dataPtr->dynamicRenderable.reset(new OgreDynamicLines(MT_LINE_STRIP));

  if (!this->dataPtr->mesh)
  {
    this->dataPtr->mesh =
      std::dynamic_pointer_cast<OgreMesh>(this->scene->CreateBox());
  }
}

//////////////////////////////////////////////////
void OgreLidarVisual::SetMaterial(MaterialPtr _material, bool _unique)
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
  std::string materialName = derived->Name();
  Ogre::MaterialPtr ogreMaterial = derived->Material();
  this->dataPtr->material = derived;

  this->dataPtr->material->SetReceiveShadows(false);
  this->dataPtr->material->SetCastShadows(false);
  this->dataPtr->material->SetLightingEnabled(false);

  switch (this->lidarVisualType)
  {
    case MT_NONE:
      break;
    case MT_LINE_STRIP:
    case MT_LINE_LIST:
    case MT_POINTS:
    case MT_TRIANGLE_FAN:
    case MT_TRIANGLE_LIST:
    case MT_TRIANGLE_STRIP:
#if (OGRE_VERSION <= ((1 << 16) | (10 << 8) | 7))
      this->dataPtr->dynamicRenderable->setMaterial(materialName);
#else
      this->dataPtr->dynamicRenderable->setMaterial(ogreMaterial);
#endif
      break;
    default:
      ignerr << "Invalid Lidar Visual type " << this->lidarVisualType << "\n";
      break;
  }
}

//////////////////////////////////////////////////
MaterialPtr OgreLidarVisual::Material() const
{
  return this->dataPtr->material;
}

//////////////////////////////////////////////////
void OgreLidarVisual::SetPoint(unsigned int _index,
    const ignition::math::Vector3d &_value)
{
  this->dataPtr->dynamicRenderable->SetPoint(_index, _value);
}

//////////////////////////////////////////////////
void OgreLidarVisual::AddPoint(const ignition::math::Vector3d &_pt,
    const ignition::math::Color &_color)
{
  this->dataPtr->dynamicRenderable->AddPoint(_pt, _color);
}

//////////////////////////////////////////////////
void OgreLidarVisual::ClearPoints()
{
  this->dataPtr->dynamicRenderable->Clear();
}

//////////////////////////////////////////////////
void OgreLidarVisual::SetType(LidarVisualType _lidarVisualType)
{
  this->lidarVisualType = _lidarVisualType;
  switch (_lidarVisualType)
  {
    case MT_NONE:
      break;
    case MT_LINE_STRIP:
    case MT_LINE_LIST:
    case MT_POINTS:
    case MT_TRIANGLE_FAN:
    case MT_TRIANGLE_LIST:
    case MT_TRIANGLE_STRIP:
      this->dataPtr->dynamicRenderable->SetOperationType(_lidarVisualType);
      break;
    default:
      ignerr << "Invalid Lidar Visual type\n";
      break;
  }
}

//////////////////////////////////////////////////
LidarVisualType OgreLidarVisual::Type() const
{
  return this->lidarVisualType;
}
