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

#include "ignition/rendering/Material.hh"

#include "ignition/rendering/ogre2/Ogre2Includes.hh"
#include "ignition/rendering/ogre2/Ogre2RenderEngine.hh"
#include "ignition/rendering/ogre2/Ogre2Conversions.hh"
#include "ignition/rendering/ogre2/Ogre2Material.hh"
#include "ignition/rendering/ogre2/Ogre2RenderTarget.hh"
#include "ignition/rendering/ogre2/Ogre2Scene.hh"

using namespace ignition;
using namespace rendering;

//////////////////////////////////////////////////
// Ogre2RenderTarget
//////////////////////////////////////////////////
Ogre2RenderTarget::Ogre2RenderTarget()
{
  this->ogreBackgroundColor = Ogre::ColourValue::Black;
}

//////////////////////////////////////////////////
Ogre2RenderTarget::~Ogre2RenderTarget()
{
  // TODO(anyone)
}

//////////////////////////////////////////////////
void Ogre2RenderTarget::BuildCompositor()
{
  auto engine = Ogre2RenderEngine::Instance();
  auto ogreRoot = engine->OgreRoot();
  Ogre::CompositorManager2 *ogreCompMgr = ogreRoot->getCompositorManager2();

  const Ogre::String workspaceName = "PbsMaterialsWorkspace";
  this->ogreCompositorWorkspace =
      ogreCompMgr->addWorkspace(this->scene->OgreSceneManager(),
      this->RenderTarget(), this->ogreCamera, workspaceName, false);
}

//////////////////////////////////////////////////
void Ogre2RenderTarget::DestroyCompositor()
{
  if (!this->ogreCompositorWorkspace)
    return;

  auto engine = Ogre2RenderEngine::Instance();
  auto ogreRoot = engine->OgreRoot();
  Ogre::CompositorManager2 *ogreCompMgr = ogreRoot->getCompositorManager2();
  ogreCompMgr->removeWorkspace(this->ogreCompositorWorkspace);
  this->ogreCompositorWorkspace = nullptr;
}

//////////////////////////////////////////////////
void Ogre2RenderTarget::RebuildCompositor()
{
  this->DestroyCompositor();
  this->BuildCompositor();
}

//////////////////////////////////////////////////
void Ogre2RenderTarget::Copy(Image &_image) const
{
  // TODO(anyone) handle Bayer conversions
  // TODO(anyone) handle ogre version differences

  if (_image.Width() != this->width || _image.Height() != this->height)
  {
    ignerr << "Invalid image dimensions" << std::endl;
    return;
  }

  void *data = _image.Data();
  Ogre::PixelFormat imageFormat = Ogre2Conversions::Convert(_image.Format());
  Ogre::PixelBox ogrePixelBox(this->width, this->height, 1, imageFormat, data);
  this->RenderTarget()->copyContentsToMemory(ogrePixelBox);
}

//////////////////////////////////////////////////
Ogre::Camera *Ogre2RenderTarget::Camera() const
{
  return this->ogreCamera;
}

//////////////////////////////////////////////////
void Ogre2RenderTarget::SetCamera(Ogre::Camera *_camera)
{
  this->ogreCamera = _camera;
  this->targetDirty = true;
}

//////////////////////////////////////////////////
math::Color Ogre2RenderTarget::BackgroundColor() const
{
  return Ogre2Conversions::Convert(this->ogreBackgroundColor);
}

//////////////////////////////////////////////////
void Ogre2RenderTarget::SetBackgroundColor(math::Color _color)
{
  this->ogreBackgroundColor = Ogre2Conversions::Convert(_color);
  this->colorDirty = true;
}

//////////////////////////////////////////////////
unsigned int Ogre2RenderTarget::AntiAliasing() const
{
  return this->antiAliasing;
}

//////////////////////////////////////////////////
void Ogre2RenderTarget::SetAntiAliasing(unsigned int _aa)
{
  this->antiAliasing = _aa;
  this->targetDirty = true;
}

//////////////////////////////////////////////////
void Ogre2RenderTarget::PreRender()
{
  BaseRenderTarget::PreRender();
  this->UpdateBackgroundColor();

  if (this->material)
  {
    this->material->PreRender();
  }
}

//////////////////////////////////////////////////
void Ogre2RenderTarget::Render()
{
  // TODO(anyone)
  // There is current not an easy solution to manually updating
  // render textures:
  // https://forums.ogre3d.org/viewtopic.php?t=84687
  this->ogreCompositorWorkspace->setEnabled(true);
  auto engine = Ogre2RenderEngine::Instance();
  engine->OgreRoot()->renderOneFrame();
  this->ogreCompositorWorkspace->setEnabled(false);

  // The code below for manual updating render textures was suggested in ogre
  // forum but it does not seem to work
  // this->scene->OgreSceneManager()->updateSceneGraph();
  // this->ogreCompositorWorkspace->_validateFinalTarget();
  // engine->OgreRoot()->getRenderSystem()->_beginFrameOnce();
  // this->ogreCompositorWorkspace->_beginUpdate(false);
  // this->ogreCompositorWorkspace->_update();
  // this->ogreCompositorWorkspace->_endUpdate(false);

  // this->scene->OgreSceneManager()->_frameEnded();
  // for (size_t i=0; i < Ogre::HLMS_MAX; ++i)
  // {
  //   Ogre::Hlms *hlms = engine->OgreRoot()->getHlmsManager()->getHlms(
  //       static_cast<Ogre::HlmsTypes>(i));
  //   if(hlms)
  //     hlms->frameEnded();
  // }
  // engine->OgreRoot()->getRenderSystem()->_update();
}

//////////////////////////////////////////////////
void Ogre2RenderTarget::UpdateBackgroundColor()
{
  if (this->colorDirty)
  {
    this->colorDirty = false;
  }
}

//////////////////////////////////////////////////
void Ogre2RenderTarget::RebuildImpl()
{
  this->RebuildTarget();
  this->RebuildMaterial();
  this->RebuildCompositor();
}

//////////////////////////////////////////////////
void Ogre2RenderTarget::SetMaterial(MaterialPtr _material)
{
  this->material = _material;

  // Have to rebuild the target so there is something to apply the applicator to
  this->targetDirty = true;
}

//////////////////////////////////////////////////
void Ogre2RenderTarget::RebuildMaterial()
{
  if (this->material)
  {
    Ogre2Material *ogreMaterial = dynamic_cast<Ogre2Material *>(
        this->material.get());
    Ogre::MaterialPtr matPtr = ogreMaterial->Material();

    Ogre::SceneManager *sceneMgr = this->scene->OgreSceneManager();
    Ogre::RenderTarget *target = this->RenderTarget();
    this->materialApplicator.reset(new Ogre2RenderTargetMaterial(
        sceneMgr, target, matPtr.get()));
  }
}

//////////////////////////////////////////////////
// Ogre2RenderTexture
//////////////////////////////////////////////////
Ogre2RenderTexture::Ogre2RenderTexture()
{
}

//////////////////////////////////////////////////
Ogre2RenderTexture::~Ogre2RenderTexture()
{
}

//////////////////////////////////////////////////
void Ogre2RenderTexture::Destroy()
{
  std::string ogreName = this->ogreTexture->getName();
  Ogre::TextureManager::getSingleton().remove(ogreName);
}

//////////////////////////////////////////////////
Ogre::RenderTarget *Ogre2RenderTexture::RenderTarget() const
{
  return this->ogreTexture->getBuffer()->getRenderTarget();
}


//////////////////////////////////////////////////
void Ogre2RenderTexture::RebuildTarget()
{
  this->DestroyTarget();
  this->BuildTarget();
}

//////////////////////////////////////////////////
void Ogre2RenderTexture::DestroyTarget()
{
  // TODO(anyone)
}

//////////////////////////////////////////////////
void Ogre2RenderTexture::BuildTarget()
{
  Ogre::TextureManager &manager = Ogre::TextureManager::getSingleton();
  Ogre::PixelFormat ogreFormat = Ogre2Conversions::Convert(this->format);

  // Ogre 2 PBS expects gamma correction to be enabled
  this->ogreTexture = (manager.createManual(this->name, "General",
      Ogre::TEX_TYPE_2D, this->width, this->height, 0, ogreFormat,
      Ogre::TU_RENDERTARGET, 0, true, this->antiAliasing)).getPointer();
}

//////////////////////////////////////////////////
// Ogre2RenderWindow
//////////////////////////////////////////////////
Ogre2RenderWindow::Ogre2RenderWindow()
{
}

//////////////////////////////////////////////////
Ogre2RenderWindow::~Ogre2RenderWindow()
{
}

//////////////////////////////////////////////////
Ogre::RenderTarget *Ogre2RenderWindow::RenderTarget() const
{
  return this->ogreRenderWindow;
}

//////////////////////////////////////////////////
void Ogre2RenderWindow::Destroy()
{
  // TODO(anyone)
}

//////////////////////////////////////////////////
void Ogre2RenderWindow::RebuildTarget()
{
  // TODO(anyone): determine when to rebuild
  // ie. only when ratio or handle changes!
  // e.g. sizeDirty?
  if (!this->ogreRenderWindow)
    this->BuildTarget();

  Ogre::RenderWindow *window =
      dynamic_cast<Ogre::RenderWindow *>(this->ogreRenderWindow);
  window->resize(this->width, this->height);
  window->windowMovedOrResized();
}

//////////////////////////////////////////////////
void Ogre2RenderWindow::BuildTarget()
{
  auto engine = Ogre2RenderEngine::Instance();
  std::string renderTargetName =
      engine->CreateRenderWindow(this->handle,
          this->width,
          this->height,
          this->ratio,
          this->antiAliasing);
  this->ogreRenderWindow =
      engine->OgreRoot()->getRenderTarget(renderTargetName);
}
