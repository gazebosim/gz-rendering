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
#include "ignition/rendering/optix/OptixCamera.hh"
#include "ignition/rendering/optix/OptixIncludes.hh"
#include "ignition/rendering/optix/OptixRenderTarget.hh"
#include "ignition/rendering/optix/OptixScene.hh"

using namespace ignition;
using namespace rendering;

//////////////////////////////////////////////////
const std::string OptixCamera::PTX_BASE_NAME("OptixCamera");

const std::string OptixCamera::PTX_RENDER_FUNCTION("Render");

//////////////////////////////////////////////////
OptixCamera::OptixCamera() :
  optixRenderProgram(NULL),
  optixImageBuffer(NULL),
  cameraBuffer(true),
  bufferDirty(true)
{
}

//////////////////////////////////////////////////
OptixCamera::~OptixCamera()
{
}

//////////////////////////////////////////////////
unsigned int OptixCamera::GetImageWidth() const
{
}

//////////////////////////////////////////////////
void OptixCamera::SetImageWidth(unsigned int _width)
{
  this->bufferDirty = true;
}

//////////////////////////////////////////////////
unsigned int OptixCamera::GetImageHeight() const
{
}

//////////////////////////////////////////////////
void OptixCamera::SetImageHeight(unsigned int _height)
{
  this->bufferDirty = true;
}

//////////////////////////////////////////////////
void OptixCamera::SetImageSize(unsigned int _width, unsigned int _height)
{
  this->bufferDirty = true;
}

//////////////////////////////////////////////////
PixelFormat OptixCamera::GetImageFormat() const
{
}

//////////////////////////////////////////////////
void OptixCamera::SetImageFormat(PixelFormat _format)
{
}

//////////////////////////////////////////////////
unsigned int OptixCamera::GetImageDepth() const
{
}

//////////////////////////////////////////////////
unsigned int OptixCamera::GetImageMemorySize() const
{
}

//////////////////////////////////////////////////
gazebo::math::Angle OptixCamera::GetHFOV() const
{
}

//////////////////////////////////////////////////
void OptixCamera::SetHFOV(const gazebo::math::Angle &_angle)
{
}

//////////////////////////////////////////////////
double OptixCamera::GetAspectRatio() const
{
}

//////////////////////////////////////////////////
void OptixCamera::SetAspectRatio(double _ratio)
{
}

//////////////////////////////////////////////////
unsigned int OptixCamera::GetAntiAliasing() const
{
}

//////////////////////////////////////////////////
void OptixCamera::SetAntiAliasing(unsigned int _aa)
{
}

//////////////////////////////////////////////////
void OptixCamera::PreRender()
{
  BaseCamera::PreRender();
  this->WriteCameraToDevice();
  this->WriteBufferToDevice();
}

//////////////////////////////////////////////////
void OptixCamera::WriteCameraToDevice()
{
  if (this->cameraDirty)
  {
    this->WriteCameraToDeviceImpl();
    this->cameraDirty = false;
  }
}

//////////////////////////////////////////////////
void OptixCamera::WriteCameraToDeviceImpl()
{
  this->optixRenderProgram["aspectRatio"] = this->aspectRatio;
  this->optixRenderProgram["antiAliasing"] = this->antiAliasing;
}

//////////////////////////////////////////////////
void OptixCamera::WriteBufferToDevice()
{
  if (this->bufferDirty)
  {
    this->WriteBufferToDeviceImpl();
    this->bufferDirty = false;
  }
}

//////////////////////////////////////////////////
void OptixCamera::WriteBufferToDeviceImpl()
{
  this->optixImageBuffer->setSize(imageWidth, imageHeight);

  delete image;
  image = new unsigned char[imageWidth * imageHeight * imageDepth];
}

//////////////////////////////////////////////////
void OptixCamera::WritePoseToDeviceImpl()
{
  BaseCamera::WritePoseToDeviceImpl();

  gazebo::math::Pose worldPose = this->GetWorldPose();
  gazebo::math::Vector3 pos = worldPose.pos;
  gazebo::math::Matrix3 rot = worldPose.rot.GetAsMatrix3();

  this->optixRenderProgram["eye"]->setFloat(pos.x, pos.y, pos.z);
  this->optixRenderProgram["u"]->setFloat(-rot[0][1], -rot[1][1], -rot[2][1]);
  this->optixRenderProgram["v"]->setFloat(-rot[0][2], -rot[1][2], -rot[2][2]);
  this->optixRenderProgram["w"]->setFloat( rot[0][0],  rot[1][0],  rot[2][0]);
}

//////////////////////////////////////////////////
void OptixCamera::Init()
{
  BaseCamera::Init();
  this->CreateRenderProgram();
  this->Reset();
}

//////////////////////////////////////////////////
void OptixCamera::CreateRenderProgram()
{
  this->optixRenderFunction =
      this->scene->CreatePtxProgram(PTX_BASE_NAME, PTX_RENDER_FUNCTION);

  optix::Context context = this->scene->GetOptixContext();
  context->setRayGenerationProgram(this->entryId, this->optixRenderProgram);

  this->optixImageBuffer = context->createBuffer(RT_BUFFER_OUTPUT);
  this->optixRenderProgram["buffer"]->setBuffer(imageBuffer);
  this->optixImageBuffer->setFormat(RT_FORMAT_FLOAT3);
}
