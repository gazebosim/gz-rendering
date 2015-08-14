/*
 * Copyright (C) 2015 Open Source Robotics Foundation
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
#include "ignition/rendering/optix/OptixNode.hh"
#include "ignition/rendering/optix/OptixConversions.hh"
#include "ignition/rendering/optix/OptixIncludes.hh"
#include "ignition/rendering/optix/OptixScene.hh"
#include "ignition/rendering/optix/OptixVisual.hh"

#include "gazebo/common/Console.hh"

using namespace ignition;
using namespace rendering;

//////////////////////////////////////////////////
OptixNode::OptixNode() :
  poseDirty(true)
{
  this->pose = gazebo::math::Matrix4::IDENTITY.GetAsPose();
}

//////////////////////////////////////////////////
OptixNode::~OptixNode()
{
}

//////////////////////////////////////////////////
bool OptixNode::HasParent() const
{
  return this->parent != NULL;
}

//////////////////////////////////////////////////
VisualPtr OptixNode::GetParent() const
{
  return this->parent;
}

//////////////////////////////////////////////////
void OptixNode::PreRender()
{
  BaseNode::PreRender();
  this->WritePoseToDevice();
  this->optixAccel->markDirty();
}

//////////////////////////////////////////////////
optix::Transform OptixNode::GetOptixTransform() const
{
  return this->optixTransform;
}

//////////////////////////////////////////////////
gazebo::math::Pose OptixNode::GetRawLocalPose() const
{
  return this->pose;
}

//////////////////////////////////////////////////
void OptixNode::SetRawLocalPose(const gazebo::math::Pose &_pose)
{
  this->pose = _pose;
  this->poseDirty = true;
}

//////////////////////////////////////////////////
void OptixNode::WritePoseToDevice()
{
  if (this->poseDirty)
  {
    this->WritePoseToDeviceImpl();
    this->poseDirty = false;
  }
}

//////////////////////////////////////////////////
void OptixNode::WritePoseToDeviceImpl()
{
  float matrix[16];
  gazebo::math::Vector3 position = this->pose.pos;
  gazebo::math::Matrix3 rotation = this->pose.rot.GetAsMatrix3();

  // assign row 1
  matrix[ 0] = rotation[0][0];
  matrix[ 1] = rotation[0][1];
  matrix[ 2] = rotation[0][2];
  matrix[ 3] = position.x;

  // assign row 2
  matrix[ 4] = rotation[1][0];
  matrix[ 5] = rotation[1][1];
  matrix[ 6] = rotation[1][2];
  matrix[ 7] = position.y;

  // assign row 3
  matrix[ 8] = rotation[2][0];
  matrix[ 9] = rotation[2][1];
  matrix[10] = rotation[2][2];
  matrix[11] = position.z;

  // assign row 4
  matrix[12] = 0;
  matrix[13] = 0;
  matrix[14] = 0;
  matrix[15] = 1;

  this->optixTransform->setMatrix(0, matrix, 0);
}

//////////////////////////////////////////////////
void OptixNode::SetParent(OptixVisualPtr _parent)
{
  this->parent = _parent;
}

//////////////////////////////////////////////////
void OptixNode::Init()
{
  optix::Context optixContext = this->scene->GetOptixContext();
  this->optixTransform = optixContext->createTransform();
  // this->optixAccel = optixContext->createAcceleration("MedianBvh", "Bvh");
  // this->optixAccel = optixContext->createAcceleration("Lbvh", "Bvh");
  this->optixAccel = optixContext->createAcceleration("Sbvh", "Bvh");
  this->optixGroup = optixContext->createGroup();
  this->optixGroup->setAcceleration(this->optixAccel);
  this->optixTransform->setChild(this->optixGroup);
}
