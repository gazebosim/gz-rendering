include (${project_cmake_dir}/Utils.cmake)

add_subdirectory(base)
add_subdirectory(ogre)

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

find_package(Boost REQUIRED system filesystem)
include_directories(${BOOST_INCLUDE_DIRS})
link_directories(${BOOST_LIBRARY_DIRS})

ign_build_tests(${gtest_sources})
ign_add_library(${PROJECT_NAME_LOWER} ${sources})
ign_install_library(${PROJECT_NAME_LOWER})

target_link_libraries(${PROJECT_NAME_LOWER}
  ${GAZEBO_LIBRARIES}
  boost_system
  ignition-rendering-ogre
)
