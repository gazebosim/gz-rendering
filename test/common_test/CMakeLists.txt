set(TEST_TYPE "COMMON_TEST")

set(tests
  camera
)

function(configure_common_test RENDER_ENGINE_NAME test_name)
  add_test(NAME ${test_name}_${RENDER_ENGINE_NAME}
    COMMAND
      ${test_name}
  )
  set_tests_properties(${test_name}_${RENDER_ENGINE_NAME}
    PROPERTIES ENVIRONMENT "GZ_ENGINE_TO_TEST=${RENDER_ENGINE_NAME}")
endfunction()

foreach(test ${tests})
  add_executable(${TEST_TYPE}_${test} ${test}.cc)

  target_link_libraries(${TEST_TYPE}_${test}
    PUBLIC
      gz-plugin${GZ_PLUGIN_VER}::loader
      gz-common${GZ_COMMON_VER}::gz-common${GZ_COMMON_VER}
      ${PROJECT_LIBRARY_TARGET_NAME}
      gtest_main
  )

  if (HAVE_OGRE)
    configure_common_test("ogre" ${TEST_TYPE}_${test})
  endif()
  if (HAVE_OGRE2)
    configure_common_test("ogre2" ${TEST_TYPE}_${test})
  endif()
  if (HAVE_OPTIX)
    configure_common_test("optix" ${TEST_TYPE}_${test})
  endif()
endforeach()
