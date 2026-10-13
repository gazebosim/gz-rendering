/*
 * Copyright (C) 2022 Open Source Robotics Foundation
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

#include <algorithm>
#include <cstring>

#include <gz/common/Console.hh>

#include "gz/rendering/Camera.hh"

namespace gz::rendering
{

Camera::~Camera() = default;

//////////////////////////////////////////////////
bool Camera::CopyTo(const PixelBuffer &_buffer) const
{
  if (_buffer.Data() == nullptr)
  {
    gzerr << "CopyTo: pixel buffer has a null data pointer" << std::endl;
    return false;
  }

  if (!_buffer.Valid())
  {
    gzerr << "CopyTo: pixel buffer too small, " << _buffer.MemorySize()
          << " bytes needed but " << _buffer.Size() << " given" << std::endl;
    return false;
  }

  if (_buffer.Width() != this->ImageWidth() ||
      _buffer.Height() != this->ImageHeight())
  {
    gzerr << "CopyTo: buffer is " << _buffer.Width() << "x"
          << _buffer.Height() << " but the camera image is "
          << this->ImageWidth() << "x" << this->ImageHeight() << std::endl;
    return false;
  }

  // Wrap the caller's buffer for the duration of this call only.
  Image wrapped(_buffer);
  this->Copy(wrapped);

  if (wrapped.Data() != _buffer.Data())
  {
    // An implementation replaced the image instead of writing into it.
    // In tree engines route every pixel write through a PixelBuffer so this
    // cannot happen there; keep the result correct for the ones that do not.
    gzerr << "Copy replaced its destination image instead of writing into "
          << "it. This is a bug in the render engine; copying the pixels "
          << "back." << std::endl;
    std::memcpy(_buffer.Data(), wrapped.Data(),
        std::min<std::size_t>(_buffer.Size(), wrapped.MemorySize()));
  }
  return true;
}

}  // namespace gz::rendering
