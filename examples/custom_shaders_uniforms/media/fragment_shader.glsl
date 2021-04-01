#define M_PI 3.1415926535897932384626433832795

float random(vec2 uv, float seed) {
  return fract(sin(mod(dot(uv, vec2(12.9898, 78.233)) + 1113.1 * seed, M_PI)) * 43758.5453);;
}

uniform int frameCount;

void main()
{
  vec3 color = vec3(random(gl_FragCoord.xy, frameCount));
  gl_FragColor = vec4(color, 1.0);
}
