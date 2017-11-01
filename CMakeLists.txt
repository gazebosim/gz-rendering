cmake_minimum_required(VERSION 3.5.1 FATAL_ERROR)

#============================================================================
# Find ignition-cmake
#============================================================================
# If you get an error at this line, you need to install ignition-cmake
find_package(ignition-cmake0 REQUIRED)

#============================================================================
# Set up the project
#============================================================================
ign_configure_project(rendering 0.1.0)

#============================================================================
# Set project-specific options
#============================================================================

# ignition-rendering currently has no options that are unique to it

#============================================================================
# Search for project-specific dependencies
#============================================================================
message(STATUS "\n\n-- ====== Finding Dependencies ======")

set (project_cmake_dir ${PROJECT_SOURCE_DIR}/cmake
  CACHE PATH "Location of CMake scripts")

#--------------------------------------
# Find ignition-math
set(IGN_MATH_VER 4)
ign_find_package(ignition-math${IGN_MATH_VER}
                 REQUIRED)

#if (NOT ignition-math${IGN-MATH_VER}_FOUND)
#  message(STATUS "Looking for ignition-math${IGN-MATH_VERSION}-config.cmake - not found, trying ignition-math3 instead")
#  set(IGN_MATH_VER 3)
#  find_package(ignition-math${IGN_MATH_VER} QUIET)
#  if (NOT ignition-math${IGN_MATH_VER}_FOUND)
#    BUILD_ERROR ("Missing: Ignition math4 or Ignition math3 library.")
#  endif()
#endif()

#--------------------------------------
# Find ignition-common
ign_find_package(ignition-common0 REQUIRED)

#--------------------------------------
# Find FreeImage
ign_find_package(FreeImage VERSION 3.9 REQUIRED PRIVATE)

#--------------------------------------
# Find ogre
# On Windows, we assume that all the OGRE* defines are passed in manually
# to CMake.

include (FindPkgConfig)

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
#if (NOT OGRE_FOUND)
#  set (HAVE_OGRE OFF CACHE BOOL "HAVE OGRE" FORCE)
#else ()
if (OGRE_FOUND)

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
#  set (HAVE_OGRE ON CACHE BOOL "HAVE OGRE" FORCE)
#  set (HAVE_RENDERING TRUE)


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

if (OGRE_FOUND)
  set (HAVE_OGRE ON CACHE BOOL "HAVE OGRE" FORCE)
  set (HAVE_RENDERING TRUE)
  message(STATUS "Looking for OGRE - found")
else()
  set (HAVE_OGRE OFF CACHE BOOL "HAVE OGRE" FORCE)
  message(STATUS "Looking for OGRE - not found")
endif()


#--------------------------------------
# Find CUDA
ign_find_package(CUDA
                 VERSION 5.5.0
                 PKGCONFIG_VER_COMPARISON >=
                 REQUIRED)

#--------------------------------------
# Find optix
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

if (NOT HAVE_RENDERING)
  message(FATAL_ERROR "No rendering engines found")
endif()



#####################################
# Define compile-time default variables
set(IGN_RENDERING_PLUGIN_PATH ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/ignition/rendering-${PROJECT_VERSION_MAJOR}/plugins)
set(IGN_RENDERING_RESOURCE_PATH ${CMAKE_INSTALL_PREFIX}/share/ignition/rendering-${PROJECT_VERSION_MAJOR})


#============================================================================
# Configure the build
#============================================================================
ign_configure_build(QUIT_IF_BUILD_ERRORS)

#============================================================================
# Create package information
#============================================================================
ign_create_packages()
