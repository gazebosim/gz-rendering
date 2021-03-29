cmake_minimum_required(VERSION 3.10.2 FATAL_ERROR)
project(ignition-rendering-custom_shaders_uniforms)

include_directories(SYSTEM
  ${PROJECT_BINARY_DIR}
)

find_package(ignition-rendering4)

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

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")

add_executable(custom_shaders_uniforms Main.cc GlutWindow.cc)

target_link_libraries(custom_shaders_uniforms
  ${GLUT_LIBRARIES}
  ${OPENGL_LIBRARIES}
  ${GLEW_LIBRARIES}
  ${IGNITION-RENDERING_LIBRARIES}
  )

add_custom_command(TARGET custom_shaders_uniforms POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  ${CMAKE_SOURCE_DIR}/media
  $<TARGET_FILE_DIR:custom_shaders_uniforms>/media)