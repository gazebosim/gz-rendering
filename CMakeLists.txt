cmake_minimum_required(VERSION 3.5.1 FATAL_ERROR)

#============================================================================
# Initialize the project
#============================================================================
project(ignition-rendering0 VERSION 0.1.0)

#============================================================================
# Find ignition-cmake
#============================================================================
# If you get an error at this line, you need to install ignition-cmake
find_package(ignition-cmake1 REQUIRED)

#============================================================================
# Set up the project
#============================================================================
ign_configure_project()

#============================================================================
# Set project-specific options
#============================================================================

# ignition-rendering currently has no options that are unique to it

#============================================================================
# Search for project-specific dependencies
#============================================================================

#--------------------------------------
# Find ignition-math
set(IGN_MATH_VER 4)
ign_find_package(ignition-math4 REQUIRED)

#--------------------------------------
# Find ignition-common
set(IGN_COMMON_VER 1)
ign_find_package(ignition-common1 REQUIRED)

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
      COMPONENTS RTShaderSystem Terrain Overlay
      REQUIRED_BY ogre
      PRIVATE_FOR ogre)

  if (OGRE_FOUND)
    set(HAVE_OGRE TRUE)
  endif()


  #--------------------------------------
  # Find CUDA
  find_package(CUDA)

  #--------------------------------------
  # Find OptiX
  ign_find_package(OptiX VERSION 3.8.0
      REQUIRED_BY optix
      PRIVATE_FOR optix)

  if (OptiX_FOUND AND CUDA_FOUND)
    set(HAVE_OPTIX TRUE)
  endif()

endif()

#####################################
# Define compile-time default variables
set(IGN_RENDERING_PLUGIN_PATH ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR})
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
