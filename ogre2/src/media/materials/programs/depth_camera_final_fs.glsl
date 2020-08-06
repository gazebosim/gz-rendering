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

#version 330

in block
{
  vec2 uv0;
} inPs;

uniform sampler2D inputTexture;

out vec4 fragColor;

uniform float near;
uniform float far;
uniform float min;
uniform float max;

void main()
{
  float tolerance = 1e-6;
  vec4 p = texture(inputTexture, inPs.uv0);

  vec3 point = p.xyz;

  // Clamp again in case render passes changed depth values
  // to be outside of min/max range

  // clamp xyz
  if (point.x > far - tolerance)
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

  fragColor = vec4(point, p.a);
}
