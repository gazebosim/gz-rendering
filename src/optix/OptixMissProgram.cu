#include <optix.h>
#include <optix_math.h>
#include "ignition/rendering/optix/OptixRayTypes.hh"

rtDeclareVariable(float3, color, , );
rtDeclareVariable(OptixRadianceRayData, payload, rtPayload, );

RT_PROGRAM void Miss()
{
  payload.color = color;
}
