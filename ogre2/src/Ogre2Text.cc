/*
 * Copyright (C) 2026 cellumation GmbH
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

#include "gz/rendering/config.hh"

#ifdef GZ_RENDERING_HAVE_OGRE2_OVERLAY

#ifdef _WIN32
  // Ensure that windows types like LONGLONG are defined
  #include <windows.h>
#endif

#include <gz/common/Console.hh>

#include "gz/rendering/ogre2/Ogre2Material.hh"
#include "gz/rendering/ogre2/Ogre2Scene.hh"
#include "gz/rendering/ogre2/Ogre2Text.hh"

#ifdef _MSC_VER
  #pragma warning(push, 0)
#endif
#include <OgreCamera.h>
#include <OgreHardwareBufferManager.h>
#include <OgreMaterialManager.h>
#include <OgreRenderOperation.h>
#include <OgreRenderQueue.h>
#include <OgreRoot.h>
#include <OgreSceneManager.h>
#include <OgreSceneNode.h>
#include <OgreTechnique.h>
#include <OgreVertexIndexData.h>
#include <Overlay/OgreFont.h>
#include <Overlay/OgreFontManager.h>
#include <Overlay/OgreOverlayManager.h>
#include <Overlay/OgreOverlaySystem.h>
#ifdef _MSC_VER
  #pragma warning(pop)
#endif

#define POS_TEX_BINDING    0
#define COLOUR_BINDING     1

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
      class Ogre2MovableText;
      class Ogre2TextPrivate;
    }
  }
}

class gz::rendering::Ogre2MovableText
  : public Ogre::MovableObject, public Ogre::Renderable
{
  /// \brief Constructor
  /// \param[in] _id Object id
  /// \param[in] _memoryManager Object memory manager
  /// \param[in] _sceneManager Scene manager
  /// \param[in] _renderQueueId Render queue id
  public: Ogre2MovableText(Ogre::IdType _id,
      Ogre::ObjectMemoryManager *_memoryManager,
      Ogre::SceneManager *_sceneManager,
      uint8_t _renderQueueId);

  /// \brief Destructor
  public: virtual ~Ogre2MovableText();

  /// \brief Set the font. Valid fonts are defined in
  /// media/fonts/gz-rendering.fontdef
  /// \param[in] _font Name of the font
  public: void SetFontName(const std::string &_font);

  /// \brief Set the text to display.
  /// \param[in] _text The text to display.
  public: void SetTextString(const std::string &_text);

  /// \brief Set the text color.
  /// \param[in] _color Text color.
  public: void SetColor(const gz::math::Color &_color);

  /// \brief Set the height of the character in meters.
  /// \param[in] _height Height of the characters.
  public: void SetCharHeight(const float _height);

  /// \brief Set the width of spaces between words.
  /// \param[in] _width Space width
  public: void SetSpaceWidth(const float _width);

  /// \brief Set the alignment of the text
  /// \param[in] _horizAlign Horizontal alignment
  /// \param[in] _vertAlign Vertical alignment
  public: void SetTextAlignment(const TextHorizontalAlign &_horizAlign,
                                const TextVerticalAlign &_vertAlign);

  /// \brief Set the baseline height of the text
  /// \param[in] _height Baseline height
  public: void SetBaseline(const float _baseline);

  /// \brief True = text always is displayed on top.
  /// \param[in] _show Set to true to render the text on top of
  /// all other drawables.
  public: void SetShowOnTop(const bool _onTop);

  /// \brief Get the axis aligned bounding box of the text.
  /// \return The axis aligned bounding box.
  public: gz::math::AxisAlignedBox AABB() const;

  /// \brief Setup the geometry based on input text string.
  public: void SetupGeometry();

  /// \brief Update color of text.
  public: void UpdateColors();

  /// \brief Update material properties, mainly if onTop has changed.
  public: void UpdateMaterial();

  /// \brief Update font.
  public: void UpdateFont();

  /// \brief Update all text properties if dirty.
  public: void Update();

  /// \brief Set font name implementation.
  /// \param[in] _newFontName Name of font
  public: void SetFontNameImpl(const std::string &_newFontName);

  /// \internal
  /// \brief Get the world transform (from Renderable)
  protected: void getWorldTransforms(Ogre::Matrix4 *_xform) const override;

  /// \internal
  /// \brief Get the render operation
  protected: void getRenderOperation(Ogre::v1::RenderOperation &_op,
      bool _casterPass) override;

  /// \internal
  /// \brief Get the material
  protected: Ogre::MaterialPtr getMaterial() const;

  /// \internal
  /// \brief Get the lights
  protected: const Ogre::LightList &getLights() const override;

  /// \internal
  /// \brief Get whether the object casts shadows
  protected: bool getCastsShadows() const override;

  /// \internal
  private: const Ogre::String &getMovableType() const override;

  /// \internal
  private: void _updateRenderQueue(Ogre::RenderQueue *_queue,
      Ogre::Camera *_camera, const Ogre::Camera *_lodCamera) override;

  /// \brief Flag to indicate text properties have changed
  private: bool textDirty = false;

  /// \brief Flag to indicate text color has changed
  private: bool colorDirty = true;

  /// \brief Flag to indicate text font has changed
  private: bool fontDirty = true;

  /// \brief Flag to indicate text material has changed
  private: bool materialDirty = false;

  /// \brief Bounding radius
  private: float radius = 0;

  /// \brief Viewport aspect coefficient
  private: float viewportAspectCoef = 0.75;

  /// \brief Ogre render operation
  private: Ogre::v1::RenderOperation renderOp;

  /// \brief Axis aligned box
  private: Ogre::AxisAlignedBox *aabb = nullptr;

  /// \brief Pointer to camera which the text is facing - never set.
  private: Ogre::Camera *camera = nullptr;

  /// \brief Pointer to font
  private: Ogre::Font *font = nullptr;

  /// \brief Text ogreMaterial
  private: Ogre::MaterialPtr ogreMaterial;

  /// \brief Keep an empty list of lights.
  private: Ogre::LightList lightList;

  /// \brief Font name, such as "Liberation Sans"
  private: std::string fontName;

  /// \brief Text being displayed
  private: std::string text;

  /// \brief Text color
  private: gz::math::Color color;

  /// \brief Character height in meters
  private: float charHeight = 0.0;

  /// \brief Width of space between letters
  private: float spaceWidth = 0.0;

  /// \brief Horizontal alignment
  private: TextHorizontalAlign horizontalAlign;

  /// \brief Vertical alignment
  private: TextVerticalAlign verticalAlign;

  /// \brief Baseline height in meters.
  private: float baseline = 0.0;

  /// \brief True for text to be displayed on top of other objects in the
  /// scene.
  private: bool onTop = false;
};

/// \brief Private data for the Ogre2Text class.
class gz::rendering::Ogre2TextPrivate
{
  /// \brief Text material
  public: Ogre2MaterialPtr material;

  /// Pointer to ogre movable text object
  public: std::unique_ptr<Ogre2MovableText> ogreObj;
};

using namespace gz;
using namespace rendering;

//////////////////////////////////////////////////
Ogre2MovableText::Ogre2MovableText(Ogre::IdType _id,
    Ogre::ObjectMemoryManager *_memoryManager,
    Ogre::SceneManager *_sceneManager,
    uint8_t _renderQueueId)
    : Ogre::MovableObject(_id, _memoryManager, _sceneManager, _renderQueueId)
{
  this->renderOp.vertexData = nullptr;
  this->aabb = new Ogre::AxisAlignedBox;

  // Provide a custom parameter for solid-color render passes (e.g. selection
  // buffer, segmentation, thermal, gpu-rays). Unlike Ogre::Items, this is a
  // custom v1 Renderable that the material switchers do not iterate over, so
  // they never call setCustomParameter(1u, ...) on it. Without this, the
  // GzHlmsUnlit listener throws Ogre::ItemIdentityException in
  // GORM_SOLID_COLOR mode (getCustomParameter(1u)) and crashes the process.
  this->setCustomParameter(1u, Ogre::Vector4(0, 0, 0, 1));
}

//////////////////////////////////////////////////
Ogre2MovableText::~Ogre2MovableText()
{
  delete this->renderOp.vertexData;
  delete this->aabb;
}

//////////////////////////////////////////////////
void Ogre2MovableText::SetFontName(const std::string &_font)
{
  if (this->fontName != _font)
  {
    this->fontName = _font;
    this->fontDirty = true;
  }
}

//////////////////////////////////////////////////
void Ogre2MovableText::SetTextString(const std::string &_text)
{
  if (this->text != _text)
  {
    this->text = _text;
    this->textDirty = true;
  }
}

//////////////////////////////////////////////////
void Ogre2MovableText::SetColor(const math::Color &_color)
{
  if (this->color != _color)
  {
    this->color = _color;
    this->colorDirty = true;
  }
}

//////////////////////////////////////////////////
void Ogre2MovableText::SetCharHeight(const float _height)
{
  if (!math::equal(this->charHeight, _height))
  {
    this->charHeight = _height;
    this->textDirty = true;
  }
}

//////////////////////////////////////////////////
void Ogre2MovableText::SetSpaceWidth(const float _width)
{
  if (!math::equal(this->spaceWidth, _width))
  {
    this->spaceWidth = _width;
    this->textDirty = true;
  }
}

//////////////////////////////////////////////////
void Ogre2MovableText::SetTextAlignment(const TextHorizontalAlign &_horizAlign,
                                       const TextVerticalAlign &_vertAlign)
{
  if (this->horizontalAlign != _horizAlign)
  {
    this->horizontalAlign = _horizAlign;
    this->textDirty = true;
  }
  if (this->verticalAlign != _vertAlign)
  {
    this->verticalAlign = _vertAlign;
    this->textDirty = true;
  }
}

//////////////////////////////////////////////////
void Ogre2MovableText::SetBaseline(const float _baseline)
{
  if (!math::equal(this->baseline, _baseline))
  {
    this->baseline = _baseline;
    this->textDirty = true;
  }
}

//////////////////////////////////////////////////
void Ogre2MovableText::SetShowOnTop(const bool _onTop)
{
  if (this->onTop != _onTop)
  {
    this->onTop = _onTop;
    this->materialDirty = true;
  }
}

//////////////////////////////////////////////////
math::AxisAlignedBox Ogre2MovableText::AABB() const
{
  return math::AxisAlignedBox(
      math::Vector3d(this->aabb->getMinimum().x,
                    this->aabb->getMinimum().y,
                    this->aabb->getMinimum().z),
      math::Vector3d(this->aabb->getMaximum().x,
                    this->aabb->getMaximum().y,
                    this->aabb->getMaximum().z));
}

//////////////////////////////////////////////////
void Ogre2MovableText::UpdateFont()
{
  this->SetFontNameImpl(this->fontName);
}

//////////////////////////////////////////////////
void Ogre2MovableText::SetFontNameImpl(const std::string &_newFontName)
{
  if ((Ogre::MaterialManager::getSingletonPtr()->resourceExists(
          this->mName + "Material")))
  {
    Ogre::MaterialManager::getSingleton().remove(this->mName + "Material");
  }

  if (this->fontName != _newFontName ||
      !this->ogreMaterial || !this->font)
  {
    auto fontPtr = Ogre::FontManager::getSingleton()
        .getByName(_newFontName,
        Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME);

    if (!fontPtr)
    {
      gzerr << "Could not find font " + _newFontName << std::endl;
      return;
    }
    this->font = fontPtr.get();
    this->fontName = _newFontName;

    this->font->load();

    if (this->ogreMaterial)
    {
      Ogre::MaterialManager::getSingletonPtr()->remove(
          this->ogreMaterial->getName());
      this->ogreMaterial = Ogre::MaterialPtr();
    }

    this->ogreMaterial = Ogre::MaterialManager::getSingleton().create(
        this->mName + "Material",
        Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

    this->ogreMaterial->load();

    if (Ogre::HlmsDatablock *datablock = this->font->getHlmsDatablock())
      this->setDatablock(datablock);

    this->textDirty = true;
  }
}

//////////////////////////////////////////////////
void Ogre2MovableText::SetupGeometry()
{
  GZ_ASSERT(this->font, "font class member is null");
  GZ_ASSERT(this->ogreMaterial, "ogreMaterial class member is null");

  Ogre::v1::VertexDeclaration *decl = nullptr;
  Ogre::v1::VertexBufferBinding *bind = nullptr;
  Ogre::v1::HardwareVertexBufferSharedPtr ptbuf;
  Ogre::v1::HardwareVertexBufferSharedPtr cbuf;
  float *pVert = nullptr;
  float largestWidth = 0;
  float left = 0;
  float top = 0;
  size_t offset = 0;
  float maxSquaredRadius = 0.0f;
  bool first = true;
  bool newLine = true;
  float len = 0.0f;

  // for calculation of AABB
  Ogre::Vector3 min(0, 0, 0);
  Ogre::Vector3 max(0, 0, 0);
  Ogre::Vector3 currPos(0, 0, 0);

  auto vertexCount = static_cast<unsigned int>(this->text.size() * 6);

  if (this->renderOp.vertexData)
  {
    {
      delete this->renderOp.vertexData;
      this->renderOp.vertexData = nullptr;
      this->colorDirty = true;
    }
  }

  if (!this->renderOp.vertexData)
    this->renderOp.vertexData = new Ogre::v1::VertexData(
        Ogre::v1::HardwareBufferManager::getSingletonPtr());

  this->renderOp.indexData = 0;
  this->renderOp.vertexData->vertexStart = 0;
  this->renderOp.vertexData->vertexCount = vertexCount;
  this->renderOp.operationType =
      Ogre::OT_TRIANGLE_LIST;
  this->renderOp.useIndexes = false;

  decl = this->renderOp.vertexData->vertexDeclaration;
  bind = this->renderOp.vertexData->vertexBufferBinding;

  // create/bind positions/tex.ccord. buffer
  if (!decl->findElementBySemantic(Ogre::VES_POSITION))
    decl->addElement(POS_TEX_BINDING, offset, Ogre::VET_FLOAT3,
                     Ogre::VES_POSITION);

  offset += Ogre::v1::VertexElement::getTypeSize(Ogre::VET_FLOAT3);

  if (!decl->findElementBySemantic(Ogre::VES_TEXTURE_COORDINATES))
    decl->addElement(POS_TEX_BINDING, offset, Ogre::VET_FLOAT2,
                     Ogre::VES_TEXTURE_COORDINATES, 0);

  ptbuf = Ogre::v1::HardwareBufferManager::getSingleton().createVertexBuffer(
            decl->getVertexSize(POS_TEX_BINDING),
            this->renderOp.vertexData->vertexCount,
            Ogre::v1::HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY);

  bind->setBinding(POS_TEX_BINDING, ptbuf);

  // Colours - store these in a separate buffer because they change less often
  if (!decl->findElementBySemantic(Ogre::VES_DIFFUSE))
    decl->addElement(COLOUR_BINDING, 0, Ogre::VET_COLOUR, Ogre::VES_DIFFUSE);

  cbuf = Ogre::v1::HardwareBufferManager::getSingleton().createVertexBuffer(
           decl->getVertexSize(COLOUR_BINDING),
           this->renderOp.vertexData->vertexCount,
           Ogre::v1::HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY);

  bind->setBinding(COLOUR_BINDING, cbuf);

  pVert = static_cast<float*>(ptbuf->lock(Ogre::v1::HardwareBuffer::HBL_DISCARD));

  // Derive space width from a capital A
  if (math::equal(this->spaceWidth, 0.0f))
  {
    this->spaceWidth = this->font->getGlyphAspectRatio('A') *
        this->charHeight * 2.0;
  }

  if (this->verticalAlign == TextVerticalAlign::TOP)
  {
    // Raise the first line of the caption
    top += this->charHeight;
    for (auto it = this->text.begin(); it != this->text.end(); ++it)
    {
      if (*it == '\n')
        top += this->charHeight * 2.0;
    }
  }

  for (auto it = this->text.begin(); it != this->text.end(); ++it)
  {
    if (newLine)
    {
      len = 0.0;
      for (std::string::iterator j = it; j != this->text.end(); ++j)
      {
        Ogre::Font::CodePoint character = *j;
        if (character == '\n')
        {
          break;
        }
        // space
        else if (character == ' ')
        {
          len += this->spaceWidth;
        }
        else
        {
          len += this->font->getGlyphAspectRatio(character) *
                 this->charHeight * 2.0 *
                 this->viewportAspectCoef;
        }
      }

      newLine = false;
    }

    Ogre::Font::CodePoint character = (*it);
    if (character == '\n')
    {
      left = 0.0;
      top -= this->charHeight * 2.0;
      newLine = true;

      // Also reduce tri count
      this->renderOp.vertexData->vertexCount -= 6;
      continue;
    }
    // space
    else if (character == ' ')
    {
      // Just leave a gap, no tris
      left += this->spaceWidth;

      // Also reduce tri count
      this->renderOp.vertexData->vertexCount -= 6;
      continue;
    }

    float horizHeight = this->font->getGlyphAspectRatio(character) *
                         this->viewportAspectCoef;

    const auto &uvRect = this->font->getGlyphTexCoords(character);

    // each vert is (x, y, z, u, v)
    //------------------------------------------------------------------------
    // First tri
    //
    // Upper left
    if (this->horizontalAlign == TextHorizontalAlign::LEFT)
      *pVert++ = left;
    else
      *pVert++ = left - (len/2.0);

    *pVert++ = top;
    *pVert++ = 0;
    *pVert++ = uvRect.left;
    *pVert++ = uvRect.top;

    // Deal with bounds
    if (this->horizontalAlign == TextHorizontalAlign::LEFT)
      currPos = Ogre::Vector3(left, top, 0);
    else
      currPos = Ogre::Vector3(left - (len/2.0), top, 0);

    if (first)
    {
      min = max = currPos;
      maxSquaredRadius = currPos.squaredLength();
      first = false;
    }
    else
    {
      min.makeFloor(currPos);
      max.makeCeil(currPos);
      maxSquaredRadius = std::max(maxSquaredRadius, currPos.squaredLength());
    }

    top -= this->charHeight * 2.0;

    // Bottom left
    if (this->horizontalAlign == TextHorizontalAlign::LEFT)
      *pVert++ = left;
    else
      *pVert++ = left - (len / 2.0);

    *pVert++ = top;
    *pVert++ = 0;
    *pVert++ = uvRect.left;
    *pVert++ = uvRect.bottom;

    // Deal with bounds
    if (this->horizontalAlign == TextHorizontalAlign::LEFT)
      currPos = Ogre::Vector3(left, top, 0);
    else
      currPos = Ogre::Vector3(left - (len/2), top, 0);

    min.makeFloor(currPos);
    max.makeCeil(currPos);
    maxSquaredRadius = std::max(maxSquaredRadius, currPos.squaredLength());


    top += this->charHeight * 2.0;
    left += horizHeight * this->charHeight * 2.0;

    // Top right
    if (this->horizontalAlign == TextHorizontalAlign::LEFT)
      *pVert++ = left;
    else
      *pVert++ = left - (len/2.0);

    *pVert++ = top;
    *pVert++ = 0;
    *pVert++ = uvRect.right;
    *pVert++ = uvRect.top;

    // Deal with bounds
    if (this->horizontalAlign == TextHorizontalAlign::LEFT)
      currPos = Ogre::Vector3(left, top, 0);
    else
      currPos = Ogre::Vector3(left - (len/2), top, 0);

    min.makeFloor(currPos);
    max.makeFloor(currPos);
    maxSquaredRadius = std::max(maxSquaredRadius, currPos.squaredLength());


    //------------------------------------------------------------------------

    //------------------------------------------------------------------------
    // Second tri
    //
    // Top right (again)
    if (this->horizontalAlign == TextHorizontalAlign::LEFT)
      *pVert++ = left;
    else
      *pVert++ = left - (len/2.0);
    *pVert++ = top;
    *pVert++ = 0;
    *pVert++ = uvRect.right;
    *pVert++ = uvRect.top;

    // Deal with bounds
    currPos = Ogre::Vector3(left, top, 0);
    min.makeFloor(currPos);
    max.makeFloor(currPos);
    maxSquaredRadius = std::max(maxSquaredRadius, currPos.squaredLength());


    top -= this->charHeight * 2.0;
    left -= horizHeight  * this->charHeight * 2.0;

    // Bottom left (again)
    if (this->horizontalAlign == TextHorizontalAlign::LEFT)
      *pVert++ = left;
    else
      *pVert++ = left - (len/2.0);
    *pVert++ = top;
    *pVert++ = 0;
    *pVert++ = uvRect.left;
    *pVert++ = uvRect.bottom;

    // Deal with bounds
    currPos = Ogre::Vector3(left, top, 0);
    min.makeFloor(currPos);
    max.makeFloor(currPos);
    maxSquaredRadius = std::max(maxSquaredRadius, currPos.squaredLength());


    left += horizHeight  * this->charHeight * 2.0;

    // Bottom right
    if (this->horizontalAlign == TextHorizontalAlign::LEFT)
      *pVert++ = left;
    else
      *pVert++ = left - (len/2.0);
    *pVert++ = top;
    *pVert++ = 0;
    *pVert++ = uvRect.right;
    *pVert++ = uvRect.bottom;

    // Deal with bounds
    currPos = Ogre::Vector3(left, top, 0);
    min.makeFloor(currPos);
    max.makeFloor(currPos);
    maxSquaredRadius = std::max(maxSquaredRadius, currPos.squaredLength());

    //-------------------------------------------------------------------------


    // Go back up with top
    top += this->charHeight * 2.0;

    float currentWidth = (left + 1.0)/2.0;
    if (currentWidth > largestWidth)
    {
      largestWidth = currentWidth;
    }
  }

  // Unlock vertex buffer
  ptbuf->unlock();

  // update AABB/Sphere radius
  this->aabb->setMinimum(min);
  this->aabb->setMaximum(max);
  this->radius = Ogre::Math::Sqrt(maxSquaredRadius);

  this->setLocalAabb(Ogre::Aabb::newFromExtents(min, max));

  if (this->colorDirty)
    this->UpdateColors();

  this->textDirty = false;
}

//////////////////////////////////////////////////
void Ogre2MovableText::UpdateColors()
{
  Ogre::RGBA clr;
  Ogre::v1::HardwareVertexBufferSharedPtr vbuf;
  Ogre::RGBA *pDest{nullptr};

  GZ_ASSERT(this->font, "font class member is null");
  GZ_ASSERT(this->ogreMaterial, "ogreMaterial class member is null");

  // Convert to system-specific
  Ogre::ColourValue cv(this->color.R(), this->color.G(),
                       this->color.B(), this->color.A());
  Ogre::Root::getSingleton().convertColourValue(cv, &clr);

  vbuf = this->renderOp.vertexData->vertexBufferBinding->getBuffer(
         COLOUR_BINDING);

  pDest = static_cast<Ogre::RGBA*>(
      vbuf->lock(Ogre::v1::HardwareBuffer::HBL_DISCARD));

  for (Ogre::uint32 i = 0; i < this->renderOp.vertexData->vertexCount; ++i)
  {
    *pDest++ = clr;
  }

  vbuf->unlock();
  this->colorDirty = false;
}

//////////////////////////////////////////////////
void Ogre2MovableText::UpdateMaterial()
{
  this->materialDirty = false;
}

//////////////////////////////////////////////////
void Ogre2MovableText::Update()
{
  if (this->fontDirty)
    this->UpdateFont();
  if (this->textDirty)
    this->SetupGeometry();
  if (this->colorDirty)
    this->UpdateColors();
  if (this->materialDirty)
    this->UpdateMaterial();
}

//////////////////////////////////////////////////
const Ogre::String &Ogre2MovableText::getMovableType() const
{
  static Ogre::String movType = "Ogre2MovableText";
  return movType;
}

//////////////////////////////////////////////////
void Ogre2MovableText::getWorldTransforms(Ogre::Matrix4 *_xform) const
{
  if (this->isVisible() && this->camera)
  {
    Ogre::Matrix3 rot3x3, scale3x3 = Ogre::Matrix3::IDENTITY;

    // store rotation in a matrix
    this->camera->getDerivedOrientation().ToRotationMatrix(rot3x3);

    // parent node position
    Ogre::Vector3 ppos = mParentNode->_getDerivedPosition() +
                         Ogre::Vector3::UNIT_Z * this->baseline;

    // apply scale
    scale3x3[0][0] = mParentNode->_getDerivedScale().x / 2;
    scale3x3[1][1] = mParentNode->_getDerivedScale().y / 2;
    scale3x3[2][2] = mParentNode->_getDerivedScale().z / 2;

    // apply all transforms to xform
    *_xform = (rot3x3 * scale3x3);
    _xform->setTrans(ppos);
  }
}

//////////////////////////////////////////////////
void Ogre2MovableText::getRenderOperation(Ogre::v1::RenderOperation &_op,
    bool /*_casterPass*/)
{
  if (this->isVisible())
  {
    this->Update();
    _op = this->renderOp;
  }
}

//////////////////////////////////////////////////
Ogre::MaterialPtr Ogre2MovableText::getMaterial(void) const
{
  GZ_ASSERT(this->ogreMaterial, "ogreMaterial class member is null");
  return this->ogreMaterial;
}

//////////////////////////////////////////////////
const Ogre::LightList &Ogre2MovableText::getLights(void) const
{
  return this->lightList;
}

//////////////////////////////////////////////////
bool Ogre2MovableText::getCastsShadows(void) const
{
  return false;
}

//////////////////////////////////////////////////
void Ogre2MovableText::_updateRenderQueue(Ogre::RenderQueue* _queue,
    Ogre::Camera *_camera, const Ogre::Camera * /*_lodCamera*/)
{
  if (this->isVisible())
  {
    this->camera = _camera;

    this->Update();

    _queue->addRenderableV1(mRenderQueueID, false, this, this);
  }
}

//////////////////////////////////////////////////
Ogre2Text::Ogre2Text()
    : dataPtr(new Ogre2TextPrivate)
{
}

//////////////////////////////////////////////////
Ogre2Text::~Ogre2Text()
{
}

//////////////////////////////////////////////////
void Ogre2Text::Init()
{
  auto scenePtr = this->Scene();
  auto ogreScene = std::dynamic_pointer_cast<Ogre2Scene>(scenePtr);
  GZ_ASSERT(ogreScene != nullptr, "Ogre2 scene is null");

  Ogre::SceneManager *sceneManager = ogreScene->OgreSceneManager();
  Ogre::ObjectMemoryManager *memoryManager =
      &sceneManager->_getEntityMemoryManager(Ogre::SCENE_DYNAMIC);

  this->dataPtr->ogreObj.reset(new Ogre2MovableText(
      Ogre::Id::generateNewId<Ogre::MovableObject>(),
      memoryManager, sceneManager, Ogre::v1::OverlayManager::getSingleton().mDefaultRenderQueueId));

  // set default properties inherited from base
  this->SetFontName(this->fontName);
  this->SetCharHeight(this->charHeight);
  this->SetSpaceWidth(this->spaceWidth);
  this->SetBaseline(this->baseline);
  this->SetShowOnTop(this->onTop);
  this->SetColor(this->color);
  this->SetTextAlignment(this->horizontalAlign, this->verticalAlign);
}

//////////////////////////////////////////////////
void Ogre2Text::PreRender()
{
  this->dataPtr->ogreObj->Update();
}

//////////////////////////////////////////////////
void Ogre2Text::Destroy()
{
  this->dataPtr->ogreObj.reset();
}

//////////////////////////////////////////////////
Ogre::MovableObject *Ogre2Text::OgreObject() const
{
  return this->dataPtr->ogreObj.get();
}

//////////////////////////////////////////////////
void Ogre2Text::SetMaterial(MaterialPtr _material, bool _unique)
{
  _material = (_unique) ? _material->Clone() : _material;

  Ogre2MaterialPtr derived =
      std::dynamic_pointer_cast<Ogre2Material>(_material);

  if (!derived)
  {
    gzerr << "Cannot assign ogreMaterial created by another render-engine"
        << std::endl;

    return;
  }

  this->SetMaterialImpl(derived);
}

//////////////////////////////////////////////////
void Ogre2Text::SetMaterialImpl(Ogre2MaterialPtr _material)
{
  // only colors are support for now
  this->SetColor(_material->Diffuse());
  this->dataPtr->material = _material;
}

//////////////////////////////////////////////////
MaterialPtr Ogre2Text::Material() const
{
  return this->dataPtr->material;
}

//////////////////////////////////////////////////
void Ogre2Text::SetFontName(const std::string &_font)
{
  BaseText::SetFontName(_font);
  this->dataPtr->ogreObj->SetFontName(_font);
}

//////////////////////////////////////////////////
void Ogre2Text::SetTextString(const std::string &_text)
{
  BaseText::SetTextString(_text);
  this->dataPtr->ogreObj->SetTextString(_text);
}

//////////////////////////////////////////////////
void Ogre2Text::SetColor(const math::Color &_color)
{
  BaseText::SetColor(_color);
  this->dataPtr->ogreObj->SetColor(_color);
}

//////////////////////////////////////////////////
void Ogre2Text::SetCharHeight(const float _height)
{
  BaseText::SetCharHeight(_height);
  this->dataPtr->ogreObj->SetCharHeight(_height);
}

//////////////////////////////////////////////////
void Ogre2Text::SetSpaceWidth(const float _width)
{
  BaseText::SetSpaceWidth(_width);
  this->dataPtr->ogreObj->SetSpaceWidth(_width);
}

//////////////////////////////////////////////////
void Ogre2Text::SetTextAlignment(const TextHorizontalAlign &_horizAlign,
                                const TextVerticalAlign &_vertAlign)
{
  BaseText::SetTextAlignment(_horizAlign, _vertAlign);
  this->dataPtr->ogreObj->SetTextAlignment(_horizAlign, _vertAlign);
}

//////////////////////////////////////////////////
void Ogre2Text::SetBaseline(const float _baseline)
{
  BaseText::SetBaseline(_baseline);
  this->dataPtr->ogreObj->SetBaseline(_baseline);
}

//////////////////////////////////////////////////
void Ogre2Text::SetShowOnTop(const bool _onTop)
{
  BaseText::SetShowOnTop(_onTop);
  this->dataPtr->ogreObj->SetShowOnTop(_onTop);
}

//////////////////////////////////////////////////
math::AxisAlignedBox Ogre2Text::AABB() const
{
  return this->dataPtr->ogreObj->AABB();
}

#endif  // GZ_RENDERING_HAVE_OGRE2_OVERLAY
