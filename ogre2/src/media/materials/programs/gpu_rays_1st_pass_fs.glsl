#version 330

in block
{
  vec2 uv0;
} inPs;


uniform sampler2D depthTexture;
uniform vec2 projectionParams;
uniform float near;
uniform float far;
uniform float min;
uniform float max;

out vec4 fragColor;

float getDepth(vec2 uv)
{
  float fDepth = texture(depthTexture, uv).x;
  float linearDepth = projectionParams.y / (fDepth - projectionParams.x);
  return linearDepth;

/*
// formula taken from https://stackoverflow.com/questions/6652253
  float zb = texture(depthTexture, uv).x;
  float zn = 2.0 * zb - 1.0f;
  float ze = 2.0 * near * far / (far + near - zn *(far - near));
  return ze;
*/
}


void main()
{
  float d = getDepth(inPs.uv0);

  if (d > far)
    d = max;
  if (d < near)
    d = min;

  // todo(anyone) set retro values
  float retro = 0.0;

  fragColor = vec4(d, retro, 0, 1.0);
}
