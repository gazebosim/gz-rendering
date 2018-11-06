// Model Level Inputs
uniform mat4 worldViewProj;

void main()
{
  // Calculate output position
  gl_Position = worldViewProj * gl_Vertex;

  // Simply copy the input vertex UV to the output
  gl_TexCoord[0] = gl_MultiTexCoord0;
}
