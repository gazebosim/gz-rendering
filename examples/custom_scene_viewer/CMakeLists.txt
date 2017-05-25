find_package(ignition-rendering0 REQUIRED)

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

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wfatal-errors")

add_executable(custom_scene_viewer
  DemoWindow.cc
  ManualSceneDemo.cc
  SceneBuilder.cc
)

target_link_libraries(custom_scene_viewer
  ${GLUT_LIBRARIES}
  ${OPENGL_LIBRARIES}
  ${GLEW_LIBRARIES}
  ${IGNITION-COMMON_LIBRARIES}
  ${IGNITION-RENDERING_LIBRARIES}
)

add_custom_command(TARGET custom_scene_viewer POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  ${CMAKE_SOURCE_DIR}/examples/custom_scene_viewer/media
  $<TARGET_FILE_DIR:custom_scene_viewer>/media)
