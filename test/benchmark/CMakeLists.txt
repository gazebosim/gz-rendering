set(TEST_TYPE "BENCHMARK")

include(IgnBenchmark OPTIONAL RESULT_VARIABLE IgnBenchmark_FOUND)

if (IgnBenchmark_FOUND)
  set(tests
    render.cc
  )

  ign_add_benchmarks(SOURCES ${tests})
endif()

