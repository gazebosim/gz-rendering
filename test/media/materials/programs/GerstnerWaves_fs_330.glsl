// Copyright (c) 2016 The UUV Simulator Authors.
// All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#version ogre_glsl_ver_330

////////// Input parameters //////////
// Textures
vulkan_layout( ogre_t0 ) uniform texture2D bumpMap;
vulkan_layout( ogre_t1 ) uniform textureCube cubeMap;

vulkan( layout( ogre_s0 ) uniform sampler texSampler0 );
vulkan( layout( ogre_s1 ) uniform sampler texSampler1 );

// Colors
vulkan( layout( ogre_P0 ) uniform Params { )
  uniform vec4 deepColor;
  uniform vec4 shallowColor;
  uniform float fresnelPower;
  uniform float hdrMultiplier;
vulkan( }; )

////////// Input computed in vertex shader //////////
vulkan_layout( location = 0 )
in block
{
  mat3 rotMatrix;
  vec3 eyeVec;
  vec2 bumpCoord;
} inPs;

vulkan_layout( location = 0 )
out vec4 fragColor;

void main()
{
  // Apply bump mapping to normal vector to make waves look more detailed:
  vec4 bump = texture(vkSampler2D(bumpMap, texSampler0),
                      inPs.bumpCoord)*2.0 - 1.0;
  vec3 N = normalize(inPs.rotMatrix * bump.xyz);

  // Reflected ray:
  vec3 E = normalize(inPs.eyeVec);
  vec3 R = reflect(E, N);

  // negate z for use with the skybox texture that comes with gz-rendering
  R = vec3(R.x, R.y, -R.z);

  // uncomment this line if using other textures that are Y up
  // Gazebo requires rotated cube map lookup.
  // R = vec3(R.x, R.z, R.y);

  // Get environment color of reflected ray:
  vec4 envColor = texture(vkSamplerCube(cubeMap, texSampler1), R, 0.0);

  // Cheap hdr effect:
  envColor.rgb *= (envColor.r+envColor.g+envColor.b)*hdrMultiplier;

  // Compute refraction ratio (Fresnel):
  float facing = 1.0 - dot(-E, N);
  float waterEnvRatio = clamp(pow(facing, fresnelPower), 0.0, 1.0);

  // Refracted ray only considers deep and shallow water colors:
  vec4 waterColor = mix(shallowColor, deepColor, facing);

  // Perform linear interpolation between reflection and refraction.
  vec4 color = mix(waterColor, envColor, waterEnvRatio);

  fragColor = vec4(color.xyz, 0.9);
}
