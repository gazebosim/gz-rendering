include (${project_cmake_dir}/Utils.cmake)

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

ign_build_tests(${gtest_sources})
ign_add_library(${PROJECT_NAME_LOWER}-ogre ${sources})
ign_install_library(${PROJECT_NAME_LOWER}-ogre)

target_link_libraries(${PROJECT_NAME_LOWER}-ogre
  ignition-rendering-base
  ${OGRE_LIBRARIES}
  ${OPENGL_LIBRARIES}
  ${OGRE_RTShaderSystem_LIBRARIES}
)
