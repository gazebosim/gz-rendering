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
#include "ignition/rendering/optix/OptixRenderTarget.hh"

#include "gazebo/common/Console.hh"
#include "ignition/rendering/optix/OptixConversions.hh"
#include "ignition/rendering/optix/OptixScene.hh"

using namespace ignition;
using namespace rendering;

//////////////////////////////////////////////////
// OptixRenderTarget
//////////////////////////////////////////////////
OptixRenderTarget::OptixRenderTarget()
{
}

//////////////////////////////////////////////////
OptixRenderTarget::~OptixRenderTarget()
{
}

//////////////////////////////////////////////////
void OptixRenderTarget::GetImage(Image &_image) const
{
  // TODO: handle Bayer conversions
  // TODO: move shared code to base 

  unsigned int width = this->GetWidth();
  unsigned int height = this->GetHeight();

  if (_image.GetWidth() != width || _image.GetHeight() != height)
  {
    gzerr << "Invalid image dimensions" << std::endl;
    return;
  }

  void* hostData = _image.GetData();
  void* deviceData = this->GetOptixBuffer()->map();
  std::memcpy(hostData, deviceData, this->GetMemorySize());
  this->GetOptixBuffer()->unmap();
}

//////////////////////////////////////////////////
unsigned int OptixRenderTarget::GetMemorySize() const
{
  unsigned int elementSize = this->GetOptixBuffer()->getElementSize();
  return elementSize * this->GetWidth() * this->GetHeight();
}

//////////////////////////////////////////////////
// OptixRenderTexture
//////////////////////////////////////////////////
OptixRenderTexture::OptixRenderTexture()
{
}

//////////////////////////////////////////////////
OptixRenderTexture::~OptixRenderTexture()
{
}

//////////////////////////////////////////////////
void OptixRenderTexture::Destroy()
{
}

//////////////////////////////////////////////////
optix::Buffer OptixRenderTexture::GetOptixBuffer() const
{
  return this->optixBuffer;
}

//////////////////////////////////////////////////
void OptixRenderTexture::RebuildImpl()
{
  this->optixBuffer->setSize(this->width, this->height);
}

//////////////////////////////////////////////////
void OptixRenderTexture::Init()
{
  BaseRenderTarget::Init();
  optix::Context optixContext = this->scene->GetOptixContext();
  this->optixBuffer = optixContext->createBuffer(RT_BUFFER_OUTPUT);
  this->optixBuffer->setFormat(RT_FORMAT_UNSIGNED_BYTE3);
}
