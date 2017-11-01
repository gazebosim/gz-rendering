set(TEST_TYPE "INTEGRATION")

set(tests
  shadows.cc
)

link_directories(${PROJECT_BINARY_DIR}/test)

ign_build_tests(TYPE INTEGRATION SOURCES ${tests})
