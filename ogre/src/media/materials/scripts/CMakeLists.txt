file(GLOB files "*.material" "*.compositor")

install(FILES ${files} DESTINATION ${GZ_RENDERING_RESOURCE_PATH}/ogre/media/materials/scripts)

