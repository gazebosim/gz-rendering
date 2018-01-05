include (${project_cmake_dir}/Utils.cmake)

########################################
# Find Boost, if not specified manually
include(FindBoost)
find_package(Boost REQUIRED filesystem)
if (NOT Boost_FOUND)
  BUILD_ERROR ("Boost not found. Please install boost filesystem.")
endif()


include_directories(SYSTEM
  ${OGRE_INCLUDE_DIRS}
  ${Boost_INCLUDE_DIRS}
)

link_directories(
  ${ogre_library_dirs}
  ${Boost_LIBRARY_DIRS}
)

set (sources
  OgreArrowVisual.cc
  OgreAxisVisual.cc
  OgreCamera.cc
  OgreConversions.cc
  OgreGeometry.cc
  OgreLight.cc
  OgreMaterial.cc
  OgreMesh.cc
  OgreMeshFactory.cc
  OgreNode.cc
  OgreObject.cc
  OgreRenderEngine.cc
  OgreRenderTarget.cc
  OgreRTShaderSystem.cc
  OgreScene.cc
  OgreSensor.cc
  OgreVisual.cc
)

set (gtest_sources
)

add_subdirectory(selection_buffer)
find_package(OpenGL)

ign_build_tests(${gtest_sources})
ign_add_library(${PROJECT_NAME_LOWER}-ogre ${sources})
ign_install_library(${PROJECT_NAME_LOWER}-ogre)

target_link_libraries(${PROJECT_NAME_LOWER}-ogre
  ignition-rendering-base
  ${IGNITION-MATH_LIBRARIES}
  ${IGNITION-COMMON_LIBRARIES}
  ${OPENGL_LIBRARIES}
  ${ogre_libraries}
  ${Boost_LIBRARIES}
)
