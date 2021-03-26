uniform float testFloat;
uniform int testInt;
uniform float testFloatBuffer[6];
uniform int testIntBuffer[5];
uniform vec4 testVector4;
uniform mat4 textMatrix4;

void main()
{
  gl_FragColor = vec4(testFloat, 0, 0, 1.0);
}
