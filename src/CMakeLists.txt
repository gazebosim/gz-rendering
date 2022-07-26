# set compile definitions for tests
set_property(
  SOURCE RenderTarget.cc
  PROPERTY COMPILE_DEFINITIONS
  HAVE_OPENGL="${HAVE_OPENGL}"
)

# Collect source files into the "sources" variable and unit test files into the
# "gtest_sources" variable.
gz_get_libsources_and_unittests(sources gtest_sources)

add_subdirectory(base)

if (MSVC)
  # Warning #4251 is the "dll-interface" warning that tells you when types used
  # by a class are not being exported. These generated source files have private
  # members that don't get exported, so they trigger this warning. However, the
  # warning is not important since those members do not need to be interfaced
  # with.
  set_source_files_properties(${sources} ${gtest_sources} COMPILE_FLAGS "/wd4251")
endif()

# Create the library target.
gz_create_core_library(SOURCES ${sources})

# Include the interface directories that we always need.
#gz_target_interface_include_directories(${PROJECT_LIBRARY_TARGET_NAME}
#  gz-math${GZ_MATH_VER})

set_property(
  SOURCE RenderEngineManager.cc
  PROPERTY COMPILE_DEFINITIONS
  GZ_RENDERING_PLUGIN_PATH="${GZ_RENDERING_PLUGIN_PATH}"
)

target_link_libraries(${PROJECT_LIBRARY_TARGET_NAME}
  PUBLIC
  gz-common${GZ_COMMON_VER}::requested
  gz-utils${GZ_UTILS_VER}::gz-utils${GZ_UTILS_VER}
  PRIVATE
  gz-plugin${GZ_PLUGIN_VER}::loader
)
if (UNIX AND NOT APPLE)
  target_link_libraries(${PROJECT_LIBRARY_TARGET_NAME} PRIVATE X11)
endif()

# Build the unit tests.
gz_build_tests(TYPE UNIT SOURCES ${gtest_sources})

