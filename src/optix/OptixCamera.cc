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
  renderTexture(NULL),
  cameraDirty(true),
  entryId(0)
{
}

//////////////////////////////////////////////////
OptixCamera::~OptixCamera()
{
}

//////////////////////////////////////////////////
PixelFormat OptixCamera::GetImageFormat() const
{
  return PF_UNKNOWN;
}

//////////////////////////////////////////////////
void OptixCamera::SetImageFormat(PixelFormat /*_format*/)
{
}

//////////////////////////////////////////////////
gazebo::math::Angle OptixCamera::GetHFOV() const
{
  return this->xFieldOfView;
}

//////////////////////////////////////////////////
void OptixCamera::SetHFOV(const gazebo::math::Angle &_angle)
{
  this->xFieldOfView = _angle;
  this->poseDirty = true;
}

//////////////////////////////////////////////////
double OptixCamera::GetAspectRatio() const
{
  return this->aspectRatio;
}

//////////////////////////////////////////////////
void OptixCamera::SetAspectRatio(double _ratio)
{
  this->aspectRatio = _ratio;
  this->cameraDirty = true;
}

//////////////////////////////////////////////////
unsigned int OptixCamera::GetAntiAliasing() const
{
  return this->antiAliasing;
}

//////////////////////////////////////////////////
void OptixCamera::SetAntiAliasing(unsigned int _aa)
{
  this->antiAliasing = _aa;
  this->cameraDirty = true;
}

//////////////////////////////////////////////////
void OptixCamera::PreRender()
{
  BaseCamera::PreRender();
  this->WriteCameraToDevice();
}

//////////////////////////////////////////////////
void OptixCamera::Render()
{
  unsigned int width = this->GetImageWidth();
  unsigned int height = this->GetImageHeight();
  optix::Context optixContext = this->scene->GetOptixContext();
  optixContext->launch(this->entryId, width, height);
}

//////////////////////////////////////////////////
RenderTexturePtr OptixCamera::GetRenderTexture() const
{
  return this->renderTexture;
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
  this->optixRenderProgram["aspectRatio"]->setFloat(this->aspectRatio);
  this->optixRenderProgram["antiAliasing"]->setFloat(antiAliasing);
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
  this->CreateRenderTexture();
  this->CreateRenderProgram();
  this->Reset();
}

//////////////////////////////////////////////////
void OptixCamera::CreateRenderTexture()
{
  RenderTexturePtr base = this->scene->CreateRenderTexture();
  this->renderTexture = boost::dynamic_pointer_cast<OptixRenderTexture>(base);
  this->renderTexture->SetFormat(PF_R8G8B8);
}

//////////////////////////////////////////////////
void OptixCamera::CreateRenderProgram()
{
  this->optixRenderProgram =
      this->scene->CreateOptixProgram(PTX_BASE_NAME, PTX_RENDER_FUNCTION);

  optix::Context context = this->scene->GetOptixContext();
  context->setRayGenerationProgram(this->entryId, this->optixRenderProgram);

  optix::Buffer optixBuffer = this->renderTexture->GetOptixBuffer();
  this->optixRenderProgram["buffer"]->setBuffer(optixBuffer);
}
