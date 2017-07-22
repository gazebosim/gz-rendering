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

#include <sstream>

#include "ignition/rendering/base/BaseRayQuery.hh"

using namespace ignition;
using namespace rendering;

//////////////////////////////////////////////////
BaseRayQuery::BaseRayQuery(const math::Vector3d &_origin,
    const ignition::math::Vector3d &_dir)
{
  this->origin = _origin;
  this->direction = _dir;
}

//////////////////////////////////////////////////
BaseRayQuery::~BaseRayQuery()
{
}

//////////////////////////////////////////////////
void BaseRayQuery::SetOrigin(const math::Vector3d &_origin)
{
  this->origin = _origin;
}

//////////////////////////////////////////////////
void BaseRayQuery::SetDirection(const math::Vector3d &_dir)
{
  this->direction = _dir;
}

//////////////////////////////////////////////////
void BaseRayQuery::SetFromCamera(CameraPtr _camera,
    const ignition::math::Vector2d &_coord)
{
  math::Matrix4d projectionMatrix = _camera->ProjectionMatrix();
  math::Matrix4d viewMatrix = math::Matrix4d(_camera->WorldPose());
  math::Vector3d start(_coord.X(), _coord.Y(), -1.0);
  math::Vector3d end(_coord.X(), _coord.Y(), 0.0);
  math::Matrix4d viewProjInv = (projectionMatrix * viewMatrix).Inverse();
  start = viewProjInv * start;
  end = viewProjInv * end;

  this->origin =  start;
  this->direction = (end - start).Normalize();
}

//////////////////////////////////////////////////
bool BaseRayQuery::Intersect(const ScenePtr &/*_scene*/,
    std::vector<RayQueryResult> &/*_results*/)
{
  // TODO implement here?
  return false;
}

