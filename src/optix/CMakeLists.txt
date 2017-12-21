include (${project_cmake_dir}/Utils.cmake)

find_package(CUDA REQUIRED system filesystem)

include_directories(${CUDA_INCLUDE_DIRS})
link_directories(${CUDA_LIBRARY_DIRS})

set (sources OptixArrowVisual.cc
  OptixAxisVisual.cc
  OptixBox.cc
  OptixCamera.cc
  OptixCone.cc
  OptixConversions.cc
  OptixCylinder.cc
  OptixGeometry.cc
  OptixGrid.cc
  OptixLight.cc
  OptixLightManager.cc
  OptixMaterial.cc
  OptixMesh.cc
  OptixMeshFactory.cc
  OptixNode.cc
  OptixObject.cc
  OptixPrimitive.cc
  OptixRenderEngine.cc
  OptixRenderTarget.cc
  OptixScene.cc
  OptixSensor.cc
  OptixSphere.cc
  OptixTextureFactory.cc
  OptixVisual.cc
)

set(cuda_sources
  OptixBox.cu
  OptixCone.cu
  OptixCylinder.cu
  OptixCamera.cu
  OptixErrorProgram.cu
  OptixMaterial.cu
  OptixMissProgram.cu
  OptixMesh.cu
  # OptixPlane.cu
  # OptixBackgroundColor.cu
  OptixSphere.cu
)

set (gtest_sources
)

set_property(
  SOURCE OptixRenderEngine.cc
  PROPERTY COMPILE_DEFINITIONS
  IGN_RENDERING_RESOURCE_PATH="${IGN_RENDERING_RESOURCE_PATH}"
)

ign_build_tests(${gtest_sources})
ign_add_library(${PROJECT_NAME_LOWER}-optix ${sources})
ign_install_library(${PROJECT_NAME_LOWER}-optix)

target_link_libraries(${PROJECT_NAME_LOWER}-optix
  ignition-rendering-base
  ${IGNITION-MATH_LIBRARIES}
  ${IGNITION-COMMON_LIBRARIES}
  ${optix_LIBRARY}
)

if(${CUDA_VERSION} VERSION_LESS "9")
  set(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS};-gencode arch=compute_20,code=sm_20)
endif()

cuda_compile_ptx(ptx_files ${cuda_sources})

add_custom_target(${PROJECT_NAME_LOWER}-optix-ptx ALL
                  DEPENDS ${ptx_files} ${cuda_sources}
                  SOURCES ${cuda_sources})

set(ptx_dir "${IGN_RENDERING_RESOURCE_PATH}/optix")
install(FILES ${ptx_files} DESTINATION ${ptx_dir})
