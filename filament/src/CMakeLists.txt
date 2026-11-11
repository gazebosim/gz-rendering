gz_get_libsources_and_unittests(sources gtest_sources)

set(engine_name "filament")
gz_add_component(${engine_name} SOURCES ${sources} GET_TARGET_NAME filament_target)

# Filament's public headers need C++20
target_compile_features(${filament_target} PUBLIC cxx_std_20)

# Compile .mat sources into .filamat packages in the build tree. The
# engine looks them up in <resources>/filament/media/materials once
# installed, else here (FILAMENT_BUILD_MEDIA_PATH).
set(FILAMENT_MATC "${FILAMENT_ROOT}/bin/matc")
set(filament_media_dir "${CMAKE_CURRENT_BINARY_DIR}/media")
set(filament_material_dir "${filament_media_dir}/materials")
file(MAKE_DIRECTORY ${filament_material_dir})
file(GLOB filament_mat_sources CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/media/materials/*.mat")
# gz_lit_transparent: gz_lit with alpha blending, for lit materials with
# transparency (ogre2 blends them without writing depth)
set(lit_mat "${CMAKE_CURRENT_SOURCE_DIR}/media/materials/gz_lit.mat")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${lit_mat})
file(READ ${lit_mat} lit_src)
string(REPLACE "name : gz_lit," "name : gz_lit_transparent,
    blending : transparent,
    depthWrite : false," lit_transparent_src "${lit_src}")
set(lit_transparent_mat "${CMAKE_CURRENT_BINARY_DIR}/gz_lit_transparent.mat")
file(CONFIGURE OUTPUT ${lit_transparent_mat} CONTENT "${lit_transparent_src}"
  @ONLY)
list(APPEND filament_mat_sources ${lit_transparent_mat})
# gz_lit_masked: gz_lit alpha tested against the texture alpha
# (Material::SetAlphaFromTexture, ogre2's alpha test), also in shadow maps;
# double-sided per instance
string(REPLACE "name : gz_lit," "name : gz_lit_masked,
    blending : masked,
    doubleSided : true," lit_masked_src "${lit_src}")
set(lit_masked_mat "${CMAKE_CURRENT_BINARY_DIR}/gz_lit_masked.mat")
file(CONFIGURE OUTPUT ${lit_masked_mat} CONTENT "${lit_masked_src}" @ONLY)
list(APPEND filament_mat_sources ${lit_masked_mat})
foreach(mat ${filament_mat_sources})
  get_filename_component(mat_name ${mat} NAME_WE)
  set(mat_out "${filament_material_dir}/${mat_name}.filamat")
  add_custom_command(
    OUTPUT ${mat_out}
    COMMAND ${FILAMENT_MATC} --platform=all --api=all --optimize-size
            -o ${mat_out} ${mat}
    DEPENDS ${mat} ${FILAMENT_MATC}
    COMMENT "Compiling filament material ${mat_name}")
  list(APPEND filament_materials ${mat_out})
endforeach()
# ogre2's skybox cube map, for Scene::SetSkyEnabled
configure_file(
  "${PROJECT_SOURCE_DIR}/ogre2/src/media/materials/textures/skybox.dds"
  "${filament_media_dir}/textures/skybox.dds" COPYONLY)
add_custom_target(filament_materials DEPENDS ${filament_materials})
add_dependencies(${filament_target} filament_materials)

# derives from a filament class: filament is built without RTTI
set_property(SOURCE FilamentSharedEglPlatform.cc FilamentColorGrading.cc
  PROPERTY COMPILE_OPTIONS -fno-rtti)

if (GZ_FILAMENT_POLYNOMIAL_ATTENUATION)
  target_compile_definitions(${filament_target}
    PRIVATE GZ_FILAMENT_POLYNOMIAL_ATTENUATION)
endif()

set_property(
  SOURCE FilamentRenderEngine.cc
  PROPERTY COMPILE_DEFINITIONS
  FILAMENT_BUILD_MEDIA_PATH="${filament_media_dir}"
)

target_include_directories(${filament_target}
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
  SYSTEM PUBLIC ${FILAMENT_ROOT}/include)

# Filament ships static archives only; link them all as one group so the
# linker resolves their mutual dependencies.
file(GLOB filament_archives "${FILAMENT_LIB_DIR}/*.a")
target_link_libraries(${filament_target}
  PUBLIC
    ${gz-common_LIBRARIES}
    gz-math::eigen3
  PRIVATE
    gz-plugin::register
    -Wl,--start-group ${filament_archives} -Wl,--end-group
    ${CMAKE_DL_LIBS}
    ${OPENGL_LIBRARIES}
    pthread)

if (TARGET OpenGL::EGL)
  target_link_libraries(${filament_target} PRIVATE OpenGL::EGL)
endif()

install(TARGETS ${filament_target} DESTINATION ${GZ_RENDERING_ENGINE_RELATIVE_INSTALL_DIR})
install(DIRECTORY "${filament_media_dir}" DESTINATION ${GZ_RENDERING_RELATIVE_RESOURCE_PATH}/filament)

gz_build_tests(TYPE UNIT
               SOURCES ${gtest_sources}
               LIB_DEPS ${filament_target})
# FilamentRenderTarget_TEST plays the caller sharing its EGL context
if (BUILD_TESTING AND TARGET OpenGL::EGL)
  target_link_libraries(UNIT_FilamentRenderTarget_TEST OpenGL::EGL)
endif()
