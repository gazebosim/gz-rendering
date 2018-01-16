cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
project(ignition-rendering-simple-demo)
find_package(ignition-rendering0 REQUIRED)
find_package(ignition-common1 REQUIRED)

include_directories(SYSTEM
  ${IGNITION-RENDERING_INCLUDE_DIRS}
  ${IGNITION-COMMON_INCLUDE_DIRS}
)

link_directories(
  ${IGNITION-RENDERING_LIBRARY_DIRS}
  ${IGNITION-COMMON_LIBRARY_DIRS}
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

add_executable(simple_demo Main.cc GlutWindow.cc)

target_link_libraries(simple_demo
  ${GLUT_LIBRARIES}
  ${OPENGL_LIBRARIES}
  ${GLEW_LIBRARIES}
  ${IGNITION-COMMON_LIBRARIES}
  ${IGNITION-MATH_LIBRARIES}
  ${IGNITION-RENDERING_LIBRARIES}
)
