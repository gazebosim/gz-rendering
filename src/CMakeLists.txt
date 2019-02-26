# set compile definitions for tests
set_property(
  SOURCE Camera_TEST.cc RenderTarget.cc
  PROPERTY COMPILE_DEFINITIONS
  HAVE_OPENGL="${HAVE_OPENGL}"
)

# Collect source files into the "sources" variable and unit test files into the
# "gtest_sources" variable.
ign_get_libsources_and_unittests(sources gtest_sources)

add_subdirectory(base)

# Create the library target.
ign_create_core_library(SOURCES ${sources} CXX_STANDARD 17)

# Include the interface directories that we always need.
#ign_target_interface_include_directories(${PROJECT_LIBRARY_TARGET_NAME}
#  ignition-math${IGN_MATH_VER})

set_property(
  SOURCE RenderEngineManager.cc
  PROPERTY COMPILE_DEFINITIONS
  IGN_RENDERING_PLUGIN_PATH="${IGN_RENDERING_PLUGIN_PATH}"
)

target_link_libraries(${PROJECT_LIBRARY_TARGET_NAME}
  PUBLIC
  ignition-common${IGN_COMMON_VER}::requested
  PRIVATE
  ignition-plugin${IGN_PLUGIN_VER}::loader)

# Build the unit tests.
ign_build_tests(TYPE UNIT SOURCES ${gtest_sources})

