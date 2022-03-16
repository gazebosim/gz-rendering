#version 120

// Draws an 8-bit image using RGB values from a 256x1 palette texture.
// This vertex shader just passes through the UV texture coordinates.

attribute vec4 uv0;
varying vec2 UV;

void main()
{
  gl_Position = ftransform();
  UV = vec2(uv0);
}
