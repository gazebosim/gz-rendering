cmake_minimum_required(VERSION 3.22.1 FATAL_ERROR)
project(gz-rendering-rgbd-throughput-bench)
find_package(gz-rendering REQUIRED)

add_executable(rgbd_throughput_bench Main.cc)
target_link_libraries(rgbd_throughput_bench ${GZ-RENDERING_LIBRARIES})
