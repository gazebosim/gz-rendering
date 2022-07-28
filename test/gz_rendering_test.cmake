include(GzPython)

macro(gz_configure_rendering_test)
  set(options HEADLESS)
  set(oneValueArgs TARGET RENDER_ENGINE RENDER_ENGINE_BACKEND)
  set(multiValueArgs)

  _gz_cmake_parse_arguments(gz_configure_rendering_test
    "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  if(NOT DEFINED gz_configure_rendering_test_RENDER_ENGINE_BACKEND)
    set(gz_configure_rendering_test_RENDER_ENGINE_BACKEND "gl3plus")
  endif()

  set(test_name ${gz_configure_rendering_test_TARGET}_${gz_configure_rendering_test_RENDER_ENGINE}_${gz_configure_rendering_test_RENDER_ENGINE_BACKEND})

  add_test(NAME ${test_name} 
    COMMAND ${gz_configure_rendering_test_TARGET} --gtest_output=xml:${CMAKE_BINARY_DIR}/test_results/${test_name}.xml)
  set_tests_properties(${test_name} PROPERTIES TIMEOUT 240)

  set_tests_properties(${test_name}
    PROPERTIES ENVIRONMENT "GZ_ENGINE_TO_TEST=${gz_configure_rendering_test_RENDER_ENGINE}")

  set_property(
      TEST ${test_name}
      APPEND PROPERTY
        ENVIRONMENT "GZ_ENGINE_BACKEND=${gz_configure_rendering_test_RENDER_ENGINE_BACKEND}")

  if(gz_configure_rendering_test_HEADLESS)
    set_property(
        TEST ${test_name}
        APPEND PROPERTY
        ENVIRONMENT "GZ_ENGINE_HEADLESS=1")
  endif()

  if(Python3_Interpreter_FOUND)
    # Check that the test produced a result and create a failure if it didn't.
    # Guards against crashed and timed out tests.
    add_test(check_${test_name} ${Python3_EXECUTABLE} ${GZ_CMAKE_TOOLS_DIR}/check_test_ran.py
      ${CMAKE_BINARY_DIR}/test_results/${test_name}.xml)
  endif()
endmacro()

if (UNIX AND NOT APPLE)
  execute_process(COMMAND lsb_release -cs
    OUTPUT_VARIABLE RELEASE_CODENAME
    RESULT_VARIABLE LSB_RESULT
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )

  if( NOT (${LSB_RESULT} STREQUAL "0"))
    message(WARNING "lsb_release executable not found. Disabling focal-specific workarounds")
  elseif (${RELEASE_CODENAME} STREQUAL "jammy")
    set(UBUNTU_JAMMY 1)
  endif()
endif()

macro(gz_rendering_test)
  set(options)
  set(oneValueArgs TYPE SOURCE)
  set(multiValueArgs LIB_DEPS)

  _gz_cmake_parse_arguments(gz_rendering_test "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  set(TEST_NAME ${gz_rendering_test_TYPE}_${gz_rendering_test_SOURCE})

  add_executable(${TEST_NAME} ${gz_rendering_test_SOURCE}.cc)

  target_link_libraries(${TEST_NAME} PUBLIC gtest gtest_main ${gz_rendering_test_LIB_DEPS})
  if (UNIX)
    target_link_libraries(${TEST_NAME} PUBLIC pthread)
  endif()

  if (TARGET ${TEST_NAME})
    target_compile_definitions(${TEST_NAME}
      PRIVATE
      "PROJECT_SOURCE_PATH=\"${PROJECT_SOURCE_DIR}\""
      "PROJECT_BUILD_PATH=\"${PROJECT_BINARY_DIR}\""
      )
    target_include_directories(${TEST_NAME}
      PRIVATE
      ${PROJECT_SOURCE_DIR}/test/common_test
    )
  endif()

  if (HAVE_OGRE)
    gz_configure_rendering_test(
      TARGET ${TEST_NAME}
      RENDER_ENGINE "ogre"
    )
  endif()

  if (HAVE_OGRE2)
    if (APPLE)
      gz_configure_rendering_test(
        TARGET ${TEST_NAME}
        RENDER_ENGINE "ogre2"
        RENDER_ENGINE_BACKEND "metal")
    else()
      gz_configure_rendering_test(
        TARGET ${TEST_NAME}
        RENDER_ENGINE "ogre2"
        RENDER_ENGINE_BACKEND "gl3plus")
      # \TODO(mjcarroll) Re-enable when Github Actions
      # supports it
      # if(UBUNTU_JAMMY)
      #   gz_configure_rendering_test(
      #     TARGET ${TEST_NAME}
      #     RENDER_ENGINE "ogre2"
      #     RENDER_ENGINE_BACKEND "vulkan"
      #     HEADLESS)
      # endif()
    endif()
  endif()
  if (HAVE_OPTIX)
    gz_configure_rendering_test(
      TARGET ${TEST_NAME}
      RENDER_ENGINE "optix"
    )
  endif()


endmacro()
