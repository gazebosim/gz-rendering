# Collect source files into the "sources" variable and unit test files into the
# "gtest_sources" variable
ign_get_libsources_and_unittests(sources gtest_sources)

ign_add_component(base SOURCES ${sources} GET_TARGET_NAME base_target)

target_link_libraries(${base_target}
  PUBLIC
    ${ignition-common${IGN_COMMON_VER}_LIBRARIES}
)

# Create installation instructions for the library target. This must be called
# in the same scope that the target is created.
#ign_install_library()

# Build the unit tests
#ign_build_tests(TYPE UNIT SOURCES ${gtest_sources})

#restore library target name
#set(PROJECT_LIBRARY_TARGET_NAME ${TMP_PROJECT_LIBRARY_TARGET_NAME})



