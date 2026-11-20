/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

Copyright (c) 2000-2012 Torus Knot Software Ltd
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-----------------------------------------------------------------------------
*/
//-----------------------------------------------------------------------------
// Program Name: SGXLib_IntegratedPSSM
// Program Desc: Integrated PSSM functions.
// Program Type: Vertex/Pixel shader
// Language: GLSL
//-----------------------------------------------------------------------------

// Gazebo: keeps the interpolated 3x3 PCF and the 0.4 shadow floor of the
// previous version of this file, with the entry points updated to the
// signatures that the OGRE 1.12 RTSS IntegratedPSSM3 sub-render state
// generates calls for.

#ifdef PSSM_SAMPLE_CMP
#define SAMPLER_TYPE sampler2DShadow
#else
#define SAMPLER_TYPE sampler2D
#endif

//-----------------------------------------------------------------------------
void SGX_ApplyShadowFactor_Diffuse(in vec4 ambient,
					  in vec4 lightSum,
					  in float fShadowFactor,
					  out vec4 oLight)
{
	oLight.rgb = ambient.rgb + (lightSum.rgb - ambient.rgb) * (fShadowFactor);
	oLight.a   = lightSum.a;
}

//-----------------------------------------------------------------------------
// texture2D/shadow2D instead of texture(): the stock terrain material
// generator also includes this file from #version 120 programs, and the
// RTSS defines them to texture() for GLSL >= 130.
float texture2DCompare(SAMPLER_TYPE depths, vec2 uv, float compare)
{
#ifdef PSSM_SAMPLE_CMP
#ifdef OGRE_GLSLES
  return mix(0.4, 1.0, shadow2D(depths, vec3(uv, compare)));
#else
  return mix(0.4, 1.0, shadow2D(depths, vec3(uv, compare)).r);
#endif
#else
  float depth = texture2D(depths, uv).r;
  return (step(compare, depth) >= 1.0) ? 1.0 : 0.4;
#endif
}

//-----------------------------------------------------------------------------
float _SGX_ShadowPCF4(SAMPLER_TYPE shadowMap, vec4 shadowMapPos, vec2 offset)
{
  // Interpolated 3x3 PCF
  // Adapted from http://www.ogre3d.org/forums/viewtopic.php?f=1&t=78834

  if (offset.x <= 0.0 || offset.y <= 0.0 || shadowMapPos.w <= 0.0)
    return 1.0;

  shadowMapPos = shadowMapPos / shadowMapPos.w;
#if defined(PSSM_SAMPLE_CMP) && !defined(OGRE_REVERSED_Z)
  // A comparison sampler reads the hardware depth buffer, which holds
  // window depth in [0, 1]. The gz shadow caster writes NDC depth only for
  // the colour shadow map read by the default path.
  shadowMapPos.z = clamp(shadowMapPos.z * 0.5 + 0.5, 0.0, 1.0);
#endif
  vec2 uv = shadowMapPos.xy;

  vec2 texelSize = offset;
  vec2 size = 1.0 / offset;
  vec2 centroidUV = floor(uv * size + 0.5) / size;
  vec2 f = fract(uv * size + 0.5);

  // 3 x 3 kernel
  const int   X = 3;

  vec2 topLeft = centroidUV - texelSize * 1.5;

  // load all pixels needed for the computation
  // this way a pixel won't be loaded twice
  float kernel[9];
  for (int i = 0; i < X; ++i)
  {
    for (int j = 0; j < X; ++j)
    {
       kernel[i * X + j] = texture2DCompare(shadowMap,
          topLeft + vec2(i, j) * texelSize, shadowMapPos.z);
    }
  }

  float kernel_interpolated[4];

  kernel_interpolated[0] = kernel[0] + kernel[1] + kernel[3] + kernel[4];
  kernel_interpolated[0] /= 4.0;
  kernel_interpolated[1] = kernel[1] + kernel[2] + kernel[4] + kernel[5];
  kernel_interpolated[1] /= 4.0;
  kernel_interpolated[2] = kernel[3] + kernel[4] + kernel[6] + kernel[7];
  kernel_interpolated[2] /= 4.0;
  kernel_interpolated[3] = kernel[4] + kernel[5] + kernel[7] + kernel[8];
  kernel_interpolated[3] /= 4.0;

  float a = mix(kernel_interpolated[0], kernel_interpolated[1], f.y);
  float b = mix(kernel_interpolated[2], kernel_interpolated[3], f.y);
  float c = mix(a, b, f.x);
  return c;
}

//-----------------------------------------------------------------------------
// Called by the RTSS when there is a single shadow texture
void SGX_ShadowPCF4(in SAMPLER_TYPE shadowMap, in vec4 shadowMapPos,
                    in vec2 invTexSize, out float c)
{
  c = _SGX_ShadowPCF4(shadowMap, shadowMapPos, invTexSize);
}

//-----------------------------------------------------------------------------
// The RTSS passes (light position, shadow map, inverse texture size) for
// each of the PSSM_NUM_SPLITS splits. The last split always uses the
// "3" parameters.
void SGX_ComputeShadowFactor_PSSM3(in float fDepth,
							in vec4 vSplitPoints,
							in vec4 lightPosition0,
							in SAMPLER_TYPE shadowMap0,
							in vec2 invShadowMapSize0,
#if PSSM_NUM_SPLITS > 2
							in vec4 lightPosition1,
							in SAMPLER_TYPE shadowMap1,
							in vec2 invShadowMapSize1,
#endif
#if PSSM_NUM_SPLITS > 3
							in vec4 lightPosition2,
							in SAMPLER_TYPE shadowMap2,
							in vec2 invShadowMapSize2,
#endif
							in vec4 lightPosition3,
							in SAMPLER_TYPE shadowMap3,
							in vec2 invShadowMapSize3,
							out float oShadowFactor)
{
  if (fDepth  <= vSplitPoints.x)
  {
    oShadowFactor =
      _SGX_ShadowPCF4(shadowMap0, lightPosition0, invShadowMapSize0);
  }
#if PSSM_NUM_SPLITS > 2
  else if (fDepth <= vSplitPoints.y)
  {
    oShadowFactor =
      _SGX_ShadowPCF4(shadowMap1, lightPosition1, invShadowMapSize1);
  }
#endif
#if PSSM_NUM_SPLITS > 3
  else if (fDepth <= vSplitPoints.z)
  {
    oShadowFactor =
      _SGX_ShadowPCF4(shadowMap2, lightPosition2, invShadowMapSize2);
  }
#endif
  else
  {
    oShadowFactor =
      _SGX_ShadowPCF4(shadowMap3, lightPosition3, invShadowMapSize3);
  }
}
