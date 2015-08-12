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

using namespace ignition;
using namespace rendering;

//////////////////////////////////////////////////
OptixRenderTarget::OptixRenderTarget()
{
}

//////////////////////////////////////////////////
OptixRenderTarget::~OptixRenderTarget()
{
}

//////////////////////////////////////////////////
unsigned int OptixRenderTarget::GetWidth() const
{
  return 0;
}

//////////////////////////////////////////////////
unsigned int OptixRenderTarget::GetHeight() const
{
  return 0;
}

//////////////////////////////////////////////////
void OptixRenderTarget::Update()
{
}

//////////////////////////////////////////////////
void OptixRenderTarget::Initialize()
{
}

//////////////////////////////////////////////////
OptixRenderTexture::OptixRenderTexture()
{
}

//////////////////////////////////////////////////
OptixRenderTexture::~OptixRenderTexture()
{
}

//////////////////////////////////////////////////
void OptixRenderTexture::GetData(void *data) const
{
}

//////////////////////////////////////////////////
void OptixRenderTexture::Destroy()
{
}

//////////////////////////////////////////////////  
OptixRenderTextureBuilder::OptixRenderTextureBuilder(OptixScenePtr _scene) :
  scene(_scene),
  name(""),
  backgroundColor(gazebo::common::Color::Black)
{
}

//////////////////////////////////////////////////
OptixRenderTextureBuilder::~OptixRenderTextureBuilder()
{
}

//////////////////////////////////////////////////
std::string OptixRenderTextureBuilder::GetName() const
{
  return this->name;
}

//////////////////////////////////////////////////
void OptixRenderTextureBuilder::SetName(const std::string &_name)
{
  this->name = _name;
}

//////////////////////////////////////////////////
void OptixRenderTextureBuilder::SetFormat(PixelFormat _format)
{
  _format = PixelUtil::Sanitize(_format);
  this->format = _format;
}

//////////////////////////////////////////////////
gazebo::common::Color OptixRenderTextureBuilder::GetBackgroundColor() const
{
  return this->backgroundColor;
}

//////////////////////////////////////////////////
void OptixRenderTextureBuilder::SetBackgroundColor(gazebo::common::Color _color)
{
  this->backgroundColor = _color;
}

//////////////////////////////////////////////////
BaseRenderTexturePtr OptixRenderTextureBuilder::Build() const
{
  return this->BuildSafe();
}

//////////////////////////////////////////////////
OptixRenderTexturePtr OptixRenderTextureBuilder::BuildSafe() const
{
  return NULL;
}
