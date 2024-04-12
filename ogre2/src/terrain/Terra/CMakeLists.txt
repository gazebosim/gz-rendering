
project(terra)

file( GLOB_RECURSE TERRA_SOURCES
  ${CMAKE_CURRENT_LIST_DIR}/include/*.h
  ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp
)

add_library(${PROJECT_NAME} STATIC ${TERRA_SOURCES})
set_property(TARGET ${PROJECT_NAME} PROPERTY POSITION_INDEPENDENT_CODE ON)


# disable all warnings for Terra
if (UNIX)
  target_compile_options(${PROJECT_NAME} PRIVATE
      -Wno-unused-parameter
      -Wno-float-equal
      -Wno-sign-compare
      -Wno-strict-aliasing)
elseif (MSVC)
  set_source_files_properties(${TERRA_SOURCES} COMPILE_FLAGS "/wd4244")
endif()

add_definitions(-DOGRE_IGNORE_UNKNOWN_DEBUG)

#target_compile_definitions(${PROJECT_NAME} PUBLIC
#  $<$<CONFIG:Debug>:DEBUG=1 _DEBUG=1>)

target_include_directories(${PROJECT_NAME}
  PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/include
)

target_link_libraries(${PROJECT_NAME} PRIVATE ogre_next_vendor::OgreMain ogre_next_vendor::HlmsPbs ogre_next_vendor::PlanarReflections)
