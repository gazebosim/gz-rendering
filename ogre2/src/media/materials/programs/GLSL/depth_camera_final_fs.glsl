/*
 * Copyright (C) 2020 Open Source Robotics Foundation
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

#version ogre_glsl_ver_330

vulkan_layout( location = 0 )
in block
{
  vec2 uv0;
} inPs;

vulkan_layout( ogre_t0 ) uniform utexture2D inputTexture;

vulkan_layout( location = 0 )
out uvec4 fragColor;

vulkan( layout( ogre_P0 ) uniform Params { )
	uniform float near;
	uniform float far;
	uniform float min;
	uniform float max;

	uniform vec4 texResolution;
	// 1 to pack the x, y, z of the input (the xyz + colour output) into a
	// target 3/4 as wide, rows without padding: texel u of a row holds floats
	// 4u to 4u+3 of the row's x, y, z triplets (an xyz PointCloudPacked)
	uniform float packXyz;
vulkan( }; )

void main()
{
  if (packXyz > 0.5)
  {
    // texResolution is the input's size
    int u = int(inPs.uv0.x * texResolution.x * 0.75);
    int v = int(inPs.uv0.y * texResolution.y);
    // the first float, 4u, is component r of point p
    int p = (4 * u) / 3;
    int r = u % 3;
    uvec4 a = texelFetch(inputTexture, ivec2(p, v), 0);
    // p + 1 < width: the last texel starts at component 2 of point
    // width - 2
    uvec4 b = texelFetch(inputTexture, ivec2(p + 1, v), 0);
    fragColor = r == 0 ? uvec4(a.xyz, b.x) :
        (r == 1 ? uvec4(a.yz, b.xy) : uvec4(a.z, b.xyz));
    return;
  }

  float tolerance = 1e-6;

  // Note: We use PFG_RGBA32_UINT because p.a contains an uint32 and sampling
  // (even w/ point filtering) causes p.a to loss information (e.g.
  // values close to 0 get rounded to 0)
  //
  // See https://github.com/gazebosim/gz-rendering/issues/332
  uvec4 p = texelFetch(inputTexture, ivec2(inPs.uv0 *texResolution.xy), 0);

  vec3 point = uintBitsToFloat(p.xyz);

  // Clamp again in case render passes changed depth values
  // to be outside of min/max range

  // clamp xyz
  if (!isinf(point.x) && length(point) > far - tolerance)
  {
    if (isinf(max))
    {
      point = vec3(max);
    }
    else
    {
      point.x = max;
    }
  }
  else if (point.x < near + tolerance)
  {
    if (isinf(min))
    {
      point = vec3(min);
    }
    else
    {
      point.x = min;
    }
  }

  fragColor = uvec4(floatBitsToUint(point), p.a);
}
