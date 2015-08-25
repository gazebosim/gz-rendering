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

static __inline__ __device__ bool ReportPotentialIntersect(float _t,
    const float3 &_n, const float2 &_uv)
{
  if (rtPotentialIntersection(_t))
  {
    shadingNormal = geometricNormal = _n;
    texCoord = _uv;
    return rtReportIntersection(0);
  }

  return false;
}

RT_PROGRAM void Intersect(int)
{
  float height = scale.z;
  float radius = scale.x / 2; // TODO: handle scale.y

  float3 direction = ray.direction;
  float3 origin = ray.origin;
  origin.z -= scale.z / 2;

  float k = radius / height;
  float k2 = k * k;

  float a = direction.x * direction.x + direction.y * direction.y - direction.z
    * direction.z * k2;

  float b = 2 * (origin.x * direction.x + origin.y * direction.y - origin.z *
      direction.z * k2);

  float c = origin.x * origin.x + origin.y * origin.y - origin.z * origin.z *
    k2;

  float s = b * b - 4 * a * c;

  if (s < 0)
  {
    return;
  }

  float t1 = (-b + sqrt(s)) / (2 * a);
  float t2 = (-b - sqrt(s)) / (2 * a);

  float3 p1 = origin + t1 * direction;
  float3 p2 = origin + t2 * direction;

  float ymin = fminf(p1.z, p2.z);
  float ymax = fmaxf(p1.z, p2.z);

  if (ymin > 0 || ymax < -height || (ymin < -height && ymax > 0))
  {
    return;
  }

  float3 p = (t1 < t2) ? p1 : p2;
  float3 n;
  float2 uv;
  float t;

  float r2 = radius * radius;

  if (origin.z < -height)
  {
    t = (-height - origin.z) / direction.z;
    float3 p3 = origin + t * direction;

    if (p3.x * p3.x + p3.y * p3.y < r2)
    {
      n = make_float3(0, 0, -1);
      ReportPotentialIntersect(t, n, uv);
      return;
    }

    uv.x = atan2(p.y, p.x) / M_PI;
    uv.y = (p.x * p.x + p.y * p.y) / (radius * radius);
  }

  if (p.z > 0)
  {
    p = (t1 < t2) ? p2 : p1;
    t = (t1 < t2) ? t2 : t1;
    float r = radius * p.z / height;
    n = make_float3(0, 0, r * k) - p;
    n.z = -n.z;
    n = normalize(n);
    uv.x = atan2(p.y, p.x) / M_PI;
    uv.y = p.z;
  }
  else if (p.z < -height)
  {
    t = (-height - origin.z) / direction.z;
    p = origin + t * direction;
    n = make_float3(0, 0, -1);

    if (p.x * p.x + p.y * p.y > r2)
    {
      return;
    }

    uv.x = atan2(p.y, p.x) / M_PI;
    uv.y = (p.x * p.x + p.y * p.y) / (radius * radius);
  }
  else
  {
    t = fminf(t1, t2);
    float r = radius * p.z / height;
    n = make_float3(0, 0, r * k) - p;
    n.z = -n.z;
    n = normalize(n);
    uv.x = atan2(p.y, p.x) / M_PI;
    uv.y = p.z;
  }

  ReportPotentialIntersect(t, n, uv);
}

RT_PROGRAM void Bounds(int, float _result[6])
{
  float3 ex = scale / 2;
  optix::Aabb* aabb = (optix::Aabb*)_result;
  aabb->set(-ex, ex);
}
