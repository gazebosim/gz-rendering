# TODO find a better way to include the RenderSystems include dir
if (APPLE)
  include_directories(${OGRE_INCLUDE_DIRS}/RenderSystems)
endif()

ign_install_all_headers(COMPONENT ogre)
