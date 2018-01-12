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
#include "ignition/rendering/ogre/OgreIncludes.hh"

using namespace ignition;
using namespace rendering;

//////////////////////////////////////////////////
OgreScene::OgreScene(unsigned int _id, const std::string &_name) :
  BaseScene(_id, _name),
  rootVisual(nullptr),
  meshFactory(nullptr),
  ogreRoot(nullptr),
  ogreSceneManager(nullptr)
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

  // Create ray scene query
  this->raySceneQuery = ogreSceneManager->createRayQuery(Ogre::Ray());
  this->raySceneQuery->setSortByDistance(true);
  this->raySceneQuery->setQueryMask(
      Ogre::SceneManager::ENTITY_TYPE_MASK);

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
  return (result) ? light : nullptr;
}

//////////////////////////////////////////////////
PointLightPtr OgreScene::CreatePointLightImpl(unsigned int _id,
    const std::string &_name)
{
  OgrePointLightPtr light(new OgrePointLight);
  bool result = this->InitObject(light, _id, _name);
  return (result) ? light : nullptr;
}

//////////////////////////////////////////////////
SpotLightPtr OgreScene::CreateSpotLightImpl(unsigned int _id,
    const std::string &_name)
{
  OgreSpotLightPtr light(new OgreSpotLight);
  bool result = this->InitObject(light, _id, _name);
  return (result) ? light : nullptr;
}

//////////////////////////////////////////////////
CameraPtr OgreScene::CreateCameraImpl(unsigned int _id,
    const std::string &_name)
{
  OgreCameraPtr camera(new OgreCamera);
  bool result = this->InitObject(camera, _id, _name);
  camera->SetBackgroundColor(this->backgroundColor);
  return (result) ? camera : nullptr;
}

//////////////////////////////////////////////////
VisualPtr OgreScene::CreateVisualImpl(unsigned int _id,
    const std::string &_name)
{
  OgreVisualPtr visual(new OgreVisual);
  bool result = this->InitObject(visual, _id, _name);
  return (result) ? visual : nullptr;
}

//////////////////////////////////////////////////
ArrowVisualPtr OgreScene::CreateArrowVisualImpl(unsigned int _id,
    const std::string &_name)
{
  OgreArrowVisualPtr visual(new OgreArrowVisual);
  bool result = this->InitObject(visual, _id, _name);
  return (result) ? visual : nullptr;
}

//////////////////////////////////////////////////
AxisVisualPtr OgreScene::CreateAxisVisualImpl(unsigned int _id,
    const std::string &_name)
{
  OgreAxisVisualPtr visual(new OgreAxisVisual);
  bool result = this->InitObject(visual, _id, _name);
  return (result) ? visual : nullptr;
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
  return (result) ? mesh : nullptr;
}

//////////////////////////////////////////////////
MaterialPtr OgreScene::CreateMaterialImpl(unsigned int _id,
    const std::string &_name)
{
  OgreMaterialPtr material(new OgreMaterial);
  bool result = this->InitObject(material, _id, _name);
  return (result) ? material : nullptr;
}

//////////////////////////////////////////////////
RenderTexturePtr OgreScene::CreateRenderTextureImpl(unsigned int _id,
    const std::string &_name)
{
  OgreRenderTexturePtr renderTexture(new OgreRenderTexture);
  bool result = this->InitObject(renderTexture, _id, _name);
  return (result) ? renderTexture : nullptr;
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
    this->rootVisual = nullptr;
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
VisualPtr OgreScene::VisualAt(OgreCameraPtr _camera,
                          const ignition::math::Vector2i &_mousePos)
{
  VisualPtr visual;
  ignwarn << "VisualAt call" << "\n";

  Ogre::Entity *closestEntity = this->OgreEntityAt(_camera,
                                                    _mousePos, true);
  if (closestEntity)
  {
    try
    {
      visual = this->visuals->GetByName(Ogre::any_cast<std::string>(
            closestEntity->getUserObjectBindings().getUserAny()));
    }
    //catch(boost::bad_any_cast &e)
    catch(const std::exception &e)
    {
      ignerr << "boost any_cast error:" << e.what() << "\n";
    }
  }

  return visual;
}

/////////////////////////////////////////////////
Ogre::Entity *OgreScene::OgreEntityAt(OgreCameraPtr _camera,
                                  const ignition::math::Vector2i &_mousePos,
                                  const bool _ignoreSelectionObj)
{
  Ogre::Real closest_distance = -1.0f;

  ignition::math::Vector3d origin;
  ignition::math::Vector3d dir;
  _camera->CameraToViewportRay(_mousePos.X(), _mousePos.Y(), origin, dir);
  Ogre::Ray mouseRay(OgreConversions::Convert(origin), OgreConversions::Convert(dir));

  this->raySceneQuery->setRay(mouseRay);

  // Perform the scene query
  // TODO: implement
  Ogre::RaySceneQueryResult &result= this->raySceneQuery->execute();
  Ogre::RaySceneQueryResult::iterator iter = result.begin();
  Ogre::Entity *closestEntity = NULL;

  for (iter = result.begin(); iter != result.end(); ++iter)
  {
    // is the result a MovableObject
    if (iter->movable && iter->movable->getMovableType().compare("Entity") == 0)
    {
      if (!iter->movable->isVisible() ||
          iter->movable->getName().find("__COLLISION_VISUAL__") !=
          std::string::npos)
        continue;
      if (_ignoreSelectionObj &&
          iter->movable->getName().substr(0, 15) == "__SELECTION_OBJ")
        continue;

      Ogre::Entity *ogreEntity = static_cast<Ogre::Entity*>(iter->movable);

      // mesh data to retrieve
      size_t vertex_count;
      size_t index_count;
      Ogre::Vector3 *vertices;
      uint64_t *indices;

      // Get the mesh information
      this->MeshInformation(ogreEntity->getMesh().get(), vertex_count,
          vertices, index_count, indices,
          OgreConversions::Convert(ogreEntity->getParentNode()->_getDerivedPosition()),
          OgreConversions::Convert(ogreEntity->getParentNode()->_getDerivedOrientation()),
          OgreConversions::Convert(ogreEntity->getParentNode()->_getDerivedScale()));

      bool new_closest_found = false;
      for (int i = 0; i < static_cast<int>(index_count); i += 3)
      {
        // when indices size is not divisible by 3
        if (i+2 >= static_cast<int>(index_count))
          break;

        // check for a hit against this triangle
        std::pair<bool, Ogre::Real> hit = Ogre::Math::intersects(mouseRay,
            vertices[indices[i]],
            vertices[indices[i+1]],
            vertices[indices[i+2]],
            true, false);

        // if it was a hit check if its the closest
        if (hit.first)
        {
          if ((closest_distance < 0.0f) || (hit.second < closest_distance))
          {
            // this is the closest so far, save it off
            closest_distance = hit.second;
            new_closest_found = true;
          }
        }
      }

      delete [] vertices;
      delete [] indices;

      if (new_closest_found)
      {
        closestEntity = ogreEntity;
        // break;
      }
    }
  }

  return closestEntity;
}

//////////////////////////////////////////////////
void OgreScene::MeshInformation(const Ogre::Mesh *_mesh,
                            size_t &_vertex_count,
                            Ogre::Vector3* &_vertices,
                            size_t &_index_count,
                            uint64_t* &_indices,
                            const ignition::math::Vector3d &_position,
                            const ignition::math::Quaterniond &_orient,
                            const ignition::math::Vector3d &_scale)
{
  bool added_shared = false;
  size_t current_offset = 0;
  size_t next_offset = 0;
  size_t index_offset = 0;

  _vertex_count = _index_count = 0;

  // Calculate how many vertices and indices we're going to need
  for (uint16_t i = 0; i < _mesh->getNumSubMeshes(); ++i)
  {
    Ogre::SubMesh* submesh = _mesh->getSubMesh(i);

    // We only need to add the shared vertices once
    if (submesh->useSharedVertices)
    {
      if (!added_shared)
      {
        _vertex_count += _mesh->sharedVertexData->vertexCount;
        added_shared = true;
      }
    }
    else
    {
      _vertex_count += submesh->vertexData->vertexCount;
    }

    // Add the indices
    _index_count += submesh->indexData->indexCount;
  }


  // Allocate space for the vertices and indices
  _vertices = new Ogre::Vector3[_vertex_count];
  _indices = new uint64_t[_index_count];

  added_shared = false;

  // Run through the submeshes again, adding the data into the arrays
  for (uint16_t i = 0; i < _mesh->getNumSubMeshes(); ++i)
  {
    Ogre::SubMesh* submesh = _mesh->getSubMesh(i);

    Ogre::VertexData* vertex_data = submesh->useSharedVertices ?
        _mesh->sharedVertexData : submesh->vertexData;

    if (!submesh->useSharedVertices || !added_shared)
    {
      if (submesh->useSharedVertices)
      {
        added_shared = true;
      }

      const Ogre::VertexElement* posElem =
        vertex_data->vertexDeclaration->findElementBySemantic(
            Ogre::VES_POSITION);

      Ogre::HardwareVertexBufferSharedPtr vbuf =
        vertex_data->vertexBufferBinding->getBuffer(posElem->getSource());

      unsigned char *vertex =
        static_cast<unsigned char*>(
            vbuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));

      // There is _no_ baseVertexPointerToElement() which takes an
      // Ogre::Real or a double as second argument. So make it float,
      // to avoid trouble when Ogre::Real will be comiled/typedefed as double:
      //      Ogre::Real* pReal;
      float *pReal;

      for (size_t j = 0; j < vertex_data->vertexCount;
           ++j, vertex += vbuf->getVertexSize())
      {
        posElem->baseVertexPointerToElement(vertex, &pReal);
        ignition::math::Vector3d pt(pReal[0], pReal[1], pReal[2]);
        _vertices[current_offset + j] =
            OgreConversions::Convert((_orient * (pt * _scale)) + _position);
      }

      vbuf->unlock();
      next_offset += vertex_data->vertexCount;
    }

    Ogre::IndexData* index_data = submesh->indexData;
    Ogre::HardwareIndexBufferSharedPtr ibuf = index_data->indexBuffer;

    if ((ibuf->getType() == Ogre::HardwareIndexBuffer::IT_32BIT))
    {
      uint32_t*  pLong = static_cast<uint32_t*>(
          ibuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));

      for (size_t k = 0; k < index_data->indexCount; k++)
      {
        _indices[index_offset++] = pLong[k];
      }
    }
    else
    {
      uint64_t*  pLong = static_cast<uint64_t*>(
          ibuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));

      uint16_t* pShort = reinterpret_cast<uint16_t*>(pLong);
      for (size_t k = 0; k < index_data->indexCount; k++)
      {
        _indices[index_offset++] = static_cast<uint64_t>(pShort[k]);
      }
    }

    ibuf->unlock();
    current_offset = next_offset;
  }
}

//////////////////////////////////////////////////
