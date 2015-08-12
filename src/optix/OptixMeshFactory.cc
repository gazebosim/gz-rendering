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
#include "ignition/rendering/optix/OptixMeshFactory.hh"

#include <sstream>
#include "gazebo/common/Console.hh"
#include "gazebo/common/Mesh.hh"
#include "ignition/rendering/optix/OptixMesh.hh"
#include "ignition/rendering/optix/OptixStorage.hh"

using namespace ignition;
using namespace rendering;

//////////////////////////////////////////////////
// OptixMeshFactory
//////////////////////////////////////////////////
OptixMeshFactory::OptixMeshFactory(OptixScenePtr _scene) :
  scene(_scene)
{
}

//////////////////////////////////////////////////
OptixMeshFactory::~OptixMeshFactory()
{
}

//////////////////////////////////////////////////
OptixMeshPtr OptixMeshFactory::Create(const MeshDescriptor &_desc)
{
  // create optix entity
  OptixMeshPtr mesh(new OptixMesh);
  MeshDescriptor normDesc = _desc.Normalize();
  // mesh->optixGeomInstance = this->GetOptixGeomInstance(normDesc);

  // // check if invalid mesh
  // if (!mesh->optixEntity)
  // {
  //   return NULL;
  // }

  // create sub-mesh store
  OptixSubMeshStoreFactory storeFactory;
  mesh->subMeshes = storeFactory.Create();
  return mesh;
}

//////////////////////////////////////////////////
optix::Geometry OptixMeshFactory::GetOptixGeometry(
    const MeshDescriptor &_desc)
{
  if (this->Load(_desc))
  {
    return NULL;
  }

  std::string name = this->GetMeshName(_desc);
  return this->geometries[name];
}

//////////////////////////////////////////////////
bool OptixMeshFactory::Load(const MeshDescriptor &_desc)
{
  if (!this->Validate(_desc))
  {
    return false;
  }

  if (this->IsLoaded(_desc))
  {
    return true;
  }

  return this->LoadImpl(_desc);
}

//////////////////////////////////////////////////
bool OptixMeshFactory::IsLoaded(const MeshDescriptor &_desc)
{
  std::string name = this->GetMeshName(_desc);
  auto iter = this->geometries.find(name);
  return iter != this->geometries.end();
}

//////////////////////////////////////////////////
bool OptixMeshFactory::LoadImpl(const MeshDescriptor &/*_desc*/)
{
  return true;
}

//////////////////////////////////////////////////
std::string OptixMeshFactory::GetMeshName(const MeshDescriptor &_desc)
{
  std::stringstream ss;
  ss << _desc.meshName << "::" << _desc.subMeshName << "::";
  ss << ((_desc.centerSubMesh) ? "CENTERED" : "ORIGINAL");
  return ss.str();
}

//////////////////////////////////////////////////
bool OptixMeshFactory::Validate(const MeshDescriptor &_desc)
{
  if (!_desc.mesh && _desc.meshName.empty())
  {
    gzerr << "Invalid mesh-descriptor, no mesh specified" << std::endl;
    return false;
  }

  if (!_desc.mesh)
  {
    gzerr << "Cannot load null mesh" << std::endl;
    return false;
  }
  
  if (_desc.mesh->GetSubMeshCount() == 0)
  {
    gzerr << "Cannot load mesh with zero sub-meshes" << std::endl;
    return false;
  }

  return true;
}

//////////////////////////////////////////////////
// OptixSubMeshFactory
//////////////////////////////////////////////////
OptixSubMeshStoreFactory::OptixSubMeshStoreFactory()
{
}

//////////////////////////////////////////////////
OptixSubMeshStoreFactory::~OptixSubMeshStoreFactory()
{
}

//////////////////////////////////////////////////
OptixSubMeshStorePtr OptixSubMeshStoreFactory::Create()
{
  return OptixSubMeshStorePtr(new OptixSubMeshStore);
}
