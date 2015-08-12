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
  this->WritePoseToDevice();
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
  float optixMatrix[16];
  gazebo::math::Vector3 position = this->pose.pos;
  gazebo::math::Matrix3 rotation = this->pose.rot.GetAsMatrix3();

  // assign row 1
  optixMatrix[ 0] = rotation[0][0];
  optixMatrix[ 1] = rotation[0][1];
  optixMatrix[ 2] = rotation[0][2];
  optixMatrix[ 3] = position.x;

  // assign row 2
  optixMatrix[ 4] = rotation[1][0];
  optixMatrix[ 5] = rotation[1][1];
  optixMatrix[ 6] = rotation[1][2];
  optixMatrix[ 7] = position.y;

  // assign row 3
  optixMatrix[ 8] = rotation[2][0];
  optixMatrix[ 9] = rotation[2][1];
  optixMatrix[10] = rotation[2][2];
  optixMatrix[11] = position.z;

  // assign row 4
  optixMatrix[12] = 0;
  optixMatrix[13] = 0;
  optixMatrix[14] = 0;
  optixMatrix[15] = 1;

  this->optixTransform->getMatrix(0, optixMatrix, 0);
}

//////////////////////////////////////////////////
void OptixNode::Init()
{
  optix::Context optixContext = this->scene->GetOptixContext();
  this->optixTransform = optixContext->createTransform();
}
