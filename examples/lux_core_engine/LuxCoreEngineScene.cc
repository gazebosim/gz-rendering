#include "LuxCoreEngineScene.hh"

using namespace ignition;
using namespace rendering;

void LuxCoreEngineScene::LogHandler(const char *msg) {}

LuxCoreEngineScene::LuxCoreEngineScene(unsigned int _id, const std::string &_name) : BaseScene(_id, _name) 
{
  this->id_ = _id;
  this->name_ = _name;

}

LuxCoreEngineScene::~LuxCoreEngineScene() {}

void LuxCoreEngineScene::Fini() {}

RenderEngine *LuxCoreEngineScene::Engine() const 
{
  ignerr << "engine() was called 246" << std::endl;
  // return LuxCoreEngineRenderEngine::Instance();
  return nullptr;
}

VisualPtr LuxCoreEngineScene::RootVisual() const 
{
  return nullptr;
}

math::Color LuxCoreEngineScene::AmbientLight() const 
{
  return math::Color::Black;
}

void LuxCoreEngineScene::SetAmbientLight(const math::Color &_color) 
{

}

DirectionalLightPtr LuxCoreEngineScene::CreateDirectionalLightImpl(
                 unsigned int _id, const std::string &_name) 
{
  LuxCoreEngineDirectionalLightPtr light(new LuxCoreEngineDirectionalLight);
  bool result = this->InitObject(light, _id, _name);
  return (result) ? light : nullptr;
}

PointLightPtr LuxCoreEngineScene::CreatePointLightImpl(unsigned int _id,
                 const std::string &_name)
{
  return nullptr;
}

SpotLightPtr LuxCoreEngineScene::CreateSpotLightImpl(unsigned int _id,
                 const std::string &_name)
{
  return nullptr;
}

CameraPtr LuxCoreEngineScene::CreateCameraImpl(unsigned int _id,
                 const std::string &_name)
{
  LuxCoreEngineCameraPtr camera(new LuxCoreEngineCamera);
  bool result = this->InitObject(camera, _id, _name);
  return (result) ? camera : nullptr;
}

DepthCameraPtr LuxCoreEngineScene::CreateDepthCameraImpl(unsigned int _id,
                 const std::string &_name) 
{
  return nullptr;
}

VisualPtr LuxCoreEngineScene::CreateVisualImpl(unsigned int _id,
                 const std::string &_name)
{
  LuxCoreEngineVisualPtr visual(new LuxCoreEngineVisual);
  bool result = this->InitObject(visual, _id, _name);
  return (result) ? visual : nullptr;
}

ArrowVisualPtr LuxCoreEngineScene::CreateArrowVisualImpl(unsigned int _id,
                 const std::string &_name)
{
  return nullptr;
}

AxisVisualPtr LuxCoreEngineScene::CreateAxisVisualImpl(unsigned int _id,
                 const std::string &_name)
{
  return nullptr;
}

GeometryPtr LuxCoreEngineScene::CreateBoxImpl(unsigned int _id,
                 const std::string &_name)
{
  return this->CreateMeshImpl(_id, _name, "unit_box");
}

GeometryPtr LuxCoreEngineScene::CreateConeImpl(unsigned int _id,
                 const std::string &_name)
{
  return nullptr;
}

GeometryPtr LuxCoreEngineScene::CreateCylinderImpl(unsigned int _id,
                 const std::string &_name)
{
  return nullptr;
}

GeometryPtr LuxCoreEngineScene::CreatePlaneImpl(unsigned int _id,
                 const std::string &_name)
{
  return this->CreateMeshImpl(_id, _name, "unit_plane");
}

GeometryPtr LuxCoreEngineScene::CreateSphereImpl(unsigned int _id,
                 const std::string &_name)
{
  return nullptr;
}

MeshPtr LuxCoreEngineScene::CreateMeshImpl(unsigned int _id,
                 const std::string &_name, const std::string &_meshName)
{
  MeshDescriptor descriptor(_meshName);
  return this->CreateMeshImpl(_id, _name, descriptor);
}

MeshPtr LuxCoreEngineScene::CreateMeshImpl(unsigned int _id,
                 const std::string &_name, const MeshDescriptor &_desc)
{
  LuxCoreEngineMeshPtr mesh = this->meshFactory->Create(_desc, _name);
  if (nullptr == mesh)
    return nullptr;

  sceneLux->Parse(
	    luxrays::Property("scene.objects." + _name + ".shape")(_name + "-mesh") <<
	    luxrays::Property("scene.objects." + _name  + ".material")("Default/White"));

  bool result = this->InitObject(mesh, _id, _name);
  return (result) ? mesh : nullptr;
}

CapsulePtr LuxCoreEngineScene::CreateCapsuleImpl(unsigned int _id,
                 const std::string &_name)
{
  return nullptr;
}

GridPtr LuxCoreEngineScene::CreateGridImpl(unsigned int _id,
                 const std::string &_name)
{
  return nullptr;
}

MarkerPtr LuxCoreEngineScene::CreateMarkerImpl(unsigned int _id,
                 const std::string &_name)
{
  return nullptr;
}

LidarVisualPtr LuxCoreEngineScene::CreateLidarVisualImpl(unsigned int _id,
                 const std::string &_name)
{
  return nullptr;
}

HeightmapPtr LuxCoreEngineScene::CreateHeightmapImpl(unsigned int _id,
                 const std::string &_name,
                 const HeightmapDescriptor &_desc)
{
  return nullptr;
}

WireBoxPtr LuxCoreEngineScene::CreateWireBoxImpl(unsigned int _id,
                 const std::string &_name)
{
  return nullptr;
}

MaterialPtr LuxCoreEngineScene::CreateMaterialImpl(unsigned int _id,
                 const std::string &_name)
{
  LuxCoreEngineMaterialPtr material(new LuxCoreEngineMaterial);

  sceneLux->Parse(
	    luxrays::Property() <<
	    luxrays::Property("scene.materials." + Name() + ".id")(_id));

  bool result = this->InitObject(material, _id, _name);
  return (result) ? material : nullptr;
}

RenderTexturePtr LuxCoreEngineScene::CreateRenderTextureImpl(
                 unsigned int _id, const std::string &_name)
{
  return nullptr;
}

RenderWindowPtr LuxCoreEngineScene::CreateRenderWindowImpl(
                 unsigned int _id, const std::string &_name)
{
  return nullptr;
}

RayQueryPtr LuxCoreEngineScene::CreateRayQueryImpl(
                 unsigned int _id, const std::string &_name)
{
  return nullptr;
}

LightStorePtr LuxCoreEngineScene::Lights() const
{
  return this->lights;
}

SensorStorePtr LuxCoreEngineScene::Sensors() const
{
  return this->sensors;
}

VisualStorePtr LuxCoreEngineScene::Visuals() const
{
  return this->visuals;
}

MaterialMapPtr LuxCoreEngineScene::Materials() const
{
  return this->materials;
}

bool LuxCoreEngineScene::LoadImpl()
{
  return true;
}

bool LuxCoreEngineScene::InitImpl()
{  
  luxcore::Init(
//    LogHandler
  );

  sceneLux = luxcore::Scene::Create();
  sceneLux->Parse(
      luxrays::Property("scene.camera.lookat.orig")(1.f , 6.f , 3.f) <<
		  luxrays::Property("scene.camera.lookat.target")(0.f , 0.f , .5f) <<
		  luxrays::Property("scene.camera.fieldofview")(60.f));

  sceneLux->Parse(
		  luxrays::Property("scene.lights.skyl.type")("sky2") <<
		  luxrays::Property("scene.lights.skyl.dir")(0.166974f, 0.59908f, 0.783085f) <<
		  luxrays::Property("scene.lights.skyl.turbidity")(2.2f) <<
		  luxrays::Property("scene.lights.skyl.gain")(0.8f, 0.8f, 0.8f) <<
		  luxrays::Property("scene.lights.sunl.type")("sun") <<
		  luxrays::Property("scene.lights.sunl.dir")(0.166974f, 0.59908f, 0.783085f) <<
		  luxrays::Property("scene.lights.sunl.turbidity")(2.2f) <<
		  luxrays::Property("scene.lights.sunl.gain")(0.8f, 0.8f, 0.8f));
  
  this->CreateStores();
  this->CreateMeshFactory();
  return true;
}

bool LuxCoreEngineScene::IsInitialized() 
{
  return true;
}

unsigned int LuxCoreEngineScene::Id() 
{
  return this->id_;
}

std::string LuxCoreEngineScene::Name() 
{
  return this->name_;
}

ignition::rendering::LightVisualPtr LuxCoreEngineScene::CreateLightVisualImpl(unsigned int _id,
                 const std::string &_name)
{
  return nullptr;
}

void LuxCoreEngineScene::CreateMeshFactory()
{
  LuxCoreEngineScenePtr sharedThis = this->SharedThis();
  this->meshFactory = LuxCoreEngineMeshFactoryPtr(new LuxCoreEngineMeshFactory(sharedThis));
}

void LuxCoreEngineScene::CreateStores()
{
  this->lights = LuxCoreEngineLightStorePtr(new LuxCoreEngineLightStore);
  this->sensors = LuxCoreEngineSensorStorePtr(new LuxCoreEngineSensorStore);
  this->visuals = LuxCoreEngineVisualStorePtr(new LuxCoreEngineVisualStore);
  this->materials = LuxCoreEngineMaterialMapPtr(new LuxCoreEngineMaterialMap);
}

bool LuxCoreEngineScene::InitObject(LuxCoreEngineObjectPtr _object, unsigned int _id,
                const std::string &_name)
{
  // assign needed varibles
  _object->id = _id;
  _object->name = _name;
  _object->scene = this->SharedThis();

  // initialize object
  _object->Load();
  _object->Init();

  return true;
}

LuxCoreEngineScenePtr LuxCoreEngineScene::SharedThis()
{
  ScenePtr sharedBase = this->shared_from_this();
  return std::dynamic_pointer_cast<LuxCoreEngineScene>(sharedBase);
}

luxcore::Scene *LuxCoreEngineScene::SceneLux()
{
  return this->sceneLux;
}
