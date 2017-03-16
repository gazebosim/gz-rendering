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
#ifndef IGNITION_RENDERING_OPTIXLIGHTTYPES_HH_
#define IGNITION_RENDERING_OPTIXLIGHTTYPES_HH_

#include <optix_math.h>

#ifndef __CUDA_ARCH__
namespace ignition
{
  namespace rendering
  {
#endif

  typedef enum OptixLightType_t
  {
    LT_DIRECTIONAL = 0,
    LT_POINT       = 1,
    LT_SPOT        = 2,
    LT_COUNT       = 3,
  } OptixLightType;

  struct OptixLightColor
  {
    float4 diffuse;
    float4 specular;
  };

  struct OptixLightAttenuation
  {
    float range;
    float constant;
    float linear;
    float quadratic;
  };

  struct OptixLightSpot
  {
    float innerAngle;
    float outerAngle;
    float falloff;
  };

  struct OptixCommonLightData
  {
    float3 position;
    OptixLightColor color;
    OptixLightAttenuation atten;
    bool castShadows;
  };

  struct OptixDirectionalLightData
  {
    OptixCommonLightData common;
    float3 direction;
  };

  struct OptixPointLightData
  {
    OptixCommonLightData common;
  };

  struct OptixSpotLightData
  {
    OptixCommonLightData common;
    float3 direction;
    OptixLightSpot spot;
  };

#ifndef __CUDA_ARCH__
  }
}
#endif

#endif
