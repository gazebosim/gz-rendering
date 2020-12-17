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

#include "ignition/rendering/ogre/OgreVisual.hh"
#include "ignition/rendering/ogre/OgreWireBox.hh"
#include "ignition/rendering/ogre/OgreConversions.hh"
#include "ignition/rendering/ogre/OgreStorage.hh"
#include "ignition/rendering/Utils.hh"

using namespace ignition;
using namespace rendering;

//////////////////////////////////////////////////
OgreVisual::OgreVisual()
{
}

//////////////////////////////////////////////////
OgreVisual::~OgreVisual()
{
}

//////////////////////////////////////////////////
void OgreVisual::SetVisible(bool _visible)
{
  this->ogreNode->setVisible(_visible);
}

//////////////////////////////////////////////////
void OgreVisual::SetVisibilityFlags(uint32_t _flags)
{
  BaseVisual::SetVisibilityFlags(_flags);

  if (!this->ogreNode)
    return;

  for (unsigned int i = 0; i < this->ogreNode->numAttachedObjects(); ++i)
    this->ogreNode->getAttachedObject(i)->setVisibilityFlags(_flags);
}

//////////////////////////////////////////////////
GeometryStorePtr OgreVisual::Geometries() const
{
  return this->geometries;
}

//////////////////////////////////////////////////
bool OgreVisual::AttachGeometry(GeometryPtr _geometry)
{
  OgreGeometryPtr derived =
      std::dynamic_pointer_cast<OgreGeometry>(_geometry);

  if (!derived)
  {
    ignerr << "Cannot attach geometry created by another render-engine"
          << std::endl;

    return false;
  }

  // Some geometries, like heightmaps, may not have an OgreObject
  Ogre::MovableObject *ogreObj = derived->OgreObject();
  if (ogreObj)
  {
    // set user data for mouse queries
    ogreObj->getUserObjectBindings().setUserAny(
        Ogre::Any(this->Id()));
    ogreObj->setVisibilityFlags(this->visibilityFlags);
    this->ogreNode->attachObject(ogreObj);
  }

  derived->SetParent(this->SharedThis());
  return true;
}

//////////////////////////////////////////////////
bool OgreVisual::DetachGeometry(GeometryPtr _geometry)
{
  if (!this->ogreNode)
    return true;

  OgreGeometryPtr derived =
      std::dynamic_pointer_cast<OgreGeometry>(_geometry);

  if (!derived)
  {
    ignerr << "Cannot detach geometry created by another render-engine"
          << std::endl;

    return false;
  }

  if (nullptr != derived->OgreObject())
    this->ogreNode->detachObject(derived->OgreObject());
  derived->SetParent(nullptr);
  return true;
}

//////////////////////////////////////////////////
ignition::math::AxisAlignedBox OgreVisual::LocalBoundingBox() const
{
  ignition::math::AxisAlignedBox box;
  this->BoundsHelper(box, true /* local frame */);
  return box;
}

//////////////////////////////////////////////////
ignition::math::AxisAlignedBox OgreVisual::BoundingBox() const
{
  ignition::math::AxisAlignedBox box;
  this->BoundsHelper(box, false /* world frame */);
  return box;
}

//////////////////////////////////////////////////
void OgreVisual::BoundsHelper(ignition::math::AxisAlignedBox &_box,
    bool _local) const
{
  this->BoundsHelper(_box, _local, this->WorldPose());
}

//////////////////////////////////////////////////
void OgreVisual::BoundsHelper(ignition::math::AxisAlignedBox &_box,
    bool _local, const ignition::math::Pose3d &_pose) const
{
  this->ogreNode->_updateBounds();
  this->ogreNode->_update(false, true);

  ignition::math::Vector3d scale = this->WorldScale();

  for (int i = 0; i < this->ogreNode->numAttachedObjects(); i++)
  {
    Ogre::MovableObject *obj = this->ogreNode->getAttachedObject(i);

    if (obj->isVisible() && obj->getVisibilityFlags() != IGN_VISIBILITY_GUI)
    {
      Ogre::AxisAlignedBox bb = obj->getBoundingBox();

      ignition::math::Vector3d min(0, 0, 0);
      ignition::math::Vector3d max(0, 0, 0);
      ignition::math::AxisAlignedBox box(min, max);

      // Ogre does not return a valid bounding box for lights.
      if (obj->getMovableType() == Ogre::LightFactory::FACTORY_TYPE_NAME)
      {
        box.Min() = ignition::math::Vector3d(-0.5, -0.5, -0.5);
        box.Max()  = ignition::math::Vector3d(0.5, 0.5, 0.5);
      }
      else
      {
        Ogre::Vector3 ogreMin = bb.getMinimum();
        Ogre::Vector3 ogreMax = bb.getMaximum();

        min = scale * ignition::math::Vector3d(ogreMin.x, ogreMin.y, ogreMin.z);
        max = scale * ignition::math::Vector3d(ogreMax.x, ogreMax.y, ogreMax.z);
        box.Min() = min,
        box.Max() = max;

        // Assume world transform
        ignition::math::Pose3d transform = _pose;

        // If local frame, calculate transform matrix and set
        if (_local)
        {
          ignition::math::Pose3d worldPose = this->WorldPose();
          ignition::math::Quaternion parentRot = _pose.Rot();
          ignition::math::Vector3d parentPos = _pose.Pos();
          ignition::math::Quaternion parentRotInv = parentRot.Inverse();
          ignition::math::Pose3d localTransform =
            ignition::math::Pose3d(
                (parentRotInv * (worldPose.Pos() - parentPos)),
                (parentRotInv * worldPose.Rot()));
          transform = localTransform;
        }

        // Transform to world or local space
        box = transformAxisAlignedBox(box, transform);
      }
      _box.Merge(box);
    }
  }

  auto childNodes = std::dynamic_pointer_cast<OgreNodeStore>(this->Children());
  if (!childNodes)
    return;

  for (auto it = childNodes->Begin(); it != childNodes->End(); ++it)
  {
    NodePtr child = it->second;
    OgreVisualPtr visual = std::dynamic_pointer_cast<OgreVisual>(child);
    if (visual)
      visual->BoundsHelper(_box, _local, _pose);
  }
}

//////////////////////////////////////////////////
void OgreVisual::Init()
{
  BaseVisual::Init();
  this->geometries = OgreGeometryStorePtr(new OgreGeometryStore);
}

//////////////////////////////////////////////////
OgreVisualPtr OgreVisual::SharedThis()
{
  ObjectPtr object = shared_from_this();
  return std::dynamic_pointer_cast<OgreVisual>(object);
}
