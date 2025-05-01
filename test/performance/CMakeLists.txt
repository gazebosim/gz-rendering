set(TEST_TYPE "PERFORMANCE")

set(tests
  scene_factory
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
