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
#ifndef GZ_RENDERING_OGRE2_OGRE2TEXTUREREADBACK_HH_
#define GZ_RENDERING_OGRE2_OGRE2TEXTUREREADBACK_HH_

#include <cstdint>
#include <vector>

#include "gz/rendering/config.hh"
#include "gz/rendering/ogre2/Export.hh"
#include "gz/rendering/PixelBuffer.hh"

#ifdef _MSC_VER
  #pragma warning(push, 0)
#endif
#include <OgrePixelFormatGpu.h>
#ifdef _MSC_VER
  #pragma warning(pop)
#endif

namespace Ogre
{
  class Image2;
  struct TextureBox;
  class TextureGpu;
}

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    //
    /// \brief Copy the first mip of a texture into memory, converting it to
    /// another format, like Ogre::Image2::copyContentsToMemory(). Only the
    /// download, and the map that waits for it to finish, count as GPU wait
    /// (see ScopedGpuWait). Copying out of the mapped buffer is CPU work.
    /// \param[in] _texture Texture to copy
    /// \param[in] _dst Memory to copy to, the same size as the texture
    /// \param[in] _dstFormat Pixel format to write to _dst
    /// \return True if the texture was copied, false with an error printed if
    /// it isn't resident or _dst is not the size of the texture
    GZ_RENDERING_OGRE2_HIDDEN
    bool Ogre2CopyTextureToMemory(Ogre::TextureGpu *_texture,
        const Ogre::TextureBox &_dst, Ogre::PixelFormatGpu _dstFormat);

    /// \brief Read the first mip of a texture back into an image in the
    /// texture's own format, like Ogre::Image2::convertFromTexture(_texture,
    /// 0, 0), counting only the GPU wait as Ogre2CopyTextureToMemory() does.
    /// \param[out] _image Image to read into
    /// \param[in] _texture Texture to read
    /// \return True if the texture was read back
    GZ_RENDERING_OGRE2_HIDDEN
    bool Ogre2ConvertFromTexture(Ogre::Image2 &_image,
        Ogre::TextureGpu *_texture);

    /// \brief Reads a texture back into memory, converting it to another
    /// format.
    class GZ_RENDERING_OGRE2_HIDDEN Ogre2TextureReadback
    {
      /// \brief Read _texture back into _dst.
      /// \param[in] _texture Texture to read back
      /// \param[in] _dstFormat Pixel format to write to _dst. May differ from
      /// the buffer's own format in sRGB-ness only.
      /// \param[in] _dst Destination buffer, must hold the whole texture in
      /// _dstFormat. Left untouched, with an error printed, if it is too
      /// small.
      /// \return True if the texture was read back, false if _dst is too
      /// small
      public: bool ReadBack(Ogre::TextureGpu *_texture,
          Ogre::PixelFormatGpu _dstFormat, const PixelBuffer &_dst);

      /// \brief Raw readback in the texture's own format, used when the
      /// formats differ. Empty until then; reused across calls.
      private: std::vector<uint8_t> conversionBuffer;
    };
    }
  }
}
#endif
