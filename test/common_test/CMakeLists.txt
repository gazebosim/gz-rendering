set(TEST_TYPE "UNIT")

set(tests
  ArrowVisual_TEST
  AxisVisual_TEST
  BoundingBox_TEST
  Camera_TEST
  Capsule_TEST
  COMVisual_TEST
  GaussianNoisePass_TEST
  GizmoVisual_TEST
  Grid_TEST
  Heightmap_TEST 
  InertiaVisual_TEST
  LidarVisual_TEST
  Light_TEST
  LightVisual_TEST
  Marker_TEST
  Material_TEST
  Mesh_TEST
  MeshDescriptor_TEST
  MoveToHelper_TEST 
  Node_TEST 
  OrbitViewController_TEST 
  OrthoViewController_TEST 
  ParticleEmitter_TEST
  RayQuery_TEST
  RenderEngine_TEST
  RenderPassSystem_TEST
  RenderTarget_TEST
  Scene_TEST
  SegmentationCamera_TEST
  Text_TEST
  ThermalCamera_TEST
  TransformController_TEST
  Utils_TEST
  Visual_TEST
  WireBox_TEST
)

function(configure_common_test RENDER_ENGINE_NAME test_name)
  add_test(NAME ${test_name}_${RENDER_ENGINE_NAME}
    COMMAND
      ${test_name}
  )
  set_tests_properties(${test_name}_${RENDER_ENGINE_NAME}
    PROPERTIES ENVIRONMENT "GZ_ENGINE_TO_TEST=${RENDER_ENGINE_NAME}")

  if (APPLE)
    set_tests_properties(${test_name}_${RENDER_ENGINE_NAME}
      PROPERTIES ENVIRONMENT "GZ_ENGINE_BACKEND=metal")
  endif()
endfunction()

foreach(test ${tests})
  set(TEST_NAME ${TEST_TYPE}_${test})
  add_executable(${TEST_NAME} ${test}.cc)

  target_link_libraries(${TEST_NAME}
    PUBLIC
      gz-plugin${GZ_PLUGIN_VER}::loader
      gz-common${GZ_COMMON_VER}::gz-common${GZ_COMMON_VER}
      ${PROJECT_LIBRARY_TARGET_NAME}
      gtest_main
  )

  if (HAVE_OGRE)
    configure_common_test("ogre" ${TEST_NAME})
  endif()
  if (HAVE_OGRE2)
    configure_common_test("ogre2" ${TEST_NAME})
  endif()
  if (HAVE_OPTIX)
    configure_common_test("optix" ${TEST_NAME})
  endif()
endforeach()

set(media_tests
  Heightmap_TEST
  Material_TEST
  Mesh_TEST
  ParticleEmitter_TEST
)

foreach(test ${media_tests})
  set(TEST_NAME ${TEST_TYPE}_${test})
  if (TARGET ${TEST_NAME})
    target_compile_definitions(${TEST_NAME}
      PRIVATE
      "PROJECT_SOURCE_PATH=\"${PROJECT_SOURCE_DIR}\"")
  endif()
endforeach()
