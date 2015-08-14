include (${project_cmake_dir}/Utils.cmake)

set (headers
  OptixArrowVisual.hh
  OptixAxisVisual.hh
  OptixCamera.hh
  OptixConversions.hh
  OptixGeometry.hh
  OptixIncludes.hh
  OptixLight.hh
  OptixLightManager.hh
  OptixLightTypes.hh
  OptixMaterial.hh
  OptixMesh.hh
  OptixMeshFactory.hh
  OptixNode.hh
  OptixObject.hh
  OptixRayTypes.hh
  OptixRenderEngine.hh
  OptixRenderTarget.hh
  OptixRenderTypes.hh
  OptixScene.hh
  OptixSensor.hh
  OptixStorage.hh
  OptixTextureFactory.hh
  OptixVisual.hh
)

#################################################
### Do not change the following

set (ign_headers "" CACHE INTERNAL "Ignition headers" FORCE)
foreach (hdr ${headers})
  APPEND_TO_CACHED_STRING(ign_headers
    "Ignition headers" "#include <ignition/${IGN_PROJECT_NAME}/optix/${hdr}>\n")
endforeach()

set(ign_group_in ${CMAKE_CURRENT_SOURCE_DIR}/optix.hh.in)
set(ign_group_header ${CMAKE_CURRENT_BINARY_DIR}/optix.hh)
configure_file (${ign_group_in} ${ign_group_header})

ign_install_includes(${IGN_PROJECT_NAME}/optix ${headers})
