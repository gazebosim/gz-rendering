cmake_minimum_required(VERSION 3.22.1 FATAL_ERROR)
project(gz-rendering-cpu-bench)

# Pick up the locally-built gz-rendering (sourced install overlays the system one).
find_package(gz-rendering)

if(NOT MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")
endif()

add_executable(cpu_bench Main.cc)

target_link_libraries(cpu_bench
  ${GZ-RENDERING_LIBRARIES}
)
