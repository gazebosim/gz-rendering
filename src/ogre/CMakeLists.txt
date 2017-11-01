# Turn on C++11 support for the library.
set(CMAKE_CXX_STANDARD 11)


include_directories(SYSTEM
  ${OGRE_INCLUDE_DIRS}
)

link_directories(
  ${ogre_library_dirs}
)

# Collect source files into the "sources" variable and unit test files into the
# "gtest_sources" variable.
ign_get_libsources_and_unittests(sources gtest_sources)

# Create the library target.
# override library target name
set(TMP_PROJECT_LIBRARY_TARGET_NAME ${PROJECT_LIBRARY_TARGET_NAME})
set(PROJECT_LIBRARY_TARGET_NAME ${PROJECT_LIBRARY_TARGET_NAME}-ogre)
ign_add_library(${PROJECT_LIBRARY_TARGET_NAME} ${sources})

#include_directories(SYSTEM
#  ${OGRE_INCLUDE_DIRS}
#)

#link_directories(
#  ${ogre_library_dirs}
#)

find_package(OpenGL)

set_property(
  SOURCE OgreRenderEngine.cc OgreRTShaderSystem.cc
  PROPERTY COMPILE_DEFINITIONS
  IGN_RENDERING_RESOURCE_PATH="${IGN_RENDERING_RESOURCE_PATH}"
  OGRE_RESOURCE_PATH="${OGRE_RESOURCE_PATH}"
)

target_link_libraries(${PROJECT_LIBRARY_TARGET_NAME}
  ignition-rendering0-base
  ignition-common0::ignition-common0
  ${OPENGL_LIBRARIES}
  ${ogre_libraries}
)

# Create installation instructions for the library target. This must be called
# in the same scope that the target is created.
ign_install_library()

# Build the unit tests
ign_build_tests(TYPE UNIT SOURCES ${gtest_sources})

#restore library target name
set(PROJECT_LIBRARY_TARGET_NAME ${TMP_PROJECT_LIBRARY_TARGET_NAME})

add_subdirectory(media)
