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

#include <gz/common/Console.hh>

#include "gz/rendering/ogre_next/OgreNextCamera.hh"
#include "gz/rendering/ogre_next/OgreNextNode.hh"
#include "gz/rendering/ogre_next/OgreNextConversions.hh"
#include "gz/rendering/ogre_next/OgreNextScene.hh"
#include "gz/rendering/ogre_next/OgreNextStorage.hh"

#ifdef _MSC_VER
  #pragma warning(push, 0)
#endif
#include <OgreSceneManager.h>
#ifdef _MSC_VER
  #pragma warning(pop)
#endif

using namespace gz;
using namespace rendering;

//////////////////////////////////////////////////
OgreNextNode::OgreNextNode()
{
}

//////////////////////////////////////////////////
OgreNextNode::~OgreNextNode()
{
}

//////////////////////////////////////////////////
bool OgreNextNode::HasParent() const
{
  return this->parent != nullptr;
}

//////////////////////////////////////////////////
NodePtr OgreNextNode::Parent() const
{
  return this->parent;
}

//////////////////////////////////////////////////
Ogre::SceneNode *OgreNextNode::Node() const
{
  return this->ogreNode;
}

//////////////////////////////////////////////////
void OgreNextNode::Destroy()
{
  if (!ogreNode)
    return;

  BaseNode::Destroy();

  if (nullptr != this->scene)
  {
    Ogre::SceneManager *ogreSceneManager = this->scene->OgreSceneManager();
    if (nullptr != ogreSceneManager)
      ogreSceneManager->destroySceneNode(this->ogreNode);
  }
  this->ogreNode = nullptr;
}

//////////////////////////////////////////////////
math::Pose3d OgreNextNode::RawLocalPose() const
{
  math::Pose3d Pose3d;
  Pose3d.Pos() = this->RawLocalPosition();
  Pose3d.Rot() = this->RawLocalRotation();
  return Pose3d;
}

//////////////////////////////////////////////////
void OgreNextNode::SetRawLocalPose(const math::Pose3d &_Pose3d)
{
  this->SetRawLocalPosition(_Pose3d.Pos());
  this->SetRawLocalRotation(_Pose3d.Rot());
}

//////////////////////////////////////////////////
math::Vector3d OgreNextNode::RawLocalPosition() const
{
  if (nullptr == this->ogreNode)
    return math::Vector3d();

  return OgreNextConversions::Convert(this->ogreNode->getPosition());
}

//////////////////////////////////////////////////
void OgreNextNode::SetRawLocalPosition(const math::Vector3d &_position)
{
  if (nullptr == this->ogreNode)
    return;

  // ogre crashes in the compositor shadow pass with an
  // Ogre::AxisAlignedBox::setExtents assertion error when the camera scene node
  // position has large values. Added a workaround that places a max limit on
  // the length of the position vector.
  if (dynamic_cast<OgreNextCamera *>(this) && _position.Length() > 1e9)
  {
    gzerr << "Unable to set camera node position to a distance larger than "
          << "1e9 from origin" << std::endl;
    return;
  }
  this->ogreNode->setPosition(OgreNextConversions::Convert(_position));
}

//////////////////////////////////////////////////
math::Quaterniond OgreNextNode::RawLocalRotation() const
{
  if (nullptr == this->ogreNode)
    return math::Quaterniond();

  return OgreNextConversions::Convert(this->ogreNode->getOrientation());
}

//////////////////////////////////////////////////
void OgreNextNode::SetRawLocalRotation(const math::Quaterniond &_rotation)
{
  if (nullptr == this->ogreNode)
    return;

  this->ogreNode->setOrientation(OgreNextConversions::Convert(_rotation));
}

//////////////////////////////////////////////////
void OgreNextNode::SetParent(OgreNextNodePtr _parent)
{
  this->parent = _parent;
}

//////////////////////////////////////////////////
void OgreNextNode::Load()
{
}

//////////////////////////////////////////////////
void OgreNextNode::Init()
{
  if (nullptr == this->scene)
  {
    gzerr << "Failed to initialize node: scene is NULL" << std::endl;
    return;
  }

  auto sceneManager = this->scene->OgreSceneManager();
  if (nullptr == sceneManager)
  {
    gzerr << "Failed to initialize node: scene manager is NULL" << std::endl;
    return;
  }

  this->ogreNode = sceneManager->createSceneNode();
  if (nullptr == this->ogreNode)
  {
    gzerr << "Failed to create Ogre node" << std::endl;
    return;
  }
  this->ogreNode->setInheritScale(true);
  this->children = OgreNextNodeStorePtr(new OgreNextNodeStore);
}

//////////////////////////////////////////////////
NodeStorePtr OgreNextNode::Children() const
{
  return this->children;
}

//////////////////////////////////////////////////
bool OgreNextNode::AttachChild(NodePtr _child)
{
  if (nullptr == this->ogreNode)
    return false;

  OgreNextNodePtr derived = std::dynamic_pointer_cast<OgreNextNode>(_child);

  if (!derived)
  {
    gzerr << "Cannot attach node created by another render-engine"
        << std::endl;
    return false;
  }

  // Check for loop. Ogre throws exception if child node to be added
  // is a direct ancestor of this node
  auto p = this->ogreNode->getParent();
  while (p != nullptr)
  {
    if (p == derived->Node())
    {
      gzerr << "Node cycle detected. Not adding Node: " << _child->Name()
             << std::endl;
      return false;
    }
    p = p->getParent();
  }

  derived->SetParent(this->SharedThis());
  this->ogreNode->addChild(derived->Node());
  return true;
}

//////////////////////////////////////////////////
bool OgreNextNode::DetachChild(NodePtr _child)
{
  if (nullptr == this->ogreNode)
    return false;

  OgreNextNodePtr derived = std::dynamic_pointer_cast<OgreNextNode>(_child);

  if (!derived)
  {
    gzerr << "Cannot detach node created by another render-engine"
        << std::endl;
    return false;
  }

  this->ogreNode->removeChild(derived->Node());

  return true;
}

//////////////////////////////////////////////////
OgreNextNodePtr OgreNextNode::SharedThis()
{
  ObjectPtr object = shared_from_this();
  return std::dynamic_pointer_cast<OgreNextNode>(object);
}

//////////////////////////////////////////////////
math::Vector3d OgreNextNode::LocalScale() const
{
  if (nullptr == this->ogreNode)
    return math::Vector3d();

  return OgreNextConversions::Convert(this->ogreNode->getScale());
}

//////////////////////////////////////////////////
bool OgreNextNode::InheritScale() const
{
  if (nullptr == this->ogreNode)
    return false;

  return this->ogreNode->getInheritScale();
}

//////////////////////////////////////////////////
void OgreNextNode::SetInheritScale(bool _inherit)
{
  if (nullptr == this->ogreNode)
    return;

  this->ogreNode->setInheritScale(_inherit);
}

//////////////////////////////////////////////////
void OgreNextNode::SetLocalScaleImpl(const math::Vector3d &_scale)
{
  if (nullptr == this->ogreNode)
    return;

  this->ogreNode->setScale(OgreNextConversions::Convert(_scale));
}
