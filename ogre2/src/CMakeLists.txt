# Collect source files into the "sources" variable and unit test files into the
# "gtest_sources" variable.
ign_get_libsources_and_unittests(sources gtest_sources)

ign_add_component(ogre2 SOURCES ${sources} GET_TARGET_NAME ogre2_target)

find_package(OpenGL)

set_property(
  SOURCE Ogre2RenderEngine.cc
  PROPERTY COMPILE_DEFINITIONS
  IGN_RENDERING_RESOURCE_PATH="${IGN_RENDERING_RESOURCE_PATH}"
  OGRE2_RESOURCE_PATH="${OGRE2_RESOURCE_PATH}"
  OGRE2_VERSION="${OGRE2_VERSION}"
)

target_link_libraries(${ogre2_target}
  PUBLIC
    ${ignition-common${IGN_COMMON_VER}_LIBRARIES}
  PRIVATE
    ignition-plugin${IGN_PLUGIN_VER}::register
    ${OPENGL_LIBRARIES}
    IgnOGRE2::IgnOGRE2)

# Build the unit tests
ign_build_tests(TYPE UNIT SOURCES ${gtest_sources} LIB_DEPS ${ogre2_target})

install(DIRECTORY "media"  DESTINATION ${IGN_RENDERING_RESOURCE_PATH}/ogre2)
