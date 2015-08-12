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
#include "ignition/rendering/base/BaseScene.hh"

#include <limits>
#include <sstream>
#include "gazebo/common/Console.hh"
#include "gazebo/common/Mesh.hh"
#include "gazebo/common/MeshManager.hh"
#include "ignition/rendering/base/base.hh"

using namespace ignition;
using namespace rendering;

//////////////////////////////////////////////////
BaseScene::BaseScene(unsigned int _id, const std::string &_name) :
  id(_id),
  name(_name),
  loaded(false),
  initialized(false),
  nextObjectId(UINT_MAX),
  nodes(NULL)
{
}

//////////////////////////////////////////////////
BaseScene::~BaseScene()
{
}

//////////////////////////////////////////////////
void BaseScene::Load()
{
  if (!this->loaded)
  {
    this->loaded = this->LoadImpl();
  }
}

//////////////////////////////////////////////////
void BaseScene::Init()
{
  if (!this->loaded)
  {
    gzerr << "Scene must be loaded first" << std::endl;
    return;
  }

  if (!this->initialized)
  {
    this->initialized = this->InitImpl();
    this->CreateNodeStore();
  }
}

//////////////////////////////////////////////////
void BaseScene::Fini()
{
}

//////////////////////////////////////////////////
bool BaseScene::IsLoaded() const
{
  return this->loaded;
}

//////////////////////////////////////////////////
bool BaseScene::IsInitialized() const
{
  return this->initialized;
}

//////////////////////////////////////////////////
unsigned int BaseScene::GetId() const
{
  return this->id;
}

//////////////////////////////////////////////////
std::string BaseScene::GetName() const
{
  return this->name;
}

//////////////////////////////////////////////////
gazebo::common::Time BaseScene::GetSimTime() const
{
  return this->simTime;
}

//////////////////////////////////////////////////
void BaseScene::SetSimTime(const gazebo::common::Time &_time)
{
  this->simTime = _time;
}

//////////////////////////////////////////////////
unsigned int BaseScene::GetNodeCount() const
{
  return this->nodes->Size();
}

//////////////////////////////////////////////////
bool BaseScene::HasNode(ConstNodePtr _node) const
{
  return this->nodes->Contains(_node);
}

//////////////////////////////////////////////////
bool BaseScene::HasNodeId(unsigned int _id) const
{
  return this->nodes->ContainsId(_id);
}

//////////////////////////////////////////////////
bool BaseScene::HasNodeName(const std::string &_name) const
{
  return this->nodes->ContainsName(_name);
}

//////////////////////////////////////////////////
NodePtr BaseScene::GetNodeById(unsigned int _id) const
{
  return this->nodes->GetById(_id);
}

//////////////////////////////////////////////////
NodePtr BaseScene::GetNodeByName(const std::string &_name) const
{
  return this->nodes->GetByName(_name);
}

//////////////////////////////////////////////////
NodePtr BaseScene::GetNodeByIndex(unsigned int _index) const
{
  return this->nodes->GetByIndex(_index);
}

//////////////////////////////////////////////////
void BaseScene::DestroyNode(NodePtr _node)
{
  this->nodes->Destroy(_node);
}

//////////////////////////////////////////////////
void BaseScene::DestroyNodeById(unsigned int _id)
{
  this->nodes->DestroyById(_id);
}

//////////////////////////////////////////////////
void BaseScene::DestroyNodeByName(const std::string &_name)
{
  this->nodes->DestroyByName(_name);
}

//////////////////////////////////////////////////
void BaseScene::DestroyNodeByIndex(unsigned int _index)
{
  this->nodes->DestroyByIndex(_index);
}

//////////////////////////////////////////////////
void BaseScene::DestroyNodes()
{
  this->nodes->DestroyAll();
}

//////////////////////////////////////////////////
unsigned int BaseScene::GetLightCount() const
{
  return this->GetLights()->Size();
}

//////////////////////////////////////////////////
bool BaseScene::HasLight(ConstLightPtr _light) const
{
  return this->GetLights()->Contains(_light);
}

//////////////////////////////////////////////////
bool BaseScene::HasLightId(unsigned int _id) const
{
  return this->GetLights()->ContainsId(_id);
}

//////////////////////////////////////////////////
bool BaseScene::HasLightName(const std::string &_name) const
{
  return this->GetLights()->ContainsName(_name);
}

//////////////////////////////////////////////////
LightPtr BaseScene::GetLightById(unsigned int _id) const
{
  return this->GetLights()->GetById(_id);
}

//////////////////////////////////////////////////
LightPtr BaseScene::GetLightByName(const std::string &_name) const
{
  return this->GetLights()->GetByName(_name);
}

//////////////////////////////////////////////////
LightPtr BaseScene::GetLightByIndex(unsigned int _index) const
{
  return this->GetLights()->GetByIndex(_index);
}

//////////////////////////////////////////////////
void BaseScene::DestroyLight(LightPtr _light)
{
  this->GetLights()->Destroy(_light);
}

//////////////////////////////////////////////////
void BaseScene::DestroyLightById(unsigned int _id)
{
  this->GetLights()->DestroyById(_id);
}

//////////////////////////////////////////////////
void BaseScene::DestroyLightByName(const std::string &_name)
{
  this->GetLights()->DestroyByName(_name);
}

//////////////////////////////////////////////////
void BaseScene::DestroyLightByIndex(unsigned int _index)
{
  this->GetLights()->DestroyByIndex(_index);
}

//////////////////////////////////////////////////
void BaseScene::DestroyLights()
{
  this->GetLights()->DestroyAll();
}

//////////////////////////////////////////////////
unsigned int BaseScene::GetSensorCount() const
{
  return this->GetSensors()->Size();
}

//////////////////////////////////////////////////
bool BaseScene::HasSensor(ConstSensorPtr _sensor) const
{
  return this->GetSensors()->Contains(_sensor);
}

//////////////////////////////////////////////////
bool BaseScene::HasSensorId(unsigned int _id) const
{
  return this->GetSensors()->ContainsId(_id);
}

//////////////////////////////////////////////////
bool BaseScene::HasSensorName(const std::string &_name) const
{
  return this->GetSensors()->ContainsName(_name);
}

//////////////////////////////////////////////////
SensorPtr BaseScene::GetSensorById(unsigned int _id) const
{
  return this->GetSensors()->GetById(_id);
}

//////////////////////////////////////////////////
SensorPtr BaseScene::GetSensorByName(const std::string &_name) const
{
  return this->GetSensors()->GetByName(_name);
}

//////////////////////////////////////////////////
SensorPtr BaseScene::GetSensorByIndex(unsigned int _index) const
{
  return this->GetSensors()->GetByIndex(_index);
}

//////////////////////////////////////////////////
void BaseScene::DestroySensor(SensorPtr _sensor)
{
  this->GetSensors()->Destroy(_sensor);
}

//////////////////////////////////////////////////
void BaseScene::DestroySensorById(unsigned int _id)
{
  this->GetSensors()->DestroyById(_id);
}

//////////////////////////////////////////////////
void BaseScene::DestroySensorByName(const std::string &_name)
{
  this->GetSensors()->DestroyByName(_name);
}

//////////////////////////////////////////////////
void BaseScene::DestroySensorByIndex(unsigned int _index)
{
  this->GetSensors()->DestroyByIndex(_index);
}

//////////////////////////////////////////////////
void BaseScene::DestroySensors()
{
  this->GetSensors()->DestroyAll();
}

//////////////////////////////////////////////////
unsigned int BaseScene::GetVisualCount() const
{
  return this->GetVisuals()->Size();
}

//////////////////////////////////////////////////
bool BaseScene::HasVisual(ConstVisualPtr _visual) const
{
  return this->GetVisuals()->Contains(_visual);
}

//////////////////////////////////////////////////
bool BaseScene::HasVisualId(unsigned int _id) const
{
  return this->GetVisuals()->ContainsId(_id);
}

//////////////////////////////////////////////////
bool BaseScene::HasVisualName(const std::string &_name) const
{
  return this->GetVisuals()->ContainsName(_name);
}

//////////////////////////////////////////////////
VisualPtr BaseScene::GetVisualById(unsigned int _id) const
{
  return this->GetVisuals()->GetById(_id);
}

//////////////////////////////////////////////////
VisualPtr BaseScene::GetVisualByName(const std::string &_name) const
{
  return this->GetVisuals()->GetByName(_name);
}

//////////////////////////////////////////////////
VisualPtr BaseScene::GetVisualByIndex(unsigned int _index) const
{
  return this->GetVisuals()->GetByIndex(_index);
}

//////////////////////////////////////////////////
void BaseScene::DestroyVisual(VisualPtr _visual)
{
  this->GetVisuals()->Destroy(_visual);
}

//////////////////////////////////////////////////
void BaseScene::DestroyVisualById(unsigned int _id)
{
  this->GetVisuals()->DestroyById(_id);
}

//////////////////////////////////////////////////
void BaseScene::DestroyVisualByName(const std::string &_name)
{
  this->GetVisuals()->DestroyByName(_name);
}

//////////////////////////////////////////////////
void BaseScene::DestroyVisualByIndex(unsigned int _index)
{
  this->GetVisuals()->DestroyByIndex(_index);
}

//////////////////////////////////////////////////
void BaseScene::DestroyVisuals()
{
  this->GetVisuals()->DestroyAll();
}

//////////////////////////////////////////////////
bool BaseScene::MaterialRegistered(const std::string &_name) const
{
  return this->GetMaterials()->ContainsKey(_name);
}

//////////////////////////////////////////////////
MaterialPtr BaseScene::GetMaterial(const std::string &_name) const
{
  return this->GetMaterials()->Get(_name);
}

//////////////////////////////////////////////////
void BaseScene::RegisterMaterial(const std::string &_name,
    MaterialPtr _material)
{
  this->GetMaterials()->Put(_name, _material);
}

//////////////////////////////////////////////////
void BaseScene::UnregisterMaterial(const std::string &_name)
{
  this->GetMaterials()->Remove(_name);
}

//////////////////////////////////////////////////
void BaseScene::UnregisterMaterials()
{
  this->GetMaterials()->RemoveAll();
}

//////////////////////////////////////////////////
DirectionalLightPtr BaseScene::CreateDirectionalLight()
{
  unsigned int objId = this->CreateObjectId();
  return this->CreateDirectionalLight(objId);
}

//////////////////////////////////////////////////
DirectionalLightPtr BaseScene::CreateDirectionalLight(unsigned int _id)
{
  std::string objName = this->CreateObjectName(_id, "DirectionalLight");
  return this->CreateDirectionalLight(_id, objName);
}

//////////////////////////////////////////////////
DirectionalLightPtr BaseScene::CreateDirectionalLight(const std::string &_name)
{
  unsigned int objId = this->CreateObjectId();
  return this->CreateDirectionalLight(objId, _name);
}

//////////////////////////////////////////////////
DirectionalLightPtr BaseScene::CreateDirectionalLight(unsigned int _id,
    const std::string &_name)
{
  DirectionalLightPtr light = this->CreateDirectionalLightImpl(_id, _name);
  bool result = this->RegisterLight(light);
  return (result) ? light : NULL;
}

//////////////////////////////////////////////////
PointLightPtr BaseScene::CreatePointLight()
{
  unsigned int objId = this->CreateObjectId();
  return this->CreatePointLight(objId);
}
//////////////////////////////////////////////////
PointLightPtr BaseScene::CreatePointLight(unsigned int _id)
{
  std::string objName = this->CreateObjectName(_id, "PointLight");
  return this->CreatePointLight(_id, objName);
}
//////////////////////////////////////////////////
PointLightPtr BaseScene::CreatePointLight(const std::string &_name)
{
  unsigned int objId = this->CreateObjectId();
  return this->CreatePointLight(objId, _name);
}
//////////////////////////////////////////////////
PointLightPtr BaseScene::CreatePointLight(unsigned int _id,
    const std::string &_name)
{
  PointLightPtr light = this->CreatePointLightImpl(_id, _name);
  bool result = this->RegisterLight(light);
  return (result) ? light : NULL;
}

//////////////////////////////////////////////////
SpotLightPtr BaseScene::CreateSpotLight()
{
  unsigned int objId = this->CreateObjectId();
  return this->CreateSpotLight(objId);
}
//////////////////////////////////////////////////
SpotLightPtr BaseScene::CreateSpotLight(unsigned int _id)
{
  std::string objName = this->CreateObjectName(_id, "SpotLight");
  return this->CreateSpotLight(_id, objName);
}
//////////////////////////////////////////////////
SpotLightPtr BaseScene::CreateSpotLight(const std::string &_name)
{
  unsigned int objId = this->CreateObjectId();
  return this->CreateSpotLight(objId, _name);
}
//////////////////////////////////////////////////
SpotLightPtr BaseScene::CreateSpotLight(unsigned int _id,
    const std::string &_name)
{
  SpotLightPtr light = this->CreateSpotLightImpl(_id, _name);
  bool result = this->RegisterLight(light);
  return (result) ? light : NULL;
}

//////////////////////////////////////////////////
CameraPtr BaseScene::CreateCamera()
{
  unsigned int objId = this->CreateObjectId();
  return this->CreateCamera(objId);
}
//////////////////////////////////////////////////
CameraPtr BaseScene::CreateCamera(unsigned int _id)
{
  std::string objName = this->CreateObjectName(_id, "Camera");
  return this->CreateCamera(_id, objName);
}
//////////////////////////////////////////////////
CameraPtr BaseScene::CreateCamera(const std::string &_name)
{
  unsigned int objId = this->CreateObjectId();
  return this->CreateCamera(objId, _name);
}
//////////////////////////////////////////////////
CameraPtr BaseScene::CreateCamera(unsigned int _id, const std::string &_name)
{
  CameraPtr camera = this->CreateCameraImpl(_id, _name);
  bool result = this->RegisterSensor(camera);
  return (result) ? camera : NULL;
}

//////////////////////////////////////////////////
VisualPtr BaseScene::CreateVisual()
{
  unsigned int objId = this->CreateObjectId();
  return this->CreateVisual(objId);
}
//////////////////////////////////////////////////
VisualPtr BaseScene::CreateVisual(unsigned int _id)
{
  std::string objName = this->CreateObjectName(_id, "Visual");
  return this->CreateVisual(_id, objName);
}
//////////////////////////////////////////////////
VisualPtr BaseScene::CreateVisual(const std::string &_name)
{
  unsigned int objId = this->CreateObjectId();
  return this->CreateVisual(objId, _name);
}
//////////////////////////////////////////////////
VisualPtr BaseScene::CreateVisual(unsigned int _id, const std::string &_name)
{
  VisualPtr visual = this->CreateVisualImpl(_id, _name);
  bool result = this->RegisterVisual(visual);
  return (result) ? visual : NULL;
}

//////////////////////////////////////////////////
ArrowVisualPtr BaseScene::CreateArrowVisual()
{
  unsigned int objId = this->CreateObjectId();
  return this->CreateArrowVisual(objId);
}
//////////////////////////////////////////////////
ArrowVisualPtr BaseScene::CreateArrowVisual(unsigned int _id)
{
  std::string objName = this->CreateObjectName(_id, "ArrowVisual");
  return this->CreateArrowVisual(_id, objName);
}
//////////////////////////////////////////////////
ArrowVisualPtr BaseScene::CreateArrowVisual(const std::string &_name)
{
  unsigned int objId = this->CreateObjectId();
  return this->CreateArrowVisual(objId, _name);
}
//////////////////////////////////////////////////
ArrowVisualPtr BaseScene::CreateArrowVisual(unsigned int _id,
    const std::string &_name)
{
  ArrowVisualPtr visual = this->CreateArrowVisualImpl(_id, _name);
  bool result = this->RegisterVisual(visual);
  return (result) ? visual : NULL;
}

//////////////////////////////////////////////////
AxisVisualPtr BaseScene::CreateAxisVisual()
{
  unsigned int objId = this->CreateObjectId();
  return this->CreateAxisVisual(objId);
}
//////////////////////////////////////////////////
AxisVisualPtr BaseScene::CreateAxisVisual(unsigned int _id)
{
  std::string objName = this->CreateObjectName(_id, "AxisVisual");
  return this->CreateAxisVisual(_id, objName);
}
//////////////////////////////////////////////////
AxisVisualPtr BaseScene::CreateAxisVisual(const std::string &_name)
{
  unsigned int objId = this->CreateObjectId();
  return this->CreateAxisVisual(objId, _name);
}
//////////////////////////////////////////////////
AxisVisualPtr BaseScene::CreateAxisVisual(unsigned int _id,
    const std::string &_name)
{
  AxisVisualPtr visual = this->CreateAxisVisualImpl(_id, _name);
  bool result = this->RegisterVisual(visual);
  return (result) ? visual : NULL;
}

//////////////////////////////////////////////////
GeometryPtr BaseScene::CreateBox()
{
  unsigned int objId = this->CreateObjectId();
  std::string objName = this->CreateObjectName(objId, "Box");
  return this->CreateBoxImpl(objId, objName);
}

//////////////////////////////////////////////////
GeometryPtr BaseScene::CreateCone()
{
  unsigned int objId = this->CreateObjectId();
  std::string objName = this->CreateObjectName(objId, "Cone");
  return this->CreateConeImpl(objId, objName);
}

//////////////////////////////////////////////////
GeometryPtr BaseScene::CreateCylinder()
{
  unsigned int objId = this->CreateObjectId();
  std::string objName = this->CreateObjectName(objId, "Cylinder");
  return this->CreateCylinderImpl(objId, objName);
}

//////////////////////////////////////////////////
GeometryPtr BaseScene::CreatePlane()
{
  unsigned int objId = this->CreateObjectId();
  std::string objName = this->CreateObjectName(objId, "Plane");
  return this->CreatePlaneImpl(objId, objName);
}

//////////////////////////////////////////////////
GeometryPtr BaseScene::CreateSphere()
{
  unsigned int objId = this->CreateObjectId();
  std::string objName = this->CreateObjectName(objId, "Sphere");
  return this->CreateSphereImpl(objId, objName);
}

//////////////////////////////////////////////////
MeshPtr BaseScene::CreateMesh(const std::string &_meshName)
{
  MeshDescriptor descriptor(_meshName);
  return this->CreateMesh(descriptor);
}

//////////////////////////////////////////////////
MeshPtr BaseScene::CreateMesh(const gazebo::common::Mesh *_mesh)
{
  MeshDescriptor descriptor(_mesh);
  return this->CreateMesh(descriptor);
}

//////////////////////////////////////////////////
MeshPtr BaseScene::CreateMesh(const MeshDescriptor &_desc)
{
  std::string meshName = (_desc.mesh) ?
      _desc.mesh->GetName() : _desc.meshName;

  unsigned int objId = this->CreateObjectId();
  std::string objName = this->CreateObjectName(objId, "Mesh-" + meshName);
  return this->CreateMeshImpl(objId, name, _desc);
}

//////////////////////////////////////////////////
MaterialPtr BaseScene::CreateMaterial()
{
  unsigned int objId = this->CreateObjectId();
  std::string objName = this->CreateObjectName(objId, "Material");
  return this->CreateMaterialImpl(objId, objName);
}

//////////////////////////////////////////////////
MaterialPtr BaseScene::CreateMaterial(const gazebo::common::Material &_material)
{
  MaterialPtr material = this->CreateMaterial();
  material->CopyFrom(_material);
  return material;
}

//////////////////////////////////////////////////
RenderTexturePtr BaseScene::CreateRenderTexture()
{
  unsigned int objId = this->CreateObjectId();
  std::string objName = this->CreateObjectName(objId, "RenderTexture");
  return this->CreateRenderTextureImpl(objId, objName);
}

//////////////////////////////////////////////////
void BaseScene::PreRender()
{
  this->GetRootVisual()->PreRender();
}

//////////////////////////////////////////////////
void BaseScene::Clear()
{
  this->nodes->DestroyAll();
  this->GetMaterials()->RemoveAll();
  this->nextObjectId = UINT_MAX;
}

//////////////////////////////////////////////////
void BaseScene::Destroy()
{
  // TODO: destroy context
  this->Clear();
}

//////////////////////////////////////////////////
unsigned int BaseScene::CreateObjectId()
{
  return this->nextObjectId--;
}

//////////////////////////////////////////////////
std::string BaseScene::CreateObjectName(unsigned int _id,
    const std::string &_prefix)
{
  std::stringstream ss;
  ss << this->name << "::" << _prefix;
  ss << "(" << std::to_string(_id) << ")";
  return ss.str();
}

//////////////////////////////////////////////////
bool BaseScene::RegisterLight(LightPtr _light)
{
  return (_light) ? this->GetLights()->Add(_light) : false;
}

//////////////////////////////////////////////////
bool BaseScene::RegisterSensor(SensorPtr _sensor)
{
  return (_sensor) ? this->GetSensors()->Add(_sensor) : false;
}

//////////////////////////////////////////////////
bool BaseScene::RegisterVisual(VisualPtr _visual)
{
  return (_visual) ? this->GetVisuals()->Add(_visual) : false;
}

//////////////////////////////////////////////////
void BaseScene::CreateNodeStore()
{
  NodeCompositeStorePtr compStore(new BaseNodeCompositeStore);

  // get specific stores
  LightStorePtr lights = this->GetLights();
  SensorStorePtr sensors = this->GetSensors();
  VisualStorePtr visuals = this->GetVisuals();

  // convert to node stores
  NodeStorePtr lightNodes(new BaseStoreWrapper<Node, Light>(lights));
  NodeStorePtr sensorNodes(new BaseStoreWrapper<Node, Sensor>(sensors));
  NodeStorePtr visualNodes(new BaseStoreWrapper<Node, Visual>(visuals));

  // add to composite store
  compStore->AddStore(lightNodes);
  compStore->AddStore(sensorNodes);
  compStore->AddStore(visualNodes);

  this->nodes = compStore;
}
