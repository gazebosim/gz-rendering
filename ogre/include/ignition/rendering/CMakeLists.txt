# TODO find a better way to include the RenderSystems include dir
#if (APPLE)
  message ("=------------------------------------ OGRE INCLUDE_DIRS: ${OGRE_INCLUDE_DIRS}")
  foreach (dir ${OGRE_INCLUDE_DIRS})
    get_filename_component(dir_name "${dir}" NAME)
    message(" i-----------------------   dir name ${dir_name}")
    if ("${dir_name}" STREQUAL "OGRE")
      set(dir_include "${dir}/RenderSystems/GL")
    else()
      set(dir_include "${dir}")
    endif()
    list(APPEND OGRE_INCLUDES ${dir_include})
  endforeach()

  message ("=------------------------------------ OGRE INCLUDES: ${OGRE_INCLUDES}")
  include_directories(${OGRE_INCLUDES})
#endif()

ign_install_all_headers(COMPONENT ogre)
