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
#include "Ogre2TextureCopy.hh"

#include <algorithm>

#include <gz/common/Console.hh>

#include "gz/rendering/GpuWaitTime.hh"

#ifdef _MSC_VER
  #pragma warning(push, 0)
#endif
#include <OgreAsyncTextureTicket.h>
#include <OgreImage2.h>
#include <OgrePixelFormatGpuUtils.h>
#include <OgreTextureBox.h>
#include <OgreTextureGpu.h>
#include <OgreTextureGpuManager.h>
#ifdef _MSC_VER
  #pragma warning(pop)
#endif

using namespace gz;
using namespace rendering;

//////////////////////////////////////////////////
bool rendering::Ogre2CopyTextureToMemory(Ogre::TextureGpu *_texture,
    const Ogre::TextureBox &_dst, Ogre::PixelFormatGpu _dstFormat)
{
  if (_texture->getResidencyStatus() != Ogre::GpuResidency::Resident &&
      _texture->getNextResidencyStatus() != Ogre::GpuResidency::Resident)
  {
    gzerr << "Texture [" << _texture->getNameStr() << "] is not resident, "
          << "it can't be read back" << std::endl;
    return false;
  }
  const Ogre::TextureBox srcBox = _texture->getEmptyBox(0u);
  if (!_dst.equalSize(srcBox))
  {
    gzerr << "Memory box is not the size of texture ["
          << _texture->getNameStr() << "], it can't be read back"
          << std::endl;
    return false;
  }
  _texture->waitForData();

  // What Ogre::Image2::copyContentsToMemory() does, so that the copy out of
  // the mapped buffer is left out of the GPU wait.
  Ogre::TextureGpuManager *textureManager = _texture->getTextureManager();
  Ogre::TextureGpu *resolvedTexture = _texture;
  if (_texture->isMultisample() && _texture->hasMsaaExplicitResolves() &&
      !_texture->isOpenGLRenderWindow())
  {
    resolvedTexture = textureManager->createTexture(
        _texture->getNameStr() + "/Tmp/__ResolveTex",
        Ogre::GpuPageOutStrategy::Discard, Ogre::TextureFlags::RenderToTexture,
        _texture->getTextureType());
    resolvedTexture->copyParametersFrom(_texture);
    resolvedTexture->setPixelFormat(_texture->getPixelFormat());
    resolvedTexture->setSampleDescription(1u);
    resolvedTexture->_transitionTo(Ogre::GpuResidency::Resident, nullptr);
    _texture->_resolveTo(resolvedTexture);
  }

  Ogre::AsyncTextureTicket *ticket = textureManager->createAsyncTextureTicket(
      srcBox.width, srcBox.height, srcBox.getDepthOrSlices(),
      _texture->getTextureType(), _texture->getPixelFormat());
  Ogre::TextureBox srcCopy = srcBox;
  {
    ScopedGpuWait gpuWait;
    // With accurate tracking the first map() waits on a fence for the
    // download to finish.
    ticket->download(resolvedTexture, 0u, true, &srcCopy);
  }

  const auto mapSlice = [&](uint32_t _slice)
  {
    ScopedGpuWait gpuWait;
    return ticket->map(_slice);
  };
  Ogre::TextureBox dst = _dst;
  if (ticket->canMapMoreThanOneSlice())
  {
    Ogre::PixelFormatGpuUtils::bulkPixelConversion(
        mapSlice(0u), _texture->getPixelFormat(), dst, _dstFormat);
    ticket->unmap();
  }
  else
  {
    Ogre::TextureBox dstSlice = dst;
    dstSlice.numSlices = 1u;
    for (uint32_t i = 0u; i < ticket->getNumSlices(); ++i)
    {
      Ogre::PixelFormatGpuUtils::bulkPixelConversion(
          mapSlice(i), _texture->getPixelFormat(), dstSlice, _dstFormat);
      dstSlice.data = dstSlice.at(0u, 0u, 1u);
      ticket->unmap();
    }
  }

  textureManager->destroyAsyncTextureTicket(ticket);
  if (resolvedTexture != _texture)
    textureManager->destroyTexture(resolvedTexture);
  return true;
}

//////////////////////////////////////////////////
bool rendering::Ogre2ConvertFromTexture(Ogre::Image2 &_image,
    Ogre::TextureGpu *_texture)
{
  _image.createEmptyImage(_texture->getInternalWidth(),
      _texture->getInternalHeight(),
      std::max({1u, _texture->getDepth(), _texture->getNumSlices()}),
      _texture->getTextureType(), _texture->getPixelFormat(), 1u);
  return Ogre2CopyTextureToMemory(_texture, _image.getData(0u),
      _texture->getPixelFormat());
}
