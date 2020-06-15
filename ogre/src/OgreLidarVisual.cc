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
  public: std::vector<OgreDynamicLines *> noHitRayStrips;

  /// \brief Hitting DynamicLines Object to display
  public: std::vector<OgreDynamicLines *> rayStrips;

  /// \brief Dead Zone Geometry DynamicLines Object to display
  public: std::vector<OgreDynamicLines *> deadZoneRayFans;

  /// \brief Lidar Ray DynamicLines Object to display
  public: std::vector<OgreDynamicLines *> rayLines;

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
  //no ops
}

//////////////////////////////////////////////////
void OgreLidarVisual::Destroy()
{
  
  for (auto ray : this->dataPtr->noHitRayStrips)
  {
    ray->Clear();
    delete ray;
  }

  for (auto ray : this->dataPtr->rayStrips)
  {
    ray->Clear();
    delete ray;
  }

  for (auto ray : this->dataPtr->rayLines)
  {
    ray->Clear();
    delete ray;
  }

  for (auto ray : this->dataPtr->deadZoneRayFans)
  {
    ray->Clear();
    delete ray;
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
}


//////////////////////////////////////////////////
void OgreLidarVisual::OnMsg(std::vector<double> &_msg)
{
  this->dataPtr->laserMsg = _msg;
  this->dataPtr->receivedMsg = true;
  this->Update();
  this->PrintStatus();
}

//////////////////////////////////////////////////
void OgreLidarVisual::Update()
{

  // TODO if(!this->dataPtr->laserMsg for checking  is required in the later stage)
  if(!this->dataPtr->receivedMsg)
  {
    return;
  }
  

  this->dataPtr->receivedMsg = false;

  unsigned int vertCount = 1;
  double minRange = 0.03;
  double maxRange = 30;
  double verticalAngle = 0.0;

  ignition::math::Pose3d offset = ignition::math::Pose3d::Zero;

  // Process each point from message
  // Every line segment, and every individual triangle is saved separately as a pointer to a DynamicLine
  for (unsigned int j = 0; j < vertCount; ++j)
  {

    if (j+1 > this->dataPtr->rayStrips.size())
    {
      // Ray Strips fill in between the line areas that have intersected an object
      OgreDynamicLines *line = new OgreDynamicLines(MT_TRIANGLE_STRIP);
      line->setMaterial("Default/TransBlue");
      Ogre::MovableObject *mv = dynamic_cast<Ogre::MovableObject *>(line);
      this->Node()->attachObject(mv);
      this->dataPtr->rayStrips.push_back(line);
      
      
      line = new OgreDynamicLines(MT_TRIANGLE_STRIP);
      line->setMaterial("Default/TransRed");
      mv = dynamic_cast<Ogre::MovableObject *>(line);
      this->Node()->attachObject(mv);
      this->dataPtr->noHitRayStrips.push_back(line);

      
      line = new OgreDynamicLines(MT_TRIANGLE_FAN);
      line->setMaterial("Default/TransGreen");      
      mv = dynamic_cast<Ogre::MovableObject *>(line);
      this->Node()->attachObject(mv);
      this->dataPtr->deadZoneRayFans.push_back(line);
      this->dataPtr->deadZoneRayFans[j]->AddPoint(ignition::math::Vector3d::Zero);
      


      
      line = new OgreDynamicLines(MT_LINE_LIST);
      line->setMaterial("Default/TransYellow");      
      mv = dynamic_cast<Ogre::MovableObject *>(line);
      this->Node()->attachObject(mv);
      this->dataPtr->rayLines.push_back(line);
      

      this->SetVisibilityFlags(0x0FFFFFFF);

    }
    
    this->dataPtr->deadZoneRayFans[j]->SetPoint(0, offset.Pos());

    double angleStep = 0.1;
    double angle = -verticalAngle;


    unsigned count = this->dataPtr->laserMsg.size();
    // Process each ray in current scan
    for (unsigned int i = 0; i < count; ++i)
    {
      //calculate range of the ray
      double r = this->dataPtr->laserMsg[i];
      if ( r < minRange)
      {
        // Less than min range, do not display the ray
        r = minRange;
      }

      bool inf = std::isinf(r);
      
      ignition::math::Quaterniond ray(
        ignition::math::Vector3d(0.0, 0.0, angle));

      ignition::math::Vector3d axis = offset.Rot() * ray *
        ignition::math::Vector3d(1.0, 0.0, 0.0);

      // Check for infinite range, which indicates the ray did not
      // intersect an object.
      double hitRange = inf ? 0 : r;

      // Compute the start point of the ray
      ignition::math::Vector3d startPt = (axis * minRange) + offset.Pos();

      // Compute the end point of the ray
      ignition::math::Vector3d pt = (axis * hitRange) + offset.Pos();

      double noHitRange = inf ? maxRange : hitRange;

      // Compute the end point of the no-hit ray
      ignition::math::Vector3d noHitPt = (axis * noHitRange) + offset.Pos();
      

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

/// HOW TO DO THIS??

      angle += angleStep;
    }
    // verticalAngle += this->dataPtr->laserMsg->scan().vertical_angle_step();
  }

}

void OgreLidarVisual::PrintStatus()
{
  
  std::cerr << "Number of rayStrips" << this->dataPtr->rayStrips.size()<<std::endl;
  for( int i = 0; i < this->dataPtr->rayStrips.size(); ++i)
  {
    std::cerr << i << ") "<< this->dataPtr->rayStrips[i]->PointCount()<<std::endl;
  }



  std::cerr << "Number of rayLines" << this->dataPtr->rayLines.size()<<std::endl;
  for( int i = 0; i < this->dataPtr->rayLines.size(); ++i)
  {
    std::cerr << i << ") "<< this->dataPtr->rayLines[i]->PointCount()<<std::endl;
  }


  std::cerr << "Number of noHitRayStrips" << this->dataPtr->noHitRayStrips.size()<<std::endl;
  for( int i = 0; i < this->dataPtr->noHitRayStrips.size(); ++i)
  {
    std::cerr << i << ") "<< this->dataPtr->noHitRayStrips[i]->PointCount()<<std::endl;
  }



  std::cerr << "Number of deadZoneRayFans" << this->dataPtr->deadZoneRayFans.size()<<std::endl;
  for( int i = 0; i < this->dataPtr->deadZoneRayFans.size(); ++i)
  {
    std::cerr << i << ") "<< this->dataPtr->deadZoneRayFans[i]->PointCount()<<std::endl;
  }


  std::cerr << "Number of objects attached to this node " << this->ogreNode->numAttachedObjects()<<std::endl;

}