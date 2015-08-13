#include <optix.h>
#include <optix_math.h>
#include "ignition/rendering/optix/OptixRayTypes.hh"
#include "ignition/rendering/optix/OptixLightTypes.hh"

// scene variables
rtDeclareVariable(float3, ambientLightColor, , );
rtDeclareVariable(int, maxReflectionDepth, , );
rtDeclareVariable(float, sceneEpsilon, , );
rtDeclareVariable(rtObject, rootGroup, , );
rtBuffer<OptixPointLightData> pointLights;

// material variables
rtDeclareVariable(float3, ambient, , );
rtDeclareVariable(float3, diffuse, , );
rtDeclareVariable(float, reflectivity, , );

// ray variables
rtDeclareVariable(optix::Ray, ray, rtCurrentRay, );
rtDeclareVariable(OptixRadianceRayData, radianceData, rtPayload, );
rtDeclareVariable(OptixShadowRayData, shadowData, rtPayload, );

// intersect variables
rtDeclareVariable(float, hitDist, rtIntersectionDistance, );
rtDeclareVariable(float3, geometricNormal, attribute geometricNormal, );
rtDeclareVariable(float3, shadingNormal, attribute shadingNormal, );

RT_PROGRAM void AnyHit()
{
  shadowData.attenuation = make_float3(0);
  rtTerminateRay();
}

RT_PROGRAM void ClosestHit()
{
  float3 color = ambient * ambientLightColor;
  // color += hitDist * make_float3(0.025, 0.025, 0.025);

  float3 worldGeomNorm = normalize(
      rtTransformNormal(RT_OBJECT_TO_WORLD, geometricNormal));

  float3 worldShadeNorm = normalize(
      rtTransformNormal(RT_OBJECT_TO_WORLD, shadingNormal));

  float3 forwardNormal = faceforward(worldShadeNorm, -ray.direction,
      worldGeomNorm);

  float3 hitPoint = ray.origin + hitDist * ray.direction;

  for (int i = 0; i < pointLights.size(); ++i)
  {
    OptixPointLightData light = pointLights[i];
    float3 l = normalize(light.common.position - hitPoint);
    float ndl = dot(forwardNormal, l);

    // int c = pointLights.size();
    // rtPrintf("%d / %d : %f, %f, %f\n", i, c, l.x, l.y, l.z);

    if (ndl > 0)
    {
      OptixShadowRayData data;
      data.attenuation = make_float3(1);
      float dist = length(light.common.position - hitPoint);
      optix::Ray shadowRay(hitPoint, l, RT_SHADOW, sceneEpsilon, dist);
      rtTrace(rootGroup, shadowRay, data);
      float3 attenuation = data.attenuation;

      if (fmaxf(attenuation) > 0)
      {
        // TODO: add light's attenuation
        float4 ld4 = light.common.color.diffuse;
        float3 Lc = make_float3(ld4.x, ld4.y, ld4.z) * attenuation;
        color += diffuse * ndl * Lc;

        float3 H = normalize(l - ray.direction);
        float nDh = dot( forwardNormal, H );

        if(nDh > 0)
        {
          // TODO: include material specular
          // float3 Ks = make_float3(0.8, 0.9, 0.8);
          float4 ks4 = light.common.color.specular;
          float3 Ks = make_float3(ks4.x, ks4.y, ks4.z) * attenuation;
          float phong_exp = 88;
          color += Ks * Lc * pow(nDh, phong_exp);
        }
      }
    }
  }

  if (reflectivity > 0 && radianceData.depth < maxReflectionDepth)
  {
    OptixRadianceRayData refData;
    refData.depth = radianceData.depth + 1;
    float3 R = reflect(ray.direction, forwardNormal);
    optix::Ray refRay(hitPoint, R, RT_RADIANCE, sceneEpsilon);
    rtTrace(rootGroup, refRay, refData);
    color += reflectivity * refData.color;
  }

  radianceData.color = color;
}
