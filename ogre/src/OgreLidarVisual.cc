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
#include "ignition/rendering/ogre/OgreDynamicLines.hh"
#include "ignition/rendering/ogre/OgreLidarVisual.hh"
#include "ignition/rendering/ogre/OgreScene.hh"
#include "ignition/rendering/ogre/OgreMarker.hh"
#include "ignition/rendering/ogre/OgreGeometry.hh"


class ignition::rendering::OgreLidarVisualPrivate
{
  /// \brief Non Hitting DynamicLines Object to display
  public: std::vector<std::shared_ptr<OgreDynamicLines>> noHitRayStrips;

  /// \brief Hitting DynamicLines Object to display
  public: std::vector<std::shared_ptr<OgreDynamicLines>> rayStrips;

  /// \brief Dead Zone Geometry DynamicLines Object to display
  public: std::vector<std::shared_ptr<OgreDynamicLines>> deadZoneRayFans;

  /// \brief Lidar Ray DynamicLines Object to display
  public: std::vector<std::shared_ptr<OgreDynamicLines>> rayLines;

  /// \brief The current lidar data message
  public: std::vector<double> laserMsg;

  /// \brief True if new message is received
  public: bool receivedMsg;
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
  // no ops
}

//////////////////////////////////////////////////
void OgreLidarVisual::Destroy()
{
  for (auto ray : this->dataPtr->noHitRayStrips)
  {
    ray.get()->Clear();
    ray.reset();
  }

  for (auto ray : this->dataPtr->rayStrips)
  {
    ray.get()->Clear();
    ray.reset();
  }

  for (auto ray : this->dataPtr->rayLines)
  {
    ray.get()->Clear();
    ray.reset();
  }

  for (auto ray : this->dataPtr->deadZoneRayFans)
  {
    ray.get()->Clear();
    ray.reset();
  }

  this->ClearPoints();
}

//////////////////////////////////////////////////
void OgreLidarVisual::Init()
{
  BaseLidarVisual::Init();
  this->Create();
}

//////////////////////////////////////////////////
void OgreLidarVisual::Create()
{
  this->ClearPoints();
  this->dataPtr->receivedMsg = false;
}

//////////////////////////////////////////////////
void OgreLidarVisual::ClearPoints()
{
  this->dataPtr->noHitRayStrips.clear();
  this->dataPtr->deadZoneRayFans.clear();
  this->dataPtr->rayLines.clear();
  this->dataPtr->rayStrips.clear();
  this->dataPtr->laserMsg.clear();
  this->dataPtr->receivedMsg = false;
}


//////////////////////////////////////////////////
void OgreLidarVisual::SetLidarMessage(std::vector<double> &_msg)
{
  this->dataPtr->laserMsg.clear();
  this->dataPtr->laserMsg = _msg;
  this->dataPtr->receivedMsg = true;
  
}

//////////////////////////////////////////////////
void OgreLidarVisual::Update()
{
  if(!this->dataPtr->receivedMsg || this->dataPtr->laserMsg.size() == 0)
  {
    ignerr << "Message not received. Exiting Updat function"
           << std::endl;
    return;
  }

  this->dataPtr->receivedMsg = false;
  double horizontalAngle = this->minHorizontalAngle;
  double verticalAngle = this->minVerticalAngle;

  if (this->dataPtr->laserMsg.size() != this->verticalCount * this->horizontalCount)
  {
    ignerr << "Size of laser message inconsistent with ray count"
           << std::endl;
    return;
  }

  // Process each point from message
  // Every line segment, and every individual triangle is saved separately as a pointer to a DynamicLine
  for (unsigned int j = 0; j < this->verticalCount; ++j)
  {

    horizontalAngle = this->minHorizontalAngle;

    if (j+1 > this->dataPtr->rayLines.size())
    {
      // Ray Strips fill in between the line areas that have intersected an object
      std::shared_ptr<OgreDynamicLines> line = std::shared_ptr<OgreDynamicLines>(new OgreDynamicLines(MT_TRIANGLE_STRIP));
      line->setMaterial("Lidar/BlueLaser");
      std::shared_ptr<Ogre::MovableObject> mv = std::dynamic_pointer_cast<Ogre::MovableObject>(line);
      this->Node()->attachObject(mv.get());
      this->dataPtr->rayStrips.push_back(line);

      line = std::shared_ptr<OgreDynamicLines>(new OgreDynamicLines(MT_TRIANGLE_STRIP));
      line->setMaterial("Lidar/LightBlueLaser");
      mv = std::dynamic_pointer_cast<Ogre::MovableObject>(line);
      this->Node()->attachObject(mv.get());
      this->dataPtr->noHitRayStrips.push_back(line);

      line = std::shared_ptr<OgreDynamicLines>(new OgreDynamicLines(MT_TRIANGLE_FAN));
      line->setMaterial("Lidar/BlackTransparent");      
      mv = std::dynamic_pointer_cast<Ogre::MovableObject>(line);
      this->Node()->attachObject(mv.get());
      this->dataPtr->deadZoneRayFans.push_back(line);
      this->dataPtr->deadZoneRayFans[j]->AddPoint(ignition::math::Vector3d::Zero);


      line = std::shared_ptr<OgreDynamicLines>(new OgreDynamicLines(MT_LINE_LIST));
      line->setMaterial("Lidar/Blue");      
      mv = std::dynamic_pointer_cast<Ogre::MovableObject>(line);
      this->Node()->attachObject(mv.get());
      this->dataPtr->rayLines.push_back(line);

      this->SetVisibilityFlags(0x0FFFFFFF);
    }
    this->dataPtr->deadZoneRayFans[j]->SetPoint(0, this->offset.Pos());


    unsigned count = this->horizontalCount;
    // Process each ray in current scan
    for (unsigned int i = 0; i < count; ++i)
    {
      //calculate range of the ray
      double r = this->dataPtr->laserMsg[j * this->verticalCount + i];

      if (verticalAngle > this->maxVerticalAngle)
      {
        ignerr << "Vertical angle exceeds maximum limits. Please check the input"
           << std::endl;
      }

      if (verticalAngle < this->minVerticalAngle)
      {
        ignerr << "Vertical angle less than minimum limits. Please check the input"
           << std::endl;
      }

      if (horizontalAngle < this->minHorizontalAngle)
      {
        ignerr << "Horizontal angle less than minimum limits. Please check the input"
           << std::endl;
      }

      if (horizontalAngle > this->maxHorizontalAngle)
      {
        ignerr << "Horizontal angle exceeds maximum limits. Please check the input"
           << std::endl;
      }

      bool inf = std::isinf(r);
      ignition::math::Quaterniond ray(
        ignition::math::Vector3d(0.0, -verticalAngle, horizontalAngle));

      ignition::math::Vector3d axis = this->offset.Rot() * ray *
        ignition::math::Vector3d(1.0, 0.0, 0.0);

      // Check for infinite range, which indicates the ray did not
      // intersect an object.
      double hitRange = inf ? 0 : r;

      // Compute the start point of the ray
      ignition::math::Vector3d startPt = (axis * minRange) + this->offset.Pos();

      // Compute the end point of the ray
      ignition::math::Vector3d pt = (axis * hitRange) + this->offset.Pos();

      double noHitRange = inf ? maxRange : hitRange;

      // Compute the end point of the no-hit ray
      ignition::math::Vector3d noHitPt = (axis * noHitRange) + this->offset.Pos();

      // Draw the lines and strips that represent each simulated ray
      if (i >= this->dataPtr->rayLines[j]->PointCount()/2)
      {
        this->dataPtr->rayLines[j]->AddPoint(startPt);
        this->dataPtr->rayLines[j]->AddPoint(inf ? noHitPt : pt);
        
        this->dataPtr->rayStrips[j]->AddPoint(startPt);
        this->dataPtr->rayStrips[j]->AddPoint(inf ? startPt : pt);

        this->dataPtr->noHitRayStrips[j]->AddPoint(startPt);
        this->dataPtr->noHitRayStrips[j]->AddPoint(inf ? noHitPt : pt);
      }
      else
      {
        this->dataPtr->rayLines[j]->SetPoint(i*2, startPt);
        this->dataPtr->rayLines[j]->SetPoint(i*2+1, inf ? noHitPt : pt);
        
        this->dataPtr->rayStrips[j]->SetPoint(i*2, startPt);
        this->dataPtr->rayStrips[j]->SetPoint(i*2+1, inf ? startPt : pt);
        
        this->dataPtr->noHitRayStrips[j]->SetPoint(i*2, startPt);
        this->dataPtr->noHitRayStrips[j]->SetPoint(i*2+1, inf ? noHitPt : pt);
      }

      // Draw the triangle fan that indicates the dead zone.
      if (i+1 >= this->dataPtr->deadZoneRayFans[j]->PointCount())
        this->dataPtr->deadZoneRayFans[j]->AddPoint(startPt);
      else
        this->dataPtr->deadZoneRayFans[j]->SetPoint(i+1, startPt);
      
      // Update all the DynamicLines after adding points
      this->dataPtr->rayLines[j]->Update();
      this->dataPtr->rayStrips[j]->Update();
      this->dataPtr->noHitRayStrips[j]->Update();
      this->dataPtr->deadZoneRayFans[j]->Update();

      horizontalAngle += this->horizontalAngleStep;
    }
    verticalAngle += this->verticalAngleStep;
  }
}

//////////////////////////////////////////////////
unsigned int OgreLidarVisual::GetPointCount()
{
  return this->dataPtr->laserMsg.size();
}
