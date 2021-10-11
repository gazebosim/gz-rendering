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
 
// For details and documentation see: gpu_rays_1st_pass_fs.glsl

#include <metal_stdlib>
using namespace metal;

struct PS_INPUT
{
  float2 uv0;
  float3 cameraDir;
};

struct Params
{
  float2 projectionParams;
  float near;
  float far;
  float min;
  float max;
  float particleStddev;
  float particleScatterRatio;
  // rnd is a random number in the range of [0-1]
  float rnd;
};

// see gaussian_noise_fs.metal for documentation on the rand and gaussrand
// functions

#define PI 3.14159265358979323846264

float rand(float2 co)
{
  float r = fract(sin(dot(co.xy, float2(12.9898,78.233))) * 43758.5453);
  if(r == 0.0)
    return 0.000000000001;
  else
    return r;
}

float4 gaussrand(float2 co, float3 offsets, float mean, float stddev)
{
  float U, V, R, Z;
  U = rand(co + float2(offsets.x, offsets.x));
  V = rand(co + float2(offsets.y, offsets.y));
  R = rand(co + float2(offsets.z, offsets.z));
  if(R < 0.5)
    Z = sqrt(-2.0 * log(U)) * sin(2.0 * PI * V);
  else
    Z = sqrt(-2.0 * log(U)) * cos(2.0 * PI * V);
  Z = Z * stddev + mean;
  return float4(Z, Z, Z, 0.0);
}

fragment float4 main_metal
(
  PS_INPUT inPs [[stage_in]],
  texture2d<float>  depthTexture [[texture(0)]],
  texture2d<float>  colorTexture [[texture(1)]],
  texture2d<float>  particleDepthTexture [[texture(2)]],
  texture2d<float>  particleTexture [[texture(3)]],
  sampler           depthSampler [[sampler(0)]],
  sampler           colorSampler [[sampler(1)]],
  sampler           particleDepthSampler [[sampler(2)]],
  sampler           particleSampler [[sampler(3)]],
  constant Params &p [[buffer(PARAMETER_SLOT)]]
)
{

  float4 fragColor(0.5, 0, 0.5, 1);
  return fragColor;
}
