include (${project_cmake_dir}/Utils.cmake)

add_subdirectory(media)

include_directories(SYSTEM
  ${OGRE_INCLUDE_DIRS}
)

link_directories(
  ${ogre_library_dirs}
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
  OgreRayQuery.cc
  OgreRenderEngine.cc
  OgreRenderTarget.cc
  OgreRTShaderSystem.cc
  OgreScene.cc
  OgreSensor.cc
  OgreVisual.cc
)

set (gtest_sources
)


find_package(OpenGL)

set_property(
  SOURCE OgreRenderEngine.cc OgreRTShaderSystem.cc
  PROPERTY COMPILE_DEFINITIONS
  IGN_RENDERING_RESOURCE_PATH="${IGN_RENDERING_RESOURCE_PATH}"
  OGRE_RESOURCE_PATH="${OGRE_RESOURCE_PATH}"
)

ign_build_tests(${gtest_sources})
ign_add_library(${PROJECT_NAME_LOWER}-ogre ${sources})
ign_install_library(${PROJECT_NAME_LOWER}-ogre)

target_link_libraries(${PROJECT_NAME_LOWER}-ogre
  ignition-rendering-base
  ${IGNITION-MATH_LIBRARIES}
  ${IGNITION-COMMON_LIBRARIES}
  ${OPENGL_LIBRARIES}
  ${ogre_libraries}
)
