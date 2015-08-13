#include <optix.h>
#include <optix_math.h>
#include "ignition/rendering/optix/OptixRayTypes.hh"

// camera variables
rtDeclareVariable(float3, eye, , );
rtDeclareVariable(float3,   u, , );
rtDeclareVariable(float3,   v, , );
rtDeclareVariable(float3,   w, , );
rtDeclareVariable(uint,    aa, , );
rtBuffer<float3, 2> buffer;

// current ray variables
rtDeclareVariable(uint2, launchIndex, rtLaunchIndex, );
rtDeclareVariable(uint2, launchDim, rtLaunchDim, );

// scene variables
rtDeclareVariable(rtObject, rootGroup, , );
rtDeclareVariable(float, sceneEpsilon, , );

RT_PROGRAM void Render()
{
  // setup subpixel data
  float step = 1.0 / aa;
  float2 pixel = make_float2(launchIndex) + (step / 2);
  float2 size  = make_float2(launchDim);
  int pixCount = aa * aa;

  // clear current pixel buffer
  buffer[launchIndex] = make_float3(0, 0, 0);

  float y0 = pixel.y;
  OptixRadianceRayData data;

  // process each subpixel column
  for (int x = 0; x < aa; ++x)
  {
    // process each subpixel row
    for (int y = 0; y < aa; ++y)
    {
      // create new ray for subpixel
      float2 ratio = pixel / size - 0.5;
      float3 direction = normalize(ratio.x * u + ratio.y * v + w);
      optix::Ray ray(eye, direction, RT_RADIANCE, sceneEpsilon);

      // reset ray data
      data.depth = 0;
      data.importance = 1;
      data.color = make_float3(0, 0, 0);

      // cast ray & update buffer
      rtTrace(rootGroup, ray, data);
      buffer[launchIndex] += data.color;

      // increment y
      pixel.y += step;
    }

    // increment x & reset y
    pixel.x += step;
    pixel.y = y0;
  }

  // compute mean of all subpixels
  buffer[launchIndex] /= pixCount;
}
