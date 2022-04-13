cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

find_package(ignition-rendering7 REQUIRED)
set(IGN_RENDERING_VER ${ignition-rendering7_VERSION_MAJOR})

find_package(ignition-plugin2 REQUIRED COMPONENTS all)
set(IGN_PLUGIN_VER ${ignition-plugin2_VERSION_MAJOR})

find_package(ignition-common5 REQUIRED)
set(IGN_COMMON_VER ${ignition-common5_VERSION_MAJOR})

add_library(HelloWorldPlugin SHARED HelloWorldPlugin.cc)
target_link_libraries(HelloWorldPlugin
  PUBLIC
    ignition-rendering${IGN_RENDERING_VER}::ignition-rendering${IGN_RENDERING_VER}
    ignition-common${IGN_COMMON_VER}::ignition-common${IGN_COMMON_VER}
  PRIVATE
    ignition-plugin${IGN_PLUGIN_VER}::register
)
