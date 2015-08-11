include (${project_cmake_dir}/Utils.cmake)

set (sources
  BaseObject.cc
  BaseRenderEngine.cc
  BaseRenderTarget.cc
  BaseScene.cc
)

set (gtest_sources
)

ign_build_tests(${gtest_sources})
ign_add_library(${PROJECT_NAME_LOWER}-base ${sources})
ign_install_library(${PROJECT_NAME_LOWER}-base)
