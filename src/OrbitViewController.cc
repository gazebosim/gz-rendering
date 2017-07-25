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


#include "ignition/rendering/OrbitViewController.hh"

namespace ignition
{
  namespace rendering
  {
    class OrbitViewControllerPrivate
    {
      /// \brief Pointer to camera
      public: CameraPtr camera;

      /// \brief Target point for camera movements
      public: math::Vector3d target;
    };
  }
}


using namespace ignition;
using namespace rendering;

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
void OrbitViewController::SetTarget(const math::Vector3d &_target)
{
  this->dataPtr->target = _target;
}

//////////////////////////////////////////////////
void OrbitViewController::Zoom(const double _value)
{
}

//////////////////////////////////////////////////
void OrbitViewController::Pan(const math::Vector2d &_value)
{
  double viewportWidth = this->dataPtr->camera->ImageWidth();
  double viewportHeight = this->dataPtr->camera->ImageHeight();

  double distance =
      this->dataPtr->camera->WorldPose().Pos().Distance(this->dataPtr->target);
  double hfov = this->dataPtr->camera->HFOV().Radian();
  double vfov = 2.0f * atan(tan(hfov / 2.0f) *
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
      this->dataPtr->camera->WorldPose().Pos() +
      this->dataPtr->camera->WorldPose().Rot() * translation);
}

//////////////////////////////////////////////////
void OrbitViewController::Orbit(const math::Vector2d &_value)
{
}
