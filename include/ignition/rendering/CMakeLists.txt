include (${project_cmake_dir}/Utils.cmake)

add_subdirectory(base)

if (HAVE_OGRE)
  add_subdirectory(ogre)
endif ()

if (HAVE_OPTIX)
  add_subdirectory(optix)
endif ()


set (headers
  ArrowVisual.hh
  AxisVisual.hh
  Camera.hh
  CompositeVisual.hh
  Geometry.hh
  Image.hh
  Light.hh
  Material.hh
  MeshDescriptor.hh
  Mesh.hh
  Node.hh
  Object.hh
  PixelFormat.hh
  RenderEngine.hh
  RenderEngineManager.hh
  RenderingIface.hh
  RenderTypes.hh
  Scene.hh
  SceneManager.hh
  Sensor.hh
  ShaderType.hh
  Storage.hh
  Util.hh
  Visual.hh
)

#################################################
### Do not change the following

set (ign_headers "" CACHE INTERNAL "Ignition headers" FORCE)
foreach (hdr ${headers})
  APPEND_TO_CACHED_STRING(ign_headers
    "Ignition headers" "#include <ignition/${IGN_PROJECT_NAME}/${hdr}>\n")
endforeach()

set(ign_group_header ${CMAKE_CURRENT_BINARY_DIR}/${IGN_PROJECT_NAME}.hh)
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/rendering.hh.in ${ign_group_header})

ign_install_includes("" ${ign_group_header})
ign_install_includes(${IGN_PROJECT_NAME} ${headers})
