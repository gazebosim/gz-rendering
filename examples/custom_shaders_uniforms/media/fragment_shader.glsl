uniform float testFloat;
uniform int testInt;
uniform float testFloatBuffer[6];
uniform int testIntBuffer[5];
uniform vec4 testVector4;
uniform mat4 textMatrix4;

void main()
{
  gl_FragColor = vec4(textMatrix4[0][0], textMatrix4[3][1], textMatrix4[3][3], 1.0);
}
