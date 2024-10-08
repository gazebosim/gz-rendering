cmake_minimum_required(VERSION 3.22.1 FATAL_ERROR)

set(GZ_PLUGIN_VER 2)
set(GZ_COMMON_VER 5)

find_package(gz-rendering9 REQUIRED)
find_package(gz-plugin3 REQUIRED COMPONENTS all)

if (NOT DEFINED ENV{LUXCORE_SDK_PATH})
  message(FATAL_ERROR "Must define LUXCORE_SDK_PATH")
endif ()

include_directories($ENV{LUXCORE_SDK_PATH}/include)
find_library(LUXCORE_LIBRARY luxcore HINTS $ENV{LUXCORE_SDK_PATH}/lib/)

add_library(LuxCoreEngine SHARED
  LuxCoreEngineObject.cc
  LuxCoreEngineMaterial.cc
  LuxCoreEngineRenderTarget.cc
  LuxCoreEngineNode.cc
  LuxCoreEngineSensor.cc
  LuxCoreEngineCamera.cc
  LuxCoreEngineGeometry.cc
  LuxCoreEngineVisual.cc
  LuxCoreEngineMesh.cc
  LuxCoreEngineMeshFactory.cc
  LuxCoreEngineScene.cc
  LuxCoreEngineLight.cc
  LuxCoreEngine.cc)

target_link_libraries(LuxCoreEngine
  PUBLIC
    ${gz-common${GZ_COMMON_VER}_LIBRARIES}
    ${GZ-RENDERING_LIBRARIES}
    ${LUXCORE_LIBRARY}
  PRIVATE
    gz-plugin${GZ_PLUGIN_VER}::register
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

add_executable(luxcore_demo Main.cc GlutWindow.cc)

target_link_libraries(luxcore_demo
  ${GLUT_LIBRARIES}
  ${OPENGL_LIBRARIES}
  ${GLEW_LIBRARIES}
  ${GZ-RENDERING_LIBRARIES}
)

add_custom_target(copy_resources
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/media ${CMAKE_CURRENT_BINARY_DIR}/media
)

add_dependencies(luxcore_demo copy_resources)
