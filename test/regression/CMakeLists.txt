set(TEST_TYPE "REGRESSION")

gz_rendering_test(
  TYPE ${TEST_TYPE}
  SOURCE reload_engine
  LIB_DEPS
    gz-plugin::loader
    gz-common::gz-common
    ${PROJECT_LIBRARY_TARGET_NAME}
)

