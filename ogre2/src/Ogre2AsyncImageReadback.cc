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

#include <cstring>

#include "gz/rendering/ogre2/Ogre2AsyncImageReadback.hh"
#include "gz/rendering/ogre2/Ogre2RenderEngine.hh"

#ifdef _MSC_VER
  #pragma warning(push, 0)
#endif
#include <OgreAsyncTextureTicket.h>
#include <OgreRenderSystem.h>
#include <OgreRoot.h>
#include <OgreTextureBox.h>
#include <OgreTextureGpu.h>
#include <OgreTextureGpuManager.h>
#ifdef _MSC_VER
  #pragma warning(pop)
#endif

using namespace gz;
using namespace rendering;

//////////////////////////////////////////////////
Ogre2AsyncImageReadback::Ogre2AsyncImageReadback() = default;

//////////////////////////////////////////////////
Ogre2AsyncImageReadback::~Ogre2AsyncImageReadback()
{
  this->Reset();
}

//////////////////////////////////////////////////
static Ogre::TextureGpuManager *TexMgr()
{
  auto *engine = Ogre2RenderEngine::Instance();
  Ogre::RenderSystem *rs = engine->OgreRoot()->getRenderSystem();
  return rs->getTextureGpuManager();
}

//////////////////////////////////////////////////
void Ogre2AsyncImageReadback::Configure(unsigned int _width,
    unsigned int _height, unsigned int _ringDepth)
{
  if (_width == this->width && _height == this->height &&
      _ringDepth == this->ringDepth && this->created > 0u)
  {
    return;  // already configured for these dimensions
  }
  this->Reset();
  this->width = _width;
  this->height = _height;
  this->ringDepth = _ringDepth == 0u ? 1u : _ringDepth;
}

//////////////////////////////////////////////////
Ogre::AsyncTextureTicket *Ogre2AsyncImageReadback::AcquireTicket(
    Ogre::TextureGpu *_src)
{
  // Reuse an idle ticket if available.
  if (!this->idle.empty())
  {
    Ogre::AsyncTextureTicket *t = this->idle.back();
    this->idle.pop_back();
    return t;
  }

  // Create a fresh ticket while under the ring depth.
  if (this->created < this->ringDepth)
  {
    Ogre::AsyncTextureTicket *t = TexMgr()->createAsyncTextureTicket(
        this->width, this->height, 1u, Ogre::TextureTypes::Type2D,
        _src->getPixelFormat());
    ++this->created;
    return t;
  }

  // Ring is full: force-drain the oldest in-flight ticket (this may briefly
  // stall on the GPU fence, but only when we are over-full) and recycle it, so
  // a rendered frame is never silently dropped.
  Ogre::AsyncTextureTicket *t = this->inflight.front();
  this->inflight.pop_front();
  if (t->getNumSlices() > 0u)
  {
    t->map(0u);
    t->unmap();
  }
  return t;
}

//////////////////////////////////////////////////
void Ogre2AsyncImageReadback::RequestDownload(Ogre::TextureGpu *_src)
{
  if (!_src || this->width == 0u || this->height == 0u)
    return;

  Ogre::AsyncTextureTicket *t = this->AcquireTicket(_src);
  // Non-blocking download of mip 0; accurateTracking=false = the fast path
  // (no per-copy fence), which is exactly the latency we want to hide.
  t->download(_src, 0u, false);
  this->inflight.push_back(t);
}

//////////////////////////////////////////////////
bool Ogre2AsyncImageReadback::TryRetrieve(std::vector<unsigned char> &_out)
{
  if (this->inflight.empty())
    return false;

  Ogre::AsyncTextureTicket *t = this->inflight.front();
  if (!t->queryIsTransferDone())
    return false;  // DMA still in flight; do not stall

  this->inflight.pop_front();

  const Ogre::TextureBox box = t->map(0u);
  unsigned int bpp = box.bytesPerPixel;
  if (bpp == 0u)
    bpp = 4u;  // RGBA8 fallback
  const size_t rowBytes = static_cast<size_t>(this->width) * bpp;
  _out.resize(rowBytes * this->height);
  for (unsigned int y = 0u; y < this->height; ++y)
  {
    const auto *srcRow = static_cast<const unsigned char *>(box.data) +
        static_cast<size_t>(y) * box.bytesPerRow;
    std::memcpy(_out.data() + static_cast<size_t>(y) * rowBytes,
        srcRow, rowBytes);
  }
  t->unmap();

  this->idle.push_back(t);
  return true;
}

//////////////////////////////////////////////////
void Ogre2AsyncImageReadback::Reset()
{
  Ogre::TextureGpuManager *texMgr = nullptr;
  try
  {
    texMgr = TexMgr();
  }
  catch (...)
  {
    texMgr = nullptr;
  }

  if (texMgr)
  {
    for (auto *t : this->inflight)
      texMgr->destroyAsyncTextureTicket(t);
    for (auto *t : this->idle)
      texMgr->destroyAsyncTextureTicket(t);
  }
  this->inflight.clear();
  this->idle.clear();
  this->created = 0u;
}
