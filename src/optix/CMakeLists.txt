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
  OptixTextureFactory.cc
  OptixVisual.cc
)

set(cuda_sources
  # OptixBox.cu
  # OptixCone.cu
  # OptixCylinder.cu
  OptixCamera.cu
  OptixErrorProgram.cu
  OptixMaterial.cu
  OptixMissProgram.cu
  OptixMesh.cu
  # OptixPlane.cu
  # OptixBackgroundColor.cu
  # OptixSphere.cu
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

cuda_compile_ptx(ptx_files ${cuda_sources})

add_custom_target(${PROJECT_NAME_LOWER}-optix-ptx ALL
                  DEPENDS ${ptx_files} ${cuda_sources}
                  SOURCES ${cuda_sources})

# set(media_dir "${CMAKE_INSTALL_PREFIX}/share/gazebo-${GAZEBO_VERSION}/media")
# install(FILES ${ptx_files} DESTINATION ${media_dir}/optix)
