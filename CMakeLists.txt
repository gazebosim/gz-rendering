cmake_minimum_required(VERSION 3.5.1 FATAL_ERROR)

#============================================================================
# Initialize the project
#============================================================================
project(ignition-rendering0 VERSION 0.1.0)

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

#--------------------------------------
# Find ignition-math
set(IGN_MATH_VER 4)
ign_find_package(ignition-math${IGN_MATH_VER} REQUIRED)

#--------------------------------------
# Find ignition-common
set(IGN_COMMON_VER 0)
ign_find_package(ignition-common${IGN_COMMON_VER} REQUIRED)


#--------------------------------------
# Find dependencies that we ignore for Visual Studio
if(NOT MSVC)

  #--------------------------------------
  # Find FreeImage
  ign_find_package(FreeImage VERSION 3.9
     REQUIRED_BY optix
     PRIVATE_FOR optix)

  #--------------------------------------
  # Find OGRE
  ign_find_package(OGRE VERSION 1.8.0
     REQUIRED_BY ogre
     PRIVATE_FOR ogre)

  #--------------------------------------
  # Find CUDA
  ign_find_package(CUDA VERSION 5.5.0
     REQUIRED_BY optix
     PRIVATE_FOR optix)

  #--------------------------------------
  # Find OptiX
  ign_find_package(OptiX VERSION 3.8.0
     REQUIRED_BY optix
     PRIVATE_FOR optix)


 # ign_find_package(CUDA
 #                  VERSION 5.5.0
 #                  PKGCONFIG_VER_COMPARISON >=
 #                  REQUIRED)
 #
 # #--------------------------------------
 # # Find optix
 # include (${project_cmake_dir}/FindOptiX.cmake)
 # find_package(OptiX QUIET)
 #
 # if (NOT OptiX_INCLUDE)
 #   set (OPTIX_FOUND FALSE)
 #   message(STATUS "Looking for OptiX - not found")
 # else()
 #   set (OPTIX_FOUND TRUE)
 #   message(STATUS "Looking for OptiX - found")
 #   include_directories(SYSTEM ${OptiX_INCLUDE})
 # endif()
 #
 # if (CUDA_FOUND AND OPTIX_FOUND)
 #   set (HAVE_OPTIX ON CACHE BOOL "HAVE OPTIX" FORCE)
 #   set (HAVE_RENDERING TRUE)
 # else ()
 #   set (HAVE_OPTIX OFF CACHE BOOL "HAVE OPTIX" FORCE)
 # endif ()
endif()

if (HAVE_OGRE)
  set (HAVE_RENDERING TRUE)
endif()


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
ign_configure_build(QUIT_IF_BUILD_ERRORS
    COMPONENTS ogre optix)

#============================================================================
# Create package information
#============================================================================
ign_create_packages()
