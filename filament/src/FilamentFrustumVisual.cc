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

#include "gz/rendering/filament/FilamentFrustumVisual.hh"
#include "gz/rendering/filament/FilamentScene.hh"

using namespace gz;
using namespace rendering;

//////////////////////////////////////////////////
FilamentFrustumVisual::FilamentFrustumVisual() = default;

//////////////////////////////////////////////////
FilamentFrustumVisual::~FilamentFrustumVisual() = default;

//////////////////////////////////////////////////
void FilamentFrustumVisual::Destroy()
{
  this->ClearVisualData();
  BaseFrustumVisual::Destroy();
}

//////////////////////////////////////////////////
void FilamentFrustumVisual::ClearVisualData()
{
  if (this->rayLines)
  {
    this->rayLines->Destroy();
    this->rayLines.reset();
  }
}

//////////////////////////////////////////////////
void FilamentFrustumVisual::Update()
{
  if (!this->rayLines)
  {
    this->rayLines = this->scene->CreateDynamicMesh();
    this->rayLines->SetOperationType(MT_LINE_LIST);
    this->rayLines->SetLineMaterial(this->Scene()->Material("Frustum/BlueRay"));
    this->AddGeometry(this->rayLines);
  }
  else
  {
    this->rayLines->Clear();
  }

  auto &lines = this->rayLines;

  // Tangent of half the field of view.
  double tanFOV2 = std::tan(this->hfov() * 0.5);

  // Half-width of near plane (distance from optical axis to plane edge).
  double nearWidth = tanFOV2 * this->nearClip;

  // Half-height of near plane.
  double nearHeight = nearWidth / this->aspectRatio;

  // Half-width of far plane.
  double farWidth = tanFOV2 * this->farClip;

  // Half-height of far plane.
  double farHeight = farWidth / this->aspectRatio;

  // Up, right, and forward unit vectors.
  gz::math::Vector3d forward =
    this->pose.Rot().RotateVector(gz::math::Vector3d::UnitX);
  gz::math::Vector3d up =
    this->pose.Rot().RotateVector(gz::math::Vector3d::UnitZ);
  gz::math::Vector3d right =
    this->pose.Rot().RotateVector(-gz::math::Vector3d::UnitY);

  // Near plane center
  gz::math::Vector3d nearCenter = this->pose.Pos() + forward * this->nearClip;

  // Far plane center
  gz::math::Vector3d farCenter = this->pose.Pos() + forward * this->farClip;

  // Half-extent vectors used to offset each plane center to a corner.
  gz::math::Vector3d upNearHeight2 = up * nearHeight;
  gz::math::Vector3d rightNearWidth2 = right * nearWidth;
  gz::math::Vector3d upFarHeight2 = up * farHeight;
  gz::math::Vector3d rightFarWidth2 = right * farWidth;

  // Compute the vertices of the near plane
  gz::math::Vector3d nearTopLeft =
    nearCenter + upNearHeight2 - rightNearWidth2;
  gz::math::Vector3d nearTopRight =
    nearCenter + upNearHeight2 + rightNearWidth2;
  gz::math::Vector3d nearBottomLeft =
    nearCenter - upNearHeight2 - rightNearWidth2;
  gz::math::Vector3d nearBottomRight =
    nearCenter - upNearHeight2 + rightNearWidth2;

  // Compute the vertices of the far plane
  gz::math::Vector3d farTopLeft = farCenter + upFarHeight2 - rightFarWidth2;
  gz::math::Vector3d farTopRight = farCenter + upFarHeight2 + rightFarWidth2;
  gz::math::Vector3d farBottomLeft = farCenter - upFarHeight2 - rightFarWidth2;
  gz::math::Vector3d farBottomRight = farCenter - upFarHeight2 + rightFarWidth2;

  gz::math::Vector3d leftCenter =
    (farTopLeft + nearTopLeft + farBottomLeft + nearBottomLeft) / 4.0;

  gz::math::Vector3d rightCenter =
    (farTopRight + nearTopRight + farBottomRight + nearBottomRight) / 4.0;

  gz::math::Vector3d topCenter =
    (farTopRight + nearTopRight + farTopLeft + nearTopLeft) / 4.0;

  gz::math::Vector3d bottomCenter =
    (farBottomRight + nearBottomRight + farBottomLeft + nearBottomLeft) / 4.0;

  // For creating the frustum visuals
  lines->AddPoint(math::Vector3d(this->nearClip, nearWidth, nearHeight));
  lines->AddPoint(math::Vector3d(this->nearClip, nearWidth, -nearHeight));
  lines->AddPoint(math::Vector3d(this->nearClip, nearWidth, -nearHeight));
  lines->AddPoint(math::Vector3d(this->nearClip, -nearWidth, -nearHeight));
  lines->AddPoint(math::Vector3d(this->nearClip, -nearWidth, -nearHeight));
  lines->AddPoint(math::Vector3d(this->nearClip, -nearWidth, nearHeight));
  lines->AddPoint(math::Vector3d(this->nearClip, -nearWidth, nearHeight));
  lines->AddPoint(math::Vector3d(this->nearClip, nearWidth, nearHeight));
  lines->AddPoint(math::Vector3d(this->farClip, farWidth, farHeight));
  lines->AddPoint(math::Vector3d(this->farClip, farWidth, -farHeight));
  lines->AddPoint(math::Vector3d(this->farClip, farWidth, -farHeight));
  lines->AddPoint(math::Vector3d(this->farClip, -farWidth, -farHeight));
  lines->AddPoint(math::Vector3d(this->farClip, -farWidth, -farHeight));
  lines->AddPoint(math::Vector3d(this->farClip, -farWidth, farHeight));
  lines->AddPoint(math::Vector3d(this->farClip, -farWidth, farHeight));
  lines->AddPoint(math::Vector3d(this->farClip, farWidth, farHeight));
  lines->AddPoint(math::Vector3d(this->nearClip, nearWidth, nearHeight));
  lines->AddPoint(math::Vector3d(this->farClip, farWidth, farHeight));
  lines->AddPoint(math::Vector3d(this->nearClip, -nearWidth, nearHeight));
  lines->AddPoint(math::Vector3d(this->farClip, -farWidth, farHeight));
  lines->AddPoint(math::Vector3d(this->nearClip, -nearWidth, -nearHeight));
  lines->AddPoint(math::Vector3d(this->farClip, -farWidth, -farHeight));
  lines->AddPoint(math::Vector3d(this->nearClip, nearWidth, -nearHeight));
  lines->AddPoint(math::Vector3d(this->farClip, farWidth, -farHeight));

  // Compute plane offsets
  // Set the planes, where the first value is the plane normal and the
  // second the plane offset
  gz::math::Vector3d norm = gz::math::Vector3d::Normal(
    nearTopLeft, nearTopRight, nearBottomLeft);
  this->planes[FrustumVisualPlane::FRUSTUM_PLANE_NEAR].Set(
    norm, nearCenter.Dot(norm));

  norm = gz::math::Vector3d::Normal(
    farTopRight, farTopLeft, farBottomLeft);
  this->planes[FrustumVisualPlane::FRUSTUM_PLANE_FAR].Set(
    norm, farCenter.Dot(norm));

  norm = gz::math::Vector3d::Normal(
    farTopLeft, nearTopLeft, nearBottomLeft);
  this->planes[FrustumVisualPlane::FRUSTUM_PLANE_LEFT].Set(
    norm, leftCenter.Dot(norm));

  norm = gz::math::Vector3d::Normal(
    nearTopRight, farTopRight, farBottomRight);
  this->planes[FrustumVisualPlane::FRUSTUM_PLANE_RIGHT].Set(
    norm, rightCenter.Dot(norm));

  norm = gz::math::Vector3d::Normal(
    nearTopLeft, farTopLeft, nearTopRight);
  this->planes[FrustumVisualPlane::FRUSTUM_PLANE_TOP].Set(
    norm, topCenter.Dot(norm));

  norm = gz::math::Vector3d::Normal(
    nearBottomLeft, nearBottomRight, farBottomRight);
  this->planes[FrustumVisualPlane::FRUSTUM_PLANE_BOTTOM].Set(
    norm, bottomCenter.Dot(norm));

}
