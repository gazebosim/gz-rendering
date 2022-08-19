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
#ifndef IGNITION_RENDERING_OPTIX_OPTIXRAYTYPES_HH_
#define IGNITION_RENDERING_OPTIX_OPTIXRAYTYPES_HH_

#include <optix.h>

#ifndef __CUDA_ARCH__
namespace ignition
{
  namespace rendering
  {
  inline namespace IGNITION_RENDERING_VERSION_NAMESPACE {
  //
#endif

  typedef enum OptixRayType_t
  {
    RT_RADIANCE = 0,
    RT_SHADOW   = 1,
    RT_COUNT    = 2,
  } OptixRayType;

  struct OptixRadianceRayData
  {
    float3 color;
    // cppcheck-suppress unusedStructMember
    float importance;
    // cppcheck-suppress unusedStructMember
    int depth;
  };

  struct OptixShadowRayData
  {
    float3 attenuation;
  };

#ifndef __CUDA_ARCH__
  }
  }
}
#endif

#endif
