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
#include "ignition/rendering/optix/OptixLight.hh"

#include "gazebo/common/Exception.hh"
#include "ignition/rendering/optix/OptixConversions.hh"
#include "ignition/rendering/optix/OptixLightManager.hh"
#include "ignition/rendering/optix/OptixIncludes.hh"
#include "ignition/rendering/optix/OptixScene.hh"

using namespace ignition;
using namespace rendering;

//////////////////////////////////////////////////
/// OptixLight
OptixLight::OptixLight()
{
}

//////////////////////////////////////////////////
OptixLight::~OptixLight()
{
}

//////////////////////////////////////////////////
gazebo::common::Color OptixLight::GetDiffuseColor() const
{
  float4 optixColor = this->GetCommonData().color.diffuse;;
  return OptixConversions::ConvertColor(optixColor);
}

//////////////////////////////////////////////////
void OptixLight::SetDiffuseColor(const gazebo::common::Color &_color)
{
  float4 optixColor = OptixConversions::ConvertColor(_color);
  this->GetCommonData().color.diffuse = optixColor;
}

//////////////////////////////////////////////////
gazebo::common::Color OptixLight::GetSpecularColor() const
{
  float4 optixColor = this->GetCommonData().color.specular;;
  return OptixConversions::ConvertColor(optixColor);
}

//////////////////////////////////////////////////
void OptixLight::SetSpecularColor(const gazebo::common::Color &_color)
{
  float4 optixColor = OptixConversions::ConvertColor(_color);
  this->GetCommonData().color.specular = optixColor;
}

//////////////////////////////////////////////////
double OptixLight::GetAttenuationConstant() const
{
  return this->GetCommonData().atten.constant;
}

//////////////////////////////////////////////////
void OptixLight::SetAttenuationConstant(double _value)
{
  this->GetCommonData().atten.constant = _value;
}

//////////////////////////////////////////////////
double OptixLight::GetAttenuationLinear() const
{
  return this->GetCommonData().atten.linear;
}

//////////////////////////////////////////////////
void OptixLight::SetAttenuationLinear(double _value)
{
  this->GetCommonData().atten.linear = _value;
}

//////////////////////////////////////////////////
double OptixLight::GetAttenuationQuadratic() const
{
  return this->GetCommonData().atten.quadratic;
}

//////////////////////////////////////////////////
void OptixLight::SetAttenuationQuadratic(double _value)
{
  this->GetCommonData().atten.quadratic = _value;
}

//////////////////////////////////////////////////
double OptixLight::GetAttenuationRange() const
{
  return this->GetCommonData().atten.range;
}

//////////////////////////////////////////////////
void OptixLight::SetAttenuationRange(double _range)
{
  this->GetCommonData().atten.range = _range;
}

//////////////////////////////////////////////////
bool OptixLight::GetCastShadows() const
{
  return this->GetCommonData().castShadows;
}

//////////////////////////////////////////////////
void OptixLight::SetCastShadows(bool _castShadows)
{
  this->GetCommonData().castShadows = _castShadows;
}

//////////////////////////////////////////////////
void OptixLight::WritePoseToDeviceImpl()
{
  BaseLight::WritePoseToDeviceImpl();

  // TODO: handle rotation
  math::Pose3d worldPose = this->GetWorldPose();
  OptixCommonLightData &_data = this->GetCommonData();
  _data.position.x = worldPose.Pos().X();
  _data.position.y = worldPose.Pos().Y();
  _data.position.z = worldPose.Pos().Z();
}

//////////////////////////////////////////////////
void OptixLight::Init()
{
  BaseLight::Init();
  this->Reset();
}

//////////////////////////////////////////////////
/// OptixDirectionalLight
OptixDirectionalLight::OptixDirectionalLight()
{
}

//////////////////////////////////////////////////
OptixDirectionalLight::~OptixDirectionalLight()
{
}

//////////////////////////////////////////////////
math::Vector3d OptixDirectionalLight::GetDirection() const
{
  return OptixConversions::ConvertVector(this->data.direction);
}

//////////////////////////////////////////////////
void OptixDirectionalLight::SetDirection(const math::Vector3d &_dir)
{
  this->data.direction = OptixConversions::ConvertVector(_dir);
}

//////////////////////////////////////////////////
OptixDirectionalLightData OptixDirectionalLight::GetData() const
{
  return this->data;
}

//////////////////////////////////////////////////
void OptixDirectionalLight::PreRender()
{
  BaseDirectionalLight::PreRender();
  OptixLightManagerPtr lightManager = this->scene->GetLightManager();
  lightManager->AddDirectionalLight(this->SharedThis());
}

//////////////////////////////////////////////////
OptixCommonLightData &OptixDirectionalLight::GetCommonData()
{
  return this->data.common;
}

//////////////////////////////////////////////////
const OptixCommonLightData &OptixDirectionalLight::GetCommonData() const
{
  return this->data.common;
}

//////////////////////////////////////////////////
OptixDirectionalLightPtr OptixDirectionalLight::SharedThis()
{
  ObjectPtr sharedBase = this->shared_from_this();
  return std::dynamic_pointer_cast<OptixDirectionalLight>(sharedBase);
}

//////////////////////////////////////////////////
/// OptixPointLight
OptixPointLight::OptixPointLight()
{
}

//////////////////////////////////////////////////
OptixPointLight::~OptixPointLight()
{
}

//////////////////////////////////////////////////
OptixPointLightData OptixPointLight::GetData() const
{
  return this->data;
}

//////////////////////////////////////////////////
void OptixPointLight::PreRender()
{
  BasePointLight::PreRender();
  OptixLightManagerPtr lightManager = this->scene->GetLightManager();
  lightManager->AddPointLight(this->SharedThis());
}

//////////////////////////////////////////////////
OptixCommonLightData &OptixPointLight::GetCommonData()
{
  return this->data.common;
}

//////////////////////////////////////////////////
const OptixCommonLightData &OptixPointLight::GetCommonData() const
{
  return this->data.common;
}

//////////////////////////////////////////////////
OptixPointLightPtr OptixPointLight::SharedThis()
{
  ObjectPtr sharedBase = this->shared_from_this();
  return std::dynamic_pointer_cast<OptixPointLight>(sharedBase);
}

//////////////////////////////////////////////////
// OptixSpotLight
//////////////////////////////////////////////////
OptixSpotLight::OptixSpotLight()
{
}

//////////////////////////////////////////////////
OptixSpotLight::~OptixSpotLight()
{
}

//////////////////////////////////////////////////
math::Vector3d OptixSpotLight::GetDirection() const
{
  return OptixConversions::ConvertVector(this->data.direction);
}

//////////////////////////////////////////////////
void OptixSpotLight::SetDirection(const math::Vector3d &_dir)
{
  this->data.direction = OptixConversions::ConvertVector(_dir);
}

//////////////////////////////////////////////////
math::Angle OptixSpotLight::GetInnerAngle() const
{
  return math::Angle(this->data.spot.outerAngle);
}

//////////////////////////////////////////////////
void OptixSpotLight::SetInnerAngle(const math::Angle &_angle)
{
  this->data.spot.outerAngle = _angle.Radian();
}

//////////////////////////////////////////////////
math::Angle OptixSpotLight::GetOuterAngle() const
{
  return math::Angle(this->data.spot.outerAngle);
}

//////////////////////////////////////////////////
void OptixSpotLight::SetOuterAngle(const math::Angle &_angle)
{
  this->data.spot.outerAngle = _angle.Radian();
}

//////////////////////////////////////////////////
double OptixSpotLight::GetFalloff() const
{
  return this->data.spot.falloff;
}

//////////////////////////////////////////////////
void OptixSpotLight::SetFalloff(double _falloff)
{
  this->data.spot.falloff = _falloff;
}

//////////////////////////////////////////////////
OptixSpotLightData OptixSpotLight::GetData() const
{
  return this->data;
}

//////////////////////////////////////////////////
void OptixSpotLight::PreRender()
{
  BaseSpotLight::PreRender();
  OptixLightManagerPtr lightManager = this->scene->GetLightManager();
  lightManager->AddSpotLight(this->SharedThis());
}

//////////////////////////////////////////////////
OptixCommonLightData &OptixSpotLight::GetCommonData()
{
  return this->data.common;
}

//////////////////////////////////////////////////
const OptixCommonLightData &OptixSpotLight::GetCommonData() const
{
  return this->data.common;
}

//////////////////////////////////////////////////
OptixSpotLightPtr OptixSpotLight::SharedThis()
{
  ObjectPtr sharedBase = this->shared_from_this();
  return std::dynamic_pointer_cast<OptixSpotLight>(sharedBase);
}
