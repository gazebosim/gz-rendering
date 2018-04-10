file(GLOB files "*ttf" "*png" "*fontdef")

install(FILES ${files} DESTINATION ${IGN_RENDERING_RESOURCE_PATH}/ogre/media/fonts)
