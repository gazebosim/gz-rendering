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
  PixelFormat.cc
  RenderEngineManager.cc
  RenderingIface.cc
  SceneManager.cc
  ShaderType.cc
)

set (gtest_sources
)

find_package(gazebo REQUIRED gazebo_common gazebo_math)
include_directories(${GAZEBO_INCLUDE_DIRS})
link_directories(${GAZEBO_LIBRARY_DIRS})

find_package(Boost REQUIRED filesystem)
include_directories(${BOOST_INCLUDE_DIRS})
link_directories(${BOOST_LIBRARY_DIRS})

ign_build_tests(${gtest_sources})
ign_add_library(${PROJECT_NAME_LOWER} ${sources})
ign_install_library(${PROJECT_NAME_LOWER})

target_link_libraries(${PROJECT_NAME_LOWER}
  ${GAZEBO_LIBRARIES}
  boost_filesystem
)

if (HAVE_OGRE)
  target_link_libraries(${PROJECT_NAME_LOWER}
    ignition-rendering-ogre
)
endif ()

if (HAVE_OPTIX)
  target_link_libraries(${PROJECT_NAME_LOWER}
    ignition-rendering-optix
)
endif ()
