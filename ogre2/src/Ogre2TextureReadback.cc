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

#include <gz/common/Console.hh>

#ifdef _MSC_VER
  #pragma warning(push, 0)
#endif
#include <OgreImage2.h>
#include <OgrePixelFormatGpuUtils.h>
#include <OgreTextureBox.h>
#include <OgreTextureGpu.h>
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
    // Same format: Ogre copies the rows with a plain memcpy.
    Ogre::Image2::copyContentsToMemory(
        _texture, _texture->getEmptyBox(0u), dstBox, _dstFormat);
  }
  else
  {
    // Formats differ: read back unconverted into regular memory, then
    // convert there. Image2::copyContentsToMemory() would otherwise convert
    // straight out of the mapped GPU buffer, one byte at a time. Where the
    // driver maps that buffer uncached (Mesa freedreno on Adreno) that costs
    // ~40 ms per 848x480 RGB frame. The buffer is kept across calls so it is
    // not allocated on every frame.
    Ogre::TextureBox rawBox = textureBoxFor(_texture, texFormat);
    this->conversionBuffer.resize(rawBox.getSizeBytes());
    rawBox.data = this->conversionBuffer.data();

    Ogre::Image2::copyContentsToMemory(
        _texture, _texture->getEmptyBox(0u), rawBox, texFormat);
    Ogre::PixelFormatGpuUtils::bulkPixelConversion(
        rawBox, texFormat, dstBox, _dstFormat);
  }
  return true;
}
