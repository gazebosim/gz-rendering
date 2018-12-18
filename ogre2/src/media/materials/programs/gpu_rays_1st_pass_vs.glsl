/*varying vec4 point;

void main()
{
  gl_Position = ftransform();

  // Vertex in world space
   point = gl_ModelViewMatrix * gl_Vertex;
}
*/


#version 330

in vec4 vertex;
in vec2 uv0;
uniform mat4 worldViewProj;
uniform mat4 worldView;

out gl_PerVertex
{
	vec4 gl_Position;
};

out block
{
	vec2 uv0;
} outVs;

out vec4 point;

void main()
{
  gl_Position = worldViewProj * vertex;
  outVs.uv0.xy = uv0.xy;
  point = worldView * vertex;
}
