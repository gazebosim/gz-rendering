#################################################
macro (ign_build_tests)
  # Build all the tests
  foreach(GTEST_SOURCE_file ${ARGN})
    string(REGEX REPLACE ".cc" "" BINARY_NAME ${GTEST_SOURCE_file})
    set(BINARY_NAME ${TEST_TYPE}_${BINARY_NAME})
    if(USE_LOW_MEMORY_TESTS)
      add_definitions(-DUSE_LOW_MEMORY_TESTS=1)
    endif(USE_LOW_MEMORY_TESTS)
    add_executable(${BINARY_NAME} ${GTEST_SOURCE_file})

    add_dependencies(${BINARY_NAME}
      ${PROJECT_NAME_LOWER}${PROJECT_MAJOR_VERSION}
      gtest gtest_main
      )

    if (UNIX)
      target_link_libraries(${BINARY_NAME}
        libgtest.a
        libgtest_main.a
        pthread
        ${PROJECT_NAME_LOWER}${PROJECT_MAJOR_VERSION}
      )
    elseif(WIN32)
      target_link_libraries(${BINARY_NAME}
        gtest
        gtest_main
        ${PROJECT_NAME_LOWER}${PROJECT_MAJOR_VERSION}.lib
      )
    endif()

    add_test(${BINARY_NAME} ${CMAKE_CURRENT_BINARY_DIR}/${BINARY_NAME}
	--gtest_output=xml:${CMAKE_BINARY_DIR}/test_results/${BINARY_NAME}.xml)

    set_tests_properties(${BINARY_NAME} PROPERTIES TIMEOUT 240)

    # Check that the test produced a result and create a failure if it didn't.
    # Guards against crashed and timed out tests.
    add_test(check_${BINARY_NAME} ${PROJECT_SOURCE_DIR}/tools/check_test_ran.py
	${CMAKE_BINARY_DIR}/test_results/${BINARY_NAME}.xml)
  endforeach()
endmacro()

if (VALID_DRI_DISPLAY)
  macro (ign_build_dri_tests)
    ign_build_tests(${ARGV})
  endmacro()
else()
  # Fake macro when no valid DRI display is found
  macro (ign_build_dri_tests)
  endmacro()
endif()

