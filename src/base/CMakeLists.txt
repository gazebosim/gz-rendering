# Turn on C++11 support for the library.
set(CMAKE_CXX_STANDARD 11)

# Collect source files into the "sources" variable and unit test files into the
# "gtest_sources" variable
ign_get_libsources_and_unittests(sources gtest_sources)

# Create the library target
# override library target name
#set(TMP_PROJECT_LIBRARY_TARGET_NAME ${PROJECT_LIBRARY_TARGET_NAME})
#set(PROJECT_LIBRARY_TARGET_NAME ${BASE_LIBRARY_TARGET_NAME})
#ign_add_library(${PROJECT_LIBRARY_TARGET_NAME} ${sources})

#set(BASE_LIBRARY_TARGET_NAME ${PROJECT_LIBRARY_TARGET_NAME}-base)
#add_library(${BASE_LIBRARY_TARGET_NAME} ${sources})

ign_add_component(base SOURCES ${sources} GET_TARGET_NAME base_target)


# Include the interface directories that we always need.
#ign_target_interface_include_directories(${BASE_LIBRARY_TARGET_NAME}
ign_target_interface_include_directories(${base_target}
  ignition-math${IGN_MATH_VER}::ignition-math${IGN_MATH_VER}
  ignition-common${IGN_COMMON_VER}::ignition-common${IGN_COMMON_VER}
)


# Create installation instructions for the library target. This must be called
# in the same scope that the target is created.
#ign_install_library()

# Build the unit tests
#ign_build_tests(TYPE UNIT SOURCES ${gtest_sources})

#restore library target name
#set(PROJECT_LIBRARY_TARGET_NAME ${TMP_PROJECT_LIBRARY_TARGET_NAME})



