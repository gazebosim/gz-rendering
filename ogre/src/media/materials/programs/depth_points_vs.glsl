varying vec4 position;

void main()
{
  gl_Position = ftransform();
  position = gl_Position;
  gl_TexCoord[0] = gl_MultiTexCoord0;
}

