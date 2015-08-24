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
#include <optix.h>
#include <optixu/optixu_math.h>
#include <optixu/optixu_aabb.h>

rtDeclareVariable(float3, scale, , );
rtDeclareVariable(optix::Ray, ray, rtCurrentRay, );
rtDeclareVariable(float3, geometricNormal, attribute geometricNormal, );
rtDeclareVariable(float3, shadingNormal, attribute shadingNormal, );
rtDeclareVariable(float2, texCoord, attribute texCoord, );

static __inline__ __device__ bool ReportPotentialIntersect(float t, float3 n,
    float2 uv)
{
  if (rtPotentialIntersection(t))
  {
    shadingNormal = geometricNormal = n;
    texCoord = uv;
    return rtReportIntersection(0);
  }

  return false;
}

RT_PROGRAM void Intersect(int)
{
  float hh = scale.z / 2;
  float radius = scale.x / 2; // TODO: handle scale.y
  float3 origin = ray.origin;
  float3 direction = ray.direction;

  float a = direction.x * direction.x + direction.y * direction.y;
  float b = 2 * (origin.x * direction.x + origin.y * direction.y);
  float c = origin.x * origin.x + origin.y * origin.y - radius * radius;

  float s = b * b - 4 * a * c;

  if (s < 0)
  {
    return;
  }

  float t1 = (-b + sqrt(s)) / (2 * a);
  float t2 = (-b - sqrt(s)) / (2 * a);

  float3 p1 = origin + t1 * direction;
  float3 p2 = origin + t2 * direction;

  float zmin = fminf(p1.z, p2.z);
  float zmax = fmaxf(p1.z, p2.z);

  if (zmin > hh || zmax < -hh)
  {
    return;
  }

  float3 p = (t1 < t2) ? p1 : p2;
  float3 n;
  float2 uv;
  float t;

  if (p.z > hh)
  {
    t = (hh - origin.z) / direction.z;
    p = origin + t * direction;
    n = make_float3(0, 0, 1);

    uv.x = atan2(p.y, p.x) / M_PI;
    uv.y = (p.x * p.x + p.y * p.y) / (radius * radius);
  }
  else if (p.z < -hh)
  {
    t = (-hh - origin.z) / direction.z;
    p = origin + t * direction;
    n = make_float3(0, 0, -1);

    uv.x = atan2(p.y, p.x) / M_PI;
    uv.y = (p.x * p.x + p.y * p.y) / (radius * radius);
  }
  else
  {
    t = fminf(t1, t2);
    n = make_float3(p.x, p.y, 0);
    n = normalize(n);
    uv.x = atan2(p.y, p.x) / M_PI;
    uv.y = p.z;
  }

  ReportPotentialIntersect(t, n, uv);
}

RT_PROGRAM void Bounds(int, float result[6])
{
  float3 ex = scale / 2;
  optix::Aabb* aabb = (optix::Aabb*)result;
  aabb->set(-ex, ex);
}
