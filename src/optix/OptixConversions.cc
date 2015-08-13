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
#include "ignition/rendering/optix/OptixConversions.hh"

using namespace ignition;
using namespace rendering;

//////////////////////////////////////////////////
float4 OptixConversions::ConvertColor(const gazebo::common::Color &_color)
{
  return make_float4(_color.r, _color.g, _color.b, _color.a);
}

//////////////////////////////////////////////////
gazebo::common::Color OptixConversions::ConvertColor(const float4 &_color)
{
  return gazebo::common::Color(_color.w, _color.x, _color.y, _color.z);
}

//////////////////////////////////////////////////
float3 OptixConversions::ConvertVector(const gazebo::math::Vector3 &_vector)
{
  return make_float3(_vector.x, _vector.y, _vector.z);
}

//////////////////////////////////////////////////
gazebo::math::Vector3 OptixConversions::ConvertVector(const float3 &_vector)
{
  return gazebo::math::Vector3(_vector.x, _vector.y, _vector.z);
}
