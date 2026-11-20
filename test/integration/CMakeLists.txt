set(TEST_TYPE "INTEGRATION")

set(tests
  axis_visual
  boundingbox_camera
  camera
  depth_camera
  gpu_rays
  heightmap
  lidar_visual
  mesh
  projector
  render_pass
  scene
  segmentation_camera
  shadows
  sky
  thermal_camera
  load_unload
  waves
  wide_angle_camera
)

foreach(test ${tests})
  gz_rendering_test(
    TYPE ${TEST_TYPE}
    SOURCE ${test}
    LIB_DEPS
      gz-plugin::loader
      gz-common::gz-common
      ${PROJECT_LIBRARY_TARGET_NAME}
  )
endforeach()

# The camera test again, with ogre2 reading textures back through a new
# staging buffer per copy (GZ_RENDERING_OGRE2_LEGACY_READBACK)
if (GZ_RENDERING_HAVE_OGRE2 AND NOT APPLE)
  set(legacy_test INTEGRATION_camera_ogre2_gl3plus_legacy_readback)
  add_test(NAME ${legacy_test}
    COMMAND INTEGRATION_camera
      --gtest_output=xml:${CMAKE_BINARY_DIR}/test_results/${legacy_test}.xml)
  set_tests_properties(${legacy_test} PROPERTIES
    TIMEOUT 240
    ENVIRONMENT "GZ_ENGINE_TO_TEST=ogre2;GZ_ENGINE_BACKEND=gl3plus;GZ_RENDERING_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX};GZ_RENDERING_OGRE2_LEGACY_READBACK=1")
endif()

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
