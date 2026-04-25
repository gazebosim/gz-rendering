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
#include <unordered_set>

#include <cstring>

#include <OgreEntity.h>
#include <OgreGpuProgramParams.h>
#include <OgreHardwarePixelBuffer.h>
#include <OgreHighLevelGpuProgramManager.h>
#include <OgreMaterialManager.h>
#include <OgrePass.h>
#include <OgreRenderTargetListener.h>
#include <OgreSceneManager.h>
#include <OgreSceneNode.h>
#include <OgreTechnique.h>
#include <OgreTextureManager.h>
#include <RTShaderSystem/OgreShaderGenerator.h>

#include <gz/common/Filesystem.hh>
#include <gz/common/Image.hh>
#include <gz/common/Profiler.hh>
#include "gz/rendering/ogre/OgreCamera.hh"
#include "gz/rendering/ogre/OgreConversions.hh"
#include "gz/rendering/ogre/OgreDepthCamera.hh"
#include "gz/rendering/ogre/OgreProjector.hh"
#include "gz/rendering/ogre/OgreRTShaderSystem.hh"
#include "gz/rendering/ogre/OgreScene.hh"
#include "gz/rendering/Utils.hh"

using namespace gz;
using namespace rendering;

namespace
{
  /// \brief Load a PNG (or any gz-common-decodable image) from disk and
  /// upload it to an OGRE texture under the given alias name. OGRE 1.12 ships
  /// without a PNG codec, so we must decode via gz-common and push raw RGBA.
  /// \param[in] _alias Unique OGRE texture name to register.
  /// \param[in] _path Absolute path to the image file.
  /// \return True on success, false if the file can't be decoded.
  bool LoadImageAsOgreTexture(const std::string &_alias,
      const std::string &_path)
  {
    if (Ogre::TextureManager::getSingleton().getByName(_alias,
        Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME))
    {
      return true;
    }

    common::Image img;
    if (img.Load(_path) != 0 || img.Width() == 0 || img.Height() == 0)
      return false;

    auto ogreTexture = Ogre::TextureManager::getSingleton().createManual(
        _alias,
        Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
        Ogre::TEX_TYPE_2D,
        img.Width(), img.Height(), 0, Ogre::PF_BYTE_RGBA);
    auto pixelBuffer = ogreTexture->getBuffer();
    pixelBuffer->lock(Ogre::HardwareBuffer::HBL_NORMAL);
    const auto &pixelBox = pixelBuffer->getCurrentLock();
    auto data = img.RGBAData();
    // Same heap-overflow class fixed in OgreMaterial::CreateOgreTextureFromImage:
    // gz-common's RGBAData() returns 2x the bytes for 16-bit-per-channel
    // sources (e.g. an embedded 16-bit PNG used as a projector texture).
    // PF_BYTE_RGBA expects 8 bits/channel; a naive memcpy(data.size())
    // overflows the OGRE pixel buffer and corrupts the heap.
    const size_t bytes8 = static_cast<size_t>(img.Width()) *
                          static_cast<size_t>(img.Height()) * 4u;
    if (data.size() == bytes8)
    {
      std::memcpy(pixelBox.data, data.data(), bytes8);
    }
    else if (data.size() == bytes8 * 2u)
    {
      // 16-bit per channel: stbi keeps native uint16_t order, so on
      // little-endian hosts the high (most-significant) byte is at offset 1.
      auto *dst = static_cast<unsigned char *>(pixelBox.data);
      const auto *src = data.data();
      for (size_t i = 0; i < bytes8; ++i)
        dst[i] = src[i * 2u + 1u];
    }
    else
    {
      gzwarn << "Skipping projector texture upload for '" << _alias
             << "': RGBAData size " << data.size() << " does not match "
             << img.Width() << "x" << img.Height()
             << " 8-bit RGBA (" << bytes8 << ") or 16-bit RGBA ("
             << (bytes8 * 2u) << ")" << std::endl;
    }
    pixelBuffer->unlock();
    return true;
  }
}

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    //
    /// \brief Projector listener, used to add a new decal material pass
    /// onto other entities' materials
    class OgreProjectorListener
        : public Ogre::RenderTargetListener, Ogre::MaterialManager::Listener
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

      /// \brief Destroy the projector listener by cleaning up resources
      public: void Destroy();

      /// \brief Set whether to enable the projector
      /// \param[in] _enabled True to enable projector, false to disable
      public: void SetEnabled(bool _enabled);

      /// \brief Add decal to materials of entity visible in the frustum
      public: void AddDecalToVisibleMaterials();

      /// \brief Remove decal from materials of entities
      public: void RemoveDecalFromMaterials();

      /// \brief Find a list of materials visible in the frustum
      /// and update the visibleMaterials variable. This function is used
      /// when decal has visibility flags
      public: void UpdateVisibleMaterials();

      /// \brief Find a list of materials visible in the frustum
      private: std::unordered_set<std::string> FindVisibleMaterials();

      /// \brief Remove decal from materials of entities that are no longer
      /// visible
      /// \param[in] _matSet A set of materials
      private: void RemoveDecalFromInvisibleMaterials(
          std::unordered_set<std::string>& _matSet);

      //// \brief Set the visibility flags for this projector
      /// \param[in] _flags Visibility flags to set
      public: void SetVisibilityFlags(uint32_t _flags);

      /// \brief Set texture to use for projection
      /// \param[in] _textureName Name of texture
      private: void SetTexture(const std::string &_textureName);

      /// \brief Create the frustum scene nodes
      private: void CreateSceneNode();

      /// \brief Set the frustum near and far clip planes
      /// \param[in] _near Near clip plane
      /// \param[in] _far Far clip plane
      private: void SetFrustumClipDistance(double _near, double _far);

      /// \brief Add decal to a set of entity materials.
      /// \param[in] _matSet A set  of material names
      private: void AddDecalToMaterials(
          std::unordered_set<std::string> &_matSet);

      /// \brief Add decal to an entity's material.
      /// \param[in] _matSet Name of material
      private: void AddDecalToMaterial(const std::string &_matName);

      /// \brief Attach the hand-written projector GLSL programs to a decal
      /// pass. The programs are created once per process and reused. Sets
      /// the required auto-constants and texture sampler bindings.
      /// \param[in] _pass Decal pass freshly populated with the decal +
      /// filter texture units.
      private: void BindProjectorShader(Ogre::Pass *_pass);

      /// \brief Remove decal from an entity  material
      /// \param[in] _matSet Name of material
      private: void RemoveDecalFromMaterial(const std::string &_matName);

      /// \brief Ogre's pre render update callback
      /// \param[in] _evt Ogre render target event containing information about
      /// the source render target.
      private: virtual void preRenderTargetUpdate(
                  const Ogre::RenderTargetEvent &_evt);

      /// \brief Ogre's post render update callback
      /// \param[in] _evt Ogre render target event containing information about
      /// the source render target.
      private: virtual void postRenderTargetUpdate(
                   const Ogre::RenderTargetEvent &_evt);

      // Documentation inherited
      public: virtual Ogre::Technique *handleSchemeNotFound(
                  uint16_t _schemeIndex, const Ogre::String &_schemeName,
                  Ogre::Material *_originalMaterial, uint16_t _lodIndex,
                  const Ogre::Renderable *_rend);

      /// \brief Return the "projector" scheme technique on the given clone
      /// material, falling back to technique 0 if none is found. The projector
      /// scheme technique is the RTSS-generated shader-based variant that GL3+
      /// core can actually execute.
      /// \param[in] _clone Clone material that has been through
      /// AddDecalToMaterial.
      private: Ogre::Technique *ProjectorSchemeTechnique(
                  Ogre::Material *_clone) const;

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

      /// \brief Projector's visibility flags
      private: uint32_t visibilityFlags = 0u;

      /// \brief Name of the default material scheme. We restore the camera
      /// viewports to this default once the camera is done rendering.
      ///  Used when decal has custom visibility flags
      private: std::string defaultScheme;

      /// \brief A clone of materials for projecting texture onto. Used when
      /// decal has custom visibility flags
      private: std::unordered_map<std::string, Ogre::Material *> matClones;

      /// \brief A set of visible materials in the frustum. Used when decal
      /// has custom visibility flags
      private: std::unordered_set<std::string> visibleMaterials;
    };
    }
  }
}

/// \brief Private data for the OgreProjector class
class gz::rendering::OgreProjector::Implementation
{
  /// \brief The projection frame listener.
  public: OgreProjectorListener projector;

  /// \brief Indicate whether the projector is initialized or not
  public: bool initialized{false};

  /// \brief A map of cameras (<Camera ptr, name>) that the listener has been
  /// added to
  public: std::unordered_map<Ogre::Camera *, Ogre::String>
      camerasWithListener;
};

/////////////////////////////////////////////////
OgreProjector::OgreProjector()
    : dataPtr(utils::MakeUniqueImpl<Implementation>())
{
}

/////////////////////////////////////////////////
OgreProjector::~OgreProjector()
{
  if (!this->Scene()->IsInitialized())
    return;
  this->SetEnabled(false);
  this->dataPtr->projector.Destroy();
}

/////////////////////////////////////////////////
void OgreProjector::PreRender()
{
  GZ_PROFILE("OgreWireBox::PreRender");
  if (this->dataPtr->initialized)
  {
    this->UpdateCameraListener();
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
  this->Destroy();
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
void OgreProjectorListener::Destroy()
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
    this->sceneMgr->destroySceneNode(this->node);
    this->node = nullptr;
  }

  this->frustum.reset();
  this->filterFrustum.reset();

  if (this->projectorQuery)
  {
    this->sceneMgr->destroyQuery(this->projectorQuery);
    this->projectorQuery = nullptr;
  }

  this->visibleMaterials.clear();
  this->matClones.clear();
  this->initialized = false;
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
  GZ_PROFILE("OgreProjectorListener::CreateSceneNode");
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
std::unordered_set<std::string> OgreProjectorListener::FindVisibleMaterials()
{
  GZ_PROFILE("OgreProjectorListener::FindVisibleMaterials");
  std::unordered_set<std::string> newVisibleMaterials;
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
      for (size_t i = 0; i < entity->getNumSubEntities(); i++)
      {
        newVisibleMaterials.insert(
          entity->getSubEntity(i)->getMaterialName());
      }
    }
  }

  return newVisibleMaterials;
}

/////////////////////////////////////////////////
void OgreProjectorListener::AddDecalToVisibleMaterials()
{
  auto newVisibleMaterials = this->FindVisibleMaterials();

  this->AddDecalToMaterials(newVisibleMaterials);
}

/////////////////////////////////////////////////
void OgreProjectorListener::RemoveDecalFromInvisibleMaterials(
    std::unordered_set<std::string> &_matSet)
{
  GZ_PROFILE("OgreProjectorListener::RemoveDecalFromInvisibleMaterials");
  std::string invisibleMaterial;
  std::unordered_set<std::string>::iterator visibleMaterial;

  // Loop through all existing passes, removing those for materials
  //   not in the newlist and skipping pass creation for those in the
  //   newlist that have already been created
  auto used = this->projectorTargets.begin();
  while (used != this->projectorTargets.end())
  {
    visibleMaterial = std::find(_matSet.begin(), _matSet.end(), used->first);

    // Remove the pass if it applies to a material not in the new list
    if (visibleMaterial == _matSet.end())
    {
      invisibleMaterial = used->first;
      ++used;
      this->RemoveDecalFromMaterial(invisibleMaterial);
    }
    // Otherwise remove it from the list of passes to be added
    else
    {
      _matSet.erase(used->first);
      ++used;
    }
  }
}

/////////////////////////////////////////////////
void OgreProjectorListener::AddDecalToMaterials(
    std::unordered_set<std::string> &_matSet)
{
  GZ_PROFILE("OgreProjectorListener::AddDecalToMaterials");
  this->RemoveDecalFromInvisibleMaterials(_matSet);

  if (!_matSet.empty())
  {
    // Add pass for new materials
    while (!_matSet.empty())
    {
      this->AddDecalToMaterial(*_matSet.begin());
      _matSet.erase(_matSet.begin());
    }

    OgreRTShaderSystem::Instance()->UpdateShaders();
  }
}

/////////////////////////////////////////////////
void OgreProjectorListener::AddDecalToMaterial(
    const std::string &_matName)
{
  GZ_PROFILE("OgreProjectorListener::AddDecalToMaterial");
  if (this->projectorTargets.find(_matName) != this->projectorTargets.end())
  {
    return;
  }

  Ogre::MaterialPtr mat = static_cast<Ogre::MaterialPtr>(
    Ogre::MaterialManager::getSingleton().getByName(_matName));
  // OGRE 1.12's MaterialManager::getByName returns a null MaterialPtr on miss
  // instead of throwing — and this code path is reached from the scheme
  // listener with a name that has been trimmed (the "_MissingTech_" prefix
  // stripped), so the lookup *can* legitimately fail. Don't dereference null.
  if (!mat)
  {
    gzerr << "OgreProjector: cannot add decal — material '" << _matName
          << "' not found in MaterialManager" << std::endl;
    return;
  }
  Ogre::Pass *pass = mat->getTechnique(0)->createPass();

  pass->setSceneBlending(Ogre::SBT_TRANSPARENT_ALPHA);
  pass->setDepthBias(1);
  pass->setDepthWriteEnabled(false);

  // Wire a hand-written GLSL projector shader pair onto this decal pass. The
  // RTSS-generated FFP_GenerateTexCoord_Projection path is broken for this
  // case under OGRE 1.12 + GL3+: the auto-constants get bound but the
  // generated FS samples zero, so the wall ends up rendered black. Bypass
  // RTSS entirely for the decal pass.
  this->BindProjectorShader(pass);
  pass->setLightingEnabled(false);

  // OGRE 1.12 dropped the FreeImage-based PNG codec; loading textures by
  // file name directly results in "Can not find codec for 'png' format"
  // and a blank sampler. Pre-decode both the decal and filter textures via
  // gz-common and upload them as manual textures, then attach by alias.
  const std::string decalAlias =
      "__gz_proj_decal_" + this->nodeName;
  const std::string filterAlias =
      "__gz_proj_filter_" + this->nodeName;
  LoadImageAsOgreTexture(decalAlias, this->textureName);

  std::string filterPath = "projection_filter.png";
  // projection_filter.png lives in ogre/media/materials/textures, registered
  // as a resource location by OgreRenderEngine. Resolve to an absolute path
  // via the resource group manager so gz-common can decode it.
  {
    auto &rgm = Ogre::ResourceGroupManager::getSingleton();
    if (rgm.resourceExistsInAnyGroup(filterPath))
    {
      auto group = rgm.findGroupContainingResource(filterPath);
      auto archives = rgm.listResourceLocations(group);
      for (const auto &loc : *archives)
      {
        std::string candidate = loc + "/" + filterPath;
        if (common::isFile(candidate))
        {
          filterPath = candidate;
          break;
        }
      }
    }
  }
  LoadImageAsOgreTexture(filterAlias, filterPath);

  Ogre::TextureUnitState *texState = pass->createTextureUnitState(decalAlias);
  texState->setProjectiveTexturing(true, this->frustum.get());
  texState->setTextureAddressingMode(Ogre::TextureUnitState::TAM_BORDER);
  texState->setTextureFiltering(Ogre::TFO_ANISOTROPIC);
  texState->setTextureBorderColour(Ogre::ColourValue(0.0, 0.0, 0.0, 0.0));

  texState = pass->createTextureUnitState(filterAlias);
  texState->setProjectiveTexturing(true, this->filterFrustum.get());
  texState->setTextureAddressingMode(Ogre::TextureUnitState::TAM_CLAMP);
  texState->setTextureFiltering(Ogre::TFO_NONE);

  this->projectorTargets[_matName] = pass;

  // Under OGRE 1.12 + GL3+ core, the inherited Default scheme technique has
  // no GPU programs. Ask the RTSS to synthesize a shader-based technique
  // (for the wall pass only — the decal pass above already carries its own
  // hand-written GLSL programs) in the "projector" scheme so the viewport's
  // scheme switch resolves to a fully compiled technique. Without this the
  // listener returns an unloaded FF technique and GL3+ silently renders
  // nothing.
  auto *shaderGenerator =
      Ogre::RTShader::ShaderGenerator::getSingletonPtr();
  if (shaderGenerator)
  {
    try
    {
      shaderGenerator->createShaderBasedTechnique(
          *mat,
          Ogre::MaterialManager::DEFAULT_SCHEME_NAME,
          "projector");
      shaderGenerator->validateMaterial("projector", _matName,
          mat->getGroup());
    }
    catch (const Ogre::Exception &)
    {
      // Non-fatal; fall back to whatever the clone already has.
    }
  }
  mat->load();
}

/////////////////////////////////////////////////
void OgreProjectorListener::BindProjectorShader(Ogre::Pass *_pass)
{
  static const std::string vpName = "__gz_ProjectorDecalVS";
  static const std::string fpName = "__gz_ProjectorDecalFS";
  auto &hmgr = Ogre::HighLevelGpuProgramManager::getSingleton();
  if (!hmgr.getByName(vpName,
          Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME))
  {
    auto vp = hmgr.createProgram(vpName,
        Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
        "glsl", Ogre::GPT_VERTEX_PROGRAM);
    vp->setSource(
        "#version 130\n"
        "uniform mat4 worldviewproj_matrix;\n"
        "uniform mat4 world_matrix;\n"
        "uniform mat4 texture_viewproj_matrix0;\n"
        "uniform mat4 texture_viewproj_matrix1;\n"
        "in vec4 vertex;\n"
        "out vec4 vDecalCoord;\n"
        "out vec4 vFilterCoord;\n"
        "void main() {\n"
        "  gl_Position = worldviewproj_matrix * vertex;\n"
        "  vec4 worldPos = world_matrix * vertex;\n"
        "  vDecalCoord  = texture_viewproj_matrix0 * worldPos;\n"
        "  vFilterCoord = texture_viewproj_matrix1 * worldPos;\n"
        "}\n");
    vp->load();

    auto fp = hmgr.createProgram(fpName,
        Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
        "glsl", Ogre::GPT_FRAGMENT_PROGRAM);
    fp->setSource(
        "#version 130\n"
        "uniform sampler2D decalSampler;\n"
        "uniform sampler2D filterSampler;\n"
        "in vec4 vDecalCoord;\n"
        "in vec4 vFilterCoord;\n"
        "out vec4 oColor;\n"
        "void main() {\n"
        // Reject fragments behind the projector (w <= 0).
        "  if (vDecalCoord.w <= 0.0 || vFilterCoord.w <= 0.0) discard;\n"
        "  vec4 dtex = textureProj(decalSampler, vDecalCoord);\n"
        "  vec4 ftex = textureProj(filterSampler, vFilterCoord);\n"
        "  float alpha = dtex.a * ftex.a;\n"
        "  oColor = vec4(dtex.rgb, alpha);\n"
        "}\n");
    fp->load();
  }

  _pass->setVertexProgram(vpName);
  _pass->setFragmentProgram(fpName);

  auto vparams = _pass->getVertexProgramParameters();
  vparams->setNamedAutoConstant("worldviewproj_matrix",
      Ogre::GpuProgramParameters::ACT_WORLDVIEWPROJ_MATRIX);
  vparams->setNamedAutoConstant("world_matrix",
      Ogre::GpuProgramParameters::ACT_WORLD_MATRIX);
  vparams->setNamedAutoConstant("texture_viewproj_matrix0",
      Ogre::GpuProgramParameters::ACT_TEXTURE_VIEWPROJ_MATRIX, 0u);
  vparams->setNamedAutoConstant("texture_viewproj_matrix1",
      Ogre::GpuProgramParameters::ACT_TEXTURE_VIEWPROJ_MATRIX, 1u);

  auto fparams = _pass->getFragmentProgramParameters();
  fparams->setNamedConstant("decalSampler", 0);
  fparams->setNamedConstant("filterSampler", 1);
}

/////////////////////////////////////////////////
void OgreProjectorListener::RemoveDecalFromMaterials()
{
  GZ_PROFILE("OgreProjectorListener::RemoveDecalFromMaterials");
  for (auto it = this->projectorTargets.begin();
      it != this->projectorTargets.end(); ++it)
  {
    it->second->getParent()->removePass(it->second->getIndex());
  }
  this->projectorTargets.clear();

  OgreRTShaderSystem::Instance()->UpdateShaders();
}

/////////////////////////////////////////////////
void OgreProjectorListener::RemoveDecalFromMaterial(
    const std::string &_matName)
{
  GZ_PROFILE("OgreProjectorListener::RemoveDecalFromMaterial");
  auto projectorTargetIt = this->projectorTargets.find(_matName);
  if (projectorTargetIt != this->projectorTargets.end())
  {
    projectorTargetIt->second->getParent()->removePass(
        projectorTargetIt->second->getIndex());
    this->projectorTargets.erase(projectorTargetIt);
  }
}

/////////////////////////////////////////////////
void OgreProjector::UpdateCameraListener()
{
  GZ_PROFILE("OgreProjector::UpdateCameraListener");
  // if projector does not have custom visibility flags
  // project the texture onto entity's original material. It'll be visible
  // to all cameras
  if (this->VisibilityFlags() == GZ_VISIBILITY_ALL)
  {
    for (const auto &ogreCamIt : this->dataPtr->camerasWithListener)
    {
      const Ogre::String &camName = ogreCamIt.second;
      // instead of getting the camera pointer through ogreCamIt.first,
      // find camera pointer again to make sure the camera still exists
      // because there is a chance that we are holding onto a dangling pointer
      // if that camera was deleted already
      auto ogreCam = this->scene->OgreSceneManager()->getCamera(camName);
      ogreCam->getViewport()->getTarget()->removeListener(
          &this->dataPtr->projector);
    }
    this->dataPtr->camerasWithListener.clear();

    this->dataPtr->projector.AddDecalToVisibleMaterials();
    return;
  }

  // if a custom visibility flag is set, we will need to use a listener
  // for toggling the visibility of the decal
  // Modifying the original material directly on each projector update
  // didn't seem to work. So the strategy here is to clone the
  // object's material and add the projected texture onto the cloned material
  // \todo(anyone) figure out if it is possible to do achieve the same
  // result without cloning materials

  // Collect all materials that are visible in the frustum in current frame
  this->dataPtr->projector.UpdateVisibleMaterials();

  this->dataPtr->projector.SetVisibilityFlags(this->VisibilityFlags());

  // loop through color cameras and add listener to toggle visibility of
  // decals in these cameras
  for (unsigned int i = 0; i < this->scene->SensorCount(); ++i)
  {
    auto sensor = this->scene->SensorByIndex(i);
    OgreCameraPtr camera = std::dynamic_pointer_cast<OgreCamera>(sensor);
    if (camera)
    {
      auto ogreCam = camera->Camera();
      if (this->dataPtr->camerasWithListener.find(ogreCam)
          == this->dataPtr->camerasWithListener.end())
      {
        ogreCam->getViewport()->getTarget()->addListener(
            &this->dataPtr->projector);
        this->dataPtr->camerasWithListener[ogreCam] = ogreCam->getName();
      }
    }
    else
    {
      // depth camera can also generate rgb output (when simulating
      // RGBD cameras)
      OgreDepthCameraPtr depthCamera =
          std::dynamic_pointer_cast<OgreDepthCamera>(sensor);
      if (depthCamera)
      {
        auto ogreCam = depthCamera->Camera();
        if (this->dataPtr->camerasWithListener.find(ogreCam)
            == this->dataPtr->camerasWithListener.end())
        {
          ogreCam->getViewport()->getTarget()->addListener(
              &this->dataPtr->projector);
          this->dataPtr->camerasWithListener[ogreCam] = ogreCam->getName();
        }
      }
    }
  }
}

//////////////////////////////////////////////////
void OgreProjectorListener::SetVisibilityFlags(uint32_t _flags)
{
  this->visibilityFlags = _flags;
}

/////////////////////////////////////////////////
void OgreProjectorListener::UpdateVisibleMaterials()
{
  this->visibleMaterials = this->FindVisibleMaterials();
}

/////////////////////////////////////////////////
void OgreProjectorListener::preRenderTargetUpdate(
    const Ogre::RenderTargetEvent &_evt)
{
  GZ_PROFILE("OgreProjectorListener::preRenderTargetUpdate");
  if (this->defaultScheme.empty())
  {
    this->defaultScheme =
      _evt.source->getViewport(0)->getMaterialScheme();
  }

  // set material scheme so that we can switch an entity's
  // material to a cloned copy that has the projected texture
  // for cameras that can see the projector
  uint32_t mask = _evt.source->getViewport(0)->getVisibilityMask();
  if (this->visibilityFlags & mask)
  {
    Ogre::MaterialManager::getSingleton().addListener(this);
    _evt.source->getViewport(0)->setMaterialScheme("projector");
  }
}

/////////////////////////////////////////////////
void OgreProjectorListener::postRenderTargetUpdate(
    const Ogre::RenderTargetEvent &_evt)
{
  // remove the material scheme for the camera so it does not interfere
  // with other rendering operations
  _evt.source->getViewport(0)->setMaterialScheme(this->defaultScheme);
  Ogre::MaterialManager::getSingleton().removeListener(this);
}

/////////////////////////////////////////////////
Ogre::Technique *OgreProjectorListener::handleSchemeNotFound(
    uint16_t /*_schemeIndex*/, const Ogre::String &_schemeName,
    Ogre::Material *_originalMaterial, uint16_t /*_lodIndex*/,
    const Ogre::Renderable *_rend)
{
  GZ_PROFILE("OgreProjectorListener::handleSchemeNotFound");
  if (_schemeName != "projector")
    return nullptr;

  if (!_rend || typeid(*_rend) != typeid(Ogre::SubEntity))
    return nullptr;

  std::string projectedMaterialName =
      _originalMaterial->getName() + "_" + this->nodeName;

  // check if the material for the current entity is visible in the frustum
  if (this->visibleMaterials.find(_originalMaterial->getName())
      == this->visibleMaterials.end())
  {
    // if the material is not visible, check to see if it was visible before
    auto it = this->projectorTargets.find(projectedMaterialName);
    if (it != this->projectorTargets.end())
    {
      this->RemoveDecalFromMaterial(projectedMaterialName);
    }

    return nullptr;
  }

  // if visible check to see if we have a clone of the material already
  Ogre::Material *clone = nullptr;
  auto it = this->matClones.find(_originalMaterial->getName());
  if (it != this->matClones.end())
  {
    clone = it->second;
    // if the clone material is in the view, that means it has the projected
    // texture already
    if (this->projectorTargets.find(projectedMaterialName) !=
        this->projectorTargets.end())
    {
      return this->ProjectorSchemeTechnique(clone);
    }
  }
  // if clone is not available, clone it and add the projected texture to the
  // material
  else
  {
    clone = _originalMaterial->clone(projectedMaterialName).get();
    this->matClones[_originalMaterial->getName()] = clone;
  }

  this->AddDecalToMaterial(clone->getName());
  return this->ProjectorSchemeTechnique(clone);
}

/////////////////////////////////////////////////
Ogre::Technique *OgreProjectorListener::ProjectorSchemeTechnique(
    Ogre::Material *_clone) const
{
  // Prefer the RTSS-generated "projector" scheme technique set up in
  // AddDecalToMaterial. Under GL3+ core this is the only technique that has
  // GPU programs and can be executed; the inherited Default-scheme technique
  // is fixed-function only.
  for (size_t i = 0; i < _clone->getNumTechniques(); ++i)
  {
    Ogre::Technique *t = _clone->getTechnique(i);
    if (t->getSchemeName() == "projector" && t->isSupported())
      return t;
  }
  return _clone->getTechnique(0u);
}
