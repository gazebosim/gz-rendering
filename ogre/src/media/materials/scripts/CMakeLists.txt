file(GLOB files "*.material" "*.program")

install(FILES ${files} DESTINATION ${IGN_RENDERING_RESOURCE_PATH}/ogre/media/materials/scripts)

