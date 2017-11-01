add_subdirectory(base)

if (HAVE_OGRE)
  add_subdirectory(ogre)
endif ()

if (HAVE_OPTIX)
  add_subdirectory(optix)
endif ()


# Turn on C++11 support for the library.
set(CMAKE_CXX_STANDARD 11)

# Collect source files into the "sources" variable and unit test files into the
# "gtest_sources" variable.
ign_get_libsources_and_unittests(sources gtest_sources)

# Create the library target.
ign_add_library(${PROJECT_LIBRARY_TARGET_NAME} ${sources})

# Include the interface directories that we always need.
#ign_target_interface_include_directories(${PROJECT_LIBRARY_TARGET_NAME}
#  ignition-math${IGN_MATH_VER})


if (HAVE_OGRE)
  target_link_libraries(${PROJECT_LIBRARY_TARGET_NAME}
    PUBLIC
      ignition-rendering0-ogre
    PRIVATE
  )
endif()

if (HAVE_OPTIX)
  target_link_libraries(${PROJECT_LIBRARY_TARGET_NAME}
    PUBLIC
      ignition-rendering0-optix
    PRIVATE
  )
endif()

# Create installation instructions for the library target. This must be called
# in the same scope that the target is created.
ign_install_library()

# Build the unit tests.
ign_build_tests(TYPE UNIT SOURCES ${gtest_sources})

