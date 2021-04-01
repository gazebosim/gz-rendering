#define M_PI 3.1415926535897932384626433832795

varying vec3 interpolatedPosition;

uniform int u_seed;
uniform vec2 u_resolution;
uniform vec3 u_color;
uniform mat4 u_adjustments;

float random(vec2 uv, float seed) {
  return fract(sin(mod(dot(uv, vec2(12.9898, 78.233)) + 1113.1 * seed, M_PI)) * 43758.5453);;
}

void main()
{
  vec3 a = vec3(u_adjustments[0][0], u_adjustments[1][0], u_adjustments[2][0]);
  vec2 b = vec2(distance(interpolatedPosition, a)) * u_adjustments[3][0];
  vec2 normalizedFragCoord = b / u_resolution;

  vec3 color = vec3(random(normalizedFragCoord, u_seed));
  gl_FragColor = vec4(color * u_color, 1.0);
}
