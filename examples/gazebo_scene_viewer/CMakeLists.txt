cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
project(ignition-rendering-gazebo-scene-viewer)
find_package(ignition-rendering0 REQUIRED)
find_package(gazebo REQUIRED)

include_directories(SYSTEM
  ${IGNITION-RENDERING_INCLUDE_DIRS}
)

link_directories(
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

add_executable(gazebo_scene_viewer
  CameraWindow.cc
  GazeboDemo.cc
  SceneManager.cc
)

target_link_libraries(gazebo_scene_viewer
  ${GLUT_LIBRARIES}
  ${OPENGL_LIBRARIES}
  ${GLEW_LIBRARIES}
  ${GAZEBO_LIBRARIES}
  ${IGNITION-COMMON_LIBRARIES}
  ${IGNITION-RENDERING_LIBRARIES}
)

add_executable(gazebo_scene_viewer2_demo
  CameraWindow.cc
  GazeboWorldDemo.cc
  SceneManager.cc
)

target_link_libraries(gazebo_scene_viewer2_demo
  ${GLUT_LIBRARIES}
  ${OPENGL_LIBRARIES}
  ${GLEW_LIBRARIES}
  ${GAZEBO_LIBRARIES}
  ${IGNITION-COMMON_LIBRARIES}
  ${IGNITION-RENDERING_LIBRARIES}
)
