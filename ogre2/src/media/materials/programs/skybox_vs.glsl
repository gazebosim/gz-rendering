#version 330

in vec4 vertex;
in vec3 normal;
uniform mat4 worldViewProj;

out gl_PerVertex
{
    vec4 gl_Position;
};

out block
{
    vec3 cameraDir;
} outVs;

void main()
{
  gl_Position.xyw = (worldViewProj * vertex).xyw;
  gl_Position.z = 0.0;
  outVs.cameraDir.xyz = normal.xyz;
}
