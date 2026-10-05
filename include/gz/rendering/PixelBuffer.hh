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
#ifndef GZ_RENDERING_PIXELBUFFER_HH_
#define GZ_RENDERING_PIXELBUFFER_HH_

#include <cstddef>

#include <gz/utils/ImplPtr.hh>

#include "gz/rendering/config.hh"
#include "gz/rendering/Export.hh"
#include "gz/rendering/PixelFormat.hh"

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    //
    class Image;

    /// \class PixelBuffer PixelBuffer.hh gz/rendering/PixelBuffer.hh
    /// \brief Non owning, writable view of a caller owned pixel buffer.
    ///
    /// A PixelBuffer describes where a frame should be written: dimensions,
    /// pixel format, the start of a contiguous row major buffer and its size
    /// in bytes. It owns nothing and never frees the memory. Build one right
    /// before handing it to Camera::CopyTo and let it die when the call
    /// returns; nothing in the library keeps a pointer to the memory after
    /// that.
    ///
    /// Every field is immutable and assignment is deleted, so code that
    /// receives a PixelBuffer can write through it but cannot point it
    /// somewhere else. That is what lets the render engines write straight
    /// into caller memory without being able to drop it by mistake.
    class GZ_RENDERING_VISIBLE PixelBuffer
    {
      /// \brief Describe a caller owned buffer.
      /// \param[in] _width Width in pixels
      /// \param[in] _height Height in pixels
      /// \param[in] _format Pixel format of the buffer
      /// \param[in] _data Start of the buffer. Not owned.
      /// \param[in] _size Size of the buffer in bytes
      public: PixelBuffer(unsigned int _width, unsigned int _height,
                  PixelFormat _format, void *_data, std::size_t _size);

      /// \brief View the storage of an existing image. The view is only valid
      /// while the image, or a copy of it, is alive.
      /// \param[in] _image Image whose buffer is viewed
      public: explicit PixelBuffer(Image &_image);

      /// \brief Copy constructor. Copies of a view share the same buffer.
      /// \param[in] _other View to copy
      public: PixelBuffer(const PixelBuffer &_other);

      /// \brief Views cannot be rebound.
      public: PixelBuffer &operator=(const PixelBuffer &) = delete;

      /// \brief Views cannot be rebound.
      public: PixelBuffer &operator=(PixelBuffer &&) = delete;

      /// \brief Destructor. Does not free the buffer.
      public: ~PixelBuffer();

      /// \brief Get the width in pixels
      /// \return Width in pixels
      public: unsigned int Width() const;

      /// \brief Get the height in pixels
      /// \return Height in pixels
      public: unsigned int Height() const;

      /// \brief Get the pixel format
      /// \return Pixel format
      public: PixelFormat Format() const;

      /// \brief Get the size of the buffer in bytes, as given by the caller
      /// \return Buffer size in bytes
      public: std::size_t Size() const;

      /// \brief Get the number of bytes Width x Height pixels of Format need
      /// \return Required size in bytes
      public: std::size_t MemorySize() const;

      /// \brief Check that the buffer can hold a frame: the pointer is not
      /// null and Size() is at least MemorySize().
      /// \return True if a frame can be written into the buffer
      public: bool Valid() const;

      /// \brief Get a pointer to the first byte of the buffer. The pointer is
      /// writable even through a const view: this is a span, not a read only
      /// view.
      /// \return Pointer to the buffer
      public: unsigned char *Data() const;

      /// \internal
      /// \brief Private data pointer
      GZ_UTILS_IMPL_PTR(dataPtr)
    };
    }
  }
}
#endif
