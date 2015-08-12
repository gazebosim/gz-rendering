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
#include "ignition/rendering/ogre/OgreRenderTarget.hh"

#include "gazebo/common/Console.hh"
#include "ignition/rendering/ogre/OgreConversions.hh"
#include "ignition/rendering/ogre/OgreRTShaderSystem.hh"
#include "ignition/rendering/ogre/OgreIncludes.hh"

using namespace ignition;
using namespace rendering;

//////////////////////////////////////////////////
/// OgreRenderTarget
OgreRenderTarget::OgreRenderTarget()
{
}

//////////////////////////////////////////////////
OgreRenderTarget::~OgreRenderTarget()
{
  // TODO: clean up check null

  OgreRTShaderSystem::Instance()->DetachViewport(this->ogreViewport,
      this->scene);

  // this->Destroy();
}

//////////////////////////////////////////////////
unsigned int OgreRenderTarget::GetWidth() const
{
  return this->ogreViewport->getActualWidth();
}

//////////////////////////////////////////////////
unsigned int OgreRenderTarget::GetHeight() const
{
  return this->ogreViewport->getActualHeight();
}

//////////////////////////////////////////////////
void OgreRenderTarget::GetImage(Image &_image) const
{
  // TODO: handle Bayer conversions
  // TODO: handle ogre version differences

  unsigned int width = this->ogreViewport->getActualWidth();
  unsigned int height = this->ogreViewport->getActualHeight();

  if (_image.GetWidth() != width || _image.GetHeight() != height)
  {
    gzerr << "Invalid image dimensions" << std::endl;
    return;
  }

  Ogre::PixelFormat format = OgreConversions::Convert(_image.GetFormat());
  void* data = _image.GetData();

  Ogre::PixelBox ogrePixelBox(width, height, 1, format, data);
  this->GetOgreRenderTarget()->copyContentsToMemory(ogrePixelBox);
}

//////////////////////////////////////////////////
void OgreRenderTarget::Update()
{
  this->GetOgreRenderTarget()->update();
}

//////////////////////////////////////////////////
void OgreRenderTarget::Initialize()
{
  Ogre::RenderTarget *ogreRenderTarget = this->GetOgreRenderTarget();
  this->ogreViewport = ogreRenderTarget->addViewport(this->ogreCamera);
  this->ogreViewport->setBackgroundColour(this->ogreBackgroundColor);
  this->ogreViewport->setClearEveryFrame(true);
  this->ogreViewport->setShadowsEnabled(true);
  this->ogreViewport->setOverlaysEnabled(false);

  OgreRTShaderSystem::Instance()->AttachViewport(this->ogreViewport,
      this->scene);
}

//////////////////////////////////////////////////
/// OgreRenderTexture
OgreRenderTexture::OgreRenderTexture() :
  ogreCamera2(NULL),
  colorDirty(true)
{
}

//////////////////////////////////////////////////
OgreRenderTexture::~OgreRenderTexture()
{
}

//////////////////////////////////////////////////
Ogre::Camera *OgreRenderTexture::GetCamera() const
{
  return this->ogreCamera2;
}

//////////////////////////////////////////////////
void OgreRenderTexture::SetCamera(Ogre::Camera *_camera)
{
  this->ogreCamera2 = _camera;
  this->textureDirty = true;
}

//////////////////////////////////////////////////
unsigned int OgreRenderTexture::GetAntiAliasing() const
{
  return this->antiAliasing;
}

//////////////////////////////////////////////////
void OgreRenderTexture::SetAntiAliasing(unsigned int _aa)
{
  this->antiAliasing = _aa;
  this->textureDirty = true;
}

//////////////////////////////////////////////////
gazebo::common::Color OgreRenderTexture::GetBackgroundColor() const
{
  return this->backgroundColor;
}

//////////////////////////////////////////////////
void OgreRenderTexture::SetBackgroundColor(gazebo::common::Color _color)
{
  this->backgroundColor = _color;
  this->colorDirty = true;
}

//////////////////////////////////////////////////
void OgreRenderTexture::PreRender()
{
  BaseRenderTexture::PreRender();
  this->UpdateBackgroundColor();
}

//////////////////////////////////////////////////
void OgreRenderTexture::Destroy()
{
  std::string ogreName = this->ogreTexture->getName();
  Ogre::TextureManager::getSingleton().remove(ogreName);
}

//////////////////////////////////////////////////
Ogre::RenderTarget *OgreRenderTexture::GetOgreRenderTarget() const
{
  return this->ogreTexture->getBuffer()->getRenderTarget();
}

//////////////////////////////////////////////////
void OgreRenderTexture::RebuildImpl()
{
}

//////////////////////////////////////////////////
void OgreRenderTexture::UpdateBackgroundColor()
{
  if (this->colorDirty)
  {
    this->UpdateBackgroundColorImpl();
    this->colorDirty = false;
  }
}

//////////////////////////////////////////////////
void OgreRenderTexture::UpdateBackgroundColorImpl()
{
  // Ogre::ColourValue ogreColor = OgreConversions::Convert(this->backgroundColor);
  // this->ogreViewport->setBackgroundColour(ogreColor);
}

//////////////////////////////////////////////////  
/// OgreRenderTextureBuilder
OgreRenderTextureBuilder::OgreRenderTextureBuilder(OgreScenePtr _scene) :
  scene(_scene),
  ogreCamera(NULL),
  name(""),
  backgroundColor(gazebo::common::Color::Black)
{
}

//////////////////////////////////////////////////
OgreRenderTextureBuilder::~OgreRenderTextureBuilder()
{
}

//////////////////////////////////////////////////
Ogre::Camera *OgreRenderTextureBuilder::GetCamera() const
{
  return this->ogreCamera;
}

//////////////////////////////////////////////////
void OgreRenderTextureBuilder::SetCamera(Ogre::Camera *_camera)
{
  this->ogreCamera = _camera;
}

//////////////////////////////////////////////////
std::string OgreRenderTextureBuilder::GetName() const
{
  return this->name;
}

//////////////////////////////////////////////////
void OgreRenderTextureBuilder::SetName(const std::string &_name)
{
  this->name = _name;
}

//////////////////////////////////////////////////
void OgreRenderTextureBuilder::SetFormat(PixelFormat _format)
{
  _format = PixelUtil::Sanitize(_format);
  this->format = _format;
}

//////////////////////////////////////////////////
gazebo::common::Color OgreRenderTextureBuilder::GetBackgroundColor() const
{
  return this->backgroundColor;
}

//////////////////////////////////////////////////
void OgreRenderTextureBuilder::SetBackgroundColor(gazebo::common::Color _color)
{
  this->backgroundColor = _color;
}

//////////////////////////////////////////////////
RenderTexturePtr OgreRenderTextureBuilder::Build() const
{
  if (!this->ogreCamera)
  {
    gzerr << "Ogre::Camera cannot be null" << std::endl;
    return NULL;
  }

  return this->BuildSafe();
}

//////////////////////////////////////////////////
OgreRenderTexturePtr OgreRenderTextureBuilder::BuildSafe() const
{
  // TODO: improve name scope
  std::string texName = (!this->name.empty()) ?
      this->name : this->ogreCamera->getName() + "::RenderTexture";

  Ogre::ColourValue ogreColor;
  ogreColor = OgreConversions::Convert(this->backgroundColor);

  Ogre::PixelFormat ogreFormat = OgreConversions::Convert(this->format);
  Ogre::TextureManager &manager = Ogre::TextureManager::getSingleton();

  Ogre::TexturePtr ogreTexture = manager.createManual(texName, "General",
      Ogre::TEX_TYPE_2D, this->width, this->height, 0, ogreFormat,
      Ogre::TU_RENDERTARGET, 0, false, this->aa);

  OgreRenderTexturePtr texture(new OgreRenderTexture());
  texture->scene = this->scene;
  texture->ogreTexture = ogreTexture.getPointer();
  texture->ogreCamera = this->ogreCamera;
  texture->ogreFormat = ogreFormat;
  texture->ogreBackgroundColor = ogreColor;
  texture->Initialize();

  return texture;
}
