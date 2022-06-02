/*
 * Copyright (C) 2022 Open Source Robotics Foundation
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
#include "LuxCoreEngineNode.hh"

using namespace ignition;
using namespace rendering;

//////////////////////////////////////////////////
LuxCoreEngineNode::LuxCoreEngineNode()
{
}

//////////////////////////////////////////////////
LuxCoreEngineNode::~LuxCoreEngineNode()
{
}

//////////////////////////////////////////////////
bool LuxCoreEngineNode::HasParent() const
{
  return true;
}

//////////////////////////////////////////////////
NodePtr LuxCoreEngineNode::Parent() const
{
  return nullptr;
}

//////////////////////////////////////////////////
void LuxCoreEngineNode::PreRender()
{
}

//////////////////////////////////////////////////
math::Vector3d LuxCoreEngineNode::LocalPosition() const
{
  return pose.Pos();
}

//////////////////////////////////////////////////
math::Vector3d LuxCoreEngineNode::LocalScale() const
{
  return math::Vector3d::One;
}

//////////////////////////////////////////////////
bool LuxCoreEngineNode::InheritScale() const
{
  return true;
}

//////////////////////////////////////////////////
void LuxCoreEngineNode::SetInheritScale(bool _inherit)
{
}

//////////////////////////////////////////////////
void LuxCoreEngineNode::SetLocalScaleImpl(const math::Vector3d &_scale)
{
}

//////////////////////////////////////////////////
math::Pose3d LuxCoreEngineNode::RawLocalPose() const
{
  return math::Pose3d::Zero;
}

//////////////////////////////////////////////////
void LuxCoreEngineNode::SetRawLocalPose(const math::Pose3d &_pose)
{
}

//////////////////////////////////////////////////
void LuxCoreEngineNode::SetParent(LuxCoreEngineNodePtr _parent)
{
}

//////////////////////////////////////////////////
void LuxCoreEngineNode::Init()
{
}

//////////////////////////////////////////////////
NodeStorePtr LuxCoreEngineNode::Children() const
{
  return nullptr;
}

//////////////////////////////////////////////////
bool LuxCoreEngineNode::AttachChild(NodePtr _child)
{
  return true;
}

//////////////////////////////////////////////////
bool LuxCoreEngineNode::DetachChild(NodePtr _child)
{
  return true;
}
