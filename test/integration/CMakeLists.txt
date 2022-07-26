set(TEST_TYPE "INTEGRATION")

set(tests
  ExamplesBuild_TEST
  gpu_rays.cc
  depth_camera.cc
  camera.cc
  render_pass.cc
  shadows.cc
  scene.cc
  thermal_camera.cc
)

link_directories(${PROJECT_BINARY_DIR}/test)

# Test symbols having the right name on linux only
if (UNIX AND NOT APPLE)
  configure_file(all_symbols_have_version.bash.in ${CMAKE_CURRENT_BINARY_DIR}/all_symbols_have_version.bash @ONLY)
  add_test(NAME INTEGRATION_versioned_symbols
    COMMAND bash ${CMAKE_CURRENT_BINARY_DIR}/all_symbols_have_version.bash $<TARGET_FILE:${PROJECT_LIBRARY_TARGET_NAME}>)

  foreach(comp ${RENDERING_COMPONENTS})
   set(comp_target_name "${PROJECT_LIBRARY_TARGET_NAME}-${comp}")
   add_test(NAME "INTEGRATION_versioned_symbols_${comp}"
    COMMAND bash ${CMAKE_CURRENT_BINARY_DIR}/all_symbols_have_version.bash $<TARGET_FILE:${comp_target_name}>)
  endforeach()
endif()

ign_build_tests(TYPE INTEGRATION SOURCES ${tests})

if(TARGET INTEGRATION_ExamplesBuild_TEST)
  add_dependencies(INTEGRATION_ExamplesBuild_TEST FAKE_INSTALL)

  set(_env_vars)
  list(APPEND _env_vars "CMAKE_PREFIX_PATH=${FAKE_INSTALL_PREFIX}:$ENV{CMAKE_PREFIX_PATH}")
  set_tests_properties(INTEGRATION_ExamplesBuild_TEST PROPERTIES
    ENVIRONMENT "${_env_vars}")
endif()
