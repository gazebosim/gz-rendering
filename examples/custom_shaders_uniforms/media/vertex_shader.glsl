varying vec3 interpolatedPosition;

void main()
{
  gl_Position = ftransform();
  interpolatedPosition = gl_Position.xyz;
}
