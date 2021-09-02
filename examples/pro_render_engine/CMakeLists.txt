cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

set(IGN_PLUGIN_VER 1)
set(IGN_COMMON_VER 3)

find_package(ignition-rendering5 REQUIRED)
find_package(ignition-plugin1 REQUIRED COMPONENTS all)

include_directories(/ignition-ws/shared/RadeonProRenderSDK/RadeonProRender/inc)
find_library(RADEON_PRO_RENDER RadeonProRender64 HINTS /ignition-ws/shared/RadeonProRenderSDK/RadeonProRender/binUbuntu18/)
find_library(RPR_LOAD_STORE RprLoadStore64 HINTS /ignition-ws/shared/RadeonProRenderSDK/RadeonProRender/binUbuntu18/)

add_library(ProRenderEngine SHARED ProRenderEngine.cc)
target_link_libraries(ProRenderEngine
  PUBLIC
    ${RADEON_PRO_RENDER}
    ${RPR_LOAD_STORE}
    ${IGNITION-RENDERING_LIBRARIES}
    ${ignition-common${IGN_COMMON_VER}_LIBRARIES}
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
