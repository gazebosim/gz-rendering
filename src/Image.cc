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
#include "ignition/rendering/Image.hh"

using namespace ignition;
using namespace rendering;

//////////////////////////////////////////////////
Image::Image(unsigned int _width, unsigned int _height, PixelFormat _format) :
  width(_width),
  height(_height)
{
  this->format = PixelUtil::Sanitize(_format);
  unsigned int size = this->GetMemorySize();
  this->data = DataPtr(new unsigned char[size]);
}

//////////////////////////////////////////////////
Image::~Image()
{
}

//////////////////////////////////////////////////
unsigned int Image::GetWidth() const
{
  return this->width;
}

//////////////////////////////////////////////////
unsigned int Image::GetHeight() const
{
  return this->height;
}

//////////////////////////////////////////////////
PixelFormat Image::GetFormat() const
{
  return this->format;
}

//////////////////////////////////////////////////
unsigned int Image::GetDepth() const
{
  return PixelUtil::GetChannelCount(this->format);
}

//////////////////////////////////////////////////
unsigned int Image::GetMemorySize() const
{
  return PixelUtil::GetMemorySize(this->format, this->width, this->height);
}

//////////////////////////////////////////////////
const void *Image::GetData() const
{
  return this->data.get();
}

//////////////////////////////////////////////////
void *Image::GetData()
{
  return this->data.get();
}
