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

#include <gz/common/Console.hh>
#include <gz/common/Profiler.hh>

#include "gz/rendering/ogre_next/OgreNextConversions.hh"
#include "gz/rendering/ogre_next/OgreNextGeometry.hh"
#include "gz/rendering/ogre_next/OgreNextParticleEmitter.hh"
#include "gz/rendering/ogre_next/OgreNextRenderTypes.hh"
#include "gz/rendering/ogre_next/OgreNextStorage.hh"
#include "gz/rendering/ogre_next/OgreNextVisual.hh"
#include "gz/rendering/Utils.hh"

#ifdef _MSC_VER
  #pragma warning(push, 0)
#endif
#include <OgreItem.h>
#ifdef _MSC_VER
  #pragma warning(pop)
#endif

using namespace gz;
using namespace rendering;

/// \brief Private data for the OgreNextVisual class
class gz::rendering::OgreNextVisualPrivate
{
  /// \brief True if wireframe mode is enabled
  public: bool wireframe;
};

//////////////////////////////////////////////////
OgreNextVisual::OgreNextVisual()
  : dataPtr(new OgreNextVisualPrivate)
{
  this->dataPtr->wireframe = false;
}

//////////////////////////////////////////////////
OgreNextVisual::~OgreNextVisual()
{
}

//////////////////////////////////////////////////
void OgreNextVisual::SetWireframe(bool _show)
{
  GZ_PROFILE("Ogre2Visual::SetWireframe");
  if (this->dataPtr->wireframe == _show)
    return;

  if (!this->ogreNode)
    return;

  this->dataPtr->wireframe = _show;
  for (unsigned int i = 0; i < this->ogreNode->numAttachedObjects();
      i++)
  {
    Ogre::MovableObject *obj = this->ogreNode->getAttachedObject(i);
    Ogre::Item *item = dynamic_cast<Ogre::Item *>(obj);

    if (!item)
      continue;

    for (unsigned int j = 0; j < item->getNumSubItems(); j++)
    {
      Ogre::SubItem *subItem = item->getSubItem(j);
      auto datablock = subItem->getDatablock();
      auto macroblock = *(datablock->getMacroblock());

      if (_show)
        macroblock.mPolygonMode = Ogre::PM_WIREFRAME;
      else
        macroblock.mPolygonMode = Ogre::PM_SOLID;

      datablock->setMacroblock(macroblock);
    }
  }
}

//////////////////////////////////////////////////
bool OgreNextVisual::Wireframe() const
{
  return this->dataPtr->wireframe;
}

//////////////////////////////////////////////////
void OgreNextVisual::SetVisible(bool _visible)
{
  if (!this->ogreNode)
    return;

  this->ogreNode->setVisible(_visible);
}

//////////////////////////////////////////////////
void OgreNextVisual::SetStatic(bool _static)
{
  this->ogreNode->setStatic(_static);
}

//////////////////////////////////////////////////
bool OgreNextVisual::Static() const
{
  return this->ogreNode->isStatic();
}

//////////////////////////////////////////////////
void OgreNextVisual::SetVisibilityFlags(uint32_t _flags)
{
  BaseVisual::SetVisibilityFlags(_flags);

  if (!this->ogreNode)
    return;

  for (unsigned int i = 0; i < this->ogreNode->numAttachedObjects(); ++i)
  {
    this->ogreNode->getAttachedObject(i)->setVisibilityFlags(_flags
      & ~OgreNextParticleEmitter::kParticleVisibilityFlags);
  }
}

//////////////////////////////////////////////////
GeometryStorePtr OgreNextVisual::Geometries() const
{
  return this->geometries;
}

//////////////////////////////////////////////////
bool OgreNextVisual::AttachGeometry(GeometryPtr _geometry)
{
  if (!_geometry)
  {
    gzerr << "Cannot attach null geometry." << std::endl;

    return false;
  }

  if (!this->ogreNode)
  {
    gzerr << "Cannot attach geometry, null Ogre node." << std::endl;
    return false;
  }

  OgreNextGeometryPtr derived =
      std::dynamic_pointer_cast<OgreNextGeometry>(_geometry);

  if (!derived)
  {
    gzerr << "Cannot attach geometry created by another render-engine"
          << std::endl;

    return false;
  }

  Ogre::MovableObject *ogreObj = derived->OgreObject();
  if (!ogreObj)
  {
    gzerr << "Cannot attach a null geometry object" << std::endl;
    return false;
  }

  // set user data for mouse queries
  ogreObj->getUserObjectBindings().setUserAny(
      Ogre::Any(this->Id()));
  ogreObj->setName(this->Name() + "_" + _geometry->Name());
  ogreObj->setVisibilityFlags(this->visibilityFlags
      & ~OgreNextParticleEmitter::kParticleVisibilityFlags);

  derived->SetParent(this->SharedThis());
  this->ogreNode->attachObject(ogreObj);

  return true;
}

//////////////////////////////////////////////////
bool OgreNextVisual::DetachGeometry(GeometryPtr _geometry)
{
  if (!this->ogreNode)
  {
    gzerr << "Cannot detach geometry, null Ogre node." << std::endl;
    return false;
  }

  OgreNextGeometryPtr derived =
      std::dynamic_pointer_cast<OgreNextGeometry>(_geometry);

  if (!derived)
  {
    gzerr << "Cannot detach geometry created by another render-engine"
          << std::endl;

    return false;
  }

  if (nullptr != derived->OgreObject())
    this->ogreNode->detachObject(derived->OgreObject());
  derived->SetParent(nullptr);
  return true;
}

//////////////////////////////////////////////////
gz::math::AxisAlignedBox OgreNextVisual::LocalBoundingBox() const
{
  gz::math::AxisAlignedBox box;
  this->BoundsHelper(box, true /* local frame */);
  return box;
}

//////////////////////////////////////////////////
gz::math::AxisAlignedBox OgreNextVisual::BoundingBox() const
{
  gz::math::AxisAlignedBox box;
  this->BoundsHelper(box, false /* world frame */);
  return box;
}

//////////////////////////////////////////////////
void OgreNextVisual::BoundsHelper(gz::math::AxisAlignedBox &_box,
    bool _local) const
{
  this->BoundsHelper(_box, _local, this->WorldPose());
}

//////////////////////////////////////////////////
void OgreNextVisual::BoundsHelper(gz::math::AxisAlignedBox &_box,
    bool _local, const gz::math::Pose3d &_pose) const
{
  GZ_PROFILE("Ogre2Visual::BoundsHelper");
  if (!this->ogreNode)
    return;

  gz::math::Vector3d scale = this->WorldScale();

  for (size_t i = 0; i < this->ogreNode->numAttachedObjects(); i++)
  {
    Ogre::MovableObject *obj = this->ogreNode->getAttachedObject(i);

    if (obj->isVisible() && obj->getVisibilityFlags() != GZ_VISIBILITY_GUI)
    {
      Ogre::Aabb bb = obj->getLocalAabb();

      gz::math::Vector3d min(0, 0, 0);
      gz::math::Vector3d max(0, 0, 0);
      gz::math::AxisAlignedBox box(min, max);

      // Ogre does not return a valid bounding box for lights.
      if (obj->getMovableType() == Ogre::LightFactory::FACTORY_TYPE_NAME)
      {
        box.Min() = gz::math::Vector3d(-0.5, -0.5, -0.5);
        box.Max()  = gz::math::Vector3d(0.5, 0.5, 0.5);
      }
      else
      {
        Ogre::Vector3 ogreMin = bb.getMinimum();
        Ogre::Vector3 ogreMax = bb.getMaximum();

        // Get ogre bounding boxes and size to object's scale
        min = scale * gz::math::Vector3d(ogreMin.x, ogreMin.y, ogreMin.z);
        max = scale * gz::math::Vector3d(ogreMax.x, ogreMax.y, ogreMax.z);
        box.Min() = min,
        box.Max() = max;

        // Assume world transform
        gz::math::Pose3d transform = _pose;

        // If local frame, calculate transform matrix and set
        if (_local)
        {
          gz::math::Pose3d worldPose = this->WorldPose();
          gz::math::Vector3d parentPos = _pose.Pos();
          gz::math::Quaternion parentRotInv = _pose.Rot().Inverse();
          gz::math::Pose3d localTransform =
            gz::math::Pose3d(
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

  auto childNodes =
      std::dynamic_pointer_cast<OgreNextNodeStore>(this->Children());
  if (!childNodes)
    return;

  for (auto it = childNodes->Begin(); it != childNodes->End(); ++it)
  {
    OgreNextVisualPtr visual = std::dynamic_pointer_cast<OgreNextVisual>(*it);
    if (visual)
      visual->BoundsHelper(_box, _local, _pose);
  }
}

//////////////////////////////////////////////////
void OgreNextVisual::Init()
{
  BaseVisual::Init();
  this->geometries = OgreNextGeometryStorePtr(new OgreNextGeometryStore);
}

//////////////////////////////////////////////////
OgreNextVisualPtr OgreNextVisual::SharedThis()
{
  ObjectPtr object = shared_from_this();
  return std::dynamic_pointer_cast<OgreNextVisual>(object);
}
