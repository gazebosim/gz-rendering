/*
 * Copyright (C) 2019 Open Source Robotics Foundation
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
uniform float range;
uniform float resolution;
uniform float ambient;
//uniform float tolerance;
//uniform vec3 backgroundColor;

float getDepth(vec2 uv)
{
  float fDepth = texture(depthTexture, uv).x;
  float linearDepth = projectionParams.y / (fDepth - projectionParams.x);
  return linearDepth;
}

void main()
{
  // temperature defaults to ambient
  float temp = ambient;
  float heatRange = range;

  // get depth
  // todo(anyone) work in linear space?
  float d = getDepth(inPs.uv0);
  d = d / (far-near);

  // check for heat source
  float heat = texture(colorTexture, inPs.uv0).x;
  if (heat > 0)
  {
    // heat is normalized so convert back to work in kelvin
    temp = heat * 655.35;

    // todo(anyone) expose this param?
    heatRange = 3;
  }

  // simulate ambient temp as a function of depth
  float delta = (1.0 - d) * heatRange;
  temp = temp - heatRange / 2.0 + delta;
  clamp(temp, min, max);

  // apply resolution factor
  temp /= resolution;
  // normalize
  temp /= 65535.0;

  fragColor = vec4(temp, 0, 0, 1.0);
  // fragColor = vec4(d, 0, 0, 1.0);

  // float heat = texture(colorTexture, inPs.uv0).x;
  // float heatRange = heat - ambient * 0.5;
  // float heatDelta = (1.0 - d) * heatRange;
  // float temp = ambient - range / 2.0 + delta;

  // fragColor = vec4(heat, 0, 0, 1.0);



//  // reconstruct 3d viewspace pos from depth
//  vec3 viewSpacePos = inPs.cameraDir * d;
//
//  // convert to z up
//  vec3 point = vec3(-viewSpacePos.z, -viewSpacePos.x, viewSpacePos.y);
//
//  // color
//  vec4 color = texture(colorTexture, inPs.uv0);
//
//  // clamp xyz and set rgb to background color
//  if (point.x > far - tolerance)
//  {
//    if (isinf(max))
//    {
//      point = vec3(max);
//    }
//    else
//    {
//      point.x = max;
//    }
//    color = vec4(backgroundColor, 1.0);
//  }
//  else if (point.x < near + tolerance)
//  {
//    if (isinf(min))
//    {
//      point = vec3(min);
//    }
//    else
//    {
//      point.x = min;
//    }
//    color = vec4(backgroundColor, 1.0);
//  }
//
//  // gamma correct - using same method as:
//  // https://bitbucket.org/sinbad/ogre/src/v2-1/Samples/Media/Hlms/Pbs/GLSL/PixelShader_ps.glsl#lines-513
//  color = sqrt(color);
//
//  float rgba = packFloat(color);
//  fragColor = vec4(point, rgba);
}
