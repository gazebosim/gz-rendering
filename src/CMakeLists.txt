include (${project_cmake_dir}/Utils.cmake)

add_subdirectory(base)

if (HAVE_OGRE)
  add_subdirectory(ogre)
endif ()

if (HAVE_OPTIX)
  add_subdirectory(optix)
endif ()

set (sources
  Image.cc
  MeshDescriptor.cc
  OrbitViewController.cc
  PixelFormat.cc
  RenderEngineManager.cc
  RenderingIface.cc
  ShaderType.cc
  SystemPaths.cc
)

set (gtest_sources
)

set (gtest_sources_dri
  RenderTarget_TEST.cc
)

set_property(
  SOURCE SystemPaths.cc
  PROPERTY COMPILE_DEFINITIONS
  IGN_RENDERING_PLUGIN_PATH="${IGN_RENDERING_PLUGIN_PATH}"
  OGRE_RESOURCE_PATH="${OGRE_RESOURCE_PATH}"
)

ign_build_tests(${gtest_sources})
ign_build_dri_tests(${gtest_sources_dri})
ign_add_library(${PROJECT_NAME_LOWER}${PROJECT_MAJOR_VERSION} ${sources})
ign_install_library(${PROJECT_NAME_LOWER}${PROJECT_MAJOR_VERSION})

if (HAVE_OGRE)
  target_link_libraries(${PROJECT_NAME_LOWER}${PROJECT_MAJOR_VERSION}
    ignition-rendering-ogre
)
endif ()

if (HAVE_OPTIX)
  target_link_libraries(${PROJECT_NAME_LOWER}${PROJECT_MAJOR_VERSION}
    ignition-rendering-optix
)
endif ()
