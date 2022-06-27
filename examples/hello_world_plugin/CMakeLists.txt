cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

find_package(gz-rendering7 REQUIRED)
set(GZ_RENDERING_VER ${gz-rendering7_VERSION_MAJOR})

find_package(gz-plugin2 REQUIRED COMPONENTS all)
set(GZ_PLUGIN_VER ${gz-plugin2_VERSION_MAJOR})

find_package(gz-common5 REQUIRED)
set(GZ_COMMON_VER ${gz-common5_VERSION_MAJOR})

add_library(HelloWorldPlugin SHARED HelloWorldPlugin.cc)
target_link_libraries(HelloWorldPlugin
  PUBLIC
    gz-rendering${GZ_RENDERING_VER}::gz-rendering${GZ_RENDERING_VER}
    gz-common${GZ_COMMON_VER}::gz-common${GZ_COMMON_VER}
  PRIVATE
    gz-plugin${GZ_PLUGIN_VER}::register
)
