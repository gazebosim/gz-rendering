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

#include "ignition/rendering/ogre/OgreScene.hh"
#include "ignition/rendering/ogre/ogre.hh"

using namespace ignition;
using namespace rendering;

//////////////////////////////////////////////////
OgreScene::OgreScene(unsigned int _id, const std::string &_name) :
  BaseScene(_id, _name),
  rootVisual(NULL),
  meshFactory(NULL),
  ogreRoot(NULL),
  ogreSceneManager(NULL)
{
  this->backgroundColor = math::Color::Black;
}

//////////////////////////////////////////////////
OgreScene::~OgreScene()
{
}

//////////////////////////////////////////////////
void OgreScene::Fini()
{
}

//////////////////////////////////////////////////
RenderEngine *OgreScene::Engine() const
{
  return OgreRenderEngine::Instance();
}

//////////////////////////////////////////////////
VisualPtr OgreScene::RootVisual() const
{
  return this->rootVisual;
}

//////////////////////////////////////////////////
math::Color OgreScene::AmbientLight() const
{
  Ogre::ColourValue ogreColor = this->ogreSceneManager->getAmbientLight();
  return OgreConversions::Convert(ogreColor);
}

//////////////////////////////////////////////////
void OgreScene::SetAmbientLight(const math::Color &_color)
{
  Ogre::ColourValue ogreColor = OgreConversions::Convert(_color);
  this->ogreSceneManager->setAmbientLight(ogreColor);
}

//////////////////////////////////////////////////
math::Color OgreScene::BackgroundColor() const
{
  return this->backgroundColor;
}

//////////////////////////////////////////////////
void OgreScene::SetBackgroundColor(const math::Color &_color)
{
  this->backgroundColor = _color;

  // TODO: clean up code
  unsigned int count = this->SensorCount();

  for (unsigned int i = 0; i < count; ++i)
  {
    SensorPtr sensor = this->SensorByIndex(i);
    OgreCameraPtr camera = std::dynamic_pointer_cast<OgreCamera>(sensor);
    if (camera) camera->SetBackgroundColor(_color);
  }
}

//////////////////////////////////////////////////
void OgreScene::PreRender()
{
  BaseScene::PreRender();
  OgreRTShaderSystem::Instance()->UpdateShaders();
}

//////////////////////////////////////////////////
void OgreScene::Clear()
{
}

//////////////////////////////////////////////////
void OgreScene::Destroy()
{
}

//////////////////////////////////////////////////
Ogre::SceneManager *OgreScene::OgreSceneManager() const
{
  return this->ogreSceneManager;
}

//////////////////////////////////////////////////
bool OgreScene::LoadImpl()
{
  return true;
}

//////////////////////////////////////////////////
bool OgreScene::InitImpl()
{
  this->CreateContext();
  this->CreateRootVisual();
  this->CreateStores();
  this->CreateMeshFactory();

  OgreRTShaderSystem::Instance()->AddScene(this->SharedThis());
  OgreRTShaderSystem::Instance()->ApplyShadows(this->SharedThis());

  return true;
}

//////////////////////////////////////////////////
LightStorePtr OgreScene::Lights() const
{
  return this->lights;
}

//////////////////////////////////////////////////
SensorStorePtr OgreScene::Sensors() const
{
  return this->sensors;
}

//////////////////////////////////////////////////
VisualStorePtr OgreScene::Visuals() const
{
  return this->visuals;
}

//////////////////////////////////////////////////
MaterialMapPtr OgreScene::Materials() const
{
  return this->materials;
}

//////////////////////////////////////////////////
DirectionalLightPtr OgreScene::CreateDirectionalLightImpl(unsigned int _id,
    const std::string &_name)
{
  OgreDirectionalLightPtr light(new OgreDirectionalLight);
  bool result = this->InitObject(light, _id, _name);
  return (result) ? light : NULL;
}

//////////////////////////////////////////////////
PointLightPtr OgreScene::CreatePointLightImpl(unsigned int _id,
    const std::string &_name)
{
  OgrePointLightPtr light(new OgrePointLight);
  bool result = this->InitObject(light, _id, _name);
  return (result) ? light : NULL;
}

//////////////////////////////////////////////////
SpotLightPtr OgreScene::CreateSpotLightImpl(unsigned int _id,
    const std::string &_name)
{
  OgreSpotLightPtr light(new OgreSpotLight);
  bool result = this->InitObject(light, _id, _name);
  return (result) ? light : NULL;
}

//////////////////////////////////////////////////
CameraPtr OgreScene::CreateCameraImpl(unsigned int _id,
    const std::string &_name)
{
  OgreCameraPtr camera(new OgreCamera);
  bool result = this->InitObject(camera, _id, _name);
  camera->SetBackgroundColor(this->backgroundColor);
  return (result) ? camera : NULL;
}

//////////////////////////////////////////////////
VisualPtr OgreScene::CreateVisualImpl(unsigned int _id,
    const std::string &_name)
{
  OgreVisualPtr visual(new OgreVisual);
  bool result = this->InitObject(visual, _id, _name);
  return (result) ? visual : NULL;
}

//////////////////////////////////////////////////
ArrowVisualPtr OgreScene::CreateArrowVisualImpl(unsigned int _id,
    const std::string &_name)
{
  OgreArrowVisualPtr visual(new OgreArrowVisual);
  bool result = this->InitObject(visual, _id, _name);
  return (result) ? visual : NULL;
}

//////////////////////////////////////////////////
AxisVisualPtr OgreScene::CreateAxisVisualImpl(unsigned int _id,
    const std::string &_name)
{
  OgreAxisVisualPtr visual(new OgreAxisVisual);
  bool result = this->InitObject(visual, _id, _name);
  return (result) ? visual : NULL;
}

//////////////////////////////////////////////////
GeometryPtr OgreScene::CreateBoxImpl(unsigned int _id,
    const std::string &_name)
{
  return this->CreateMeshImpl(_id, _name, "unit_box");
}

//////////////////////////////////////////////////
GeometryPtr OgreScene::CreateConeImpl(unsigned int _id,
    const std::string &_name)
{
  return this->CreateMeshImpl(_id, _name, "unit_cone");
}

//////////////////////////////////////////////////
GeometryPtr OgreScene::CreateCylinderImpl(unsigned int _id,
    const std::string &_name)
{
  return this->CreateMeshImpl(_id, _name, "unit_cylinder");
}

//////////////////////////////////////////////////
GeometryPtr OgreScene::CreatePlaneImpl(unsigned int _id,
    const std::string &_name)
{
  return this->CreateMeshImpl(_id, _name, "unit_plane");
}

//////////////////////////////////////////////////
GeometryPtr OgreScene::CreateSphereImpl(unsigned int _id,
    const std::string &_name)
{
  return this->CreateMeshImpl(_id, _name, "unit_sphere");
}

//////////////////////////////////////////////////
MeshPtr OgreScene::CreateMeshImpl(unsigned int _id, const std::string &_name,
    const std::string &_meshName)
{
  MeshDescriptor descriptor(_meshName);
  return this->CreateMeshImpl(_id, _name, descriptor);
}

//////////////////////////////////////////////////
MeshPtr OgreScene::CreateMeshImpl(unsigned int _id, const std::string &_name,
    const MeshDescriptor &_desc)
{
  OgreMeshPtr mesh = this->meshFactory->Create(_desc);
  bool result = this->InitObject(mesh, _id, _name);
  return (result) ? mesh : NULL;
}

//////////////////////////////////////////////////
MaterialPtr OgreScene::CreateMaterialImpl(unsigned int _id,
    const std::string &_name)
{
  OgreMaterialPtr material(new OgreMaterial);
  bool result = this->InitObject(material, _id, _name);
  return (result) ? material : NULL;
}

//////////////////////////////////////////////////
RenderTexturePtr OgreScene::CreateRenderTextureImpl(unsigned int _id,
    const std::string &_name)
{
  OgreRenderTexturePtr renderTexture(new OgreRenderTexture);
  bool result = this->InitObject(renderTexture, _id, _name);
  return (result) ? renderTexture : NULL;
}

//////////////////////////////////////////////////
bool OgreScene::InitObject(OgreObjectPtr _object, unsigned int _id,
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

//////////////////////////////////////////////////
void OgreScene::CreateContext()
{
  Ogre::Root *root = OgreRenderEngine::Instance()->OgreRoot();
  this->ogreSceneManager = root->createSceneManager(Ogre::ST_GENERIC);
}

//////////////////////////////////////////////////
void OgreScene::CreateRootVisual()
{
  // create unregistered visual
  this->rootVisual = OgreVisualPtr(new OgreVisual);
  unsigned int rootId = this->CreateObjectId();
  std::string rootName = this->CreateObjectName(rootId, "_ROOT_");

  // check if root visual created successfully
  if (!this->InitObject(this->rootVisual, rootId, rootName))
  {
    ignerr << "Unable to create root visual" << std::endl;
    this->rootVisual = NULL;
  }

  // add visual node to actual ogre root
  Ogre::SceneNode *ogreRootNode = this->rootVisual->Node();
  this->ogreSceneManager->getRootSceneNode()->addChild(ogreRootNode);
}

//////////////////////////////////////////////////
void OgreScene::CreateMeshFactory()
{
  OgreScenePtr sharedThis = this->SharedThis();
  this->meshFactory = OgreMeshFactoryPtr(new OgreMeshFactory(sharedThis));
}

//////////////////////////////////////////////////
void OgreScene::CreateStores()
{
  this->lights = OgreLightStorePtr(new OgreLightStore);
  this->sensors = OgreSensorStorePtr(new OgreSensorStore);
  this->visuals = OgreVisualStorePtr(new OgreVisualStore);
  this->materials = OgreMaterialMapPtr(new OgreMaterialMap);
}

//////////////////////////////////////////////////
OgreScenePtr OgreScene::SharedThis()
{
  ScenePtr sharedBase = this->shared_from_this();
  return std::dynamic_pointer_cast<OgreScene>(sharedBase);
}

//////////////////////////////////////////////////
