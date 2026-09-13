set (files
FFPLib_AlphaTest.glsl
FFPLib_Common.glsl
FFPLib_Fog.glsl
FFPLib_Texturing.glsl
FFPLib_Transform.glsl
SGXLib_DualQuaternion.glsl
SGXLib_IntegratedPSSM.glsl
SGXLib_LayeredBlending.glsl
SGXLib_NormalMap.glsl
SGXLib_PerPixelLighting.glsl
SGXLib_TextureAtlas.glsl
SGXLib_TriplanarTexturing.glsl
)

install(FILES ${files} DESTINATION ${GZ_RENDERING_RELATIVE_RESOURCE_PATH}/ogre/media/rtshaderlib150)

