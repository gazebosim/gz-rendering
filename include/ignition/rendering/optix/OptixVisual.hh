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
#ifndef _IGNITION_RENDERING_OPTIXVISUAL_HH_
#define _IGNITION_RENDERING_OPTIXVISUAL_HH_

#include "ignition/rendering/base/BaseVisual.hh"
#include "ignition/rendering/optix/OptixNode.hh"
#include "ignition/rendering/optix/OptixRenderTypes.hh"

namespace ignition
{
  namespace rendering
  {
    class IGNITION_VISIBLE OptixVisual :
      public BaseVisual<OptixNode>
    {
      protected: OptixVisual();

      public: virtual ~OptixVisual();

      public: virtual gazebo::math::Vector3 GetScale() const;

      public: virtual optix::Group GetOptixGroup() const;

      public: virtual optix::Acceleration GetOptixAccel() const;

      protected: virtual NodeStorePtr GetChildren() const;

      protected: virtual GeometryStorePtr GetGeometries() const;

      protected: virtual bool AttachChild(NodePtr _child);

      protected: virtual bool DetachChild(NodePtr _child);

      protected: virtual bool AttachGeometry(GeometryPtr _geometry);

      protected: virtual bool DetachGeometry(GeometryPtr _geometry);

      protected: virtual void SetScaleImpl(const gazebo::math::Vector3 &_scale);

      protected: virtual void Init();

      private: void CreateGroup();

      private: void CreateStorage();

      protected: OptixNodeStorePtr children;

      protected: OptixGeometryStorePtr geometries;

      protected: optix::Group optixGroup;

      protected: optix::Acceleration optixAccel;

      private: friend class OptixScene;
    };
  }
}
#endif
