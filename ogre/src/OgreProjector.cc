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
#include <unordered_map>
#include <string>

#include <OgreEntity.h>
#include <OgreFrameListener.h>
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
      /// \brief Frame listener, used to add projection materials when new
      /// textures are added to Ogre.
      class OgreProjectorFrameListener : public Ogre::FrameListener
      {
        /// \brief Constructor.
        public: OgreProjectorFrameListener();

        /// \brief Destructor.
        public: virtual ~OgreProjectorFrameListener();

        public: void Init(Ogre::SceneNode *_parent,
                          const std::string &_textureName,
                          double _near = 0.5,
                          double _far = 10,
                          const math::Angle &_fov = math::Angle(0.785398163));

        public: virtual bool frameStarted(const Ogre::FrameEvent &_evt);

        public: void SetTexture(const std::string &_textureName);

        public: void SetEnabled(bool _enabled);
        public: void SetUsingShaders(bool _usingShaders);

        /// \brief Set the pose of the projector.
        /// \param[in] _pose New pose of the projector
        public: void SetPose(const math::Pose3d &_pose);

        public: void SetSceneNode();

        public: void SetFrustumClipDistance(double _near, double _far);
        public: void SetFrustumFOV(double _fov);
        public: void AddPassToAllMaterials();
        public: void AddPassToVisibleMaterials();
        public: void AddPassToMaterials(std::list<std::string> &_matList);
        public: void AddPassToMaterial(const std::string &_matName);
        public: void RemovePassFromMaterials();
        public: void RemovePassFromMaterial(const std::string &_matName);

        public: bool enabled;
        public:  bool initialized;
        public: bool usingShaders;

        public: std::string nodeName;
        public: std::string filterNodeName;

        public: std::string textureName;

        public: Ogre::Frustum *frustum{nullptr};
        public: Ogre::Frustum *filterFrustum{nullptr};
        public: Ogre::PlaneBoundedVolumeListSceneQuery *projectorQuery{nullptr};

        public: Ogre::SceneNode *parentOgreNode{nullptr};

        public: Ogre::SceneNode *node{nullptr};
        public: Ogre::SceneNode *filterNode{nullptr};
        public: Ogre::SceneManager *sceneMgr{nullptr};
        public: std::unordered_map<std::string, Ogre::Pass*> projectorTargets;
      };
    }
  }
}

/// \brief Private data for the OgreProjector class
class gz::rendering::OgreProjector::Implementation
{
  /// \brief The projection frame listener.
  public: OgreProjectorFrameListener projector;

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
  // Ogre cleanup
  Ogre::Root::getSingletonPtr()->removeFrameListener(
      &this->dataPtr->projector);
}

/////////////////////////////////////////////////
void OgreProjector::PreRender()
{
  if (this->dataPtr->initialized)
    return;

  int retryCount = 0;

  // TODO remove this while loop?
  // Initialize the projector
  while (!this->dataPtr->projector.initialized && retryCount < 10)
  {
    // init
    this->dataPtr->projector.Init(this->ogreNode, this->textureName,
        this->nearClip, this->farClip, this->hfov);

    // TODO set pose? maybe not needed as it should be set downstream
    // set the projector pose relative to body
    // this->dataPtr->projector.SetPose(_pose);

    if (!this->dataPtr->projector.initialized)
    {
      gzwarn << "starting projector failed, retrying in 1 sec.\n";
      ++retryCount;
    }
  }

  // Add the projector as an Ogre frame listener
  Ogre::Root::getSingletonPtr()->addFrameListener(&this->dataPtr->projector);

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
OgreProjectorFrameListener::OgreProjectorFrameListener()
{
  this->enabled = false;
  this->initialized = false;
  this->usingShaders = false;

  this->node = nullptr;
  this->filterNode = nullptr;
  this->projectorQuery = nullptr;
  this->frustum = nullptr;
  this->filterFrustum = nullptr;

  this->nodeName = "Projector";
  this->filterNodeName = "ProjectorFilter";
}

/////////////////////////////////////////////////
OgreProjectorFrameListener::~OgreProjectorFrameListener()
{
  this->RemovePassFromMaterials();

  if (this->filterNode)
  {
    this->filterNode->detachObject(this->filterFrustum);
    this->node->removeAndDestroyChild(this->filterNodeName);
    this->filterNode = nullptr;
  }

  if (this->node)
  {
    this->node->detachObject(this->frustum);
    Ogre::SceneNode *n = this->parentOgreNode;
    if (n)
      n->removeAndDestroyChild(this->nodeName);
    this->node = nullptr;
  }

  delete this->frustum;
  delete this->filterFrustum;
  this->frustum = nullptr;
  this->filterFrustum = nullptr;

  if (this->projectorQuery)
    this->sceneMgr->destroyQuery(this->projectorQuery);
}

/////////////////////////////////////////////////
void OgreProjectorFrameListener::Init(Ogre::SceneNode *_parent,
  const std::string &_textureName,
  double _near,
  double _far,
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

  this->frustum = new Ogre::Frustum();
  this->filterFrustum = new Ogre::Frustum();
  this->filterFrustum->setProjectionType(Ogre::PT_ORTHOGRAPHIC);

  this->sceneMgr = this->parentOgreNode->getCreator();
  this->projectorQuery = this->sceneMgr->createPlaneBoundedVolumeQuery(
      Ogre::PlaneBoundedVolumeList());

  this->SetSceneNode();
  this->SetTexture(_textureName);
  this->SetFrustumClipDistance(_near, _far);

  // TODO get aspect ratio from texture
  double aspectRatio = 1.0;
  const double vfov = 2.0 * atan(tan(_fov.Radian() / 2.0)
      / aspectRatio);
  this->SetFrustumFOV(vfov);

  this->initialized = true;
}

/////////////////////////////////////////////////
bool OgreProjectorFrameListener::frameStarted(
    const Ogre::FrameEvent &/*_evt*/)
{
  if (!this->initialized || !this->enabled || this->textureName.empty())
    return true;

  this->AddPassToVisibleMaterials();

  return true;
}

/////////////////////////////////////////////////
void OgreProjectorFrameListener::SetEnabled(bool _enabled)
{
  this->enabled = _enabled;
  if (!this->enabled)
    this->RemovePassFromMaterials();
  OgreRTShaderSystem::Instance()->UpdateShaders();
}

/////////////////////////////////////////////////
void OgreProjectorFrameListener::SetUsingShaders(bool _usingShaders)
{
  this->usingShaders = _usingShaders;
}

/////////////////////////////////////////////////
void OgreProjectorFrameListener::SetSceneNode()
{
  if (this->filterNode)
  {
    this->filterNode->detachObject(this->filterFrustum);
    this->node->removeAndDestroyChild(this->filterNodeName);
    this->filterNode = nullptr;
  }

  if (this->node)
  {
    this->node->detachObject(this->frustum);
    this->parentOgreNode->removeAndDestroyChild(this->nodeName);
    this->node = nullptr;
  }

  this->node = this->parentOgreNode->createChildSceneNode(
      this->nodeName);

  this->filterNode = this->node->createChildSceneNode(
      this->filterNodeName);

  if (this->node)
    this->node->attachObject(this->frustum);

  if (this->filterNode)
  {
    this->filterNode->attachObject(this->filterFrustum);
    this->filterNode->setOrientation(
      Ogre::Quaternion(Ogre::Degree(90), Ogre::Vector3::UNIT_Y));
  }
}

/////////////////////////////////////////////////
void OgreProjectorFrameListener::SetPose(
  const math::Pose3d &_pose)
{
  Ogre::Quaternion ogreQuaternion =
      OgreConversions::Convert(_pose.Rot());
  Ogre::Vector3 ogreVec = OgreConversions::Convert(_pose.Pos());
  Ogre::Quaternion offsetQuaternion;

  this->node->setPosition(ogreVec);
  this->node->setOrientation(ogreQuaternion);
  this->filterNode->setPosition(ogreVec);

  offsetQuaternion = Ogre::Quaternion(Ogre::Degree(90), Ogre::Vector3::UNIT_Y);
  this->filterNode->setOrientation(offsetQuaternion);
}

/////////////////////////////////////////////////
void OgreProjectorFrameListener::SetTexture(
    const std::string &_textureName)
{
  this->textureName = _textureName;
}

/////////////////////////////////////////////////
void OgreProjectorFrameListener::SetFrustumClipDistance(double _near,
                                                               double _far)
{
  this->frustum->setNearClipDistance(_near);
  this->filterFrustum->setNearClipDistance(_near);
  this->frustum->setFarClipDistance(_far);
  this->filterFrustum->setFarClipDistance(_far);
}

/////////////////////////////////////////////////
void OgreProjectorFrameListener::SetFrustumFOV(double _fov)
{
  this->frustum->setFOVy(Ogre::Radian(_fov));
  this->filterFrustum->setFOVy(Ogre::Radian(_fov));
}

/////////////////////////////////////////////////
void OgreProjectorFrameListener::AddPassToAllMaterials()
{
  std::list<std::string> allMaterials;

  auto it = this->sceneMgr->getMovableObjectIterator("Entity");

  while (it.hasMoreElements())
  {
    Ogre::Entity* entity = dynamic_cast<Ogre::Entity*>(it.getNext());
    if (entity && entity->getName().find("visual") != std::string::npos)

    for (unsigned int i = 0; i < entity->getNumSubEntities(); i++)
    {
      allMaterials.push_back(entity->getSubEntity(i)->getMaterialName());
    }
  }

  this->AddPassToMaterials(allMaterials);
}

/////////////////////////////////////////////////
void OgreProjectorFrameListener::AddPassToVisibleMaterials()
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
    if (entity && entity->getName().find("visual") != std::string::npos)
    {
      for (unsigned int i = 0; i < entity->getNumSubEntities(); i++)
      {
        newVisibleMaterials.push_back(
          entity->getSubEntity(i)->getMaterialName());
      }
    }
  }

  this->AddPassToMaterials(newVisibleMaterials);
}

/////////////////////////////////////////////////
void OgreProjectorFrameListener::AddPassToMaterials(
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
      this->RemovePassFromMaterial(invisibleMaterial);
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
      this->AddPassToMaterial(_matList.front());
      _matList.erase(_matList.begin());
    }

    OgreRTShaderSystem::Instance()->UpdateShaders();
  }
}

/////////////////////////////////////////////////
void OgreProjectorFrameListener::AddPassToMaterial(
    const std::string &_matName)
{
  if (this->projectorTargets.find(_matName) != this->projectorTargets.end())
  {
    return;
  }

  Ogre::MaterialPtr mat = static_cast<Ogre::MaterialPtr>(
    Ogre::MaterialManager::getSingleton().getByName(_matName));
  Ogre::Pass *pass = mat->getTechnique(0)->createPass();

  if (this->usingShaders)
  {
    Ogre::Matrix4 viewProj = this->frustum->getProjectionMatrix() *
                             this->frustum->getViewMatrix();

    pass->setVertexProgram("Gazebo/TextureProjectionVP");

    // pass->setFragmentProgram("GazeboWorlds/TexProjectionFP");
    Ogre::GpuProgramParametersSharedPtr vsParams =
      pass->getVertexProgramParameters();

    Ogre::GpuProgramParametersSharedPtr psParams =
      pass->getFragmentProgramParameters();

    // vsParams->setNamedAutoConstant(
    //   "worldViewProjMatrix",
    //   GpuProgramParameters::ACT_WORLDVIEWPROJ_MATRIX);
    // vsParams->setNamedAutoConstant(
    //   "worldMatrix",GpuProgramParameters::ACT_WORLD_MATRIX);
    // vsParams->setNamedConstant("texViewProjMatrix", viewProj);

    vsParams->setNamedAutoConstant(
      "worldMatrix", Ogre::GpuProgramParameters::ACT_WORLD_MATRIX);

    vsParams->setNamedConstant("texProjMatrix", viewProj);

    // psParams->setNamedConstant("projMap", viewProj);

    pass->setVertexProgramParameters(vsParams);

    // pass->setFragmentProgramParameters(psParams);
  }

  pass->setSceneBlending(Ogre::SBT_TRANSPARENT_ALPHA);
  pass->setDepthBias(1);
  pass->setLightingEnabled(false);

  Ogre::TextureUnitState *texState =
    pass->createTextureUnitState(this->textureName);
  texState->setProjectiveTexturing(true, this->frustum);
  texState->setTextureAddressingMode(Ogre::TextureUnitState::TAM_BORDER);
  texState->setTextureFiltering(Ogre::TFO_ANISOTROPIC);
  texState->setTextureBorderColour(Ogre::ColourValue(0.0, 0.0, 0.0, 0.0));
  texState->setColourOperation(Ogre::LBO_ALPHA_BLEND);

  texState = pass->createTextureUnitState("projection_filter.png");
  texState->setProjectiveTexturing(true, this->filterFrustum);
  texState->setTextureAddressingMode(Ogre::TextureUnitState::TAM_CLAMP);
  texState->setTextureFiltering(Ogre::TFO_NONE);

  this->projectorTargets[_matName] = pass;
}

/////////////////////////////////////////////////
void OgreProjectorFrameListener::RemovePassFromMaterials()
{
  for (auto it = this->projectorTargets.begin();
      it != this->projectorTargets.end(); ++it)
  {
    it->second->getParent()->removePass(it->second->getIndex());
  }
  this->projectorTargets.clear();
}

/////////////////////////////////////////////////
void OgreProjectorFrameListener::RemovePassFromMaterial(
    const std::string &_matName)
{
  this->projectorTargets[_matName]->getParent()->removePass(
    this->projectorTargets[_matName]->getIndex());
  this->projectorTargets.erase(this->projectorTargets.find(_matName));
}
