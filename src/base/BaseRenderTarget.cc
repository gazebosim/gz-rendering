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
#include "ignition/rendering/base/BaseRenderTarget.hh"

using namespace ignition;
using namespace rendering;

//////////////////////////////////////////////////
BaseRenderTarget::BaseRenderTarget()
{
}

//////////////////////////////////////////////////
BaseRenderTarget::~BaseRenderTarget()
{
}

//////////////////////////////////////////////////
BaseRenderTexture::BaseRenderTexture()
{
}

//////////////////////////////////////////////////
BaseRenderTexture::~BaseRenderTexture()
{
}

//////////////////////////////////////////////////
BaseRenderTextureBuilder::BaseRenderTextureBuilder()
{
}

//////////////////////////////////////////////////
BaseRenderTextureBuilder::~BaseRenderTextureBuilder()
{
}

//////////////////////////////////////////////////
unsigned int BaseRenderTextureBuilder::GetWidth() const
{
  return this->width;
}

//////////////////////////////////////////////////
void BaseRenderTextureBuilder::SetWidth(unsigned int _width)
{
  this->width = _width;
}

//////////////////////////////////////////////////
unsigned int BaseRenderTextureBuilder::GetHeight() const
{
  return this->height;
}

//////////////////////////////////////////////////
void BaseRenderTextureBuilder::SetHeight(unsigned int _height)
{
  this->height = _height;
}

//////////////////////////////////////////////////
void BaseRenderTextureBuilder::SetSize(unsigned int _width,
    unsigned int _height)
{
  this->width = _width;
  this->height = _height;
}

//////////////////////////////////////////////////
PixelFormat BaseRenderTextureBuilder::GetFormat() const
{
  return this->format;
}

//////////////////////////////////////////////////
void BaseRenderTextureBuilder::SetFormat(PixelFormat _format)
{
  this->format = PixelUtil::Sanitize(_format);
}

//////////////////////////////////////////////////
unsigned int BaseRenderTextureBuilder::GetDepth() const
{
  return PixelUtil::GetChannelCount(this->format);
}

//////////////////////////////////////////////////
unsigned int BaseRenderTextureBuilder::GetMemorySize() const
{
  return PixelUtil::GetMemorySize(this->format, this->width, this->height);
}

//////////////////////////////////////////////////
unsigned int BaseRenderTextureBuilder::GetAntiAliasing() const
{
  return this->aa;
}

//////////////////////////////////////////////////
void BaseRenderTextureBuilder::SetAntiAliasing(unsigned int _aa)
{
  this->aa = _aa;
}
