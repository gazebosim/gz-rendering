file(GLOB files "*.png" "*.jpg")

install(FILES ${files} DESTINATION ${IGN_RENDERING_RESOURCE_PATH}/ogre/media/materials/textures)

