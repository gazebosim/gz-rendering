/*
 * Copyright (C) 2026 Open Source Robotics Foundation
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

#include <gz/common/Console.hh>

#include "gz/rendering/filament/FilamentLidarVisual.hh"
#include "gz/rendering/filament/FilamentScene.hh"

using namespace gz;
using namespace rendering;

//////////////////////////////////////////////////
FilamentLidarVisual::FilamentLidarVisual() = default;

//////////////////////////////////////////////////
FilamentLidarVisual::~FilamentLidarVisual() = default;

//////////////////////////////////////////////////
void FilamentLidarVisual::Init()
{
  BaseLidarVisual::Init();
  this->ClearPoints();
}

//////////////////////////////////////////////////
void FilamentLidarVisual::Destroy()
{
  this->ClearVisualData();
  this->lidarPoints.clear();
  BaseLidarVisual::Destroy();
}

//////////////////////////////////////////////////
std::shared_ptr<FilamentDynamicMesh> FilamentLidarVisual::NewMesh(
    MarkerType _type, const std::string &_material)
{
  auto mesh = this->scene->CreateDynamicMesh();
  mesh->SetOperationType(_type);
  if (!_material.empty())
    mesh->SetLineMaterial(this->Scene()->Material(_material));
  this->AddGeometry(mesh);
  return mesh;
}

//////////////////////////////////////////////////
void FilamentLidarVisual::ClearPoints()
{
  this->lidarPoints.clear();
  this->ClearVisualData();
  this->receivedData = false;
}

//////////////////////////////////////////////////
void FilamentLidarVisual::ClearVisualData()
{
  for (auto *list : {&this->noHitRayStrips, &this->deadZoneRayFans,
      &this->rayLines, &this->rayStrips, &this->points})
  {
    for (auto &mesh : *list)
      mesh->Destroy();
    list->clear();
  }
}

//////////////////////////////////////////////////
void FilamentLidarVisual::SetPoints(const std::vector<double> &_points)
{
  this->lidarPoints = _points;
  this->receivedData = true;
}

//////////////////////////////////////////////////
unsigned int FilamentLidarVisual::PointCount() const
{
  return this->lidarPoints.size();
}

//////////////////////////////////////////////////
std::vector<double> FilamentLidarVisual::Points() const
{
  return this->lidarPoints;
}

//////////////////////////////////////////////////
void FilamentLidarVisual::Update()
{
  if (this->lidarVisualType == LidarVisualType::LVT_NONE)
  {
    this->ClearVisualData();
    return;
  }

  if (!this->receivedData || this->lidarPoints.size() == 0)
  {
    gzwarn << "New lidar data not received. Exiting update function"
            << std::endl;
    return;
  }

  bool clearVisuals = false;

  if (this->lidarVisualType != this->lidarVisType
        || !this->displayNonHitting)
  {
    clearVisuals = true;
  }

  if (this->displayNonHitting != this->currentDisplayNonHitting)
  {
    clearVisuals = true;
    this->currentDisplayNonHitting = this->displayNonHitting;
  }

  // if visual type is changed, clear all DynamicLines
  if (clearVisuals)
  {
    this->ClearVisualData();
  }
  this->lidarVisType = this->lidarVisualType;

  this->receivedData = false;
  double horizontalAngle = this->minHorizontalAngle;
  double verticalAngle = this->minVerticalAngle;

  if (this->horizontalCount > 1)
  {
    this->horizontalAngleStep =
        (this->maxHorizontalAngle - this->minHorizontalAngle) /
              (this->horizontalCount - 1);
  }

  if (this->verticalCount > 1)
  {
    this->verticalAngleStep =
        (this->maxVerticalAngle - this->minVerticalAngle) /
              (this->verticalCount - 1);
  }

  if (this->lidarPoints.size() !=
                  this->verticalCount * this->horizontalCount)
  {
    gzwarn << "Size of lidar data inconsistent with rays."
            << " Exiting update function."
            << std::endl;
    return;
  }

  // Process each point from received data
  // Every line segment, and every triangle is saved separately,
  // as a pointer to a DynamicLine
  // This initializes and updates only the selected DynamicLine variables
  for (unsigned int j = 0; j < this->verticalCount; ++j)
  {
    horizontalAngle = this->minHorizontalAngle;

    if (this->lidarVisType == LidarVisualType::LVT_RAY_LINES ||
        this->lidarVisType == LidarVisualType::LVT_TRIANGLE_STRIPS)
    {
      if (j + 1 > this->rayLines.size())
      {
        this->rayLines.push_back(this->NewMesh(MT_LINE_LIST, "Lidar/BlueRay"));
        if (this->lidarVisType == LidarVisualType::LVT_TRIANGLE_STRIPS)
        {
          this->noHitRayStrips.push_back(
              this->NewMesh(MT_TRIANGLE_STRIP, "Lidar/LightBlueStrips"));
          this->deadZoneRayFans.push_back(
              this->NewMesh(MT_TRIANGLE_FAN, "Lidar/TransBlack"));
          this->deadZoneRayFans[j]->AddPoint(math::Vector3d::Zero);
          this->rayStrips.push_back(
              this->NewMesh(MT_TRIANGLE_STRIP, "Lidar/BlueStrips"));
        }
      }
    }
    else if (this->lidarVisType == LidarVisualType::LVT_POINTS)
    {
      if (j + 1 > this->points.size())
        this->points.push_back(this->NewMesh(MT_POINTS, ""));
    }

    if (this->lidarVisType == LidarVisualType::LVT_TRIANGLE_STRIPS)
    {
      this->deadZoneRayFans[j]->SetPoint(0, this->offset.Pos());
    }

    auto pointMat = this->Scene()->Material("Lidar/BlueRay");

    unsigned count = this->horizontalCount;
    // Process each ray in current scan
    for (unsigned int i = 0; i < count; ++i)
    {
      // calculate range of the ray
      double r = this->lidarPoints[ j * this->horizontalCount + i];

      bool inf = (std::isinf(r) || r >= this->maxRange);
      gz::math::Quaterniond ray(
        gz::math::Vector3d(0.0, -verticalAngle, horizontalAngle));

      gz::math::Vector3d axis = this->offset.Rot() * ray *
        gz::math::Vector3d(1.0, 0.0, 0.0);

      // Check for infinite range, which indicates the ray did not
      // intersect an object.
      double hitRange = inf ? 0 : r;

      // Compute the start point of the ray
      gz::math::Vector3d startPt =
                  (axis * this->minRange) + this->offset.Pos();

      // Compute the end point of the ray
      gz::math::Vector3d pt =
                  (axis * hitRange) + this->offset.Pos();

      double noHitRange = inf ? this->maxRange : hitRange;

      // Compute the end point of the no-hit ray
      gz::math::Vector3d noHitPt =
                  (axis * noHitRange) + this->offset.Pos();

      // Update the lines and strips that represent each simulated ray.

      // For TRIANGLE_STRIPS Lidar Visual to be displayed
      if (this->lidarVisType == LidarVisualType::LVT_TRIANGLE_STRIPS
        || this->lidarVisType == LidarVisualType::LVT_RAY_LINES)
      {
        if (i >= this->rayLines[j]->PointCount()/2)
        {
          if (this->displayNonHitting || !inf)
          {
            this->rayLines[j]->AddPoint(startPt);
            this->rayLines[j]->AddPoint(inf ? noHitPt: pt);
          }

          if (this->lidarVisType ==
                LidarVisualType::LVT_TRIANGLE_STRIPS)
          {
            this->rayStrips[j]->AddPoint(startPt);
            this->rayStrips[j]->AddPoint(inf ? startPt : pt);

            this->noHitRayStrips[j]->AddPoint(startPt);
            this->noHitRayStrips[j]->AddPoint(
                inf ? (this->displayNonHitting? noHitPt: startPt) : pt);
          }
        }
        else
        {
          if (this->displayNonHitting || !inf)
          {
            this->rayLines[j]->SetPoint(i*2, startPt);
            this->rayLines[j]->SetPoint(i*2+1, inf ? noHitPt: pt);
          }

          if (this->lidarVisType ==
                LidarVisualType::LVT_TRIANGLE_STRIPS)
          {
            this->rayStrips[j]->SetPoint(i*2, startPt);
            this->rayStrips[j]->SetPoint(i*2+1, inf? startPt : pt);

            this->noHitRayStrips[j]->SetPoint(i*2, startPt);
            this->noHitRayStrips[j]->SetPoint(i*2+1,
                inf ? (this->displayNonHitting? noHitPt: startPt) : pt);
          }
        }
        if (this->lidarVisType == LidarVisualType::LVT_TRIANGLE_STRIPS)
        {
          // Draw the triangle fan that indicates the dead zone.
          if (i+1 >= this->deadZoneRayFans[j]->PointCount())
            this->deadZoneRayFans[j]->AddPoint(startPt);
          else
            this->deadZoneRayFans[j]->SetPoint(i+1, startPt);
        }
      }

      // For POINTS Lidar Visual to be displayed
      else if (this->lidarVisType ==
                LidarVisualType::LVT_POINTS)
      {
        if (i >= this->points[j]->PointCount())
        {
          if (this->displayNonHitting || !inf)
          {
            this->points[j]->AddPoint(inf ? noHitPt : pt,
                pointMat->Diffuse());
          }
        }
        else
        {
          if (this->displayNonHitting || !inf)
          {
            this->points[j]->SetPoint(i, inf ? noHitPt : pt);
            this->points[j]->SetColor(i, pointMat->Diffuse());
          }
        }
      }
      horizontalAngle += this->horizontalAngleStep;
    }

    verticalAngle += this->verticalAngleStep;
  }


  if (this->lidarVisType == LidarVisualType::LVT_POINTS)
  {
    for (auto &mesh : this->points)
      mesh->SetPointSize(this->size);
  }
}
