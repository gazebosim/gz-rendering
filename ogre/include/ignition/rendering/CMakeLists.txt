# TODO find a better way to include the RenderSystems include dir
if (APPLE)
  message ("=------------------------------------ OGRE INCLUDES: ${OGRE_INCLUDE_DIRS}")
  include_directories(${OGRE_INCLUDE_DIRS}/RenderSystems)
endif()

ign_install_all_headers(COMPONENT ogre)
