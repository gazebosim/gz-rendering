cmake_minimum_required(VERSION 3.10.2 FATAL_ERROR)
project(ignition-rendering-custom-shaders)

include_directories(SYSTEM
  ${PROJECT_BINARY_DIR}
)

find_package(ignition-rendering3 REQUIRED)

find_package(GLUT REQUIRED)
include_directories(SYSTEM ${GLUT_INCLUDE_DIRS})
link_directories(${GLUT_LIBRARY_DIRS})

find_package(OpenGL REQUIRED)
include_directories(SYSTEM ${OpenGL_INCLUDE_DIRS})
link_directories(${OpenGL_LIBRARY_DIRS})

if (NOT APPLE)
  find_package(GLEW REQUIRED)
  include_directories(SYSTEM ${GLEW_INCLUDE_DIRS})
  link_directories(${GLEW_LIBRARY_DIRS})
endif()

configure_file (example_config.hh.in ${PROJECT_BINARY_DIR}/example_config.hh)

set(IGN_RENDERING_RESOURCE_PATH ${CMAKE_INSTALL_PREFIX}/share/ignition/rendering-${PROJECT_VERSION_MAJOR})
set_property(
  SOURCE custom_shaders.cc
  PROPERTY COMPILE_DEFINITIONS
  IGN_RENDERING_RESOURCE_PATH="${IGN_RENDERING_RESOURCE_PATH}"
  OGRE_RESOURCE_PATH="${OGRE_RESOURCE_PATH}"
)

add_executable(custom_shaders custom_shaders.cc)

target_link_libraries(custom_shaders
  ${GLUT_LIBRARIES}
  ${OPENGL_LIBRARIES}
  ${GLEW_LIBRARIES}
  ${IGNITION-RENDERING_LIBRARIES}
)
