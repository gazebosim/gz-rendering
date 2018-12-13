/*uniform sampler2D tex1;
uniform sampler2D tex2;
uniform sampler2D tex3;

uniform vec4 texSize;
varying float tex;

void main()
{
  gl_FragColor = vec4(1.2, 2.3, 3.4, 1.0);

  if ((gl_TexCoord[0].s < 0.0) || (gl_TexCoord[0].s > 1.0) ||
      (gl_TexCoord[0].t < 0.0) || (gl_TexCoord[0].t > 1.0))
    gl_FragColor = vec4(1,1,1,1);
  else
  {
    int int_tex = int(tex * 1000.0);
    if (int_tex == 0)
      //gl_FragColor=vec4(12,34,56,1);
      gl_FragColor = texture2D( tex1, gl_TexCoord[0].st);
    else
      if (int_tex == 1)
        //gl_FragColor=vec4(2,1,0,1);
        gl_FragColor = texture2D( tex2, gl_TexCoord[0].st);
      else
        //gl_FragColor=vec4(3,2,1,1);
        gl_FragColor = texture2D( tex3, gl_TexCoord[0].st);
   }
}
*/

#version 330

in block
{
  vec2 uv0;
} inPs;


uniform sampler2D depthTexture;

uniform sampler2D tex1;
uniform sampler2D tex2;
uniform sampler2D tex3;

uniform float tex;
uniform vec2 projectionParams;

out vec4 fragColor;

float getDepth(vec2 uv)
{
  float fDepth = texture(depthTexture, uv).x;
  float linearDepth = projectionParams.y / (fDepth - projectionParams.x);
  return linearDepth;
}


void main()
{
  // fragColor = vec4(1.2, 2.3, 3.4, 1.0);

  float d = getDepth(inPs.uv0);

  // float d = texture(tex1, inPs.uv0).x;

  fragColor = vec4(d, 2.3, 3.4, 1.0);
  return;


  if ((inPs.uv0.x < 0.0) || (inPs.uv0.x > 1.0) ||
      (inPs.uv0.y < 0.0) || (inPs.uv0.y > 1.0))
    fragColor = vec4(1,1,1,1);
  else
  {
    int int_tex = int(tex * 1000.0);
    if (int_tex == 0)
      fragColor = texture(tex1, inPs.uv0);
    else
      if (int_tex == 1)
        fragColor = texture(tex2, inPs.uv0);
      else
        fragColor = texture(tex3, inPs.uv0);
   }
}


