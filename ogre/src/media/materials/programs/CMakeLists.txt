file(GLOB files "*.glsl")

install(FILES ${files} DESTINATION ${GZ_RENDERING_RESOURCE_PATH}/ogre/media/materials/programs)

