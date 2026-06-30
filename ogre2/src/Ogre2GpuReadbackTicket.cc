/*
 * Copyright (C) 2026 Open Source Robotics Foundation
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
#include "Ogre2GpuReadbackTicket.hh"

#include <cstdlib>

#include <gz/common/Console.hh>

#include "gz/rendering/ogre2/Ogre2RenderEngine.hh"

#ifdef _MSC_VER
  #pragma warning(push, 0)
#endif
#include <OgreRoot.h>
#include <OgreRenderSystem.h>
#include <OgreTextureGpu.h>
#include <OgreTextureGpuManager.h>
#include <OgreAsyncTextureTicket.h>
#ifdef _MSC_VER
  #pragma warning(pop)
#endif

using namespace gz;
using namespace rendering;

//////////////////////////////////////////////////
/// \brief Fetch the active OGRE texture manager, or nullptr if unavailable.
static Ogre::TextureGpuManager *ReadbackTextureManager()
{
  auto engine = Ogre2RenderEngine::Instance();
  if (!engine)
    return nullptr;
  auto ogreRoot = engine->OgreRoot();
  if (!ogreRoot || !ogreRoot->getRenderSystem())
    return nullptr;
  return ogreRoot->getRenderSystem()->getTextureGpuManager();
}

//////////////////////////////////////////////////
Ogre2GpuReadbackTicket::~Ogre2GpuReadbackTicket()
{
  this->Destroy();
}

//////////////////////////////////////////////////
void Ogre2GpuReadbackTicket::Destroy()
{
  if (!this->ticket)
    return;

  if (this->mapped)
  {
    this->ticket->unmap();
    this->mapped = false;
  }

  auto textureMgr = ReadbackTextureManager();
  if (textureMgr)
    textureMgr->destroyAsyncTextureTicket(this->ticket);
  else
    gzwarn << "Ogre2GpuReadbackTicket::Destroy() called after engine "
              "teardown; ticket staging buffer leaked." << std::endl;

  this->ticket = nullptr;
  this->width = 0u;
  this->height = 0u;
  this->format = Ogre::PFG_UNKNOWN;
}

//////////////////////////////////////////////////
Ogre::TextureBox Ogre2GpuReadbackTicket::DownloadAndMap(
    Ogre::TextureGpu *_texture)
{
  Ogre::TextureBox box;  // default-constructed: data == nullptr
  if (!_texture)
    return box;

  auto textureMgr = ReadbackTextureManager();
  if (!textureMgr)
    return box;

  const unsigned int w = _texture->getWidth();
  const unsigned int h = _texture->getHeight();
  const Ogre::PixelFormatGpu fmt = _texture->getPixelFormat();

  // (Re)create the ticket on first use or when the geometry/format changed.
  if (!this->ticket || this->width != w || this->height != h ||
      this->format != fmt)
  {
    if (this->ticket)
    {
      if (this->mapped)
      {
        this->ticket->unmap();
        this->mapped = false;
      }
      textureMgr->destroyAsyncTextureTicket(this->ticket);
      this->ticket = nullptr;
    }
    this->ticket = textureMgr->createAsyncTextureTicket(
        w, h, 1u, Ogre::TextureTypes::Type2D, fmt);
    this->width = w;
    this->height = h;
    this->format = fmt;
  }

  if (!this->ticket)
    return box;

  if (this->mapped)
  {
    gzwarn << "Ogre2GpuReadbackTicket::DownloadAndMap() called while still "
              "mapped; unmapping previous frame first." << std::endl;
    this->ticket->unmap();
    this->mapped = false;
  }

  // Blocking download (accurateTracking=true): map() waits on the fence.
  this->ticket->download(_texture, 0u, true);
  box = this->ticket->map(0u);
  this->mapped = true;
  return box;
}

//////////////////////////////////////////////////
void Ogre2GpuReadbackTicket::Unmap()
{
  if (this->ticket && this->mapped)
  {
    this->ticket->unmap();
    this->mapped = false;
  }
}

//////////////////////////////////////////////////
bool gz::rendering::Ogre2UseLegacyReadback()
{
  static const bool legacy =
      (std::getenv("GZ_RENDERING_OGRE2_LEGACY_READBACK") != nullptr);
  return legacy;
}
