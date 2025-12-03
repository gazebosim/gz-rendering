include(gz_rendering_test.cmake)

set(BUILD_GMOCK FALSE)
add_subdirectory(gtest_vendor)
add_subdirectory(integration)
add_subdirectory(performance)
add_subdirectory(regression)
add_subdirectory(common_test)
