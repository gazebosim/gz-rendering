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

#ifdef _WIN32
  // Ensure that windows types like LONGLONG are defined
  #include <windows.h>
#endif

#include <ignition/common/Util.hh>
#include <ignition/common/Console.hh>

#include "ignition/rendering/ogre/OgreMaterial.hh"
#include "ignition/rendering/ogre/OgreText.hh"

#define POS_TEX_BINDING    0
#define COLOUR_BINDING     1

using namespace ignition;
using namespace rendering;

/// \brief Private data for the OgreText class.
class ignition::rendering::OgreTextPrivate
{
  /// \brief Bounding radius
  public: float radius = 0;

  /// \brief Viewport aspect coefficient
  public: float viewportAspectCoef = 0.75;

  /// \brief Ogre render operation
  public: Ogre::RenderOperation renderOp;

  /// \brief Axis aligned box
  public: Ogre::AxisAlignedBox *aabb = nullptr;

  /// \brief Pointer to camera which the text is facing - never set.
  public: Ogre::Camera *camera = nullptr;

  /// \brief Pointer to font
  public: Ogre::Font *font = nullptr;

  /// \brief Text ogreMaterial
  public: Ogre::MaterialPtr ogreMaterial;

  /// \brief Grid materal
  public: OgreMaterialPtr material;

  /// \brief Keep an empty list of lights.
  public: Ogre::LightList lightList;

  /// \brief Flag to indicate text color has changed
  public: bool colorDirty = true;

  /// \brief Flag to indicate text font has changed
  public: bool fontDirty = true;
};

//////////////////////////////////////////////////
OgreText::OgreText()
    : dataPtr(new OgreTextPrivate)
{
  this->dataPtr->renderOp.vertexData = nullptr;

  this->dataPtr->aabb = new Ogre::AxisAlignedBox;
}

//////////////////////////////////////////////////
OgreText::~OgreText()
{
  delete this->dataPtr->renderOp.vertexData;
  delete this->dataPtr->aabb;
}

/*//////////////////////////////////////////////////
void OgreText::Load(const std::string &_name,
                       const std::string &_text,
                       const std::string &_,
                       const float _charHeight,
                       const ignition::math::Color &_color)
{
  {
    std::lock_guard<std::recursive_mutex> lock(this->dataPtr->mutex);

    this->text = _text;
    this->color = _color;
    this->fontName = _fontName;
    this->charHeight = _charHeight;
    this->mName = _name;

    if (this->mName == "")
    {
      throw Ogre::Exception(Ogre::Exception::ERR_INVALIDPARAMS,
          "Trying to create OgreText without name",
          "OgreText::OgreText");
    }

    if (this->text == "")
    {
      throw Ogre::Exception(Ogre::Exception::ERR_INVALIDPARAMS,
          "Trying to create OgreText without text",
          "OgreText::OgreText");
    }

    this->dataPtr->dirty = true;
  }

  this->SetFontName(this->dataPtr->fontName);

  this->SetupGeometry();
}*/
//////////////////////////////////////////////////
void OgreText::Init()
{
  // TODO load ogre font media ogreMaterials
  this->SetFontNameImpl(this->fontName);
  this->SetupGeometry();
}

//////////////////////////////////////////////////
void OgreText::PreRender()
{
  if (this->dataPtr->fontDirty)
    this->UpdateFont();
  if (this->textDirty)
    this->SetupGeometry();
  if (this->dataPtr->colorDirty)
    this->UpdateColors();

}

//////////////////////////////////////////////////
void OgreText::SetFontName(const std::string &_font)
{
  if (this->fontName != _font)
  {
    this->fontName = _font;
    this->dataPtr->fontDirty = true;
  }
}

//////////////////////////////////////////////////
void OgreText::UpdateFont()
{
  this->SetFontNameImpl(this->fontName);
}

//////////////////////////////////////////////////
void OgreText::SetFontNameImpl(const std::string &_newFontName)
{
  if ((Ogre::MaterialManager::getSingletonPtr()->resourceExists(
          this->mName + "Material")))
  {
    Ogre::MaterialManager::getSingleton().remove(this->mName + "Material");
  }

  if (this->fontName != _newFontName ||
      this->dataPtr->ogreMaterial.isNull() || !this->dataPtr->font)
  {
    auto font = (Ogre::Font*)Ogre::FontManager::getSingleton()
        .getByName(_newFontName).getPointer();

    if (!font)
    {
      throw Ogre::Exception(Ogre::Exception::ERR_ITEM_NOT_FOUND,
                            "Could not find font " + _newFontName,
                            "OgreText::setFontName");
    }
    this->dataPtr->font = font;
    this->fontName = _newFontName;

    this->dataPtr->font->load();

    if (!this->dataPtr->ogreMaterial.isNull())
    {
      Ogre::MaterialManager::getSingletonPtr()->remove(
          this->dataPtr->ogreMaterial->getName());
      this->dataPtr->ogreMaterial.setNull();
    }

    this->dataPtr->ogreMaterial = this->dataPtr->font->getMaterial()->clone(
        this->mName + "Material");

    if (!this->dataPtr->ogreMaterial->isLoaded())
      this->dataPtr->ogreMaterial->load();

    this->dataPtr->ogreMaterial->setDepthCheckEnabled(!this->onTop);
    this->dataPtr->ogreMaterial->setDepthBias(!this->onTop, 0);
    this->dataPtr->ogreMaterial->setDepthWriteEnabled(this->onTop);
    this->dataPtr->ogreMaterial->setLightingEnabled(false);

    this->textDirty = true;
  }
}

//////////////////////////////////////////////////
void OgreText::SetShowOnTop(const bool _onTop)
{
  if (this->onTop != _onTop && !this->dataPtr->ogreMaterial.isNull())
  {
    this->onTop = _onTop;

    this->dataPtr->ogreMaterial->setDepthBias(!this->onTop, 0);
    this->dataPtr->ogreMaterial->setDepthCheckEnabled(!this->onTop);
    this->dataPtr->ogreMaterial->setDepthWriteEnabled(this->onTop);
  }
}


//////////////////////////////////////////////////
ignition::math::Box OgreText::AABB()
{
  return ignition::math::Box(
      ignition::math::Vector3d(this->dataPtr->aabb->getMinimum().x,
                    this->dataPtr->aabb->getMinimum().y,
                    this->dataPtr->aabb->getMinimum().z),
      ignition::math::Vector3d(this->dataPtr->aabb->getMaximum().x,
                    this->dataPtr->aabb->getMaximum().y,
                    this->dataPtr->aabb->getMaximum().z));
}

//////////////////////////////////////////////////
void OgreText::SetupGeometry()
{
  IGN_ASSERT(this->dataPtr->font, "font class member is null");
  IGN_ASSERT(!this->dataPtr->ogreMaterial.isNull(), "ogreMaterial class member is null");

  Ogre::VertexDeclaration *decl = nullptr;
  Ogre::VertexBufferBinding *bind = nullptr;
  Ogre::HardwareVertexBufferSharedPtr ptbuf;
  Ogre::HardwareVertexBufferSharedPtr cbuf;
  float *pVert = nullptr;
  float largestWidth = 0;
  float left = 0;
  float top = 0;
  size_t offset = 0;
  float maxSquaredRadius = 0.0f;
  bool first = true;
  std::string::iterator i;
  bool newLine = true;
  float len = 0.0f;

  // for calculation of AABB
  Ogre::Vector3 min(0, 0, 0);
  Ogre::Vector3 max(0, 0, 0);
  Ogre::Vector3 currPos(0, 0, 0);

  auto vertexCount = static_cast<unsigned int>(this->text.size() * 6);

  if (this->dataPtr->renderOp.vertexData)
  {
    // Removed this test as it causes problems when replacing a caption
    // of the same size: replacing "Hello" with "hello"
    // as well as when changing the text alignment
    // if (this->dataPtr->renderOp.vertexData->vertexCount != vertexCount)
    {
      delete this->dataPtr->renderOp.vertexData;
      this->dataPtr->renderOp.vertexData = nullptr;
      this->dataPtr->colorDirty = true;
    }
  }

  if (!this->dataPtr->renderOp.vertexData)
    this->dataPtr->renderOp.vertexData = new Ogre::VertexData();

  this->dataPtr->renderOp.indexData = 0;
  this->dataPtr->renderOp.vertexData->vertexStart = 0;
  this->dataPtr->renderOp.vertexData->vertexCount = vertexCount;
  this->dataPtr->renderOp.operationType =
      Ogre::RenderOperation::OT_TRIANGLE_LIST;
  this->dataPtr->renderOp.useIndexes = false;

  decl = this->dataPtr->renderOp.vertexData->vertexDeclaration;
  bind = this->dataPtr->renderOp.vertexData->vertexBufferBinding;

  // create/bind positions/tex.ccord. buffer
  if (!decl->findElementBySemantic(Ogre::VES_POSITION))
    decl->addElement(POS_TEX_BINDING, offset, Ogre::VET_FLOAT3,
                     Ogre::VES_POSITION);

  offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);

  if (!decl->findElementBySemantic(Ogre::VES_TEXTURE_COORDINATES))
    decl->addElement(POS_TEX_BINDING, offset, Ogre::VET_FLOAT2,
                     Ogre::VES_TEXTURE_COORDINATES, 0);

  ptbuf = Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(
            decl->getVertexSize(POS_TEX_BINDING),
            this->dataPtr->renderOp.vertexData->vertexCount,
            Ogre::HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY);

  bind->setBinding(POS_TEX_BINDING, ptbuf);

  // Colours - store these in a separate buffer because they change less often
  if (!decl->findElementBySemantic(Ogre::VES_DIFFUSE))
    decl->addElement(COLOUR_BINDING, 0, Ogre::VET_COLOUR, Ogre::VES_DIFFUSE);

  cbuf = Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(
           decl->getVertexSize(COLOUR_BINDING),
           this->dataPtr->renderOp.vertexData->vertexCount,
           Ogre::HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY);

  bind->setBinding(COLOUR_BINDING, cbuf);

  pVert = static_cast<float*>(ptbuf->lock(Ogre::HardwareBuffer::HBL_DISCARD));

  // Derive space width from a capital A
  if (ignition::math::equal(this->spaceWidth, 0.0f))
  {
    this->spaceWidth = this->dataPtr->font->getGlyphAspectRatio('A') *
        this->charHeight * 2.0;
  }

  if (this->verticalAlign == Text::V_TOP)
  {
    // Raise the first line of the caption
    top += this->charHeight;

    for (i = this->text.begin(); i != this->text.end(); ++i)
    {
      if (*i == '\n')
        top += this->charHeight * 2.0;
    }
  }

  for (i = this->text.begin(); i != this->text.end(); ++i)
  {
    if (newLine)
    {
      len = 0.0;
      for (std::string::iterator j = i; j != this->text.end(); ++j)
      {
        Ogre::Font::CodePoint character = *j;
        if (character == 0x000D  // CR
            || character == 0x0085)  // NEL
        {
          break;
        }
        else if (character == 0x0020)  // space
        {
          len += this->spaceWidth;
        }
        else
        {
          len += this->dataPtr->font->getGlyphAspectRatio(character) *
                 this->charHeight * 2.0 *
                 this->dataPtr->viewportAspectCoef;
        }
      }

      newLine = false;
    }

    Ogre::Font::CodePoint character = (*i);

    if (character == 0x000D  // CR
        || character == 0x0085)  // NEL
    {
      top -= this->charHeight * 2.0;
      newLine = true;

      // Also reduce tri count
      this->dataPtr->renderOp.vertexData->vertexCount -= 6;
      continue;
    }
    else if (character == 0x0020)  // space
    {
      // Just leave a gap, no tris
      left += this->spaceWidth;

      // Also reduce tri count
      this->dataPtr->renderOp.vertexData->vertexCount -= 6;
      continue;
    }

    float horiz_height = this->dataPtr->font->getGlyphAspectRatio(character) *
                         this->dataPtr->viewportAspectCoef;

    auto &uvRect = this->dataPtr->font->getGlyphTexCoords(character);

    // each vert is (x, y, z, u, v)
    //------------------------------------------------------------------------
    // First tri
    //
    // Upper left
    if (this->horizontalAlign == Text::H_LEFT)
      *pVert++ = left;
    else
      *pVert++ = left - (len/2.0);

    *pVert++ = top;
    *pVert++ = 0;
    *pVert++ = uvRect.left;
    *pVert++ = uvRect.top;

    // Deal with bounds
    if (this->horizontalAlign == Text::H_LEFT)
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
    if (this->horizontalAlign == Text::H_LEFT)
      *pVert++ = left;
    else
      *pVert++ = left - (len / 2.0);

    *pVert++ = top;
    *pVert++ = 0;
    *pVert++ = uvRect.left;
    *pVert++ = uvRect.bottom;

    // Deal with bounds
    if (this->horizontalAlign == Text::H_LEFT)
      currPos = Ogre::Vector3(left, top, 0);
    else
      currPos = Ogre::Vector3(left - (len/2), top, 0);

    min.makeFloor(currPos);
    max.makeCeil(currPos);
    maxSquaredRadius = std::max(maxSquaredRadius, currPos.squaredLength());


    top += this->charHeight * 2.0;
    left += horiz_height * this->charHeight * 2.0;

    // Top right
    if (this->horizontalAlign == Text::H_LEFT)
      *pVert++ = left;
    else
      *pVert++ = left - (len/2.0);

    *pVert++ = top;
    *pVert++ = 0;
    *pVert++ = uvRect.right;
    *pVert++ = uvRect.top;

    // Deal with bounds
    if (this->horizontalAlign == Text::H_LEFT)
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
    if (this->horizontalAlign == Text::H_LEFT)
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
    left -= horiz_height  * this->charHeight * 2.0;

    // Bottom left (again)
    if (this->horizontalAlign == Text::H_LEFT)
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


    left += horiz_height  * this->charHeight * 2.0;

    // Bottom right
    if (this->horizontalAlign == Text::H_LEFT)
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
  this->dataPtr->aabb->setMinimum(min);
  this->dataPtr->aabb->setMaximum(max);
  this->dataPtr->radius = Ogre::Math::Sqrt(maxSquaredRadius);

  if (this->dataPtr->colorDirty)
    this->UpdateColors();

  this->textDirty = false;
}

//////////////////////////////////////////////////
void OgreText::SetColor(const ignition::math::Color &_color)
{
  if (this->color != _color)
  {
    this->color = _color;
    this->dataPtr->colorDirty = true;
  }
}

//////////////////////////////////////////////////
void OgreText::UpdateColors()
{
  Ogre::RGBA clr;
  Ogre::HardwareVertexBufferSharedPtr vbuf;
  Ogre::RGBA *pDest = nullptr;
  unsigned int i;

  IGN_ASSERT(this->dataPtr->font, "font class member is null");
  IGN_ASSERT(!this->dataPtr->ogreMaterial.isNull(), "ogreMaterial class member is null");

  // Convert to system-specific
  Ogre::ColourValue cv(this->color.R(), this->color.G(),
                       this->color.B(), this->color.A());
  Ogre::Root::getSingleton().convertColourValue(cv, &clr);

  vbuf = this->dataPtr->renderOp.vertexData->vertexBufferBinding->getBuffer(
         COLOUR_BINDING);

  pDest = static_cast<Ogre::RGBA*>(
      vbuf->lock(Ogre::HardwareBuffer::HBL_DISCARD));

  for (i = 0; i < this->dataPtr->renderOp.vertexData->vertexCount; ++i)
  {
    *pDest++ = clr;
  }

  vbuf->unlock();
  this->dataPtr->colorDirty = false;
}

//////////////////////////////////////////////////
Ogre::MovableObject *OgreText::OgreObject() const
{
  return this;
}

//////////////////////////////////////////////////
void OgreText::SetMaterial(MaterialPtr _material, bool _unique)
{
  _material = (_unique) ? _material->Clone() : _material;

  OgreMaterialPtr derived =
      std::dynamic_pointer_cast<OgreMaterial>(_material);

  if (!derived)
  {
    ignerr << "Cannot assign ogreMaterial created by another render-engine"
        << std::endl;

    return;
  }

  this->SetMaterialImpl(derived);
}

//////////////////////////////////////////////////
void OgreText::SetMaterialImpl(OgreMaterialPtr _material)
{
  // TODO
/*  std::string ogreMaterialName = _ogreMaterial->Name();
  Ogre::MaterialPtr ogreMaterial = _ogreMaterial->Material();
  this->dataPtr->manualObject->setMaterialName(0, ogreMaterialName);
  this->dataPtr->ogreMaterial = _ogreMaterial;

  this->dataPtr->ogreMaterial->SetReceiveShadows(false);
  this->dataPtr->ogreMaterial->SetLightingEnabled(false);
*/
}

//////////////////////////////////////////////////
MaterialPtr OgreText::Material() const
{
  return this->dataPtr->material;
}

//////////////////////////////////////////////////
const Ogre::AxisAlignedBox &OgreText::getBoundingBox(void) const
{
  return *this->dataPtr->aabb;
}

//////////////////////////////////////////////////
const Ogre::String &OgreText::getMovableType() const
{
  static Ogre::String movType = "OgreText";
  return movType;
}

//////////////////////////////////////////////////
void OgreText::getWorldTransforms(Ogre::Matrix4 *_xform) const
{
  if (this->isVisible() && this->dataPtr->camera)
  {
    Ogre::Matrix3 rot3x3, scale3x3 = Ogre::Matrix3::IDENTITY;

    // store rotation in a matrix
    this->dataPtr->camera->getDerivedOrientation().ToRotationMatrix(rot3x3);
    // mParentNode->_getDerivedOrientation().ToRotationMatrix(rot3x3);

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
float OgreText::getBoundingRadius() const
{
  return this->dataPtr->radius;
}

//////////////////////////////////////////////////
float OgreText::getSquaredViewDepth(const Ogre::Camera * /*cam_*/) const
{
  return 0;
}

//////////////////////////////////////////////////
void OgreText::getRenderOperation(Ogre::RenderOperation & op)
{
  if (this->isVisible())
  {
    if (this->textDirty)
      this->SetupGeometry();
    if (this->dataPtr->colorDirty)
      this->UpdateColors();
    op = this->dataPtr->renderOp;
  }
}

//////////////////////////////////////////////////
const Ogre::MaterialPtr &OgreText::getMaterial(void) const
{
  IGN_ASSERT(!this->dataPtr->ogreMaterial.isNull(),
      "ogreMaterial class member is null");
  return this->dataPtr->ogreMaterial;
}

//////////////////////////////////////////////////
const Ogre::LightList &OgreText::getLights(void) const
{
  return this->dataPtr->lightList;
}

//////////////////////////////////////////////////
void OgreText::_notifyCurrentCamera(Ogre::Camera *cam)
{
  this->dataPtr->camera = cam;
}

//////////////////////////////////////////////////
void OgreText::_updateRenderQueue(Ogre::RenderQueue* queue)
{
  if (this->isVisible())
  {
    if (this->textDirty)
      this->SetupGeometry();

    if (this->dataPtr->colorDirty)
      this->UpdateColors();

    queue->addRenderable(this, mRenderQueueID,
                         OGRE_RENDERABLE_DEFAULT_PRIORITY);
  }
}

//////////////////////////////////////////////////
void OgreText::visitRenderables(Ogre::Renderable::Visitor* /*visitor*/,
                                 bool /*debug*/)
{
  return;
}
