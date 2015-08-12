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
#ifndef _IGNITION_RENDERING_OPTIXMESHFACTORY_HH_
#define _IGNITION_RENDERING_OPTIXMESHFACTORY_HH_

#include <map>
#include "ignition/rendering/MeshDescriptor.hh"
#include "ignition/rendering/optix/OptixRenderTypes.hh"
#include "ignition/rendering/optix/OptixMesh.hh"
#include "ignition/rendering/optix/OptixIncludes.hh"

namespace ignition
{
  namespace rendering
  {
    class IGNITION_VISIBLE OptixMeshFactory
    {
      public: OptixMeshFactory(OptixScenePtr _scene);

      public: virtual ~OptixMeshFactory();

      public: virtual OptixMeshPtr Create(const MeshDescriptor &_desc);

      protected: virtual optix::Geometry GetOgreGeometry(
                     const MeshDescriptor &_desc);

      protected: virtual bool Load(const MeshDescriptor &_desc);

      protected: virtual bool IsLoaded(const MeshDescriptor &_desc);

      protected: virtual bool LoadImpl(const MeshDescriptor &_desc);

      protected: virtual std::string GetMeshName(const MeshDescriptor &_desc);

      protected: std::map<std::string, optix::Geometry> geometries;

      protected: OptixScenePtr scene;
    };
  }
}
#endif
