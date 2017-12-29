# Turn on C++11 support for the library.
set(CMAKE_CXX_STANDARD 11)

# Collect source files into the "sources" variable and unit test files into the
# "gtest_sources" variable.
ign_get_libsources_and_unittests(sources gtest_sources)

# Create the library target.
# override library target name
set(TMP_PROJECT_LIBRARY_TARGET_NAME ${PROJECT_LIBRARY_TARGET_NAME})
set(PROJECT_LIBRARY_TARGET_NAME ${PROJECT_LIBRARY_TARGET_NAME}-optix)

ign_add_library(${PROJECT_LIBRARY_TARGET_NAME} ${sources})

find_package(CUDA REQUIRED system filesystem)

include_directories(${CUDA_INCLUDE_DIRS})
link_directories(${CUDA_LIBRARY_DIRS})

#include_directories(SYSTEM
#  ${OGRE_INCLUDE_DIRS}
#)

#link_directories(
#  ${ogre_library_dirs}
#)

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

target_link_libraries(${PROJECT_LIBRARY_TARGET_NAME}
  ignition-rendering0-base
  ignition-common0::ignition-common0
  ${optix_LIBRARY}
)

# Create installation instructions for the library target. This must be called
# in the same scope that the target is created.
ign_install_library()

# Build the unit tests
ign_build_tests(TYPE UNIT SOURCES ${gtest_sources})


if(${CUDA_VERSION} VERSION_LESS "9")
  set(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS};-gencode arch=compute_20,code=sm_20)
endif()

cuda_compile_ptx(ptx_files ${cuda_sources})

add_custom_target(${PROJECT_LIBRARY_TARGET_NAME}-ptx ALL
                  DEPENDS ${ptx_files} ${cuda_sources}
                  SOURCES ${cuda_sources})

set(ptx_dir "${IGN_RENDERING_RESOURCE_PATH}/optix")
install(FILES ${ptx_files} DESTINATION ${ptx_dir})

#restore library target name
set(PROJECT_LIBRARY_TARGET_NAME ${TMP_PROJECT_LIBRARY_TARGET_NAME})


