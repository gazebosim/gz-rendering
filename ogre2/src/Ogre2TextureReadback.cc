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
#include <cstring>

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

//////////////////////////////////////////////////
/// \brief Download a texture into memory in its own format.
/// \param[in] _texture Texture to read back
/// \param[in] _dst Box in the texture's format, covering the whole texture
/// \param[in,out] _ticket Persistent ticket to download through
/// \return True if the texture was read back
bool downloadTexture(Ogre::TextureGpu *_texture,
    Ogre::TextureBox &_dst, Ogre2GpuReadbackTicket &_ticket)
{
  const Ogre::PixelFormatGpu texFormat = _texture->getPixelFormat();
  if (Ogre2UseLegacyReadback())
  {
    // A new ticket (staging buffer) per copy, as before the persistent one.
    Ogre::Image2::copyContentsToMemory(
        _texture, _texture->getEmptyBox(0u), _dst, texFormat);
    return true;
  }
  const Ogre::TextureBox mapped = _ticket.DownloadAndMap(_texture);
  if (!mapped.data)
  {
    gzerr << "GPU texture readback failed" << std::endl;
    return false;
  }
  if (!mapped.equalSize(_dst))
  {
    gzerr << "GPU texture readback is " << mapped.width << "x"
          << mapped.height << ", expected " << _dst.width << "x"
          << _dst.height << std::endl;
    _ticket.Unmap();
    return false;
  }
  // Ogre's own same-format copy: a row memcpy honouring both row pitches.
  Ogre::PixelFormatGpuUtils::bulkPixelConversion(
      mapped, texFormat, _dst, texFormat);
  _ticket.Unmap();
  return true;
}

//////////////////////////////////////////////////
/// \brief Download a texture and convert it straight into _dst, in
/// another format. The mapped buffer is read by memcpy only, a block of
/// rows at a time into _scratch (#1345: it may be uncached memory), and
/// each block is converted by Ogre (bulkPixelConversion, pixel by pixel)
/// while it is in cache: the frame makes no round trip through a full-size
/// intermediate buffer, and the bytes are those of converting it whole.
/// \param[in] _texture Texture to read back
/// \param[in] _dst Box covering the whole texture, in _dstFormat
/// \param[in] _dstFormat Pixel format of _dst
/// \param[in,out] _scratch Block buffer, grown as needed
/// \param[in,out] _ticket Persistent ticket to download through
/// \return True if the texture was read back
bool downloadConverting(Ogre::TextureGpu *_texture,
    Ogre::TextureBox &_dst, Ogre::PixelFormatGpu _dstFormat,
    std::vector<uint8_t> &_scratch, Ogre2GpuReadbackTicket &_ticket)
{
  const Ogre::TextureBox mapped = _ticket.DownloadAndMap(_texture);
  if (!mapped.data)
  {
    gzerr << "GPU texture readback failed" << std::endl;
    return false;
  }
  if (!mapped.equalSize(_dst))
  {
    gzerr << "GPU texture readback is " << mapped.width << "x"
          << mapped.height << ", expected " << _dst.width << "x"
          << _dst.height << std::endl;
    _ticket.Unmap();
    return false;
  }
  const size_t rowBytes = mapped.width * mapped.bytesPerPixel;
  // About 64 KiB per block: well inside L2
  const size_t blockRows = std::max<size_t>(1u, (64u << 10) / rowBytes);
  _scratch.resize(std::min<size_t>(blockRows, _dst.height) * rowBytes);
  const uint8_t *src = static_cast<const uint8_t *>(mapped.data);
  uint8_t *dst = static_cast<uint8_t *>(_dst.data);
  for (size_t y0 = 0; y0 < _dst.height; y0 += blockRows)
  {
    const size_t rows = std::min<size_t>(blockRows, _dst.height - y0);
    for (size_t r = 0; r < rows; ++r)
    {
      std::memcpy(_scratch.data() + r * rowBytes,
          src + (y0 + r) * mapped.bytesPerRow, rowBytes);
    }
    Ogre::TextureBox srcBlock(mapped.width, static_cast<uint32_t>(rows), 1u,
        1u, mapped.bytesPerPixel, static_cast<uint32_t>(rowBytes),
        static_cast<uint32_t>(rows * rowBytes));
    srcBlock.data = _scratch.data();
    Ogre::TextureBox dstBlock(_dst.width, static_cast<uint32_t>(rows), 1u, 1u,
        _dst.bytesPerPixel, _dst.bytesPerRow,
        static_cast<uint32_t>(rows * _dst.bytesPerRow));
    dstBlock.data = dst + y0 * _dst.bytesPerRow;
    Ogre::PixelFormatGpuUtils::bulkPixelConversion(
        srcBlock, _texture->getPixelFormat(), dstBlock, _dstFormat);
  }
  _ticket.Unmap();
  return true;
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
    // Same format: the rows are copied with a plain memcpy.
    return downloadTexture(_texture, dstBox, this->ticket);
  }
  else if (!Ogre2UseLegacyReadback())
  {
    // Formats differ: converted block by block
    return downloadConverting(_texture, dstBox, _dstFormat,
        this->conversionBuffer, this->ticket);
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

    if (!downloadTexture(_texture, rawBox, this->ticket))
      return false;
    Ogre::PixelFormatGpuUtils::bulkPixelConversion(
        rawBox, texFormat, dstBox, _dstFormat);
  }
  return true;
}

//////////////////////////////////////////////////
void Ogre2TextureReadback::Destroy()
{
  this->ticket.Destroy();
}
