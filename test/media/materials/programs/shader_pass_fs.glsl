/*
 * Copyright (C) 2026 Open Source Robotics Foundation
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

// A shader pass for the tests: the image so far, plus the data texture
// times a gain, plus a tint.

in block
{
  vec2 uv0;
} inPs;

uniform sampler2D inputTexture;
uniform sampler2D dataTexture;
uniform vec3 tint;
uniform float gain;

out vec4 fragColor;

void main()
{
  vec4 image = texture(inputTexture, inPs.uv0);
  vec4 data = texture(dataTexture, inPs.uv0);
  fragColor = vec4(image.rgb + gain * data.rgb + tint, 1.0);
}
