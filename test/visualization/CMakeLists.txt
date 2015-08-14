include (${project_cmake_dir}/Utils.cmake)

include_directories(.)
link_directories(${CMAKE_BINARY_DIR}/src)

find_package(GLUT REQUIRED)
include_directories(${GLUT_INCLUDE_DIRS})
link_directories(${GLUT_LIBRARY_DIRS})

find_package(OpenGL REQUIRED)
include_directories(${OpenGL_INCLUDE_DIRS})
link_directories(${OpenGL_LIBRARY_DIRS})

find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIRS})
link_directories(${GLEW_LIBRARY_DIRS})

add_executable(main Main.cc GlutWindow.cc)

target_link_libraries(main
  ${GLUT_LIBRARIES}
  ${OPENGL_LIBRARIES}
  ${GLEW_LIBRARIES}
  ignition-rendering
)

add_executable(gz_demo GazeboDemo.cc CameraWindow.cc)

target_link_libraries(gz_demo
  ${GLUT_LIBRARIES}
  ${OPENGL_LIBRARIES}
  ${GLEW_LIBRARIES}
  ignition-rendering
)

add_executable(gz_world_demo GazeboWorldDemo.cc CameraWindow.cc)

target_link_libraries(gz_world_demo
  ${GLUT_LIBRARIES}
  ${OPENGL_LIBRARIES}
  ${GLEW_LIBRARIES}
  ignition-rendering
)

add_executable(man_demo ManualSceneDemo.cc SceneBuilder.cc DemoWindow.cc)

target_link_libraries(man_demo
  ${GLUT_LIBRARIES}
  ${OPENGL_LIBRARIES}
  ${GLEW_LIBRARIES}
  ignition-rendering
)
