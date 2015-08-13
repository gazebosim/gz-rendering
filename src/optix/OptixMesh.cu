#include <optix.h>
#include <optixu/optixu_math.h>
#include <optixu/optixu_aabb.h>

rtBuffer<float3> vertexBuffer;
rtBuffer<float3> normalBuffer;
rtBuffer<float2> texCoordBuffer;
rtBuffer<int3>   indexBuffer;
rtDeclareVariable(float3, scale, , );

rtDeclareVariable(optix::Ray, ray, rtCurrentRay, );
rtDeclareVariable(float3, geometricNormal, attribute geometricNormal, );
rtDeclareVariable(float3, shadingNormal, attribute shadingNormal, );
rtDeclareVariable(float3, texcoord, attribute texcoord, );

RT_PROGRAM void Intersect(int primIndex)
{
  int3 indices = indexBuffer[primIndex];

  float3 p0 = scale * vertexBuffer[ indices.x ];
  float3 p1 = scale * vertexBuffer[ indices.y ];
  float3 p2 = scale * vertexBuffer[ indices.z ];

  float3 n;
  float t, beta, gamma;

  if (intersect_triangle(ray, p0, p1, p2, n, t, beta, gamma))
  {

    if (rtPotentialIntersection(t))
    {
      if (normalBuffer.size() == 0 || indices.x < 0 || indices.y < 0 ||
          indices.z < 0 )
      {
        shadingNormal = normalize(n);
      }
      else
      {
        float3 n0 = normalBuffer[ indices.x ] / scale;
        float3 n1 = normalBuffer[ indices.y ] / scale;
        float3 n2 = normalBuffer[ indices.z ] / scale;

        shadingNormal = normalize(n1 * beta + n2 * gamma + n0 *
          (1.0f - beta - gamma));
      }

      geometricNormal = normalize(n);

      if ( texCoordBuffer.size() == 0 || indices.x < 0 || indices.y < 0 ||
        indices.z < 0 )
      {
        texcoord = make_float3( 0.0f, 0.0f, 0.0f );
      }
      else
      {
        float2 t0 = texCoordBuffer[ indices.x ];
        float2 t1 = texCoordBuffer[ indices.y ];
        float2 t2 = texCoordBuffer[ indices.z ];

        texcoord = make_float3(t1 * beta + t2 * gamma +
            t0 * (1.0f - beta-gamma));
      }

      rtReportIntersection(0);
    }

  }
}

RT_PROGRAM void Bounds(int primIndex, float result[6])
{
  // TODO: precalculate once

  const int3 indices = indexBuffer[primIndex];

  const float3 v0   = vertexBuffer[ indices.x ];
  const float3 v1   = vertexBuffer[ indices.y ];
  const float3 v2   = vertexBuffer[ indices.z ];
  const float  area = length(cross(v1 - v0, v2 - v0));

  optix::Aabb* aabb = (optix::Aabb*)result;

  if(area > 0.0f && !isinf(area))
  {
    aabb->m_min = scale * fminf(fminf(v0, v1), v2 );
    aabb->m_max = scale * fmaxf(fmaxf(v0, v1), v2 );
  }
  else
  {
    aabb->invalidate();
  }
}
