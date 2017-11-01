ign_install_all_headers()

add_subdirectory(base)

if (HAVE_OGRE)
  add_subdirectory(ogre)
endif ()

if (HAVE_OPTIX)
  add_subdirectory(optix)
endif ()

