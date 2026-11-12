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
#include "Ogre2TextureReadback.hh"

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
namespace
{
/// \brief Describe a tightly packed box covering the whole texture, laid out
/// in the given pixel format. Its data pointer is left null.
/// \param[in] _texture Texture the box covers
/// \param[in] _format Pixel format of the memory the box describes
/// \return The box, containing metadata only
Ogre::TextureBox textureBoxFor(Ogre::TextureGpu *_texture,
    Ogre::PixelFormatGpu _format)
{
  return Ogre::TextureBox(
    _texture->getInternalWidth(), _texture->getInternalHeight(),
    _texture->getDepth(), _texture->getNumSlices(),
    static_cast<uint32_t>(
      Ogre::PixelFormatGpuUtils::getBytesPerPixel(_format)),
    static_cast<uint32_t>(Ogre::PixelFormatGpuUtils::getSizeBytes(
      _texture->getInternalWidth(), 1u, 1u, 1u, _format, 1u)),
    static_cast<uint32_t>(Ogre::PixelFormatGpuUtils::getSizeBytes(
      _texture->getInternalWidth(), _texture->getInternalHeight(), 1u, 1u,
      _format, 1u)));
}
}

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

//////////////////////////////////////////////////
bool Ogre2TextureReadback::ReadBack(Ogre::TextureGpu *_texture,
    Ogre::PixelFormatGpu _dstFormat, const PixelBuffer &_dst)
{
  Ogre::TextureBox dstBox = textureBoxFor(_texture, _dstFormat);
  if (_dst.Size() < dstBox.getSizeBytes())
  {
    gzerr << "Buffer too small for texture readback: "
          << dstBox.getSizeBytes() << " bytes needed, "
          << _dst.Size() << " available" << std::endl;
    return false;
  }
  dstBox.data = _dst.Data();

  const Ogre::PixelFormatGpu texFormat = _texture->getPixelFormat();
  if (texFormat == _dstFormat)
  {
    // Same format: the rows are copied with a plain memcpy.
    return Ogre2CopyTextureToMemory(_texture, dstBox, _dstFormat);
  }
  else
  {
    // Formats differ: read back unconverted into regular memory, then
    // convert there. Converting while copying would otherwise convert
    // straight out of the mapped GPU buffer, one byte at a time. Where the
    // driver maps that buffer uncached (Mesa freedreno on Adreno) that costs
    // ~40 ms per 848x480 RGB frame. The buffer is kept across calls so it is
    // not allocated on every frame.
    Ogre::TextureBox rawBox = textureBoxFor(_texture, texFormat);
    this->conversionBuffer.resize(rawBox.getSizeBytes());
    rawBox.data = this->conversionBuffer.data();

    if (!Ogre2CopyTextureToMemory(_texture, rawBox, texFormat))
      return false;
    Ogre::PixelFormatGpuUtils::bulkPixelConversion(
        rawBox, texFormat, dstBox, _dstFormat);
  }
  return true;
}
