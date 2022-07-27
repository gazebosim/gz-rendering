set(TEST_TYPE "INTEGRATION")

set(tests
  camera
  depth_camera
  gpu_rays
  lidar_visual
  render_pass
  scene
  segmentation_camera 
  shadows 
  thermal_camera
  wide_angle_camera
)

link_directories(${PROJECT_BINARY_DIR}/test)

function(configure_test RENDER_ENGINE_NAME test_name)
  add_test(NAME ${test_name}_${RENDER_ENGINE_NAME}
    COMMAND
      ${test_name}
  )
  set(test_env "GZ_ENGINE_TO_TEST=${RENDERING_ENGINE_NAME}")

  set_tests_properties(${test_name}_${RENDER_ENGINE_NAME}
    PROPERTIES ENVIRONMENT "GZ_ENGINE_TO_TEST=${RENDER_ENGINE_NAME}")

  if (APPLE AND "${RENDER_ENGINE_NAME}" STREQUAL "ogre2")
    set_property(
        TEST ${test_name}_${RENDER_ENGINE_NAME}
        APPEND PROPERTY
            ENVIRONMENT "GZ_ENGINE_BACKEND=metal")
  endif()
endfunction()

foreach(test ${tests})
  set(TEST_NAME ${TEST_TYPE}_${test})
  add_executable(${TEST_NAME} ${test}.cc)

  target_link_libraries(${TEST_NAME}
    PUBLIC
      gz-plugin${GZ_PLUGIN_VER}::loader
      gz-common${GZ_COMMON_VER}::gz-common${GZ_COMMON_VER}
      ${PROJECT_LIBRARY_TARGET_NAME}
      gtest_main
  )

  if (UNIX)
    target_link_libraries(${TEST_NAME} PUBLIC pthread)
  endif()

  target_include_directories(${TEST_NAME}
    PRIVATE
    ${PROJECT_SOURCE_DIR}/test/common_test
  )

  if (TARGET ${TEST_NAME})
    target_compile_definitions(${TEST_NAME}
      PRIVATE
      "PROJECT_SOURCE_PATH=\"${PROJECT_SOURCE_DIR}\""
      "PROJECT_BUILD_PATH=\"${PROJECT_BINARY_DIR}\""
      )
  endif()

  if (HAVE_OGRE)
    configure_test("ogre" ${TEST_NAME})
  endif()
  if (HAVE_OGRE2)
    configure_test("ogre2" ${TEST_NAME})
  endif()
  if (HAVE_OPTIX)
    configure_test("optix" ${TEST_NAME})
  endif()

endforeach()
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
