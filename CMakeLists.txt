cmake_minimum_required(VERSION 3.10.2 FATAL_ERROR)

#============================================================================
# Initialize the project
#============================================================================
project(ignition-rendering1 VERSION 1.0.0)

#============================================================================
# Find ignition-cmake
#============================================================================
# If you get an error at this line, you need to install ignition-cmake
find_package(ignition-cmake2 REQUIRED)

#============================================================================
# Set up the project
#============================================================================
ign_configure_project(VERSION_SUFFIX pre4)

#============================================================================
# Set project-specific options
#============================================================================

# ignition-rendering currently has no options that are unique to it

#============================================================================
# Search for project-specific dependencies
#============================================================================

#--------------------------------------
# Find ignition-math
ign_find_package(ignition-math6 REQUIRED)
set(IGN_MATH_VER ${ignition-math6_VERSION_MAJOR})

#--------------------------------------
# Find ignition-common
ign_find_package(ignition-common3 REQUIRED COMPONENTS graphics events)
set(IGN_COMMON_VER ${ignition-common3_VERSION_MAJOR})

#--------------------------------------
# Find ignition-plugin
ign_find_package(ignition-plugin1 REQUIRED COMPONENTS all)
set(IGN_PLUGIN_VER ${ignition-plugin1_VERSION_MAJOR})

#--------------------------------------
# Find FreeImage
ign_find_package(FreeImage VERSION 3.9
  REQUIRED_BY optix
  PRIVATE_FOR optix)

#--------------------------------------
# Find OpenGL
ign_find_package(OpenGL
  REQUIRED_BY ogre ogre2
  PKGCONFIG gl)

if (OpenGL_FOUND)
  set(HAVE_OPENGL TRUE)
endif()

#--------------------------------------
# Find OGRE
list(APPEND ign_ogre_components "RTShaderSystem" "Terrain" "Overlay")

if (WIN32)
  list(APPEND ign_ogre_components "RenderSystem_GL")
endif()

ign_find_package(IgnOGRE VERSION 1.8.0
  COMPONENTS ${ign_ogre_components}
  REQUIRED_BY ogre
  PRIVATE_FOR ogre)

if (OGRE_FOUND)
  # there is a problem with finding gl/glew.h
  # TODO move this logic to FindIgnOGRE.cmake?
  foreach (dir ${OGRE_INCLUDE_DIRS})
    get_filename_component(dir_name "${dir}" NAME)
    if ("${dir_name}" STREQUAL "OGRE")
      set(dir_include "${dir}/RenderSystems/GL")
    else()
      set(dir_include "${dir}")
    endif()
    list(APPEND OGRE_INCLUDES ${dir_include})
  endforeach()
  include_directories(${OGRE_INCLUDES})

  # find boost - mainly needed on macOS and also by the terrain component
  find_package(Boost)
  if (Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
  endif()

  set(HAVE_OGRE TRUE)
endif()

#--------------------------------------
# Find OGRE2
ign_find_package(OGRE2 VERSION 2.1.0
    COMPONENTS HlmsPbs HlmsUnlit Overlay
    REQUIRED_BY ogre2
    PRIVATE_FOR ogre2)

if (OGRE2_FOUND)
  # there is a problem with finding gl/glew.h on macOS
  # TODO move this logic to FindIgnOGRE.cmake?
  foreach (dir ${OGRE2_INCLUDE_DIRS})
    get_filename_component(dir_name "${dir}" NAME)
    if ("${dir_name}" STREQUAL "OGRE")
      set(dir_include "${dir}/RenderSystems/GL3Plus")
    else()
      set(dir_include "${dir}")
    endif()
    list(APPEND OGRE2_INCLUDES ${dir_include})
  endforeach()
  include_directories(${OGRE2_INCLUDES})

  set(HAVE_OGRE2 TRUE)
endif()

#--------------------------------------
# Find dependencies that we ignore for Visual Studio
if(NOT MSVC)
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
if (HAVE_OGRE)
  list(APPEND RENDERING_COMPONENTS ogre)
endif()

if (HAVE_OPTIX)
  list(APPEND RENDERING_COMPONENTS optix)
endif()

if (HAVE_OGRE2)
  list(APPEND RENDERING_COMPONENTS ogre2)
endif()

ign_configure_build(QUIT_IF_BUILD_ERRORS
    COMPONENTS ${RENDERING_COMPONENTS})

#============================================================================
# Create package information
#============================================================================
ign_create_packages()

#============================================================================
# Configure documentation
#============================================================================
configure_file(${CMAKE_SOURCE_DIR}/api.md.in ${CMAKE_BINARY_DIR}/api.md)
configure_file(${CMAKE_SOURCE_DIR}/tutorials.md.in ${CMAKE_BINARY_DIR}/tutorials.md)

ign_create_docs(
  API_MAINPAGE_MD "${CMAKE_BINARY_DIR}/api.md"
  TUTORIALS_MAINPAGE_MD "${CMAKE_BINARY_DIR}/tutorials.md")
