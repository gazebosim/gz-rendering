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

#include "gz/rendering/Image.hh"
#include "gz/rendering/PixelBuffer.hh"

/// \brief Private fields of PixelBuffer
class gz::rendering::PixelBuffer::Implementation
{
  /// \brief Width in pixels
  public: unsigned int width = 0;

  /// \brief Height in pixels
  public: unsigned int height = 0;

  /// \brief Pixel format
  public: PixelFormat format = PF_UNKNOWN;

  /// \brief Start of the caller owned buffer
  public: unsigned char *data = nullptr;

  /// \brief Size of the buffer in bytes
  public: std::size_t size = 0;
};

using namespace gz;
using namespace rendering;

//////////////////////////////////////////////////
PixelBuffer::PixelBuffer(unsigned int _width, unsigned int _height,
    PixelFormat _format, void *_data, std::size_t _size)
  : dataPtr(utils::MakeImpl<Implementation>())
{
  this->dataPtr->width = _width;
  this->dataPtr->height = _height;
  this->dataPtr->format = PixelUtil::Sanitize(_format);
  this->dataPtr->data = static_cast<unsigned char *>(_data);
  this->dataPtr->size = _size;
}

//////////////////////////////////////////////////
PixelBuffer::PixelBuffer(Image &_image)
  : PixelBuffer(_image.Width(), _image.Height(), _image.Format(),
                _image.Data(), _image.MemorySize())
{
}

//////////////////////////////////////////////////
PixelBuffer::PixelBuffer(const PixelBuffer &_other) = default;

//////////////////////////////////////////////////
PixelBuffer::~PixelBuffer() = default;

//////////////////////////////////////////////////
unsigned int PixelBuffer::Width() const
{
  return this->dataPtr->width;
}

//////////////////////////////////////////////////
unsigned int PixelBuffer::Height() const
{
  return this->dataPtr->height;
}

//////////////////////////////////////////////////
PixelFormat PixelBuffer::Format() const
{
  return this->dataPtr->format;
}

//////////////////////////////////////////////////
std::size_t PixelBuffer::Size() const
{
  return this->dataPtr->size;
}

//////////////////////////////////////////////////
std::size_t PixelBuffer::MemorySize() const
{
  return PixelUtil::MemorySize(this->dataPtr->format,
      this->dataPtr->width, this->dataPtr->height);
}

//////////////////////////////////////////////////
bool PixelBuffer::Valid() const
{
  return this->dataPtr->data != nullptr &&
      this->dataPtr->size >= this->MemorySize();
}

//////////////////////////////////////////////////
unsigned char *PixelBuffer::Data() const
{
  return this->dataPtr->data;
}
