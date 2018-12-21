#version 330

in block
{
  vec2 uv0;
  vec3 cameraDir;
} inPs;

uniform sampler2D depthTexture;

out vec4 fragColor;

uniform vec2 projectionParams;
uniform float near;
uniform float far;
uniform float min;
uniform float max;

float getDepth(vec2 uv)
{
  float fDepth = texture(depthTexture, uv).x;
  float linearDepth = projectionParams.y / (fDepth - projectionParams.x);
  return linearDepth;
}


void main()
{
  float d = getDepth(inPs.uv0);
  vec3 viewSpacePos = inPs.cameraDir * d;
  float l = length(viewSpacePos);

  if (l > far)
    l = max;
  else if (l < near)
    l = min;

  fragColor = vec4(l, 0.0, 0, 1.0);
}
