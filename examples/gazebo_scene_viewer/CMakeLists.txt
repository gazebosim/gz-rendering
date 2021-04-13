cmake_minimum_required(VERSION 3.10.2 FATAL_ERROR)
project(ignition-rendering-gazebo-scene-viewer)
find_package(ignition-rendering5 REQUIRED)
find_package(gazebo REQUIRED)

include_directories(SYSTEM ${GAZEBO_INCLUDE_DIRS})

set(TARGET_THIRD_PARTY_DEPENDS "")

if (APPLE OR UNIX)
	find_package(GLUT REQUIRED)
	include_directories(SYSTEM ${GLUT_INCLUDE_DIRS})
	link_directories(${GLUT_LIBRARY_DIRS})

	find_package(OpenGL REQUIRED)
	include_directories(SYSTEM ${OpenGL_INCLUDE_DIRS})
	link_directories(${OpenGL_LIBRARY_DIRS})

	set(TARGET_THIRD_PARTY_DEPENDS
		${TARGET_THIRD_PARTY_DEPENDS}
		${OPENGL_LIBRARIES}
		${GLUT_LIBRARIES}
  )
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")
endif()

if (NOT APPLE)
  find_package(GLEW REQUIRED)
  set(TARGET_THIRD_PARTY_DEPENDS
    ${TARGET_THIRD_PARTY_DEPENDS}
    GLEW::glew
  )
  set(STD_CXX_FS_LIBRARIES "stdc++fs")
endif()

if (WIN32)
	find_package(FreeGLUT REQUIRED)
	set(TARGET_THIRD_PARTY_DEPENDS ${TARGET_THIRD_PARTY_DEPENDS} FreeGLUT::freeglut)
endif()

add_executable(gazebo_scene_viewer
  CameraWindow.cc
  GazeboDemo.cc
  SceneManager.cc
)

target_link_libraries(gazebo_scene_viewer
  ${IGNITION-RENDERING_LIBRARIES}
	${TARGET_THIRD_PARTY_DEPENDS}
  ${GAZEBO_LIBRARIES}
  ${STD_CXX_FS_LIBRARIES}
)

add_executable(gazebo_scene_viewer2_demo
  CameraWindow.cc
  GazeboWorldDemo.cc
  SceneManager.cc
)

target_link_libraries(gazebo_scene_viewer2_demo
  ${IGNITION-RENDERING_LIBRARIES}
  ${TARGET_THIRD_PARTY_DEPENDS}
  ${GAZEBO_LIBRARIES}
  ${IGNITION-RENDERING_LIBRARIES}
  ${STD_CXX_FS_LIBRARIES}
)
