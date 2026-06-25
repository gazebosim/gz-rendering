cmake_minimum_required(VERSION 3.22.1 FATAL_ERROR)
project(gz-rendering-gpu-compress-bench)
find_package(gz-rendering REQUIRED)

include_directories(SYSTEM ${PROJECT_BINARY_DIR})
configure_file(example_config.hh.in ${PROJECT_BINARY_DIR}/example_config.hh)

add_executable(gpu_compress_bench Main.cc)
target_link_libraries(gpu_compress_bench ${GZ-RENDERING_LIBRARIES})

# Compile in the NV12 compressed-output path only when the linked gz-rendering
# exposes the compression API. A system gz-rendering that predates the
# jrivero/gpu-compression-design branch lacks this header, so the same source
# builds raw-only there and raw+nv12 against the workspace install.
find_path(GZ_COMPRESSED_IMAGE_INCLUDE_DIR
  NAMES gz/rendering/CompressedImage.hh
  HINTS ${GZ-RENDERING_INCLUDE_DIRS}
  NO_DEFAULT_PATH)
if(GZ_COMPRESSED_IMAGE_INCLUDE_DIR)
  target_compile_definitions(gpu_compress_bench
    PRIVATE GZ_RENDERING_HAS_COMPRESSED_IMAGE)
  message(STATUS
    "gpu_compress_bench: CompressedImage API present -> NV12 path enabled")
else()
  message(STATUS
    "gpu_compress_bench: CompressedImage API absent -> raw-only build")
endif()
