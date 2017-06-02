cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
project(ignition-rendering-examples)

add_subdirectory(simple_demo)
add_subdirectory(custom_scene_viewer)

########################################
find_package (gazebo)
if (NOT gazebo_FOUND)
  set (HAVE_GAZEBO OFF CACHE BOOL "HAVE GAZEBO" FORCE)
  message(STATUS "Looking for Gazebo- not found")
else()
  message(STATUS "Looking for Gazebo - found")
  include_directories (${GAZEBO_INCLUDE_DIRS})
  link_directories (${GAZEBO_LIBRARY_DIRS})
  set (HAVE_GAZEBO ON CACHE BOOL "HAVE GAZEBO" FORCE)
endif()

if (HAVE_GAZEBO)
  add_subdirectory(gazebo_scene_viewer)
endif ()
