# Collect source files into the "sources" variable and unit test files into the
# "gtest_sources" variable.
ign_get_libsources_and_unittests(sources gtest_sources)

include_directories(${CUDA_INCLUDE_DIRS})
link_directories(${CUDA_LIBRARY_DIRS})

# Needed by add_custom_target: ${optix_target}-ptx
include_directories("${PROJECT_SOURCE_DIR}/optix/include")
include_directories(SYSTEM ${OptiX_INCLUDE_DIRS})

ign_add_component(optix SOURCES ${sources} GET_TARGET_NAME optix_target)

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

set_property(
  SOURCE OptixRenderEngine.cc
  PROPERTY COMPILE_DEFINITIONS
  IGN_RENDERING_RESOURCE_PATH="${IGN_RENDERING_RESOURCE_PATH}"
)

target_link_libraries(${optix_target}
  PUBLIC
    ${ignition-common${IGN_COMMON_VER}_LIBRARIES}
  PRIVATE
    ignition-plugin${IGN_PLUGIN_VER}::register
    optix::optix
    optix::optixu
    optix::optix_prime)

# Build the unit tests
ign_build_tests(TYPE UNIT SOURCES ${gtest_sources})


if("${CUDA_VERSION}" VERSION_LESS "9")
  set(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS};-gencode arch=compute_20,code=sm_20)
endif()

cuda_compile_ptx(ptx_files ${cuda_sources})

add_custom_target(${optix_target}-ptx ALL
                  DEPENDS ${ptx_files} ${cuda_sources}
                  SOURCES ${cuda_sources})

set(ptx_dir "${IGN_RENDERING_RESOURCE_PATH}/optix")
install(FILES ${ptx_files} DESTINATION ${ptx_dir})


