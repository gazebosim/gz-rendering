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

#include <ignition/common/Console.hh>

#include "ignition/rendering/RenderTypes.hh"
#include "ignition/rendering/ogre2/Ogre2Conversions.hh"

#include "ignition/rendering/ogre2/Ogre2Includes.hh"
#include "ignition/rendering/ogre2/Ogre2Node.hh"
#include "ignition/rendering/ogre2/Ogre2RenderEngine.hh"
#include "ignition/rendering/ogre2/Ogre2Scene.hh"

using namespace ignition;
using namespace rendering;

//////////////////////////////////////////////////
Ogre2Scene::Ogre2Scene(unsigned int _id, const std::string &_name) :
  BaseScene(_id, _name)
{
}

//////////////////////////////////////////////////
Ogre2Scene::~Ogre2Scene()
{
}

//////////////////////////////////////////////////
void Ogre2Scene::Fini()
{
}

//////////////////////////////////////////////////
RenderEngine *Ogre2Scene::Engine() const
{
  return Ogre2RenderEngine::Instance();
}

//////////////////////////////////////////////////
VisualPtr Ogre2Scene::RootVisual() const
{
  // TODO(anyone)
  return VisualPtr();
}

//////////////////////////////////////////////////
math::Color Ogre2Scene::AmbientLight() const
{
  // TODO(anyone)
  return math::Color::White;
}

//////////////////////////////////////////////////
void Ogre2Scene::SetAmbientLight(const math::Color &/*_color*/)
{
  // TODO(anyone)
}

//////////////////////////////////////////////////
void Ogre2Scene::PreRender()
{
  BaseScene::PreRender();
}

//////////////////////////////////////////////////
void Ogre2Scene::Clear()
{
}

//////////////////////////////////////////////////
void Ogre2Scene::Destroy()
{
}

//////////////////////////////////////////////////
Ogre::SceneManager *Ogre2Scene::OgreSceneManager() const
{
  return this->ogreSceneManager;
}

//////////////////////////////////////////////////
bool Ogre2Scene::LoadImpl()
{
  return true;
}

//////////////////////////////////////////////////
bool Ogre2Scene::InitImpl()
{
  this->CreateContext();
  this->CreateRootVisual();
  this->CreateStores();
  this->CreateMeshFactory();

  return true;
}

//////////////////////////////////////////////////
LightStorePtr Ogre2Scene::Lights() const
{
  // TODO(anyone)
  return LightStorePtr();
}

//////////////////////////////////////////////////
SensorStorePtr Ogre2Scene::Sensors() const
{
  // TODO(anyone)
  return SensorStorePtr();
}

//////////////////////////////////////////////////
VisualStorePtr Ogre2Scene::Visuals() const
{
  // TODO(anyone)
  return VisualStorePtr();
}

//////////////////////////////////////////////////
MaterialMapPtr Ogre2Scene::Materials() const
{
  // TODO(anyone)
  return MaterialMapPtr();
}

//////////////////////////////////////////////////
DirectionalLightPtr Ogre2Scene::CreateDirectionalLightImpl(unsigned int /*_id*/,
    const std::string &/*_name*/)
{
  // TODO(anyone)
  return DirectionalLightPtr();
}

//////////////////////////////////////////////////
PointLightPtr Ogre2Scene::CreatePointLightImpl(unsigned int /*_id*/,
    const std::string &/*_name*/)
{
  // TODO(anyone)
  return PointLightPtr();
}

//////////////////////////////////////////////////
SpotLightPtr Ogre2Scene::CreateSpotLightImpl(unsigned int /*_id*/,
    const std::string &/*`_name*/)
{
  // TODO(anyone)
  return SpotLightPtr();
}

//////////////////////////////////////////////////
CameraPtr Ogre2Scene::CreateCameraImpl(unsigned int /*_id*/,
    const std::string &/*_name*/)
{
  // TODO(anyone)
  return CameraPtr();
}

//////////////////////////////////////////////////
VisualPtr Ogre2Scene::CreateVisualImpl(unsigned int /*_id*/,
    const std::string &/*_name*/)
{
  // TODO(anyone)
  return VisualPtr();
}

//////////////////////////////////////////////////
ArrowVisualPtr Ogre2Scene::CreateArrowVisualImpl(unsigned int /*_id*/,
    const std::string &/*_name*/)
{
  // TODO(anyone)
  return ArrowVisualPtr();
}

//////////////////////////////////////////////////
AxisVisualPtr Ogre2Scene::CreateAxisVisualImpl(unsigned int /*_id*/,
    const std::string &/*_name*/)
{
  // TODO(anyone)
  return AxisVisualPtr();
}

//////////////////////////////////////////////////
GeometryPtr Ogre2Scene::CreateBoxImpl(unsigned int _id,
    const std::string &_name)
{
  return this->CreateMeshImpl(_id, _name, "unit_box");
}

//////////////////////////////////////////////////
GeometryPtr Ogre2Scene::CreateConeImpl(unsigned int _id,
    const std::string &_name)
{
  return this->CreateMeshImpl(_id, _name, "unit_cone");
}

//////////////////////////////////////////////////
GeometryPtr Ogre2Scene::CreateCylinderImpl(unsigned int _id,
    const std::string &_name)
{
  return this->CreateMeshImpl(_id, _name, "unit_cylinder");
}

//////////////////////////////////////////////////
GeometryPtr Ogre2Scene::CreatePlaneImpl(unsigned int _id,
    const std::string &_name)
{
  return this->CreateMeshImpl(_id, _name, "unit_plane");
}

//////////////////////////////////////////////////
GeometryPtr Ogre2Scene::CreateSphereImpl(unsigned int _id,
    const std::string &_name)
{
  return this->CreateMeshImpl(_id, _name, "unit_sphere");
}

//////////////////////////////////////////////////
MeshPtr Ogre2Scene::CreateMeshImpl(unsigned int _id, const std::string &_name,
    const std::string &_meshName)
{
  MeshDescriptor descriptor(_meshName);
  return this->CreateMeshImpl(_id, _name, descriptor);
}

//////////////////////////////////////////////////
MeshPtr Ogre2Scene::CreateMeshImpl(unsigned int /*_id*/,
    const std::string &/*_name*/, const MeshDescriptor &/*_desc*/)
{
  // TODO(anyone)
  return MeshPtr();
}

//////////////////////////////////////////////////
GridPtr Ogre2Scene::CreateGridImpl(unsigned int /*_id*/,
    const std::string &/*_name*/)
{
  // TODO(anyone)
  return GridPtr();
}

//////////////////////////////////////////////////
TextPtr Ogre2Scene::CreateTextImpl(unsigned int /*_id*/,
    const std::string &/*_name*/)
{
  // TODO(anyone)
  return TextPtr();
}

//////////////////////////////////////////////////
MaterialPtr Ogre2Scene::CreateMaterialImpl(unsigned int /*_id*/,
    const std::string &/*_name*/)
{
  // TODO(anyone)
  return MaterialPtr();
}

//////////////////////////////////////////////////
RenderTexturePtr Ogre2Scene::CreateRenderTextureImpl(unsigned int /*_id*/,
    const std::string &/*_name*/)
{
  // TODO(anyone)
  return RenderTexturePtr();
}

//////////////////////////////////////////////////
RenderWindowPtr Ogre2Scene::CreateRenderWindowImpl(unsigned int /*_id*/,
    const std::string &/*_name*/)
{
  // TODO(anyone)
  return RenderWindowPtr();
}

//////////////////////////////////////////////////
RayQueryPtr Ogre2Scene::CreateRayQueryImpl(unsigned int /*_id*/,
    const std::string &/*_name*/)
{
  // TODO(anyone)
  return RayQueryPtr();
}

//////////////////////////////////////////////////
bool Ogre2Scene::InitObject(Ogre2ObjectPtr _object, unsigned int _id,
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
void Ogre2Scene::CreateContext()
{
  Ogre::Root *root = Ogre2RenderEngine::Instance()->OgreRoot();

  Ogre::InstancingThreadedCullingMethod threadedCullingMethod =
      Ogre::INSTANCING_CULLING_SINGLETHREAD;
  // getNumLogicalCores() may return 0 if couldn't detect
  const size_t numThreads = std::max<size_t>(
      1, Ogre::PlatformInformation::getNumLogicalCores());

  // See ogre doxygen documentation regarding culling methods.
  // In some cases you may still want to use single thread.
  // if( numThreads > 1 )
  //   threadedCullingMethod = Ogre::INSTANCING_CULLING_THREADED;
  // Create the SceneManager, in this case a generic one
  this->ogreSceneManager = root->createSceneManager(Ogre::ST_GENERIC,
                                                   numThreads,
                                                   threadedCullingMethod);

  this->ogreSceneManager->addRenderQueueListener(
      Ogre2RenderEngine::Instance()->OverlaySystem());

  this->ogreSceneManager->getRenderQueue()->setSortRenderQueue(
      Ogre::v1::OverlayManager::getSingleton().mDefaultRenderQueueId,
      Ogre::RenderQueue::StableSort);
}

//////////////////////////////////////////////////
void Ogre2Scene::CreateRootVisual()
{
  // TODO(anyone)
}

//////////////////////////////////////////////////
void Ogre2Scene::CreateMeshFactory()
{
  // TODO(anyone)
}

//////////////////////////////////////////////////
void Ogre2Scene::CreateStores()
{
  // TODO(anyone)
}

//////////////////////////////////////////////////
Ogre2ScenePtr Ogre2Scene::SharedThis()
{
  ScenePtr sharedBase = this->shared_from_this();
  return std::dynamic_pointer_cast<Ogre2Scene>(sharedBase);
}
