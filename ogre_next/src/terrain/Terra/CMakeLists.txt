
project(terra)

file( GLOB_RECURSE TERRA_SOURCES
  ${CMAKE_CURRENT_LIST_DIR}/include/*.h
  ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp
)

add_library(${PROJECT_NAME} STATIC ${TERRA_SOURCES})
set_property(TARGET ${PROJECT_NAME} PROPERTY POSITION_INDEPENDENT_CODE ON)


# disable all warnings for Terra
if (UNIX)
  target_compile_options(${PROJECT_NAME} PRIVATE
      -Wno-unused-parameter
      -Wno-float-equal
      -Wno-sign-compare
      -Wno-strict-aliasing)
elseif (MSVC)
  set_source_files_properties(${TERRA_SOURCES} COMPILE_FLAGS "/wd4244")
endif()

add_definitions(-DOGRE_IGNORE_UNKNOWN_DEBUG)

# function to include terra directories and link terra libraries
function(target_terra_include_directories_and_link_libraries
  use_ogre2 # if ogre2 instead of ogre_next should be used
)

  #target_compile_definitions(${PROJECT_NAME} PUBLIC
  #  $<$<CONFIG:Debug>:DEBUG=1 _DEBUG=1>)

  set(ogre_next_include OGRE_NEXT_INCLUDE)
  if (use_ogre2)
    set(ogre_next_include OGRE2_INCLUDE)
  endif()
  target_include_directories(${PROJECT_NAME}
    PRIVATE
      # Hlms files inside Hlms/Pbs do not have #include <Hlms/Pbs/...> thus
      # we must add this one manually for this to build correctly
      ${${ogre_next_include}}/Hlms/Pbs
      ${${ogre_next_include}}/Hlms/Common
    PUBLIC
      ${CMAKE_CURRENT_LIST_DIR}/include
  )

  set(gz_ogre_next_target GzOGRENext::GzOGRENext)
  if (use_ogre2)
    set(gz_ogre_next_target GzOGRE2::GzOGRE2)
  endif()
  target_link_libraries(${PROJECT_NAME} PRIVATE ${gz_ogre_next_target})

endfunction()

if (GZ_RENDERING_HAVE_OGRE_NEXT)
  target_terra_include_directories_and_link_libraries(false) # use ogre_next
endif()
if (GZ_RENDERING_HAVE_OGRE2)
  target_terra_include_directories_and_link_libraries(true) # use ogre2
endif()
