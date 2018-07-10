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

/*#include "ignition/rendering/ogre2/Ogre2ArrowVisual.hh"
#include "ignition/rendering/ogre2/Ogre2AxisVisual.hh"
*/
#include "ignition/rendering/ogre2/Ogre2Camera.hh"
#include "ignition/rendering/ogre2/Ogre2Conversions.hh"
#include "ignition/rendering/ogre2/Ogre2Geometry.hh"
/*#include "ignition/rendering/ogre2/Ogre2Grid.hh"

*/
#include "ignition/rendering/ogre2/Ogre2Includes.hh"
/*
#include "ignition/rendering/ogre2/Ogre2Text.hh"
#include "ignition/rendering/ogre2/Ogre2Material.hh"
#include "ignition/rendering/ogre2/Ogre2MeshFactory.hh"
#include "ignition/rendering/ogre2/Ogre2RayQuery.hh"
*/
#include "ignition/rendering/ogre2/Ogre2RenderEngine.hh"
#include "ignition/rendering/ogre2/Ogre2RenderTarget.hh"
#include "ignition/rendering/ogre2/Ogre2Scene.hh"
#include "ignition/rendering/ogre2/Ogre2Storage.hh"
#include "ignition/rendering/ogre2/Ogre2Visual.hh"

namespace ignition
{
  namespace rendering
  {
    /// \class Subclassing the Ogre Rectangle2D class to create a gradient
    /// colored rectangle. The class is setting colors at the four vertices
    /// (corners) of the rectangle and Ogre/OpenGL then interpolates the colors
    /// between the vertices.
    /// \ref https://forums.ogre3d.org/viewtopic.php?f=2&t=60677
/*    class ColoredRectangle2D : public Ogre::Rectangle2D
    {
      // Documentation inherited
      public: explicit ColoredRectangle2D(
          bool _includeTextureCoordinates = false)
        : Ogre::Rectangle2D(_includeTextureCoordinates)
      {
        Ogre::VertexDeclaration* decl = mRenderOp.vertexData->vertexDeclaration;

        decl->addElement(this->kColorBinding, 0,
            Ogre::VET_COLOUR, Ogre::VES_DIFFUSE);
        Ogre::VertexBufferBinding* bind =
            mRenderOp.vertexData->vertexBufferBinding;

        Ogre::HardwareVertexBufferSharedPtr vbuf =
          Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(
            decl->getVertexSize(this->kColorBinding),
            mRenderOp.vertexData->vertexCount,
            Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY);

        // Bind buffer
        bind->setBinding(this->kColorBinding, vbuf);
      }

      /// \brief Class destructor.
      public: ~ColoredRectangle2D()
      {
      }

      /// \brief Set the degraded colors of the rectangle
      /// \param[in] _topLeft Top left color
      /// \param[in] _bottomLeft Bottom left color
      /// \param[in] _topRight Top right color
      /// \param[in] _bottomRight Bottom right color
      public: void SetColors(const Ogre::ColourValue &_topLeft,
                             const Ogre::ColourValue &_bottomLeft,
                             const Ogre::ColourValue &_topRight,
                             const Ogre::ColourValue &_bottomRight)
      {
        Ogre::HardwareVertexBufferSharedPtr vbuf =
            mRenderOp.vertexData->vertexBufferBinding->getBuffer(
                this->kColorBinding);
        unsigned int* pUint32 =
          static_cast<unsigned int*>(vbuf->lock(
          Ogre::HardwareBuffer::HBL_DISCARD));

        const Ogre::VertexElementType srcType =
          Ogre::VertexElement::getBestColourVertexElementType();

        *pUint32++ = Ogre::VertexElement::convertColourValue(_topLeft, srcType);
        *pUint32++ =
            Ogre::VertexElement::convertColourValue(_bottomLeft, srcType);
        *pUint32++ =
            Ogre::VertexElement::convertColourValue(_topRight, srcType);
        *pUint32++ =
            Ogre::VertexElement::convertColourValue(_bottomRight, srcType);

        vbuf->unlock();
      }

      /// \brief Index associated with the vertex buffer.
      private: const int kColorBinding = 3;
    };
    */
  }
}

using namespace ignition;
using namespace rendering;

//////////////////////////////////////////////////
Ogre2Scene::Ogre2Scene(unsigned int _id, const std::string &_name) :
  BaseScene(_id, _name)
{
  this->backgroundColor = math::Color::Black;
  this->gradientBackgroundColor = {math::Color::Black, math::Color::Black,
      math::Color::Black, math::Color::Black};
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
  return this->rootVisual;
}

//////////////////////////////////////////////////
math::Color Ogre2Scene::AmbientLight() const
{
/*  Ogre::ColourValue ogreColor = this->ogreSceneManager->getAmbientLight();
  return OgreConversions::Convert(ogreColor);
  */
  return math::Color::White;
}

//////////////////////////////////////////////////
void Ogre2Scene::SetAmbientLight(const math::Color &_color)
{
//  Ogre::ColourValue ogreColor = OgreConversions::Convert(_color);
//  this->ogreSceneManager->setAmbientLight(ogreColor);
}

//////////////////////////////////////////////////
void Ogre2Scene::SetBackgroundColor(const math::Color &_color)
{
  this->backgroundColor = _color;

/*  // TODO: clean up code
  unsigned int count = this->SensorCount();

  for (unsigned int i = 0; i < count; ++i)
  {
    SensorPtr sensor = this->SensorByIndex(i);
    OgreCameraPtr camera = std::dynamic_pointer_cast<OgreCamera>(sensor);
    if (camera) camera->SetBackgroundColor(_color);
  }
  */
}

//////////////////////////////////////////////////
void Ogre2Scene::SetGradientBackgroundColor(
    const std::array<math::Color, 4> &_colors)
{
/*  ColoredRectangle2D* rect = nullptr;
  Ogre::SceneNode *backgroundNodePtr = nullptr;

  // Check if we have created the scene node to render the gradient background
  if (!this->ogreSceneManager->hasSceneNode("Background"))
  {
    // Create background material
    Ogre::MaterialPtr material =
      Ogre::MaterialManager::getSingleton().create("Background", "General");
    material->getTechnique(0)->getPass(0)->setDepthCheckEnabled(false);
    material->getTechnique(0)->getPass(0)->setDepthWriteEnabled(false);
    material->getTechnique(0)->getPass(0)->setLightingEnabled(false);

    // Create background rectangle covering the whole screen
    rect = new ColoredRectangle2D();
    rect->setCorners(-1.0, 1.0, 1.0, -1.0);
    rect->setMaterial("Background");

    // Render the background before everything else
    rect->setRenderQueueGroup(Ogre::RENDER_QUEUE_BACKGROUND);

    // Use infinite AAB to always stay visible
    Ogre::AxisAlignedBox aabInf;
    aabInf.setInfinite();
    rect->setBoundingBox(aabInf);

    // Attach background to the scene
    backgroundNodePtr = this->ogreSceneManager->getRootSceneNode()->
        createChildSceneNode("Background");
    backgroundNodePtr->attachObject(rect);
  }

  backgroundNodePtr = this->ogreSceneManager->getSceneNode("Background");

  auto coloredRectangle2D = backgroundNodePtr->getAttachedObject(0);
  if (!coloredRectangle2D)
  {
    ignerr << "Unable to find the background attached object" << std::endl;
    return;
  }

  rect = dynamic_cast<ColoredRectangle2D *>(coloredRectangle2D);
  if (!rect)
  {
    ignerr << "Unable to cast from Ogre::MovableObject to ColouredRectangle2D"
           << std::endl;
    return;
  }

  // Convert the ignition::math::Color to Ogre::ColourValue.
  std::array<Ogre::ColourValue, 4> ogreColors;
  for (auto i = 0u; i < 4; ++i)
    ogreColors[i].setAsRGBA(_colors[i].AsRGBA());

  rect->SetColors(ogreColors[0], ogreColors[1], ogreColors[2], ogreColors[3]);
  rect->setVisible(true);

  this->gradientBackgroundColor = _colors;
  this->isGradientBackgroundColor = true;
  */
}

//////////////////////////////////////////////////
void Ogre2Scene::RemoveGradientBackgroundColor()
{
/*  // If the gradient background color is set, we should make it invisible,
  // otherwise the background color will not be visible.
  if (this->ogreSceneManager->hasSceneNode("Background"))
  {
    auto backgroundNodePtr = this->ogreSceneManager->getSceneNode("Background");
    auto coloredRectangle2D = backgroundNodePtr->getAttachedObject(0);
    if (coloredRectangle2D && coloredRectangle2D->isVisible())
      coloredRectangle2D->setVisible(false);
  }

  this->isGradientBackgroundColor = false;

  */
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
//  return this->lights;
  return LightStorePtr();
}

//////////////////////////////////////////////////
SensorStorePtr Ogre2Scene::Sensors() const
{
  return this->sensors;
}

//////////////////////////////////////////////////
VisualStorePtr Ogre2Scene::Visuals() const
{
  return this->visuals;
}

//////////////////////////////////////////////////
MaterialMapPtr Ogre2Scene::Materials() const
{
//  return this->materials;
  return MaterialMapPtr();
}

//////////////////////////////////////////////////
DirectionalLightPtr Ogre2Scene::CreateDirectionalLightImpl(unsigned int _id,
    const std::string &_name)
{
/*  OgreDirectionalLightPtr light(new OgreDirectionalLight);
  bool result = this->InitObject(light, _id, _name);
  return (result) ? light : nullptr;
  */
  return DirectionalLightPtr();
}

//////////////////////////////////////////////////
PointLightPtr Ogre2Scene::CreatePointLightImpl(unsigned int _id,
    const std::string &_name)
{
/*  OgrePointLightPtr light(new OgrePointLight);
  bool result = this->InitObject(light, _id, _name);
  return (result) ? light : nullptr;
  */
  return PointLightPtr();
}

//////////////////////////////////////////////////
SpotLightPtr Ogre2Scene::CreateSpotLightImpl(unsigned int _id,
    const std::string &_name)
{
/*  OgreSpotLightPtr light(new OgreSpotLight);
  bool result = this->InitObject(light, _id, _name);
  return (result) ? light : nullptr;
  */
  return SpotLightPtr();
}

//////////////////////////////////////////////////
CameraPtr Ogre2Scene::CreateCameraImpl(unsigned int _id,
    const std::string &_name)
{
  Ogre2CameraPtr camera(new Ogre2Camera);
  bool result = this->InitObject(camera, _id, _name);
  camera->SetBackgroundColor(this->backgroundColor);
  return (result) ? camera : nullptr;
}

//////////////////////////////////////////////////
VisualPtr Ogre2Scene::CreateVisualImpl(unsigned int _id,
    const std::string &_name)
{
  Ogre2VisualPtr visual(new Ogre2Visual);
  bool result = this->InitObject(visual, _id, _name);
  return (result) ? visual : nullptr;
}

//////////////////////////////////////////////////
ArrowVisualPtr Ogre2Scene::CreateArrowVisualImpl(unsigned int _id,
    const std::string &_name)
{
/*  OgreArrowVisualPtr visual(new OgreArrowVisual);
  bool result = this->InitObject(visual, _id, _name);
  return (result) ? visual : nullptr;
  */
  return ArrowVisualPtr();
}

//////////////////////////////////////////////////
AxisVisualPtr Ogre2Scene::CreateAxisVisualImpl(unsigned int _id,
    const std::string &_name)
{
/*  OgreAxisVisualPtr visual(new OgreAxisVisual);
  bool result = this->InitObject(visual, _id, _name);
  return (result) ? visual : nullptr;
  */
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
MeshPtr Ogre2Scene::CreateMeshImpl(unsigned int _id, const std::string &_name,
    const MeshDescriptor &_desc)
{
/*  OgreMeshPtr mesh = this->meshFactory->Create(_desc);
  bool result = this->InitObject(mesh, _id, _name);
  return (result) ? mesh : nullptr;
  */
  return MeshPtr();
}

//////////////////////////////////////////////////
GridPtr Ogre2Scene::CreateGridImpl(unsigned int _id, const std::string &_name)
{
/*  OgreGridPtr grid(new OgreGrid);
  bool result = this->InitObject(grid, _id, _name);
  return (result) ? grid: nullptr;
  */
  return GridPtr();
}

//////////////////////////////////////////////////
TextPtr Ogre2Scene::CreateTextImpl(unsigned int _id, const std::string &_name)
{
/*  OgreTextPtr text(new OgreText);
  bool result = this->InitObject(text, _id, _name);
  return (result) ? text: nullptr;
  */
  return TextPtr();
}

//////////////////////////////////////////////////
MaterialPtr Ogre2Scene::CreateMaterialImpl(unsigned int _id,
    const std::string &_name)
{
/*  OgreMaterialPtr material(new OgreMaterial);
  bool result = this->InitObject(material, _id, _name);
  return (result) ? material : nullptr;
  */
  return MaterialPtr();
}

//////////////////////////////////////////////////
RenderTexturePtr Ogre2Scene::CreateRenderTextureImpl(unsigned int _id,
    const std::string &_name)
{
  Ogre2RenderTexturePtr renderTexture(new Ogre2RenderTexture);
  bool result = this->InitObject(renderTexture, _id, _name);
  return (result) ? renderTexture : nullptr;
}

//////////////////////////////////////////////////
RenderWindowPtr Ogre2Scene::CreateRenderWindowImpl(unsigned int _id,
    const std::string &_name)
{
/*  OgreRenderWindowPtr renderWindow(new OgreRenderWindow);
  bool result = this->InitObject(renderWindow, _id, _name);
  return (result) ? renderWindow: nullptr;
  */
  return RenderWindowPtr();
}

//////////////////////////////////////////////////
RayQueryPtr Ogre2Scene::CreateRayQueryImpl(unsigned int _id,
    const std::string &_name)
{
/*  OgreRayQueryPtr rayQuery(new OgreRayQuery);
  bool result = this->InitObject(rayQuery, _id, _name);
  return (result) ? rayQuery : nullptr;
  */
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
  //getNumLogicalCores() may return 0 if couldn't detect
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
  // create unregistered visual
  this->rootVisual = Ogre2VisualPtr(new Ogre2Visual);
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
void Ogre2Scene::CreateMeshFactory()
{
//  Ogre2ScenePtr sharedThis = this->SharedThis();
//  this->meshFactory = OgreMeshFactoryPtr(new OgreMeshFactory(sharedThis));
}

//////////////////////////////////////////////////
void Ogre2Scene::CreateStores()
{
//  this->lights = OgreLightStorePtr(new OgreLightStore);
  this->sensors = Ogre2SensorStorePtr(new Ogre2SensorStore);
  this->visuals = Ogre2VisualStorePtr(new Ogre2VisualStore);
//  this->materials = OgreMaterialMapPtr(new OgreMaterialMap);
}

//////////////////////////////////////////////////
Ogre2ScenePtr Ogre2Scene::SharedThis()
{
  ScenePtr sharedBase = this->shared_from_this();
  return std::dynamic_pointer_cast<Ogre2Scene>(sharedBase);
}
