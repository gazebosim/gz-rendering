/*
 * Copyright (C) 2018 Open Source Robotics Foundation
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
  vec3 cameraDir;
} inPs;

uniform sampler2D depthTexture;
uniform sampler2D colorTexture;

out vec4 fragColor;

uniform vec2 projectionParams;
uniform float near;
uniform float far;
uniform float min;
uniform float max;
uniform float tolerance;
uniform vec3 backgroundColor;

float getDepth(vec2 uv)
{
  float fDepth = texture(depthTexture, uv).x;
  float linearDepth = projectionParams.y / (fDepth - projectionParams.x);
  return linearDepth;
}

float packFloat(vec4 color)
{
  // vec4 decode = vec4(1.0, 1/255.0, 1/65025.0, 1/160581375.0);
  // return dot(color, decode);

  int rgba = (int(color.x * 255.0) << 24) +
             (int(color.y * 255.0) << 16) +
             (int(color.z * 255.0) << 8) +
             int(color.w * 255.0);
  return intBitsToFloat(rgba);
}


void main()
{
  // get linear depth
  float d = getDepth(inPs.uv0);

  // reconstruct 3d viewspace pos from depth
  vec3 viewSpacePos = inPs.cameraDir * d;

  // get length of 3d point, i.e.range
  float l = length(viewSpacePos);

  // convert to z up
  vec3 point = vec3(-viewSpacePos.z, -viewSpacePos.x, viewSpacePos.y);

  // normalize - used to compute new xyz if l is clamped by near and far

  // color
  vec4 color = texture(colorTexture, inPs.uv0);

  // clamp
  if (l > far - tolerance)
  {
    float scale = max / l;
    point = scale * point;
    color = vec4(backgroundColor, 1.0);
  }
  else if (l < near + tolerance)
  {
    float scale = min / l;
    point = scale * point;
    color = vec4(backgroundColor, 1.0);
  }

  float rgba = packFloat(color);

  fragColor = vec4(point, rgba);
}
