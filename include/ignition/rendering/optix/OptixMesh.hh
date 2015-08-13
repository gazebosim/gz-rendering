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
#ifndef _IGNITION_RENDERING_OPTIXMESH_HH_
#define _IGNITION_RENDERING_OPTIXMESH_HH_

#include "ignition/rendering/base/BaseMesh.hh"
#include "ignition/rendering/optix/OptixGeometry.hh"
#include "ignition/rendering/optix/OptixObject.hh"
#include "ignition/rendering/optix/OptixRenderTypes.hh"

namespace ignition
{
  namespace rendering
  {
    class IGNITION_VISIBLE OptixMesh :
      public BaseMesh<OptixGeometry>
    {
      typedef std::vector<std::string> NameList;

      protected: OptixMesh();

      public: virtual ~OptixMesh();

      public: virtual optix::GeometryGroup GetOptixGeometryGroup() const;

      public: virtual optix::Acceleration GetOptixAccel() const;

      protected: virtual SubMeshStorePtr GetSubMeshes() const;

      protected: OptixSubMeshStorePtr subMeshes;

      protected: optix::GeometryGroup optixGeomGroup;

      protected: optix::Acceleration optixAccel;

      private: friend class OptixScene;

      private: friend class OptixMeshFactory;
    };

    class IGNITION_VISIBLE OptixSubMesh :
      public BaseSubMesh<OptixObject>
    {
      protected: OptixSubMesh();

      public: virtual ~OptixSubMesh();

      public: virtual MaterialPtr GetMaterial() const;

      public: virtual void SetMaterial(MaterialPtr _material,
                  bool unique = true);

      public: virtual optix::GeometryInstance GetOptixGeometryInstance() const;

      protected: virtual void SetMaterialImpl(OptixMaterialPtr _material);

      protected: OptixMaterialPtr material;

      protected: optix::GeometryInstance optixGeomInstance;

      protected: optix::Geometry optixGeometry;

      protected: optix::Buffer optixVertexBuffer;

      protected: optix::Buffer optixNormalBuffer;

      protected: optix::Buffer optixTexCoordBuffer;

      protected: optix::Buffer optixIndexBuffer;

      private: friend class OptixScene;

      private: friend class OptixSubMeshStoreFactory;
    };
  }
}
#endif
