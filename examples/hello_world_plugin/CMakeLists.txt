cmake_minimum_required(VERSION 3.22.1 FATAL_ERROR)

find_package(gz-rendering REQUIRED)

find_package(gz-plugin REQUIRED COMPONENTS all)

find_package(gz-common REQUIRED)

add_library(HelloWorldPlugin SHARED HelloWorldPlugin.cc)
target_link_libraries(HelloWorldPlugin
  PUBLIC
    gz-rendering::gz-rendering
    gz-common::gz-common
  PRIVATE
    gz-plugin::register
)
