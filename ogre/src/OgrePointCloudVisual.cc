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
#include "ignition/rendering/ogre/OgrePointCloudVisual.hh"
#include "ignition/rendering/ogre/OgreScene.hh"
#include "ignition/rendering/ogre/OgreMarker.hh"
#include "ignition/rendering/ogre/OgreGeometry.hh"


class ignition::rendering::OgrePointCloudVisualPrivate
{
  /// \brief The current point cloud data
  public: std::vector<math::Vector3d> points;

  /// \brief True if new points data is received
  public: bool receivedData = false;
};

using namespace ignition;
using namespace rendering;

//////////////////////////////////////////////////
OgrePointCloudVisual::OgrePointCloudVisual()
  : dataPtr(new OgrePointCloudVisualPrivate)
{
}

//////////////////////////////////////////////////
OgrePointCloudVisual::~OgrePointCloudVisual()
{
  this->Destroy();
}

//////////////////////////////////////////////////
void OgrePointCloudVisual::PreRender()
{
  // no ops
}

//////////////////////////////////////////////////
void OgrePointCloudVisual::Destroy()
{
  this->ClearPoints();
}

//////////////////////////////////////////////////
void OgrePointCloudVisual::Init()
{
  BasePointCloudVisual::Init();
  this->Create();
}

//////////////////////////////////////////////////
void OgrePointCloudVisual::Create()
{
  this->ClearPoints();
  this->dataPtr->receivedData = false;
}

//////////////////////////////////////////////////
void OgrePointCloudVisual::ClearPoints()
{
  this->dataPtr->points.clear();
  this->dataPtr->receivedData = false;
}

//////////////////////////////////////////////////
void OgrePointCloudVisual::ClearVisualData()
{
}

//////////////////////////////////////////////////
void OgrePointCloudVisual::SetPoints(const std::vector<math::Vector3d> &_points)
{
  this->dataPtr->points = _points;
  this->dataPtr->receivedData = true;
}

//////////////////////////////////////////////////
void OgrePointCloudVisual::Update()
{
  if (!this->dataPtr->receivedData || this->dataPtr->points.size() == 0)
  {
    ignwarn << "New lidar data not received. Exiting update function"
            << std::endl;
    return;
  }

  this->dataPtr->receivedData = false;
}

//////////////////////////////////////////////////
unsigned int OgrePointCloudVisual::PointCount() const
{
  return this->dataPtr->points.size();
}

//////////////////////////////////////////////////
std::vector<math::Vector3d> OgrePointCloudVisual::Points() const
{
  return this->dataPtr->points;
}
