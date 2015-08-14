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
#include "ignition/rendering/ogre/OgreNode.hh"
#include "ignition/rendering/ogre/OgreConversions.hh"
#include "ignition/rendering/ogre/OgreIncludes.hh"
#include "ignition/rendering/ogre/OgreScene.hh"
#include "ignition/rendering/ogre/OgreVisual.hh"

#include "gazebo/common/Console.hh"

using namespace ignition;
using namespace rendering;

//////////////////////////////////////////////////
OgreNode::OgreNode()
{
}

//////////////////////////////////////////////////
OgreNode::~OgreNode()
{
}

//////////////////////////////////////////////////
bool OgreNode::HasParent() const
{
  return this->parent != NULL;
}

//////////////////////////////////////////////////
VisualPtr OgreNode::GetParent() const
{
  return this->parent;
}

//////////////////////////////////////////////////
Ogre::SceneNode *OgreNode::GetOgreNode() const
{
  return this->ogreNode;
}

//////////////////////////////////////////////////
void OgreNode::Destroy()
{
  BaseNode::Destroy();
  Ogre::SceneManager *ogreSceneManager = this->scene->GetOgreSceneManager();
  ogreSceneManager->destroySceneNode(this->ogreNode);
}

//////////////////////////////////////////////////
gazebo::math::Pose OgreNode::GetRawLocalPose() const
{
  gazebo::math::Pose pose;
  pose.pos = this->GetRawLocalPosition();
  pose.rot = this->GetRawLocalRotation();
  return pose;
}

//////////////////////////////////////////////////
void OgreNode::SetRawLocalPose(const gazebo::math::Pose &_pose)
{
  this->SetRawLocalPosition(_pose.pos);
  this->SetRawLocalRotation(_pose.rot);
}

//////////////////////////////////////////////////
gazebo::math::Vector3 OgreNode::GetRawLocalPosition() const
{
  return OgreConversions::Convert(this->ogreNode->getPosition());
}

//////////////////////////////////////////////////
void OgreNode::SetRawLocalPosition(const gazebo::math::Vector3 &_position)
{
  this->ogreNode->setPosition(OgreConversions::Convert(_position));
}

//////////////////////////////////////////////////
gazebo::math::Quaternion OgreNode::GetRawLocalRotation() const
{
  return OgreConversions::Convert(this->ogreNode->getOrientation());
}

//////////////////////////////////////////////////
void OgreNode::SetRawLocalRotation(const gazebo::math::Quaternion &_rotation)
{
  this->ogreNode->setOrientation(OgreConversions::Convert(_rotation));
}

//////////////////////////////////////////////////
void OgreNode::SetParent(OgreVisualPtr _parent)
{
  this->parent = _parent;
}

//////////////////////////////////////////////////
void OgreNode::Load()
{
}

//////////////////////////////////////////////////
void OgreNode::Init()
{
  Ogre::SceneManager *sceneManager;
  sceneManager = this->scene->GetOgreSceneManager();
  this->ogreNode = sceneManager->createSceneNode(this->name);
  this->ogreNode->setInheritScale(true);
}
