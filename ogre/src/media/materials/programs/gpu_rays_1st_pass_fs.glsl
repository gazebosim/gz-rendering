uniform float retro;

uniform float near;
uniform float far;
uniform float max;

varying vec4 point;

void main()
{
  float l = length(point.xyz);

  if (l > far)
    l = max;

  gl_FragColor = vec4(l, retro, 0, 1.0);
}
