cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

set(IGN_PLUGIN_VER 1)
set(IGN_COMMON_VER 3)

find_package(ignition-rendering5 REQUIRED)
find_package(ignition-plugin1 REQUIRED COMPONENTS all)

FIND_PACKAGE(Boost REQUIRED COMPONENTS serialization system thread)

Message(${Boost_INCLUDE_DIR} ${Boost_LIBRARIES})

# Find LuxCore
include_directories("/ignition-ws/shared/LuxCore-sdk/include")
link_directories("/ignition-ws/shared/LuxCore-sdk/lib")

INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} )

add_library(LuxCore SHARED LuxCore.cc)
target_link_libraries(LuxCore
  PUBLIC
    ${IGNITION-RENDERING_LIBRARIES}
    ${ignition-common${IGN_COMMON_VER}_LIBRARIES}
    luxcore
    ${Boost_LIBRARIES}
  PRIVATE
    ignition-plugin${IGN_PLUGIN_VER}::register
)
