# Stencil-shadow programs copied verbatim from OGRE (Media/ShadowVolume).
# OGRE 1.12 requires these at engine init (OgreShadowVolumeExtrudeProgram::
# initialise() looks up the Ogre/ShadowExtrude* programs in the "ShadowVolume"
# resource location). We vendor a pinned copy instead of pointing the resource
# group at the OGRE package Media tree, so the script versions never drift from
# what gz-rendering was tested against. See README.md for the source version.
set (files
  DefaultShaders.metal
  HLSL_SM4Support.hlsl
  OgreUnifiedShader.h
  ShadowBlend.frag
  ShadowBlend.vert
  ShadowExtrudeDirLight.vert
  ShadowExtrudeDirLightFinite.vert
  ShadowExtrudePointLight.vert
  ShadowExtrudePointLightFinite.vert
  ShadowVolumeExtude.program
  spot_shadow_fade.dds
)

install(FILES ${files}
  DESTINATION ${GZ_RENDERING_RELATIVE_RESOURCE_PATH}/ogre/media/ShadowVolume)
