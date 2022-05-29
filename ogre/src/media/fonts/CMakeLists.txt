file(GLOB files "*ttf" "*png" "*fontdef")

install(FILES ${files} DESTINATION ${GZ_RENDERING_RESOURCE_PATH}/ogre/media/fonts)
install(DIRECTORY liberation-sans DESTINATION ${GZ_RENDERING_RESOURCE_PATH}/ogre/media/fonts)
