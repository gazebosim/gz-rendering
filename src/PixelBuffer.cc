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

using namespace gz;
using namespace rendering;

//////////////////////////////////////////////////
PixelBuffer::PixelBuffer(unsigned int _width, unsigned int _height,
    PixelFormat _format, void *_data, std::size_t _size)
  : width(_width), height(_height), format(PixelUtil::Sanitize(_format)),
    data(static_cast<unsigned char *>(_data)), size(_size)
{
}

//////////////////////////////////////////////////
PixelBuffer::PixelBuffer(Image &_image)
  : PixelBuffer(_image.Width(), _image.Height(), _image.Format(),
                _image.Data(), _image.MemorySize())
{
}

//////////////////////////////////////////////////
unsigned int PixelBuffer::Width() const
{
  return this->width;
}

//////////////////////////////////////////////////
unsigned int PixelBuffer::Height() const
{
  return this->height;
}

//////////////////////////////////////////////////
PixelFormat PixelBuffer::Format() const
{
  return this->format;
}

//////////////////////////////////////////////////
std::size_t PixelBuffer::Size() const
{
  return this->size;
}

//////////////////////////////////////////////////
std::size_t PixelBuffer::MemorySize() const
{
  return PixelUtil::MemorySize(this->format, this->width, this->height);
}

//////////////////////////////////////////////////
bool PixelBuffer::Valid() const
{
  return this->data != nullptr && this->size >= this->MemorySize();
}

//////////////////////////////////////////////////
unsigned char *PixelBuffer::Data() const
{
  return this->data;
}
