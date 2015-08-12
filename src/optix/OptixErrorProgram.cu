#include <optix.h>
#include <optix_math.h>

rtDeclareVariable(float3, color, , );
rtDeclareVariable(uint2, launchIndex, rtLaunchIndex, );
rtBuffer<float3, 2> buffer;

RT_PROGRAM void Error()
{
  buffer[launchIndex] = color;
}
