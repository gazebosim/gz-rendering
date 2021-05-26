/*
 * Copyright (C) 2021 Open Source Robotics Foundation
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

#include "ignition/common/Console.hh"
#include "ignition/rendering/ogre2/Ogre2Camera.hh"
#include "ignition/rendering/ogre2/Ogre2Conversions.hh"
#include "ignition/rendering/ogre2/Ogre2Includes.hh"
#include "ignition/rendering/ogre2/Ogre2RenderTarget.hh"
#include "ignition/rendering/ogre2/Ogre2Scene.hh"
#include "ignition/rendering/ogre2/Ogre2SelectionBuffer.hh"
#include "ignition/rendering/RenderTypes.hh"
#include "ignition/rendering/ogre2/Ogre2RenderTypes.hh"
#include "ignition/rendering/Utils.hh"
#include "ignition/rendering/ogre2/Ogre2RenderEngine.hh"
#include "ignition/rendering/ogre2/Ogre2SegmentationCamera.hh"
#include "ignition/rendering/ogre2/Ogre2Visual.hh"
#include "ignition/math/Color.hh"
#include <random>

using namespace ignition;
using namespace rendering;


namespace ignition
{
  namespace rendering
  {
    inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {
    /// \brief Helper class to assign unique colors to renderables
    class SegmentationMaterialSwitcher : public Ogre::RenderTargetListener
    {
      /// \brief Constructor
      public: explicit SegmentationMaterialSwitcher(Ogre2ScenePtr _scene);

      /// \brief Destructor
      public: ~SegmentationMaterialSwitcher();

      /// \brief Ogre's pre render update callback
      /// \param[in] _evt Ogre render target event containing information about
      /// the source render target.
      public: virtual void preRenderTargetUpdate(
                  const Ogre::RenderTargetEvent &_evt);

      /// \brief Ogre's post render update callback
      /// \param[in] _evt Ogre render target event containing information about
      /// the source render target.
      public: virtual void postRenderTargetUpdate(
                  const Ogre::RenderTargetEvent &_evt);

      /// \brief Convert label of semantic map to a unique color for colored map
      /// \param[in] _label id of the semantic map or encoded id of panoptic map
      /// \return _color unique color in the colored map for that label
      private: math::Color LabelToColor(int64_t _label);

      /// \brief Check if the color is taken previously & store it if not taken
      /// \param[in] _color Color to be checked
      /// \return True if taken, False elsewhere
      private: bool IsTakenColor(math::Color _color);

      /// \brief A map of ogre sub item pointer to their original hlms material
      private: std::map<Ogre::SubItem *, Ogre::HlmsDatablock *> datablockMap;

      /// \brief Ogre v1 material consisting of a shader that changes the
      /// appearance of item to use a unique color for mouse picking
      private: Ogre::MaterialPtr plainMaterial;

      /// \brief Ogre v1 material consisting of a shader that changes the
      /// appearance of item to use a unique color for mouse picking. In
      /// addition, the depth check and depth write properties disabled.
      private: Ogre::MaterialPtr plainOverlayMaterial;

      /// \brief User Data Key to set the label
      private: std::string labelKey = "label";

      /// \brief Background & unlabeled objects label id in semantic map
      private: int backgroundLabel = 0;

      /// \brief Background & unlabeled objects color in the colored map
      private: math::Color backgroundColor;

      /// \brief Segmentation Type (Semantic / Instance)
      private: SegmentationType type {SegmentationType::Semantic};

      /// \brief True to generate colored map
      /// False to generate labels ids map
      private: bool isColoredMap;

      /// \brief Keep track of num of instances of the same label
      /// Key: label id, value: num of instances
      private: std::map<int, int> instancesCount;

      /// \brief keep track of the random colors
      /// key: encoded key of r,g,b values. value: always True
      private: std::map<int64_t, bool> takenColors;

      /// \brief keep track of the labels which is already colored
      /// usful for coloring items in semantic mode in LabelToColor()
      /// key: label id. value: always True
      private: std::map<int64_t, bool> coloredLabel;

      /// \brief Pseudo num generator to generate colors from label id
      private: std::default_random_engine generator;

      /// \brief Ogre2 Scene
      private: Ogre2ScenePtr scene;

      friend class Ogre2SegmentationCamera;
    };
    }
  }
}

/////////////////////////////////////////////////
SegmentationMaterialSwitcher::SegmentationMaterialSwitcher(Ogre2ScenePtr _scene)
{
  this->scene = _scene;
  this->backgroundColor.Set(0, 0, 0);

  // plain material to switch item's material
  Ogre::ResourcePtr res =
    Ogre::MaterialManager::getSingleton().load("ign-rendering/plain_color",
        Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

  this->plainMaterial = res.staticCast<Ogre::Material>();
  this->plainMaterial->load();

  // plain overlay material
  this->plainOverlayMaterial =
      this->plainMaterial->clone("plain_color_overlay");
  if (!this->plainOverlayMaterial->getTechnique(0) ||
      !this->plainOverlayMaterial->getTechnique(0)->getPass(0))
  {
    ignerr << "Problem creating selection buffer overlay material"
        << std::endl;
    return;
  }
  Ogre::Pass *overlayPass =
      this->plainOverlayMaterial->getTechnique(0)->getPass(0);
  Ogre::HlmsMacroblock macroblock(*overlayPass->getMacroblock());
  macroblock.mDepthCheck = false;
  macroblock.mDepthWrite = false;
  overlayPass->setMacroblock(macroblock);
}

/////////////////////////////////////////////////
SegmentationMaterialSwitcher::~SegmentationMaterialSwitcher()
{
}

/////////////////////////////////////////////////
bool SegmentationMaterialSwitcher::IsTakenColor(math::Color _color)
{
  // get the int value of the 24 bit color
  int64_t colorId = _color.R() * 255 * 255 + _color.G() * 255 + _color.B();

  if (this->takenColors.count(colorId))
  {
    return true;
  }
  else
  {
    this->takenColors[colorId] = true;
    return false;
  }
}

/////////////////////////////////////////////////
math::Color SegmentationMaterialSwitcher::LabelToColor(int64_t _label)
{
  if (_label == this->backgroundLabel)
    return this->backgroundColor;

  // use label as seed to generate the same color for the label
  this->generator.seed(_label);
  std::uniform_int_distribution<int> distribution(0, 255);

  // random color
  int r = distribution(this->generator);
  int g = distribution(this->generator);
  int b = distribution(this->generator);

  math::Color color(r, g, b);

  if (this->type == SegmentationType::Semantic)
  {
    // if the label is colored before return the color
    // don't check fo taken colors in that case, all items
    // with the same label will have the same color
    if (this->coloredLabel.count(_label))
      return color;
  }

  // loop recursivly till finding a unique color
  if (this->IsTakenColor(color))
      return this->LabelToColor(_label);

  this->coloredLabel[_label] = true;
  return color;
}

////////////////////////////////////////////////
void SegmentationMaterialSwitcher::preRenderTargetUpdate(
    const Ogre::RenderTargetEvent &/*_evt*/)
{
  this->datablockMap.clear();
  auto itor = this->scene->OgreSceneManager()->getMovableObjectIterator(
      Ogre::ItemFactory::FACTORY_TYPE_NAME);

  while (itor.hasMoreElements())
  {
    Ogre::MovableObject *object = itor.peekNext();
    Ogre::Item *item = static_cast<Ogre::Item *>(object);

    // get visual from ogre item
    Ogre::Any userAny = item->getUserObjectBindings().getUserAny();

    if (!userAny.isEmpty() && userAny.getType() == typeid(unsigned int))
    {
      VisualPtr visual;
      try
      {
        visual = this->scene->VisualById(Ogre::any_cast<unsigned int>(userAny));
      }
      catch(Ogre::Exception &e)
      {
        ignerr << "Ogre Error:" << e.getFullDescription() << "\n";
      }
      Ogre2VisualPtr ogreVisual = std::dynamic_pointer_cast<Ogre2Visual>(
        visual);

      // get class user data
      Variant labelAny = ogreVisual->UserData(this->labelKey);

      int label;
      try
      {
        label = std::get<int>(labelAny);
      }
      catch(std::bad_variant_access &e)
      {
        // items with no class are considered background
        label = this->backgroundLabel;
      }

      for (unsigned int i = 0; i < item->getNumSubItems(); i++)
      {
        // save subitems material
        Ogre::SubItem *subItem = item->getSubItem(i);
        Ogre::HlmsDatablock *datablock = subItem->getDatablock();
        this->datablockMap[subItem] = datablock;

        // Material Switching
        if (this->type == SegmentationType::Semantic)
        {
          if (this->isColoredMap)
          {
            // semantic material(each pixel has item's color)
            math::Color color = this->LabelToColor(label);
            subItem->setCustomParameter(1, Ogre::Vector4(
              color.R(), color.G(), color.B(), 1.0));
          }
          else
          {
            // labels ids material(each pixel has item's label)
            float labelColor = label / 255.0;
            subItem->setCustomParameter(1, Ogre::Vector4(
              labelColor, labelColor, labelColor, 1.0));
          }
        }
        else if (this->type == SegmentationType::Panoptic)
        {
          if (!this->instancesCount.count(label))
            this->instancesCount[label] = 0;

          this->instancesCount[label]++;
          int instanceCount = this->instancesCount[label];

          if (this->isColoredMap)
          {
            // convert 24 bit number to int64
            int compositeId = label * 256 * 256 + instanceCount;

            math::Color color;
            if (label == this->backgroundLabel)
              color = this->LabelToColor(label);
            else
              color = this->LabelToColor(compositeId);

            subItem->setCustomParameter(1, Ogre::Vector4(
              color.R(), color.G(), color.B(), 1.0));
          }
          else
          {
            // 256 => 8 bits .. 255 => color percentage
            float labelColor = label / 255.0;
            float instanceColor1 = (instanceCount / 256) / 255.0;
            float instanceColor2 = (instanceCount % 256) / 255.0;

            subItem->setCustomParameter(1, Ogre::Vector4(
              labelColor, instanceColor1, instanceColor2, 1.0));
          }
        }

        // check if it's an overlay material by assuming the
        // depth check and depth write properties are off.
        if (!datablock->getMacroblock()->mDepthWrite &&
            !datablock->getMacroblock()->mDepthCheck)
          subItem->setMaterial(this->plainOverlayMaterial);
        else
          subItem->setMaterial(this->plainMaterial);
      }
    }
    itor.moveNext();
  }

  // reset the count & colors tracking
  this->instancesCount.clear();
  this->takenColors.clear();
  this->coloredLabel.clear();
}

/////////////////////////////////////////////////
void SegmentationMaterialSwitcher::postRenderTargetUpdate(
    const Ogre::RenderTargetEvent &/*_evt*/)
{
  // restore item to use hlms material
  auto itor = this->scene->OgreSceneManager()->getMovableObjectIterator(
      Ogre::ItemFactory::FACTORY_TYPE_NAME);
  while (itor.hasMoreElements())
  {
    Ogre::MovableObject *object = itor.peekNext();
    Ogre::Item *item = static_cast<Ogre::Item *>(object);
    for (unsigned int i = 0; i < item->getNumSubItems(); i++)
    {
      Ogre::SubItem *subItem = item->getSubItem(i);
      auto it = this->datablockMap.find(subItem);
      if (it != this->datablockMap.end())
        subItem->setDatablock(it->second);
    }
    itor.moveNext();
  }
}

/////////////////////////////////////////////////
class ignition::rendering::Ogre2SegmentationCameraPrivate
{
  /// \brief Material Switcher to switch item's material
  /// with colored version for segmentation
  public: SegmentationMaterialSwitcher *materialSwitcher {nullptr};

  /// \brief Compositor Manager to create workspace
  public: Ogre::CompositorManager2 *ogreCompositorManager {nullptr};

  /// \brief Workspace to interface with render texture
  public: Ogre::CompositorWorkspace *ogreCompositorWorkspace {nullptr};

  /// \brief Workspace Definition
  public: std::string workspaceDefinition;

  /// \brief Render Texture to store the final segmentation data
  public: Ogre::RenderTexture *ogreRenderTexture {nullptr};

  /// \brief Texture to create the render texture from.
  public: Ogre::TexturePtr ogreTexture;

  /// \brief Pixel Box to copy render texture data to a buffer
  public: Ogre::PixelBox *pixelBox {nullptr};

  /// \brief buffer to store render texture data & to be sent to listeners
  public: uint8_t *buffer = nullptr;

  /// \brief dummy render texture to set image dims
  public: Ogre2RenderTexturePtr dummyTexture {nullptr};

  /// \brief New Segmentation Frame Event to notify listeners with new data
  public: ignition::common::EventT<void(const uint8_t *, unsigned int,
        unsigned int, unsigned int, const std::string &)>
        newSegmentationFrame;

  /// \brief Image / Render Texture Format
  public: Ogre::PixelFormat format = Ogre::PF_R8G8B8;
};

/////////////////////////////////////////////////
Ogre2SegmentationCamera::Ogre2SegmentationCamera() :
  dataPtr(new Ogre2SegmentationCameraPrivate())
{
}

/////////////////////////////////////////////////
Ogre2SegmentationCamera::~Ogre2SegmentationCamera()
{
  this->Destroy();
}

/////////////////////////////////////////////////
void Ogre2SegmentationCamera::Init()
{
  BaseCamera::Init();
  this->CreateCamera();
  this->CreateRenderTexture();

  this->dataPtr->materialSwitcher =
    new SegmentationMaterialSwitcher(this->scene);
}

/////////////////////////////////////////////////
void Ogre2SegmentationCamera::CreateCamera()
{
  auto ogreScene = this->scene->OgreSceneManager();
  if (ogreScene == nullptr)
  {
    ignerr << "Scene manager cannot be obtained" << std::endl;
    return;
  }

  this->ogreCamera = ogreScene->createCamera(this->Name());
  if (this->ogreCamera == nullptr)
  {
    ignerr << "Ogre camera cannot be created" << std::endl;
    return;
  }

  this->ogreCamera->detachFromParent();
  this->ogreNode->attachObject(this->ogreCamera);

  // rotate to ignition gazebo coord.
  this->ogreCamera->yaw(Ogre::Degree(-90));
  this->ogreCamera->roll(Ogre::Degree(-90));
  this->ogreCamera->setFixedYawAxis(false);

  this->ogreCamera->setAutoAspectRatio(true);
  this->ogreCamera->setRenderingDistance(100);
  this->ogreCamera->setProjectionType(Ogre::ProjectionType::PT_PERSPECTIVE);
  this->ogreCamera->setCustomProjectionMatrix(false);
}

/////////////////////////////////////////////////
void Ogre2SegmentationCamera::Destroy()
{
  BaseCamera::Destroy();
  Camera::Destroy();

  delete this->dataPtr->ogreCompositorWorkspace;
  delete this->dataPtr->ogreCompositorManager;

  auto &manager = Ogre::TextureManager::getSingleton();
  manager.unload(this->dataPtr->ogreTexture->getName());
  manager.remove(this->dataPtr->ogreTexture->getName());
}

/////////////////////////////////////////////////
void Ogre2SegmentationCamera::PreRender()
{
  if (!this->dataPtr->ogreRenderTexture)
    this->CreateSegmentationTexture();
}

/////////////////////////////////////////////////
void Ogre2SegmentationCamera::CreateSegmentationTexture()
{
  // Camera Parameters
  this->ogreCamera->setNearClipDistance(this->NearClipPlane());
  this->ogreCamera->setFarClipDistance(this->FarClipPlane());
  this->ogreCamera->setAspectRatio(this->AspectRatio());
  double vfov = 2.0 * atan(tan(this->HFOV().Radian() / 2.0) /
    this->AspectRatio());
  this->ogreCamera->setFOVy(Ogre::Radian(vfov));

  auto width = this->ImageWidth();
  auto height = this->ImageHeight();

  // texture
  this->dataPtr->ogreTexture =
    Ogre::TextureManager::getSingleton().createManual(
    "SegmentationCameraTexture",
    Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
    Ogre::TEX_TYPE_2D, width, height, 0, this->dataPtr->format,
    Ogre::TU_RENDERTARGET
  );

  // render texture
  auto hardwareBuffer = this->dataPtr->ogreTexture->getBuffer();
  this->dataPtr->ogreRenderTexture = hardwareBuffer->getRenderTarget();

  // switch the material to a unique color for each object
  // in the pre render & get the original material again in the post render
  this->dataPtr->ogreRenderTexture->addListener(
    this->dataPtr->materialSwitcher);

  // workspace
  auto engine = Ogre2RenderEngine::Instance();
  auto ogreRoot = engine->OgreRoot();
  this->dataPtr->ogreCompositorManager = ogreRoot->getCompositorManager2();

  this->dataPtr->workspaceDefinition = "SegmentationCameraWorkspace_" +
    this->Name();
  auto backgroundColor = Ogre2Conversions::Convert(
    this->dataPtr->materialSwitcher->backgroundColor);

  // basic workspace consist of clear pass with the givin color &
  // a render scene pass to the givin render texture
  this->dataPtr->ogreCompositorManager->createBasicWorkspaceDef(
    this->dataPtr->workspaceDefinition, backgroundColor);

  // connect the compositor with the render texture to render the final output
  this->dataPtr->ogreCompositorWorkspace =
    this->dataPtr->ogreCompositorManager->addWorkspace(
      this->scene->OgreSceneManager(),
      this->dataPtr->ogreRenderTexture,
      this->ogreCamera,
      this->dataPtr->workspaceDefinition,
      false
    );

  // set visibility mask
  auto node = this->dataPtr->ogreCompositorWorkspace->getNodeSequence()[0];
  auto pass = node->_getPasses()[1]->getDefinition();
  auto renderScenePass =
    dynamic_cast<const Ogre::CompositorPassSceneDef *>(pass);
  const_cast<Ogre::CompositorPassSceneDef *>(
    renderScenePass)->setVisibilityMask(IGN_VISIBILITY_ALL);

  // buffer to store render texture data
  auto bufferSize = Ogre::PixelUtil::getMemorySize(
    width, height, 1, this->dataPtr->format);
  this->dataPtr->buffer = new uint8_t[bufferSize];
  this->dataPtr->pixelBox = new Ogre::PixelBox(width, height, 1,
    this->dataPtr->format, this->dataPtr->buffer);
}

/////////////////////////////////////////////////
void Ogre2SegmentationCamera::Render()
{
  this->dataPtr->ogreCompositorWorkspace->setEnabled(true);
  auto engine = Ogre2RenderEngine::Instance();
  auto ogreRoot = engine->OgreRoot();
  ogreRoot->renderOneFrame();
  this->dataPtr->ogreCompositorWorkspace->setEnabled(false);
}

/////////////////////////////////////////////////
void Ogre2SegmentationCamera::PostRender()
{
  // copy render texture data to the pixel box & its buffer
  this->dataPtr->ogreRenderTexture->copyContentsToMemory(
    *this->dataPtr->pixelBox,
    Ogre::RenderTarget::FB_FRONT
  );

  // return if no one is listening to the new frame
  if (this->dataPtr->newSegmentationFrame.ConnectionCount() == 0)
    return;

  uint width = this->ImageWidth();
  uint height = this->ImageHeight();
  uint channelCount = 3;

  this->dataPtr->newSegmentationFrame(
    this->dataPtr->buffer,
    width, height, channelCount,
    Ogre::PixelUtil::getFormatName(this->dataPtr->format)
  );
}

/////////////////////////////////////////////////
uint8_t *Ogre2SegmentationCamera::SegmentationData() const
{
  return this->dataPtr->buffer;
}

/////////////////////////////////////////////////
ignition::common::ConnectionPtr
  Ogre2SegmentationCamera::ConnectNewSegmentationFrame(
  std::function<void(const uint8_t *, unsigned int, unsigned int,
  unsigned int, const std::string &)>  _subscriber)
{
  return this->dataPtr->newSegmentationFrame.Connect(_subscriber);
}

/////////////////////////////////////////////////
void Ogre2SegmentationCamera::CreateRenderTexture()
{
  RenderTexturePtr base = this->scene->CreateRenderTexture();
  this->dataPtr->dummyTexture =
    std::dynamic_pointer_cast<Ogre2RenderTexture>(base);
  this->dataPtr->dummyTexture->SetWidth(1);
  this->dataPtr->dummyTexture->SetHeight(1);
}

/////////////////////////////////////////////////
RenderTargetPtr Ogre2SegmentationCamera::RenderTarget() const
{
  return this->dataPtr->dummyTexture;
}

/////////////////////////////////////////////////
void Ogre2SegmentationCamera::SetBackgroundColor(math::Color _color)
{
  this->dataPtr->materialSwitcher->backgroundColor.Set(
    _color.R(), _color.G(), _color.B()
  );
}

/////////////////////////////////////////////////
void Ogre2SegmentationCamera::SetBackgroundLabel(int _label)
{
  this->dataPtr->materialSwitcher->backgroundLabel = _label;
}

/////////////////////////////////////////////////
math::Color Ogre2SegmentationCamera::BackgroundColor()
{
  return this->dataPtr->materialSwitcher->backgroundColor;
}

/////////////////////////////////////////////////
int Ogre2SegmentationCamera::BackgroundLabel()
{
  return this->dataPtr->materialSwitcher->backgroundLabel;
}

/////////////////////////////////////////////////
void Ogre2SegmentationCamera::SetSegmentationType(SegmentationType _type)
{
  this->dataPtr->materialSwitcher->type = _type;
}

/////////////////////////////////////////////////
void Ogre2SegmentationCamera::EnableColoredMap(bool _enable)
{
  this->dataPtr->materialSwitcher->isColoredMap = _enable;
}
