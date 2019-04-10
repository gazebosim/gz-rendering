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

#include <ignition/common/Console.hh>

#include "ignition/rendering/ogre/OgreNode.hh"
#include "ignition/rendering/ogre/OgreConversions.hh"
#include "ignition/rendering/ogre/OgreIncludes.hh"
#include "ignition/rendering/ogre/OgreScene.hh"
#include "ignition/rendering/ogre/OgreStorage.hh"

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
  return this->parent != nullptr;
}

//////////////////////////////////////////////////
NodePtr OgreNode::Parent() const
{
  return this->parent;
}

//////////////////////////////////////////////////
Ogre::SceneNode *OgreNode::Node() const
{
  return this->ogreNode;
}

//////////////////////////////////////////////////
void OgreNode::Destroy()
{
  BaseNode::Destroy();
  Ogre::SceneManager *ogreSceneManager = this->scene->OgreSceneManager();
  ogreSceneManager->destroySceneNode(this->ogreNode);
  this->ogreNode = nullptr;
}

//////////////////////////////////////////////////
math::Pose3d OgreNode::RawLocalPose() const
{
  math::Pose3d Pose3d;
  Pose3d.Pos() = this->RawLocalPosition();
  Pose3d.Rot() = this->RawLocalRotation();
  return Pose3d;
}

//////////////////////////////////////////////////
void OgreNode::SetRawLocalPose(const math::Pose3d &_Pose3d)
{
  this->SetRawLocalPosition(_Pose3d.Pos());
  this->SetRawLocalRotation(_Pose3d.Rot());
}

//////////////////////////////////////////////////
math::Vector3d OgreNode::RawLocalPosition() const
{
  return OgreConversions::Convert(this->ogreNode->getPosition());
}

//////////////////////////////////////////////////
void OgreNode::SetRawLocalPosition(const math::Vector3d &_position)
{
  this->ogreNode->setPosition(OgreConversions::Convert(_position));
}

//////////////////////////////////////////////////
math::Quaterniond OgreNode::RawLocalRotation() const
{
  return OgreConversions::Convert(this->ogreNode->getOrientation());
}

//////////////////////////////////////////////////
void OgreNode::SetRawLocalRotation(const math::Quaterniond &_rotation)
{
  this->ogreNode->setOrientation(OgreConversions::Convert(_rotation));
}

//////////////////////////////////////////////////
void OgreNode::SetParent(OgreNodePtr _parent)
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
  sceneManager = this->scene->OgreSceneManager();
  this->ogreNode = sceneManager->createSceneNode(this->name);
  this->ogreNode->setInheritScale(true);
  this->children = OgreNodeStorePtr(new OgreNodeStore);
}
//////////////////////////////////////////////////
NodeStorePtr OgreNode::Children() const
{
  return this->children;
}

//////////////////////////////////////////////////
bool OgreNode::AttachChild(NodePtr _child)
{
  OgreNodePtr derived = std::dynamic_pointer_cast<OgreNode>(_child);

  if (!derived)
  {
    ignerr << "Cannot attach node created by another render-engine"
        << std::endl;
    return false;
  }

  derived->SetParent(this->SharedThis());
  this->ogreNode->addChild(derived->Node());
  return true;
}

//////////////////////////////////////////////////
bool OgreNode::DetachChild(NodePtr _child)
{
  OgreNodePtr derived = std::dynamic_pointer_cast<OgreNode>(_child);

  if (!derived)
  {
    ignerr << "Cannot detach node created by another render-engine"
        << std::endl;
    return false;
  }

  this->ogreNode->removeChild(derived->Node());
  return true;
}

//////////////////////////////////////////////////
OgreNodePtr OgreNode::SharedThis()
{
  ObjectPtr object = shared_from_this();
  return std::dynamic_pointer_cast<OgreNode>(object);
}

//////////////////////////////////////////////////
math::Vector3d OgreNode::LocalScale() const
{
  return OgreConversions::Convert(this->ogreNode->getScale());
}

//////////////////////////////////////////////////
bool OgreNode::InheritScale() const
{
  return this->ogreNode->getInheritScale();
}

//////////////////////////////////////////////////
void OgreNode::SetInheritScale(bool _inherit)
{
  this->ogreNode->setInheritScale(_inherit);
}

//////////////////////////////////////////////////
void OgreNode::SetLocalScaleImpl(const math::Vector3d &_scale)
{
  this->ogreNode->setScale(OgreConversions::Convert(_scale));
}


