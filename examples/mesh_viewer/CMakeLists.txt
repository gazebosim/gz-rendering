cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
project(ignition-rendering-mesh-viewer)
find_package(ignition-rendering0 REQUIRED)
find_package(ignition-common1 REQUIRED)

include_directories(SYSTEM
  ${PROJECT_BINARY_DIR}
  ${IGNITION-COMMON_INCLUDE_DIRS}
  ${IGNITION-RENDERING_INCLUDE_DIRS}
)

link_directories(
  ${IGNITION-COMMON_LIBRARY_DIRS}
  ${IGNITION-RENDERING_LIBRARY_DIRS}
)

find_package(GLUT REQUIRED)
include_directories(SYSTEM ${GLUT_INCLUDE_DIRS})
link_directories(${GLUT_LIBRARY_DIRS})

find_package(OpenGL REQUIRED)
include_directories(SYSTEM ${OpenGL_INCLUDE_DIRS})
link_directories(${OpenGL_LIBRARY_DIRS})

find_package(GLEW REQUIRED)
include_directories(SYSTEM ${GLEW_INCLUDE_DIRS})
link_directories(${GLEW_LIBRARY_DIRS})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")

configure_file (example_config.hh.in ${PROJECT_BINARY_DIR}/example_config.hh)

add_executable(mesh_viewer Main.cc GlutWindow.cc)

target_link_libraries(mesh_viewer
  ${GLUT_LIBRARIES}
  ${OPENGL_LIBRARIES}
  ${GLEW_LIBRARIES}
  ${IGNITION-COMMON_LIBRARIES}
  ${IGNITION-MATH_LIBRARIES}
  ${IGNITION-RENDERING_LIBRARIES}
)

add_custom_command(TARGET mesh_viewer POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  ${CMAKE_SOURCE_DIR}/media
  $<TARGET_FILE_DIR:mesh_viewer>/media)
