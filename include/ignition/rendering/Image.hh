/*
 * Copyright (C) 2015 Open Source Robotics Foundation
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
#ifndef _IGNITION_RENDERING_IMAGE_HH_
#define _IGNITION_RENDERING_IMAGE_HH_

#include <boost/shared_ptr.hpp>
#include "ignition/rendering/PixelFormat.hh"
#include "ignition/rendering/Util.hh"

namespace ignition
{
  namespace rendering
  {
    /// \class Image Image.hh ignition/rendering/Image.hh
    /// \brief Encapsulates an raw image buffer and relevant properties
    class IGNITION_VISIBLE Image
    {
      // TODO: convert to std::unique_ptr
      /// \brief Smart pointer to raw image buffer
      typedef boost::shared_ptr<unsigned char> DataPtr;

      /// \brief Constructor
      /// \param[in] _width Image width in pixels
      /// \param[in] _height Image height in pixels
      /// \param[in] _format Image pixel format
      public: Image(unsigned int _width, unsigned int _height,
                  PixelFormat _format);

      /// \brief Deconstructor
      public: ~Image();

      /// \brief Get image width in pixels
      /// \return The image width in pixels
      public: unsigned int GetWidth() const;

      /// \brief Get image height in pixels
      /// \return The image height in pixels
      public: unsigned int GetHeight() const;

      /// \brief Get image pixel format
      /// \return The image pixel format
      public: PixelFormat GetFormat() const;

      /// \brief Get image channel depth
      /// \return The image channel depth
      public: unsigned int GetDepth() const;

      /// \brief Get image channel depth
      /// \return The image channel depth
      public: unsigned int GetMemorySize() const;

      /// \brief Get a const pointer to image data
      /// \return The const pointer to image data
      public: const void *GetData() const;

      /// \brief Get a pointer to image data
      /// \return The pointer to image data
      public: void *GetData();

      /// \brief Get a const pointer to image data. The returned image buffer
      /// will be statically cast to the given template type T.
      /// \return The const pointer to image data
      public: template <typename T>
              const T *GetData() const;

      /// \brief Get a pointer to image data. The returned image buffer will be
      /// statically cast to the given template type T.
      /// \return The pointer to image data
      public: template <typename T>
              T *GetData();

      /// \brief Image width in pixels
      private: unsigned int width;

      /// \brief Image height in pixels
      private: unsigned int height;

      /// \brief Image pixel format
      private: PixelFormat format;

      /// \brief Pointer to the image data
      private: DataPtr data;
    };

    //////////////////////////////////////////////////
    template <typename T>
    const T *Image::GetData() const
    {
      return static_cast<const T *>(this->data.get());
    }

    //////////////////////////////////////////////////
    template <typename T>
    T *Image::GetData()
    {
      return static_cast<T *>(this->data.get());
    }
  }
}
#endif
