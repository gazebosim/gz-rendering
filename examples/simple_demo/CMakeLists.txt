cmake_minimum_required(VERSION 3.10.2 FATAL_ERROR)
project(ignition-rendering-simple-demo)

find_package(ignition-rendering5)

set (c++standard 17)

# find_package(GLUT REQUIRED)
# include_directories(SYSTEM ${GLUT_INCLUDE_DIRS})
# link_directories(${GLUT_LIBRARY_DIRS})
#
# find_package(OpenGL REQUIRED)
# include_directories(SYSTEM ${OpenGL_INCLUDE_DIRS})
# link_directories(${OpenGL_LIBRARY_DIRS})

find_package(FreeGLUT REQUIRED)

# if (NOT APPLE)
find_package(GLEW REQUIRED)

#   include_directories(SYSTEM ${GLEW_INCLUDE_DIRS})
   # include_directories(SYSTEM "C:/ProgramData/Anaconda3/Library/include")
   link_directories("C:/ProgramData/Anaconda3/Library/bin")
# endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

add_executable(simple_demo Main.cc GlutWindow.cc)

message("GLEW_LIBRARY_DIRS " ${GLEW_LIBRARY_DIRS})
message("GLEW_INCLUDE_DIRS " ${GLEW_INCLUDE_DIRS})
message("OpenGL_LIBRARY_DIRS " ${OpenGL_LIBRARY_DIRS})
message("OpenGL_INCLUDE_DIRS " ${OpenGL_INCLUDE_DIRS})
message("GLUT_LIBRARIES " ${GLUT_LIBRARIES})
message("OPENGL_LIBRARIES " ${OPENGL_LIBRARIES})
message("FreeGLUT " ${FreeGLUT_LIBRARIES})

message("GLEW_INCLUDE_DIRS " ${GLEW_INCLUDE_PATH})

set(IGN_COMMON_VER 4)
find_package(ignition-common${IGN_COMMON_VER} REQUIRED COMPONENTS events)

target_link_libraries(simple_demo
#  ${OPENGL_LIBRARIES}
  ${IGNITION-RENDERING_LIBRARIES}
  FreeGLUT::freeglut
	GLEW::glew
#  C:/ProgramData/Anaconda3/Library/lib/opengl32sw.lib
)

# add_executable(triangle WIN32 triangle.cpp)
# target_link_libraries(triangle FreeGLUT::freeglut SDL2::SDL2-static GLEW::glew)
