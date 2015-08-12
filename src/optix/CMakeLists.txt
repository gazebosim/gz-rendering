include (${project_cmake_dir}/Utils.cmake)

set (sources
  OptixArrowVisual.cc
  OptixAxisVisual.cc
  OptixCamera.cc
  OptixConversions.cc
  OptixGeometry.cc
  OptixLight.cc
  OptixLightManager.cc
  OptixMaterial.cc
  OptixMesh.cc
  OptixMeshFactory.cc
  OptixNode.cc
  OptixObject.cc
  OptixRenderEngine.cc
  OptixRenderTarget.cc
  OptixScene.cc
  OptixSensor.cc
  OptixVisual.cc
)

set (gtest_sources
)

# TODO: remove after development
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wfatal-errors")

find_package(CUDA REQUIRED system filesystem)
include_directories(${CUDA_INCLUDE_DIRS})
link_directories(${CUDA_LIBRARY_DIRS})

ign_build_tests(${gtest_sources})
ign_add_library(${PROJECT_NAME_LOWER}-optix ${sources})
ign_install_library(${PROJECT_NAME_LOWER}-optix)

target_link_libraries(${PROJECT_NAME_LOWER}-optix
  ignition-rendering-base
  ${optix_LIBRARY}
)
