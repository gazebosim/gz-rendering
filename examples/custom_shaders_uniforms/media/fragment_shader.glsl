uniform float testFloat[3];
uniform int testInt;
uniform float testFloatBuffer[6];
uniform int testIntBuffer[5];
uniform vec4 testVector4;
uniform mat4 textMatrix4;

void main()
{
  gl_FragColor = vec4(testFloat[0], testFloat[1], testFloat[2], 1.0);
}
