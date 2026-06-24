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

#include <cstring>
#include <memory>

#include "gz/rendering/CompressedImage.hh"

/// \brief Shared pointer to raw byte buffer.
typedef std::shared_ptr<unsigned char> DataPtr;

/// \brief Private fields of CompressedImage.
class gz::rendering::CompressedImage::Implementation
{
  /// \brief Source image width in pixels.
  public: unsigned int width = 0;

  /// \brief Source image height in pixels.
  public: unsigned int height = 0;

  /// \brief Wire encoding.
  public: ImageEncoding encoding = IE_NONE;

  /// \brief Colour metadata.
  public: ColorimetryInfo colorimetry;

  /// \brief Key-frame flag.
  public: bool keyFrame = true;

  /// \brief Allocated capacity in bytes.
  public: unsigned int capacity = 0;

  /// \brief Valid bytes this frame.
  public: unsigned int size = 0;

  /// \brief Pointer to the encoded data.
  public: DataPtr data = nullptr;
};

using namespace gz;
using namespace rendering;

//////////////////////////////////////////////////
template <class T>
struct ArrayDeleter
{
  void operator () (T const * p)
  {
    delete [] p;
  }
};

//////////////////////////////////////////////////
CompressedImage::CompressedImage()
  : dataPtr(utils::MakeImpl<Implementation>())
{
}

//////////////////////////////////////////////////
CompressedImage::CompressedImage(unsigned int _width, unsigned int _height,
  ImageEncoding _encoding)
  : dataPtr(utils::MakeImpl<Implementation>())
{
  this->dataPtr->width = _width;
  this->dataPtr->height = _height;
  this->dataPtr->encoding = _encoding;
}

//////////////////////////////////////////////////
CompressedImage::CompressedImage(const CompressedImage &_other)
  : dataPtr(utils::MakeImpl<Implementation>())
{
  this->dataPtr->width = _other.dataPtr->width;
  this->dataPtr->height = _other.dataPtr->height;
  this->dataPtr->encoding = _other.dataPtr->encoding;
  this->dataPtr->colorimetry = _other.dataPtr->colorimetry;
  this->dataPtr->keyFrame = _other.dataPtr->keyFrame;
  this->dataPtr->capacity = _other.dataPtr->capacity;
  this->dataPtr->size = _other.dataPtr->size;

  // Deep copy the data buffer
  if (_other.dataPtr->data && _other.dataPtr->capacity > 0)
  {
    this->dataPtr->data =
        DataPtr(new unsigned char[_other.dataPtr->capacity],
                ArrayDeleter<unsigned char>());
    std::memcpy(this->dataPtr->data.get(), _other.dataPtr->data.get(),
                _other.dataPtr->capacity);
  }
}

//////////////////////////////////////////////////
CompressedImage::~CompressedImage() = default;

//////////////////////////////////////////////////
const void *CompressedImage::Data() const
{
  return this->dataPtr->data.get();
}

//////////////////////////////////////////////////
unsigned int CompressedImage::Size() const
{
  return this->dataPtr->size;
}

//////////////////////////////////////////////////
unsigned int CompressedImage::Width() const
{
  return this->dataPtr->width;
}

//////////////////////////////////////////////////
unsigned int CompressedImage::Height() const
{
  return this->dataPtr->height;
}

//////////////////////////////////////////////////
ImageEncoding CompressedImage::Encoding() const
{
  return this->dataPtr->encoding;
}

//////////////////////////////////////////////////
ColorimetryInfo CompressedImage::Colorimetry() const
{
  return this->dataPtr->colorimetry;
}

//////////////////////////////////////////////////
bool CompressedImage::KeyFrame() const
{
  return this->dataPtr->keyFrame;
}

//////////////////////////////////////////////////
void CompressedImage::SetDimensions(unsigned int _width, unsigned int _height)
{
  this->dataPtr->width = _width;
  this->dataPtr->height = _height;
}

//////////////////////////////////////////////////
void CompressedImage::SetEncoding(ImageEncoding _encoding)
{
  this->dataPtr->encoding = _encoding;
}

//////////////////////////////////////////////////
void CompressedImage::SetColorimetry(const ColorimetryInfo &_info)
{
  this->dataPtr->colorimetry = _info;
}

//////////////////////////////////////////////////
void CompressedImage::SetKeyFrame(bool _keyFrame)
{
  this->dataPtr->keyFrame = _keyFrame;
}

//////////////////////////////////////////////////
unsigned char *CompressedImage::Reserve(unsigned int _size)
{
  if (_size > this->dataPtr->capacity)
  {
    this->dataPtr->data =
        DataPtr(new unsigned char[_size], ArrayDeleter<unsigned char>());
    this->dataPtr->capacity = _size;
  }
  this->dataPtr->size = _size;
  return this->dataPtr->data.get();
}
