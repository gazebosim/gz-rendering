include (${project_cmake_dir}/Utils.cmake)

include_directories(.)
link_directories(${CMAKE_BINARY_DIR}/src)

find_package(GLUT REQUIRED)
include_directories(SYSTEM ${GLUT_INCLUDE_DIRS})
link_directories(${GLUT_LIBRARY_DIRS})

find_package(OpenGL REQUIRED)
include_directories(SYSTEM ${OpenGL_INCLUDE_DIRS})
link_directories(${OpenGL_LIBRARY_DIRS})

find_package(GLEW REQUIRED)
include_directories(SYSTEM ${GLEW_INCLUDE_DIRS})
link_directories(${GLEW_LIBRARY_DIRS})

add_executable(gazebo_scene_viewer GazeboDemo.cc CameraWindow.cc)

target_link_libraries(gazebo_scene_viewer
  ${GLUT_LIBRARIES}
  ${OPENGL_LIBRARIES}
  ${GLEW_LIBRARIES}
  ignition-rendering
)

add_executable(gazebo_scene_viewer2_demo GazeboWorldDemo.cc CameraWindow.cc)

target_link_libraries(gazebo_scene_viewer2_demo
  ${GLUT_LIBRARIES}
  ${OPENGL_LIBRARIES}
  ${GLEW_LIBRARIES}
  ignition-rendering
)
