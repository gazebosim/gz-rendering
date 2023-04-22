/*
 * Copyright (C) 2023 Open Source Robotics Foundation
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

#include <list>
#include <string>
#include <unordered_map>

#include <OgreEntity.h>
#include <OgrePass.h>
#include <OgreSceneManager.h>
#include <OgreSceneNode.h>

#include "gz/rendering/ogre/OgreConversions.hh"
#include "gz/rendering/ogre/OgreProjector.hh"
#include "gz/rendering/ogre/OgreRTShaderSystem.hh"
#include "gz/rendering/Utils.hh"

using namespace gz;
using namespace rendering;

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    //
    /// \brief Projector listener, used to add a new decal material pass
    /// onto other entities' materials
    class OgreProjectorListener
    {
      /// \brief Constructor.
      public: OgreProjectorListener();

      /// \brief Destructor.
      public: virtual ~OgreProjectorListener();

      //// \brief Initialize the projector listener
      /// \param[in] _parent Parent node to attach the frustum to
      /// \param[in] _textureName Name of projection texture
      /// \param[in] _near Near clip plane
      /// \param[in] _far Far clip plane
      /// \param[in] _hfov Horizontal FOV
      public: void Init(Ogre::SceneNode *_parent,
                        const std::string &_textureName,
                        double _near = 0.5,
                        double _far = 10,
                        const math::Angle &_hfov = math::Angle(0.785398163));

      /// \brief Set whether to enable the projector
      /// \param[in] _enabled True to enable projector, false to disable
      public: void SetEnabled(bool _enabled);

      /// \brief Add decal to materials of entity visible in the frustum
      public: void AddDecalToVisibleMaterials();

      /// \brief Set texture to use for projection
      /// \param[in] _textureName Name of texture
      private: void SetTexture(const std::string &_textureName);

      /// \brief Create the frustum scene nodes
      private: void CreateSceneNode();

      /// \brief Set the frustum near and far clip planes
      /// \param[in] _near Near clip plane
      /// \param[in] _far Far clip plane
      private: void SetFrustumClipDistance(double _near, double _far);

      /// \brief Add decal to a list of entity materials.
      /// \param[in] _matList A list of material names
      private: void AddDecalToMaterials(std::list<std::string> &_matList);

      /// \brief Add decal to an entity's material.
      /// \param[in] _matList Name of material
      private: void AddDecalToMaterial(const std::string &_matName);

      /// \brief Remove decal from materials of entities
      private: void RemoveDecalFromMaterials();

      /// \brief Remove decal from an entity  material
      /// \param[in] _matList Name of material
      private: void RemoveDecalFromMaterial(const std::string &_matName);

      /// \brief Enabled state of projector listener
      public: bool enabled{false};

      /// \brief Indicates whether the projector listener is
      /// initialized or not
      public:  bool initialized{false};

      /// \brief Name of node that frustum is attached to
      public: std::string nodeName;

      /// \brief Name of node that frustum filter is attached to
      public: std::string filterNodeName;

      /// \brief Texture being projected
      public: std::string textureName;

      /// \brief Projection frustum
      public: std::unique_ptr<Ogre::Frustum> frustum;

      /// \brief Projection frustum for removing backface projection
      public: std::unique_ptr<Ogre::Frustum> filterFrustum;

      /// \brief Query used to find objects visible in the projector frustum
      public: Ogre::PlaneBoundedVolumeListSceneQuery *projectorQuery{nullptr};

      /// \brief Parent of frustum node
      public: Ogre::SceneNode *parentOgreNode{nullptr};

      /// \brief Frustum node
      public: Ogre::SceneNode *node{nullptr};

      /// \brief Frustum filter node
      public: Ogre::SceneNode *filterNode{nullptr};

      /// \brief Ogre scene manager
      public: Ogre::SceneManager *sceneMgr{nullptr};

      /// \brief A map of targets that has decal texture projected
      /// onto. Key value pairs are: <material name, material pass>
      public: std::unordered_map<std::string, Ogre::Pass*> projectorTargets;
    };
    }
  }
}

/// \brief Private data for the OgreProjector class
class gz::rendering::OgreProjector::Implementation
{
  /// \brief The projection frame listener.
  public: OgreProjectorListener projector;

  /// \brief Indicate whether the projector is intialized or not
  public: bool initialized{false};
};

/////////////////////////////////////////////////
OgreProjector::OgreProjector()
    : dataPtr(utils::MakeUniqueImpl<Implementation>())
{
}

/////////////////////////////////////////////////
OgreProjector::~OgreProjector()
{
  this->SetEnabled(false);
}

/////////////////////////////////////////////////
void OgreProjector::PreRender()
{
  if (this->dataPtr->initialized)
  {
    this->dataPtr->projector.AddDecalToVisibleMaterials();
    return;
  }

  // Initialize the projector
  this->dataPtr->projector.Init(this->ogreNode, this->textureName,
      this->nearClip, this->farClip, this->hfov);

  if (!this->dataPtr->projector.initialized)
  {
    gzwarn << "Starting projector failed." << std::endl;;
    return;
  }

  this->dataPtr->projector.SetEnabled(true);

  this->dataPtr->initialized = true;
}

/////////////////////////////////////////////////
void OgreProjector::SetEnabled(bool _enabled)
{
  BaseProjector::SetEnabled(_enabled);
  this->dataPtr->projector.SetEnabled(_enabled);
}

/////////////////////////////////////////////////
OgreProjectorListener::OgreProjectorListener()
{
}

/////////////////////////////////////////////////
OgreProjectorListener::~OgreProjectorListener()
{
  this->RemoveDecalFromMaterials();

  if (this->filterNode)
  {
    this->filterNode->detachObject(this->filterFrustum.get());
    this->node->removeAndDestroyChild(this->filterNodeName);
    this->filterNode = nullptr;
  }

  if (this->node)
  {
    this->node->detachObject(this->frustum.get());
    Ogre::SceneNode *n = this->parentOgreNode;
    if (n)
      n->removeAndDestroyChild(this->nodeName);
    this->node = nullptr;
  }

  this->frustum.reset();
  this->filterFrustum.reset();

  if (this->projectorQuery)
    this->sceneMgr->destroyQuery(this->projectorQuery);
}

/////////////////////////////////////////////////
void OgreProjectorListener::Init(Ogre::SceneNode *_parent,
  const std::string &_textureName, double _near, double _far,
  const math::Angle &_fov)
{
  if (this->initialized)
    return;

  if (_textureName.empty())
  {
    gzerr << "Projector is missing a texture\n";
    return;
  }

  this->parentOgreNode = _parent;

  this->nodeName = this->parentOgreNode->getName() + "_Projector";
  this->filterNodeName = this->parentOgreNode->getName() + "_ProjectorFilter";

  this->frustum = std::make_unique<Ogre::Frustum>();
  this->filterFrustum = std::make_unique<Ogre::Frustum>();
  this->filterFrustum->setProjectionType(Ogre::PT_ORTHOGRAPHIC);

  this->sceneMgr = this->parentOgreNode->getCreator();
  this->projectorQuery = this->sceneMgr->createPlaneBoundedVolumeQuery(
      Ogre::PlaneBoundedVolumeList());

  this->CreateSceneNode();
  this->SetTexture(_textureName);
  this->SetFrustumClipDistance(_near, _far);

  common::Image image(_textureName);
  double aspectRatio = image.Width() / image.Height();
  const double vfov = 2.0 * atan(tan(_fov.Radian() / 2.0)
      / aspectRatio);

  this->frustum->setFOVy(Ogre::Radian(vfov));
  this->filterFrustum->setFOVy(Ogre::Radian(vfov));

  this->initialized = true;
}

/////////////////////////////////////////////////
void OgreProjectorListener::SetEnabled(bool _enabled)
{
  this->enabled = _enabled;
  if (!this->enabled)
    this->RemoveDecalFromMaterials();
  OgreRTShaderSystem::Instance()->UpdateShaders();
}

/////////////////////////////////////////////////
void OgreProjectorListener::CreateSceneNode()
{
  if (this->filterNode)
  {
    this->filterNode->detachObject(this->filterFrustum.get());
    this->node->removeAndDestroyChild(this->filterNodeName);
    this->filterNode = nullptr;
  }

  if (this->node)
  {
    this->node->detachObject(this->frustum.get());
    this->parentOgreNode->removeAndDestroyChild(this->nodeName);
    this->node = nullptr;
  }

  this->node = this->parentOgreNode->createChildSceneNode(
      this->nodeName);
  this->node->yaw(Ogre::Degree(-90));
  this->node->roll(Ogre::Degree(-90));

  this->filterNode = this->node->createChildSceneNode(
      this->filterNodeName);

  if (this->node)
    this->node->attachObject(this->frustum.get());

  if (this->filterNode)
  {
    this->filterNode->attachObject(this->filterFrustum.get());
    this->filterNode->setOrientation(
      Ogre::Quaternion(Ogre::Degree(90), Ogre::Vector3::UNIT_Y));
  }
}

/////////////////////////////////////////////////
void OgreProjectorListener::SetTexture(
    const std::string &_textureName)
{
  this->textureName = _textureName;
}

/////////////////////////////////////////////////
void OgreProjectorListener::SetFrustumClipDistance(double _near,
                                                               double _far)
{
  this->frustum->setNearClipDistance(_near);
  this->filterFrustum->setNearClipDistance(_near);
  this->frustum->setFarClipDistance(_far);
  this->filterFrustum->setFarClipDistance(_far);
}

/////////////////////////////////////////////////
void OgreProjectorListener::AddDecalToVisibleMaterials()
{
  std::list<std::string> newVisibleMaterials;
  Ogre::PlaneBoundedVolumeList volumeList;

  volumeList.push_back(this->frustum->getPlaneBoundedVolume());

  this->projectorQuery->setVolumes(volumeList);
  Ogre::SceneQueryResult result = this->projectorQuery->execute();

  // Find all visible materials
  Ogre::SceneQueryResultMovableList::iterator it;
  for (it = result.movables.begin(); it != result.movables.end(); ++it)
  {
    Ogre::Entity *entity = dynamic_cast<Ogre::Entity*>(*it);
    if (entity && !entity->getUserObjectBindings().getUserAny().isEmpty() &&
        entity->getUserObjectBindings().getUserAny().getType() ==
        typeid(unsigned int))
    {
      for (unsigned int i = 0; i < entity->getNumSubEntities(); i++)
      {
        newVisibleMaterials.push_back(
          entity->getSubEntity(i)->getMaterialName());
      }
    }
  }

  this->AddDecalToMaterials(newVisibleMaterials);
}

/////////////////////////////////////////////////
void OgreProjectorListener::AddDecalToMaterials(
    std::list<std::string> &_matList)
{
  _matList.remove("");
  _matList.unique();

  std::string invisibleMaterial;
  std::list<std::string>::iterator visibleMaterial;

  // Loop through all existing passes, removing those for materials
  //   not in the newlist and skipping pass creation for those in the
  //   newlist that have already been created
  auto used = projectorTargets.begin();
  while (used != projectorTargets.end())
  {
    visibleMaterial = std::find(_matList.begin(), _matList.end(), used->first);

    // Remove the pass if it applies to a material not in the new list
    if (visibleMaterial == _matList.end())
    {
      invisibleMaterial = used->first;
      ++used;
      this->RemoveDecalFromMaterial(invisibleMaterial);
    }
    // Otherwise remove it from the list of passes to be added
    else
    {
      _matList.remove(used->first);
      ++used;
    }
  }

  if (!_matList.empty())
  {
    // Add pass for new materials
    while (!_matList.empty())
    {
      this->AddDecalToMaterial(_matList.front());
      _matList.erase(_matList.begin());
    }

    OgreRTShaderSystem::Instance()->UpdateShaders();
  }
}

/////////////////////////////////////////////////
void OgreProjectorListener::AddDecalToMaterial(
    const std::string &_matName)
{
  if (this->projectorTargets.find(_matName) != this->projectorTargets.end())
  {
    return;
  }

  Ogre::MaterialPtr mat = static_cast<Ogre::MaterialPtr>(
    Ogre::MaterialManager::getSingleton().getByName(_matName));
  Ogre::Pass *pass = mat->getTechnique(0)->createPass();

  pass->setSceneBlending(Ogre::SBT_TRANSPARENT_ALPHA);
  pass->setDepthBias(1);
  pass->setLightingEnabled(false);

  if (!Ogre::ResourceGroupManager::getSingleton().resourceExists(
      Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
      this->textureName ))
  {
    Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
        this->textureName, "FileSystem",
        Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
  }

  Ogre::TextureUnitState *texState =
      pass->createTextureUnitState(this->textureName);
  texState->setProjectiveTexturing(true, this->frustum.get());
  texState->setTextureAddressingMode(Ogre::TextureUnitState::TAM_BORDER);
  texState->setTextureFiltering(Ogre::TFO_ANISOTROPIC);
  texState->setTextureBorderColour(Ogre::ColourValue(0.0, 0.0, 0.0, 0.0));
  texState->setColourOperation(Ogre::LBO_ALPHA_BLEND);

  texState = pass->createTextureUnitState("projection_filter.png");
  texState->setProjectiveTexturing(true, this->filterFrustum.get());
  texState->setTextureAddressingMode(Ogre::TextureUnitState::TAM_CLAMP);
  texState->setTextureFiltering(Ogre::TFO_NONE);

  this->projectorTargets[_matName] = pass;
}

/////////////////////////////////////////////////
void OgreProjectorListener::RemoveDecalFromMaterials()
{
  for (auto it = this->projectorTargets.begin();
      it != this->projectorTargets.end(); ++it)
  {
    it->second->getParent()->removePass(it->second->getIndex());
  }
  this->projectorTargets.clear();
}

/////////////////////////////////////////////////
void OgreProjectorListener::RemoveDecalFromMaterial(
    const std::string &_matName)
{
  this->projectorTargets[_matName]->getParent()->removePass(
    this->projectorTargets[_matName]->getIndex());
  this->projectorTargets.erase(this->projectorTargets.find(_matName));
}
