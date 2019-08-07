varying vec4 position;

void main()
{
  // convert to z up
  vec3 point = vec3(position.z, -position.x, -position.y);

  float l = length(position.xyz);
  float scale = position.w / l;
  point = point * scale;

  gl_FragColor = vec4(point.xyz, 1.0);
}
