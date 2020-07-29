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

#include "ignition/rendering/ogre2/Ogre2DynamicRenderable.hh"
#include "ignition/rendering/ogre2/Ogre2PointCloudVisual.hh"
#include "ignition/rendering/ogre2/Ogre2Material.hh"
#include "ignition/rendering/ogre2/Ogre2Mesh.hh"
#include "ignition/rendering/ogre2/Ogre2Scene.hh"
#include "ignition/rendering/ogre2/Ogre2Visual.hh"

using namespace ignition;
using namespace rendering;

class ignition::rendering::Ogre2PointCloudVisualPrivate
{
  /// \brief Lidar Points DynamicLines Object to display
  public: std::shared_ptr<Ogre2DynamicRenderable> points;

  public: std::vector<math::Vector3d> data;
};

//////////////////////////////////////////////////
Ogre2PointCloudVisual::Ogre2PointCloudVisual()
  : dataPtr(new Ogre2PointCloudVisualPrivate)
{
  std::cout << "Ogre2PointCloudVisual\n";
}

//////////////////////////////////////////////////
Ogre2PointCloudVisual::~Ogre2PointCloudVisual()
{
    // no ops
}

//////////////////////////////////////////////////
void Ogre2PointCloudVisual::Init()
{
  std::cout << "INit\n";
  BasePointCloudVisual::Init();
}

//////////////////////////////////////////////////
void Ogre2PointCloudVisual::PreRender()
{
    // no ops
}

//////////////////////////////////////////////////
void Ogre2PointCloudVisual::Destroy()
{
    // no ops
}

//////////////////////////////////////////////////
void Ogre2PointCloudVisual::SetPoints(
    const std::vector<math::Vector3d> &_points)
{
  this->dataPtr->data = _points;
}

//////////////////////////////////////////////////
void Ogre2PointCloudVisual::Update()
{
  if (!this->dataPtr->points)
  {
    this->dataPtr->points = std::shared_ptr<Ogre2DynamicRenderable>(
        new Ogre2DynamicRenderable(this->Scene()));

    this->dataPtr->points->SetOperationType(MT_POINTS);
    MaterialPtr mat = this->Scene()->Material("Lidar/BlueRay");
    this->dataPtr->points->SetMaterial(mat, true);

    this->ogreNode->attachObject(this->dataPtr->points->OgreObject());
  }

  if (this->dataPtr->data.size() != this->dataPtr->points->PointCount())
  {
    for (size_t i = 0; i < this->dataPtr->data.size(); ++i)
    {
      this->dataPtr->points->AddPoint(this->dataPtr->data[i]);
    }
  }
}

//////////////////////////////////////////////////
std::vector<math::Vector3d> Ogre2PointCloudVisual::Points() const
{
  return this->dataPtr->data;
}

