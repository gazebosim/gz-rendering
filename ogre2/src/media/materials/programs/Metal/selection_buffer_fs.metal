/*
 * Copyright (C) 2018 Open Source Robotics Foundation
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
 
#include <metal_stdlib>
using namespace metal;

struct PS_INPUT
{
  float2 uv0;
	float3 cameraDir;
};

struct Params
{
	texture2d<float> colorTexture [[texture(0)]];
	texture2d<float> depthTexture [[texture(0)]];
  float2           projectionParams;
  float            far;
  float            inf;
};

fragment float4 main_metal
(
	PS_INPUT inPs [[stage_in]],
	constant Params &p [[buffer(PARAMETER_SLOT)]]
)
{
  return float4(0.5, 0, 0.5, 1.0);
}
