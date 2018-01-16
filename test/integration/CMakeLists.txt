set(TEST_TYPE "INTEGRATION")

set(tests
  camera.cc
  shadows.cc
)

link_directories(${PROJECT_BINARY_DIR}/test)

ign_build_tests(${tests})
