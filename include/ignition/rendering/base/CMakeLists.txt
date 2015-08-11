include (${project_cmake_dir}/Utils.cmake)

set (headers
  BaseArrowVisual.hh
  BaseAxisVisual.hh
  BaseCamera.hh
  BaseGeometry.hh
  BaseLight.hh
  BaseMaterial.hh
  BaseMesh.hh
  BaseNode.hh
  BaseObject.hh
  BaseRenderEngine.hh
  BaseRenderTarget.hh
  BaseRenderTypes.hh
  BaseScene.hh
  BaseSensor.hh
  BaseStorage.hh
  BaseVisual.hh
)

#################################################
### Do not change the following

set (ign_headers "" CACHE INTERNAL "Ignition headers" FORCE)
foreach (hdr ${headers})
  APPEND_TO_CACHED_STRING(ign_headers
    "Ignition headers" "#include <ignition/${IGN_PROJECT_NAME}/base/${hdr}>\n")
endforeach()

set(ign_group_in ${CMAKE_CURRENT_SOURCE_DIR}/base.hh.in)
set(ign_group_header ${CMAKE_CURRENT_BINARY_DIR}/base.hh)
configure_file (${ign_group_in} ${ign_group_header})

ign_install_includes(${IGN_PROJECT_NAME}/base ${headers})
