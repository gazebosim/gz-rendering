// The code in this file is adapted from OGRE Samples. The OGRE's license and
// copyright header is included in skybox.material.
// See: ogre-next/Samples/2.0/scripts/materials/Common/Metal/SkyCubemap_ps.metal

#include <metal_stdlib>
using namespace metal;

struct PS_INPUT
{
	float3 cameraDir;
};

fragment float4 main_metal
(
	PS_INPUT inPs [[stage_in]],
	texturecube<float> skyCubemap [[texture(0)]],
	sampler samplerState [[sampler(0)]]
)
{
	//Cubemaps are left-handed
	return skyCubemap.sample( samplerState, float3( inPs.cameraDir.xy, -inPs.cameraDir.z ) ).xyzw;
}