include (${project_cmake_dir}/Utils.cmake)
include (CheckCXXSourceCompiles)

include (${project_cmake_dir}/FindOS.cmake)
include (FindPkgConfig)

########################################
# Include man pages stuff
include (${project_cmake_dir}/Ronn2Man.cmake)
add_manpage_target()

########################################
# Find FreeImage
include (${project_cmake_dir}/FindFreeImage.cmake)
find_package(FreeImage QUIET)
if (NOT FreeImage_INCLUDE_DIRS)
  message(STATUS "Looking for FreeImage - not found")
  BUILD_ERROR("Missing: FreeImage library.")
else()
  message(STATUS "Looking for FreeImage - found")
  include_directories(SYSTEM ${FreeImage_INCLUDE_DIRS})
endif()

########################################
# Find CUDA
find_package(CUDA 5.5 QUIET)
if (NOT CUDA_FOUND)
  message(STATUS "Looking for CUDA - not found")
else()
  message(STATUS "Looking for CUDA - found")
  include_directories(SYSTEM ${CUDA_INCLUDE_DIRS})
endif()

########################################
# Find OptiX
include (${project_cmake_dir}/FindOptiX.cmake)
find_package(OptiX QUIET)
if (NOT OptiX_INCLUDE)
  set (OPTIX_FOUND FALSE)
  message(STATUS "Looking for OptiX - not found")
else()
  set (OPTIX_FOUND TRUE)
  message(STATUS "Looking for OptiX - found")
  include_directories(SYSTEM ${OptiX_INCLUDE})
endif()

if (CUDA_FOUND AND OPTIX_FOUND)
  set (HAVE_OPTIX ON CACHE BOOL "HAVE OPTIX" FORCE)
  set (HAVE_RENDERING TRUE)
else ()
  set (HAVE_OPTIX OFF CACHE BOOL "HAVE OPTIX" FORCE)
endif ()

#################################################
# Find OGRE
# On Windows, we assume that all the OGRE* defines are passed in manually
# to CMake.

set (MIN_OGRE_VERSION 1.7.4 CACHE INTERNAL "Ogre version requirement" FORCE)

if (NOT WIN32)
  execute_process(COMMAND pkg-config --modversion OGRE
                  OUTPUT_VARIABLE OGRE_VERSION)
  string(REPLACE "\n" "" OGRE_VERSION ${OGRE_VERSION})

  string (REGEX REPLACE "^([0-9]+).*" "\\1"
    OGRE_MAJOR_VERSION "${OGRE_VERSION}")
  string (REGEX REPLACE "^[0-9]+\\.([0-9]+).*" "\\1"
    OGRE_MINOR_VERSION "${OGRE_VERSION}")
  string (REGEX REPLACE "^[0-9]+\\.[0-9]+\\.([0-9]+).*" "\\1"
    OGRE_PATCH_VERSION ${OGRE_VERSION})

  set(OGRE_VERSION
    ${OGRE_MAJOR_VERSION}.${OGRE_MINOR_VERSION}.${OGRE_PATCH_VERSION})
endif()

pkg_check_modules(OGRE-RTShaderSystem
                  OGRE-RTShaderSystem>=${MIN_OGRE_VERSION})

pkg_check_modules(OGRE OGRE>=${MIN_OGRE_VERSION})
# There are some runtime problems to solve with ogre-1.9.
# Please read gazebo issues: 994, 995
if (NOT OGRE_FOUND)
  set (HAVE_OGRE OFF CACHE BOOL "HAVE OGRE" FORCE)
else ()
  if (OGRE-RTShaderSystem_FOUND)
    set(ogre_ldflags ${OGRE-RTShaderSystem_LDFLAGS})
    set(ogre_include_dirs ${OGRE-RTShaderSystem_INCLUDE_DIRS})
    set(ogre_libraries ${OGRE-RTShaderSystem_LIBRARIES})
    set(ogre_library_dirs ${OGRE-RTShaderSystem_LIBRARY_DIRS})
    set(ogre_cflags ${OGRE-RTShaderSystem_CFLAGS})

    set (INCLUDE_RTSHADER ON CACHE BOOL "Enable GPU shaders")
  else ()
    set (INCLUDE_RTSHADER OFF CACHE BOOL "Enable GPU shaders")
  endif ()

  set(ogre_ldflags ${ogre_ldflags} ${OGRE_LDFLAGS})
  set(ogre_include_dirs ${ogre_include_dirs} ${OGRE_INCLUDE_DIRS})
  set(ogre_libraries ${ogre_libraries};${OGRE_LIBRARIES})
  set(ogre_library_dirs ${ogre_library_dirs} ${OGRE_LIBRARY_DIRS})
  set(ogre_cflags ${ogre_cflags} ${OGRE_CFLAGS})
  set (HAVE_OGRE ON CACHE BOOL "HAVE OGRE" FORCE)
  set (HAVE_RENDERING TRUE)


  pkg_check_modules(OGRE-Terrain OGRE-Terrain)
  if (OGRE-Terrain_FOUND)
    set(ogre_ldflags ${ogre_ldflags} ${OGRE-Terrain_LDFLAGS})
    set(ogre_include_dirs ${ogre_include_dirs} ${OGRE-Terrain_INCLUDE_DIRS})
    set(ogre_libraries ${ogre_libraries};${OGRE-Terrain_LIBRARIES})
    set(ogre_library_dirs ${ogre_library_dirs} ${OGRE-Terrain_LIBRARY_DIRS})
    set(ogre_cflags ${ogre_cflags} ${OGRE-Terrain_CFLAGS})
  endif()

  pkg_check_modules(OGRE-Overlay OGRE-Overlay)
  if (OGRE-Overlay_FOUND)
    set(ogre_ldflags ${ogre_ldflags} ${OGRE-Overlay_LDFLAGS})
    set(ogre_include_dirs ${ogre_include_dirs} ${OGRE-Overlay_INCLUDE_DIRS})
    set(ogre_libraries ${ogre_libraries};${OGRE-Overlay_LIBRARIES})
    set(ogre_library_dirs ${ogre_library_dirs} ${OGRE-Overlay_LIBRARY_DIRS})
    set(ogre_cflags ${ogre_cflags} ${OGRE-Overlay_CFLAGS})
  endif()

  set (OGRE_INCLUDE_DIRS ${ogre_include_dirs}
       CACHE INTERNAL "Ogre include path")

  # Also find OGRE's plugin directory, which is provided in its .pc file as the
  # `plugindir` variable.  We have to call pkg-config manually to get it.
  # On Windows, we assume that all the OGRE* defines are passed in manually
  # to CMake.
  if (NOT WIN32)
    execute_process(COMMAND pkg-config --variable=plugindir OGRE
                    OUTPUT_VARIABLE _pkgconfig_invoke_result
                    RESULT_VARIABLE _pkgconfig_failed)
    if(_pkgconfig_failed)
      BUILD_WARNING ("Failed to find OGRE's plugin directory.  The build will succeed, but gazebo will likely fail to run.")
    else()
      # This variable will be substituted into cmake/setup.sh.in
      set (OGRE_PLUGINDIR ${_pkgconfig_invoke_result})
    endif()

  endif()

  set(OGRE_RESOURCE_PATH ${OGRE_PLUGINDIR})
  # Seems that OGRE_PLUGINDIR can end in a newline, which will cause problems when
  # we pass it to the compiler later.
  string(REPLACE "\n" "" OGRE_RESOURCE_PATH ${OGRE_RESOURCE_PATH})

endif ()

# Also find OGRE's plugin directory, which is provided in its .pc file as the
# `plugindir` variable.  We have to call pkg-config manually to get it.
# On Windows, we assume that all the OGRE* defines are passed in manually
# to CMake.
if (NOT WIN32)
  execute_process(COMMAND pkg-config --variable=plugindir OGRE
                  OUTPUT_VARIABLE _pkgconfig_invoke_result
                  RESULT_VARIABLE _pkgconfig_failed)
  if(_pkgconfig_failed)
    BUILD_WARNING ("Failed to find OGRE's plugin directory.  The build will succeed, but ign-rendering will likely fail to run.")
  else()
    # This variable will be substituted into cmake/setup.sh.in
    set (OGRE_PLUGINDIR ${_pkgconfig_invoke_result})
  endif()
endif()


#################################################
# Check there is at least one rendering library
if (NOT HAVE_RENDERING)
  BUILD_ERROR("No rendering libraries found")
endif ()

########################################
# Find ignition math
find_package(ignition-math4 QUIET)
if (NOT ignition-math4_FOUND)
  BUILD_ERROR ("Missing: Ignition Common (libignition-math4-dev)")
else()
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${IGNITION-MATH_CXX_FLAGS}")
  include_directories(${IGNITION-MATH_INCLUDE_DIRS})
  link_directories(${IGNITION-MATH_LIBRARY_DIRS})
endif()

include_directories(${IGNITION-MATH_INCLUDE_DIRS})
message(STATUS "Looking for ignition-math${IGNITION-MATH_REQUIRED_MAJOR_VERSION}-config.cmake - found")

########################################
# Find Ignition Common
find_package(ignition-common0 QUIET)
if (NOT ignition-common0_FOUND)
  BUILD_ERROR ("Missing: Ignition Common (libignition-common0-dev)")
else()
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${IGNITION-COMMON_CXX_FLAGS}")
  include_directories(${IGNITION-COMMON_INCLUDE_DIRS})
  link_directories(${IGNITION-COMMON_LIBRARY_DIRS})
endif()


