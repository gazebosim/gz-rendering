
# Collect source files into the "sources" variable and unit test files into the
# "gtest_sources" variable.
ign_get_libsources_and_unittests(sources gtest_sources)

add_subdirectory(base)

# Create the library target.
ign_create_main_library(SOURCES ${sources} CXX_STANDARD 11)

# Include the interface directories that we always need.
#ign_target_interface_include_directories(${PROJECT_LIBRARY_TARGET_NAME}
#  ignition-math${IGN_MATH_VER})

target_link_libraries(${PROJECT_LIBRARY_TARGET_NAME}
  PUBLIC
    ${ignition-math${IGN_MATH_VER}_LIBRARIES}
    ${ignition-common${IGN_COMMON_VER}_LIBRARIES})

# Build the unit tests.
ign_build_tests(TYPE UNIT SOURCES ${gtest_sources})

