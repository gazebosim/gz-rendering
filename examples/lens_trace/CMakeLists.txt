cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

set(IGN_PLUGIN_VER 1)
set(IGN_COMMON_VER 3)

find_package(ignition-rendering5 REQUIRED)
find_package(ignition-plugin1 REQUIRED COMPONENTS all)

find_path(LENS_TRACE_INCLUDE_DIRECTORY
  NAMES lens_trace
  HINTS /usr/local/include/lens_trace
)

find_library(LENS_TRACE_LIBRARY
  NAMES lenstrace
  HINTS /usr/local/lib/lens_trace
)

find_library(LENS_TRACE_CUDA_LIBRARY
  NAMES lenstrace-cuda
  HINTS /usr/local/lib/lens_trace
)

include_directories(${LENS_TRACE_INCLUDE_DIRECTORY})

add_library(LensTraceEngine SHARED LensTraceEngine.cc)
target_link_libraries(LensTraceEngine
  PUBLIC
    ${IGNITION-RENDERING_LIBRARIES}
    ${ignition-common${IGN_COMMON_VER}_LIBRARIES}
    ${LENS_TRACE_LIBRARY}
    ${LENS_TRACE_CUDA_LIBRARY}
  PRIVATE
    ignition-plugin${IGN_PLUGIN_VER}::register
)

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

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")

add_executable(simple_demo Main.cc GlutWindow.cc)

target_link_libraries(simple_demo
  ${GLUT_LIBRARIES}
  ${OPENGL_LIBRARIES}
  ${GLEW_LIBRARIES}
  ${IGNITION-RENDERING_LIBRARIES}
  )