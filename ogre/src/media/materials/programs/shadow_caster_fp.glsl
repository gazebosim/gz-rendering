varying vec4 vertex_depth;

void main()
{
  // gl_FragCoord.z is already in [0, 1] (window space Z).
  // The PSSM receiver shader expects the shadow map to store depth in this
  // range (see SGXLib_IntegratedPSSM.glsl: shadowMapPos.z = z*0.5 + 0.5).
  // The previous implementation wrote vertex_depth.z/vertex_depth.w which is
  // in [-1, 1] and produced a consistently-zero shadow factor under
  // OGRE 1.12's RTSS receiver.
  float depth = gl_FragCoord.z;

  gl_FragColor = vec4(depth, depth, depth, 1.0);
}

