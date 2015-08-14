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
#include "ignition/rendering/optix/OptixVisual.hh"
#include "ignition/rendering/optix/OptixConversions.hh"
#include "ignition/rendering/optix/OptixStorage.hh"

using namespace ignition;
using namespace rendering;

//////////////////////////////////////////////////
OptixVisual::OptixVisual() :
  scale(1, 1, 1),
  inheritScale(true)
{
  // TODO: move defaults to BaseVisual
}

//////////////////////////////////////////////////
OptixVisual::~OptixVisual()
{
}

//////////////////////////////////////////////////
gazebo::math::Vector3 OptixVisual::GetLocalScale() const
{
  return this->scale;
}

//////////////////////////////////////////////////
bool OptixVisual::GetInheritScale() const
{
  return this->inheritScale;
}

//////////////////////////////////////////////////
void OptixVisual::SetInheritScale(bool _inherit)
{
  this->inheritScale = _inherit;
}

//////////////////////////////////////////////////
optix::Group OptixVisual::GetOptixGroup() const
{
  return this->optixGroup;
}

//////////////////////////////////////////////////
optix::Acceleration OptixVisual::GetOptixAccel() const
{
  return this->optixAccel;
}

//////////////////////////////////////////////////
void OptixVisual::PreRender()
{
  BaseVisual::PreRender();

  // TODO: optimize this funtionality
  gazebo::math::Vector3 worldScale = this->GetWorldScale();

  for (unsigned int i = 0; i < this->GetGeometryCount(); ++i)
  {
    OptixGeometryPtr geometry = this->geometries->GetDerivedByIndex(i);
    geometry->SetScale(worldScale);
  }
}

//////////////////////////////////////////////////
NodeStorePtr OptixVisual::GetChildren() const
{
  return this->children;
}

//////////////////////////////////////////////////
GeometryStorePtr OptixVisual::GetGeometries() const
{
  return this->geometries;
}

//////////////////////////////////////////////////
bool OptixVisual::AttachChild(NodePtr _child)
{
  OptixNodePtr derived = boost::dynamic_pointer_cast<OptixNode>(_child);

  if (!derived)
  {
    gzerr << "Cannot attach node created by another render-engine" << std::endl;
    return false;
  }

  derived->SetParent(this->SharedThis());
  optix::Transform childTransform = derived->GetOptixTransform();
  this->optixGroup->addChild(childTransform);
  this->optixAccel->markDirty();
  return true;
}

//////////////////////////////////////////////////
bool OptixVisual::DetachChild(NodePtr _child)
{
  OptixNodePtr derived = boost::dynamic_pointer_cast<OptixNode>(_child);
  
  if (!derived)
  {
    return false;
  }
  
  this->optixGroup->removeChild(derived->GetOptixTransform());
  this->optixAccel->markDirty();
  return true;
}

//////////////////////////////////////////////////
bool OptixVisual::AttachGeometry(GeometryPtr _geometry)
{
  OptixGeometryPtr derived =
      boost::dynamic_pointer_cast<OptixGeometry>(_geometry);

  if (!derived)
  {
    gzerr << "Cannot attach geometry created by another render-engine"
          << std::endl;

    return false;
  }

  derived->SetParent(this->SharedThis());
  optix::GeometryGroup childGeomGroup = derived->GetOptixGeometryGroup();
  this->optixGroup->addChild(childGeomGroup);
  this->optixAccel->markDirty();
  return true;
}

//////////////////////////////////////////////////
bool OptixVisual::DetachGeometry(GeometryPtr _geometry)
{
  OptixGeometryPtr derived =
      boost::dynamic_pointer_cast<OptixGeometry>(_geometry);

  if (!derived)
  {
    gzerr << "Cannot detach geometry created by another render-engine"
          << std::endl;

    return false;
  }

  this->optixGroup->removeChild(derived->GetOptixGeometryGroup());
  this->optixAccel->markDirty();
  return true;
}

//////////////////////////////////////////////////
void OptixVisual::SetLocalScaleImpl(const gazebo::math::Vector3 &_scale)
{
  this->scale = _scale;
}

//////////////////////////////////////////////////
void OptixVisual::Init()
{
  BaseVisual::Init();
  this->CreateStorage();
}

//////////////////////////////////////////////////
void OptixVisual::CreateStorage()
{
  this->children = OptixNodeStorePtr(new OptixNodeStore);
  this->geometries = OptixGeometryStorePtr(new OptixGeometryStore);
}

//////////////////////////////////////////////////
OptixVisualPtr OptixVisual::SharedThis()
{
  return boost::dynamic_pointer_cast<OptixVisual>(shared_from_this());
}
