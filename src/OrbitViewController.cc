/*
 * Copyright (C) 2017 Open Source Robotics Foundation
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

#include "ignition/rendering/OrbitViewController.hh"
#include "ignition/rendering/Scene.hh"
#include "ignition/rendering/Visual.hh"

class ignition::rendering::OrbitViewControllerPrivate
{
  /// \brief Pointer to camera
  public: CameraPtr camera;

  /// \brief Target point for camera movements
  public: math::Vector3d target;

  /// \brief Keep yaw within limits
  /// \return Normalized yaw
  public: double NormalizeYaw(double _yaw);

  /// \brief Keep pitch within limits
  /// \return Normalized pitch
  public: double NormalizePitch(double _pitch);
};

using namespace ignition;
using namespace rendering;

static const float PITCH_LIMIT_LOW = -static_cast<float>(IGN_PI)*0.5f + 0.001f;
static const float PITCH_LIMIT_HIGH = static_cast<float>(IGN_PI)*0.5f - 0.001f;

//////////////////////////////////////////////////
OrbitViewController::OrbitViewController()
    : dataPtr(new OrbitViewControllerPrivate)
{
}

//////////////////////////////////////////////////
OrbitViewController::OrbitViewController(const CameraPtr &_camera)
    : dataPtr(new OrbitViewControllerPrivate)
{
  this->dataPtr->camera = _camera;
}

//////////////////////////////////////////////////
OrbitViewController::~OrbitViewController()
{
}

//////////////////////////////////////////////////
void OrbitViewController::SetCamera(const CameraPtr &_camera)
{
  this->dataPtr->camera = _camera;
}

//////////////////////////////////////////////////
CameraPtr OrbitViewController::Camera() const
{
  return this->dataPtr->camera;
}

//////////////////////////////////////////////////
void OrbitViewController::SetTarget(const math::Vector3d &_target)
{
  this->dataPtr->target = _target;
}

//////////////////////////////////////////////////
math::Vector3d OrbitViewController::Target() const
{
  return this->dataPtr->target;
}

//////////////////////////////////////////////////
void OrbitViewController::Zoom(const double _value)
{
  if (!this->dataPtr->camera)
  {
    ignerr << "Camera is NULL" << std::endl;
    return;
  }

  double distance =
      this->dataPtr->camera->WorldPose().Pos().Distance(this->dataPtr->target);

  distance -= _value;

  math::Vector3d delta = this->dataPtr->camera->WorldPosition() -
    this->dataPtr->target;
  delta.Normalize();
  delta *= distance;
  this->dataPtr->camera->SetWorldPosition(this->dataPtr->target + delta);
}

//////////////////////////////////////////////////
void OrbitViewController::Pan(const math::Vector2d &_value)
{
  if (!this->dataPtr->camera)
  {
    ignerr << "Camera is NULL" << std::endl;
    return;
  }

  double viewportWidth = this->dataPtr->camera->ImageWidth();
  double viewportHeight = this->dataPtr->camera->ImageHeight();

  double distance =
      this->dataPtr->camera->WorldPosition().Distance(this->dataPtr->target);
  double hfov = this->dataPtr->camera->HFOV().Radian();
  double vfov = 2.0f * atan(tan(hfov / 2.0f) /
        this->dataPtr->camera->AspectRatio());

  ignition::math::Vector3d translation;

  double factor = 2.0;

  // Translate in the "y" "z" plane.
  translation.Set(0.0,
      (_value.X() / static_cast<double>(viewportWidth)) *
      distance * tan(hfov / 2.0) * factor,
      (_value.Y() / static_cast<double>(viewportHeight)) *
      distance * tan(vfov / 2.0) * factor);

  // Translate in the local coordinate frame
  this->dataPtr->camera->SetWorldPosition(
      this->dataPtr->camera->WorldPosition() +
      this->dataPtr->camera->WorldRotation() * translation);
}

//////////////////////////////////////////////////
void OrbitViewController::Orbit(const math::Vector2d &_value)
{
  if (!this->dataPtr->camera)
  {
    ignerr << "Camera is NULL" << std::endl;
    return;
  }

  double dy = 2 * IGN_PI * _value.X() / this->dataPtr->camera->ImageWidth();
  double dp = 2 * IGN_PI * _value.Y() / this->dataPtr->camera->ImageHeight();

  // translate to make target the origin for rotation
  this->dataPtr->camera->SetWorldPosition(
      this->dataPtr->camera->WorldPosition() - this->dataPtr->target);

  // rotate around world axis at target point
  math::Quaterniond yawQuat;
  yawQuat.Axis(math::Vector3d::UnitZ, -dy);
  this->dataPtr->camera->SetWorldRotation(
      yawQuat * this->dataPtr->camera->WorldRotation());
  this->dataPtr->camera->SetWorldPosition(
      yawQuat * this->dataPtr->camera->WorldPosition());

  math::Quaterniond localPitchQuat;
  localPitchQuat.Axis(
      this->dataPtr->camera->WorldRotation()*math::Vector3d::UnitY, dp);
  this->dataPtr->camera->SetWorldRotation(
      localPitchQuat * this->dataPtr->camera->WorldRotation());
  this->dataPtr->camera->SetWorldPosition(
      localPitchQuat * this->dataPtr->camera->WorldPosition());

  // translate camera back
  this->dataPtr->camera->SetWorldPosition(
      this->dataPtr->camera->WorldPosition() + this->dataPtr->target);
}

//////////////////////////////////////////////////
double OrbitViewControllerPrivate::NormalizeYaw(double _yaw)
{
  _yaw = fmod(_yaw, IGN_PI*2);
  if (_yaw < 0.0f)
  {
    _yaw = IGN_PI * 2 + _yaw;
  }

  return _yaw;
}

//////////////////////////////////////////////////
double OrbitViewControllerPrivate::NormalizePitch(double _pitch)
{
  if (_pitch < PITCH_LIMIT_LOW)
    _pitch = PITCH_LIMIT_LOW;
  else if (_pitch > PITCH_LIMIT_HIGH)
    _pitch = PITCH_LIMIT_HIGH;

  return _pitch;
}
