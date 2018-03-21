include (${project_cmake_dir}/Utils.cmake)

set (headers
  OgreArrowVisual.hh
  OgreAxisVisual.hh
  OgreCamera.hh
  OgreConversions.hh
  OgreGeometry.hh
  OgreGrid.hh
  OgreIncludes.hh
  OgreLight.hh
  OgreMaterial.hh
  OgreMeshFactory.hh
  OgreMesh.hh
  OgreNode.hh
  OgreObject.hh
  OgreRayQuery.hh
  OgreRenderEngine.hh
  OgreRenderTarget.hh
  OgreRenderTypes.hh
  OgreRTShaderSystem.hh
  OgreScene.hh
  OgreSensor.hh
  OgreStorage.hh
  OgreVisual.hh
  selection_buffer/OgreMaterialSwitcher.hh
  selection_buffer/OgreSelectionBuffer.hh
  selection_buffer/OgreSelectionRenderListener.hh
)

#################################################
### Do not change the following

set (ign_headers "" CACHE INTERNAL "Ignition headers" FORCE)
foreach (hdr ${headers})
  APPEND_TO_CACHED_STRING(ign_headers
    "Ignition headers" "#include <ignition/${IGN_PROJECT_NAME}/ogre/${hdr}>\n")
endforeach()

set(ign_group_in ${CMAKE_CURRENT_SOURCE_DIR}/ogre.hh.in)
set(ign_group_header ${CMAKE_CURRENT_BINARY_DIR}/ogre.hh)
configure_file (${ign_group_in} ${ign_group_header})

ign_install_includes("${IGN_PROJECT_NAME}${PROJECT_MAJOR_VERSION}/ignition/${IGN_PROJECT_NAME}/ogre" ${headers})
