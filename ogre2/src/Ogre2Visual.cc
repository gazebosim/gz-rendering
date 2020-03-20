/*
 * Copyright (C) 2018 Open Source Robotics Foundation
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

#include "ignition/rendering/ogre2/Ogre2Conversions.hh"
#include "ignition/rendering/ogre2/Ogre2Geometry.hh"
#include "ignition/rendering/ogre2/Ogre2RenderTypes.hh"
#include "ignition/rendering/ogre2/Ogre2Storage.hh"
#include "ignition/rendering/ogre2/Ogre2Visual.hh"
#include "ignition/rendering/ogre2/Ogre2WireBox.hh"

using namespace ignition;
using namespace rendering;

//////////////////////////////////////////////////
Ogre2Visual::Ogre2Visual()
{
}

//////////////////////////////////////////////////
Ogre2Visual::~Ogre2Visual()
{
}

//////////////////////////////////////////////////
void Ogre2Visual::SetVisible(bool _visible)
{
  this->ogreNode->setVisible(_visible);
}

//////////////////////////////////////////////////
GeometryStorePtr Ogre2Visual::Geometries() const
{
  return this->geometries;
}

//////////////////////////////////////////////////
bool Ogre2Visual::AttachGeometry(GeometryPtr _geometry)
{
  if (!_geometry)
  {
    ignerr << "Cannot attach null geometry." << std::endl;

    return false;
  }

  Ogre2GeometryPtr derived =
      std::dynamic_pointer_cast<Ogre2Geometry>(_geometry);

  if (!derived)
  {
    ignerr << "Cannot attach geometry created by another render-engine"
          << std::endl;

    return false;
  }

  Ogre::MovableObject *ogreObj = derived->OgreObject();
  if (!ogreObj)
  {
    ignerr << "Cannot attach a null geometry object" << std::endl;
    return false;
  }

  // set user data for mouse queries
  ogreObj->getUserObjectBindings().setUserAny(
      Ogre::Any(this->Id()));
  ogreObj->setName(this->Name() + "_" + _geometry->Name());
  ogreObj->setVisibilityFlags(IGN_VISIBILITY_ALL);

  derived->SetParent(this->SharedThis());
  this->ogreNode->attachObject(ogreObj);

  return true;
}

//////////////////////////////////////////////////
bool Ogre2Visual::DetachGeometry(GeometryPtr _geometry)
{
  Ogre2GeometryPtr derived =
      std::dynamic_pointer_cast<Ogre2Geometry>(_geometry);

  if (!derived)
  {
    ignerr << "Cannot detach geometry created by another render-engine"
          << std::endl;

    return false;
  }

  this->ogreNode->detachObject(derived->OgreObject());
  derived->SetParent(nullptr);
  return true;
}

//////////////////////////////////////////////////
ignition::math::AxisAlignedBox Ogre2Visual::BoundingBox() const
{
  ignition::math::AxisAlignedBox box(
      ignition::math::Vector3d::Zero,
      ignition::math::Vector3d::Zero);
  this->BoundsHelper(box);
  return box;
}

//////////////////////////////////////////////////
void Ogre2Visual::BoundsHelper(ignition::math::AxisAlignedBox &_box) const
{
  //this->ogreNode->_updateBounds();
  //this->ogreNode->_update(false, true);
  
  Ogre::Matrix4 invTransform =
      this->ogreNode->_getFullTransform().inverse();
  
  //Ogre::SceneNode::ChildNodeIterator it = _node->getChildIterator();
  
  ignwarn << "num attached objects " << this->ogreNode->numAttachedObjects() << "\n";
  
  for (size_t i = 0; i < this->ogreNode->numAttachedObjects(); i++)
  { 
    Ogre::MovableObject *obj = this->ogreNode->getAttachedObject(i);
    
    if (obj->isVisible() && obj->getMovableType() != "ignition::dynamiclines"
        && obj->getMovableType() != "BillboardSet"
        && obj->getVisibilityFlags() != IGN_VISIBILITY_GUI)
    { 
      Ogre::Any any = obj->getUserObjectBindings().getUserAny();
      if (any.getType() == typeid(std::string))
      {
        std::string str = Ogre::any_cast<std::string>(any);
        if (str.substr(0, 3) == "rot" || str.substr(0, 5) == "trans"
            || str.substr(0, 5) == "scale" ||
            str.find("_APPLY_WRENCH_") != std::string::npos)
          continue;
      }
      
      Ogre::Aabb bb = obj->getLocalAabb();
      
      ignition::math::Vector3d min;
      ignition::math::Vector3d max;
      
      // Ogre does not return a valid bounding box for lights.
      if (obj->getMovableType() == "Light")
      {
        min = ignition::math::Vector3d(-0.5, -0.5, -0.5);
        max = ignition::math::Vector3d(0.5, 0.5, 0.5);
      }
      else
      {
        // Get transform to be applied to the current node.
        Ogre::Matrix4 transform = invTransform * this->ogreNode->_getFullTransform();
        // Correct precision error which makes ogre's isAffine check fail.
        transform[3][0] = transform[3][1] = transform[3][2] = 0;
        transform[3][3] = 1;
        // get oriented bounding box in object's local space
        bb.transformAffine(transform);
        Ogre::Vector3 ogreMin = bb.getMinimum();
        Ogre::Vector3 ogreMax = bb.getMaximum();
        min = ignition::math::Vector3d(ogreMin.x, ogreMin.y, ogreMin.z);
        max = ignition::math::Vector3d(ogreMax.x, ogreMax.y, ogreMax.z);
      }
      
      _box.Merge(ignition::math::AxisAlignedBox(min, max));
    }
  }
  
  auto childNodes = std::dynamic_pointer_cast<Ogre2NodeStore>(this->Children());
  if (!childNodes)
    return;
  
  for (auto it = childNodes->Begin(); it != childNodes->End(); ++it)
  {
    NodePtr child = it->second;
    Ogre2VisualPtr visual = std::dynamic_pointer_cast<Ogre2Visual>(child);
    if (visual)
      _box.Merge(visual->BoundingBox());
  }
  
  /*
  while (it.hasMoreElements())
  {
    Ogre::SceneNode *next = dynamic_cast<Ogre::SceneNode*>(it.getNext());
    this->BoundsHelper(next, _box);
  }
  */
}

//////////////////////////////////////////////////
void Ogre2Visual::Init()
{
  BaseVisual::Init();
  this->geometries = Ogre2GeometryStorePtr(new Ogre2GeometryStore);
}

//////////////////////////////////////////////////
Ogre2VisualPtr Ogre2Visual::SharedThis()
{
  ObjectPtr object = shared_from_this();
  return std::dynamic_pointer_cast<Ogre2Visual>(object);
}
