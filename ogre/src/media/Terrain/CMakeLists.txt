# Terrain shader helpers copied verbatim from OGRE (Media/Terrain).
# The stock TerrainMaterialGeneratorA SM2Profile emits shaders that
# #include <TerrainTransforms.glsl> / <TerrainHelpers.glsl> (and
# <OgreUnifiedShader.h>, vendored under ../ShadowVolume). We vendor a pinned
# copy instead of pointing the resource group at the OGRE package Media tree,
# so the script versions never drift from what gz-rendering was tested
# against. See README.md for the source version.
set (files
  TerrainHelpers.glsl
  TerrainTransforms.glsl
)

install(FILES ${files}
  DESTINATION ${GZ_RENDERING_RELATIVE_RESOURCE_PATH}/ogre/media/Terrain)
