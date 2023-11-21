# Collect source files into the "sources" variable and unit test files into the
# "gtest_sources" variable.
gz_get_libsources_and_unittests(sources gtest_sources)

if (MSVC)
  # Warning #4251 is the "dll-interface" warning that tells you when types used
  # by a class are not being exported. These generated source files have private
  # members that don't get exported, so they trigger this warning. However, the
  # warning is not important since those members do not need to be interfaced
  # with.
  set_source_files_properties(${sources} ${gtest_sources} COMPILE_FLAGS "/wd4251")
  set_source_files_properties(${sources} ${gtest_sources} COMPILE_FLAGS "/D_SILENCE_STDEXT_HASH_DEPRECATION_WARNINGS")
endif()

set(engine_name "ogre2")

gz_add_component(${engine_name} SOURCES ${sources} GET_TARGET_NAME ogre2_target)

set(OGRE2_RESOURCE_PATH_STR "${OGRE2_RESOURCE_PATH}")
string(REPLACE ";" ":" OGRE2_RESOURCE_PATH_STR "${OGRE2_RESOURCE_PATH}")
set_property(
  SOURCE Ogre2RenderEngine.cc
  PROPERTY COMPILE_DEFINITIONS
  OGRE2_RESOURCE_PATH="${OGRE2_RESOURCE_PATH_STR}"
  OGRE2_VERSION="${OGRE2_VERSION}"
)

target_compile_definitions(${ogre2_target}
  PRIVATE OGRE_IGNORE_UNKNOWN_DEBUG)

target_include_directories(${ogre2_target}
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(${ogre2_target}
  PUBLIC
    ${gz-common${GZ_COMMON_VER}_LIBRARIES}
    gz-math${GZ_MATH_VER}::eigen3
  PRIVATE
    gz-plugin${GZ_PLUGIN_VER}::register
    terra
    ogre_next_vendor::OgreMain
    ogre_next_vendor::Overlay
    ogre_next_vendor::HlmsPbs
    ogre_next_vendor::HlmsUnlit
    ogre_next_vendor::PlanarReflections
    GLX
)

target_compile_definitions(${ogre2_target} PRIVATE $<$<CONFIG:Debug>:DEBUG=1 _DEBUG=1>)

target_compile_definitions(${ogre2_target} PRIVATE HAVE_GLX=true HAVE_EGL=true)

set (versioned ${CMAKE_SHARED_LIBRARY_PREFIX}${PROJECT_NAME_LOWER}-${engine_name}${CMAKE_SHARED_LIBRARY_SUFFIX})
set (unversioned ${CMAKE_SHARED_LIBRARY_PREFIX}${PROJECT_NAME_NO_VERSION_LOWER}-${engine_name}${CMAKE_SHARED_LIBRARY_SUFFIX})

# Note that plugins are currently being installed in 2 places: /lib and the engine-plugins dir
install(TARGETS ${ogre2_target} DESTINATION ${GZ_RENDERING_ENGINE_INSTALL_DIR})

if (WIN32)
  # disable MSVC inherit via dominance warning
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4250")
  INSTALL(CODE "EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E copy
  ${GZ_RENDERING_ENGINE_INSTALL_DIR}\/${versioned}
  ${GZ_RENDERING_ENGINE_INSTALL_DIR}\/${unversioned})")
else()
  EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E create_symlink ${versioned} ${unversioned})
  INSTALL(FILES ${PROJECT_BINARY_DIR}/${unversioned} DESTINATION ${GZ_RENDERING_ENGINE_INSTALL_DIR})
endif()

# Build the unit tests
gz_build_tests(TYPE UNIT
               SOURCES ${gtest_sources}
               LIB_DEPS ${ogre2_target}
               ENVIRONMENT GZ_RENDERING_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX})

install(DIRECTORY "media"  DESTINATION ${GZ_RENDERING_RESOURCE_PATH}/ogre2)
