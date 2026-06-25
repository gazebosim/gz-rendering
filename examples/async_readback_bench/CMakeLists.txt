cmake_minimum_required(VERSION 3.22.1 FATAL_ERROR)
project(gz-rendering-async-readback-bench)
find_package(gz-rendering REQUIRED)

add_executable(async_readback_bench Main.cc)
target_link_libraries(async_readback_bench ${GZ-RENDERING_LIBRARIES})

# Compile in the async_raw path only when the linked gz-rendering exposes the
# experimental async readback API. A gz-rendering that predates the
# jrivero/async-readback-experiment branch lacks this header, so the same source
# builds raw-only there and raw+async_raw against this workspace install.
find_path(GZ_ASYNC_READBACK_INCLUDE_DIR
  NAMES gz/rendering/AsyncReadback.hh
  HINTS ${GZ-RENDERING_INCLUDE_DIRS}
  NO_DEFAULT_PATH)
if(GZ_ASYNC_READBACK_INCLUDE_DIR)
  target_compile_definitions(async_readback_bench
    PRIVATE GZ_RENDERING_HAS_ASYNC_READBACK)
  message(STATUS
    "async_readback_bench: async readback API present -> async_raw enabled")
else()
  message(STATUS
    "async_readback_bench: async readback API absent -> raw-only build")
endif()
