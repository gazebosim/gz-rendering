cmake_minimum_required(VERSION 3.22.1 FATAL_ERROR)
project(gz-rendering-gpu-compress-bench)
find_package(gz-rendering REQUIRED)

include_directories(SYSTEM ${PROJECT_BINARY_DIR})
configure_file(example_config.hh.in ${PROJECT_BINARY_DIR}/example_config.hh)

add_executable(gpu_compress_bench Main.cc)
target_link_libraries(gpu_compress_bench ${GZ-RENDERING_LIBRARIES})
