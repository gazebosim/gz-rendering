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
  class TextureGpu;
}

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    //
    /// \brief Reads a texture back into memory, converting it to another
    /// format.
    class Ogre2TextureReadback
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
