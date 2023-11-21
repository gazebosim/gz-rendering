cmake_minimum_required(VERSION 3.10.2 FATAL_ERROR)

#============================================================================
# Initialize the project
#============================================================================
project(gz-rendering9 VERSION 9.0.0)

#============================================================================
# Find gz-cmake
#============================================================================
# If you get an error at this line, you need to install gz-cmake
find_package(gz-cmake4 REQUIRED)
set(GZ_CMAKE_VER ${gz-cmake4_VERSION_MAJOR})

#============================================================================
# Set up the project
#============================================================================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

gz_configure_project(VERSION_SUFFIX)

#============================================================================
# Set project-specific options
#============================================================================
option(USE_UNOFFICIAL_OGRE_VERSIONS "Accept unsupported Ogre versions in the build" OFF)

#============================================================================
# Search for project-specific dependencies
#============================================================================

#--------------------------------------
# Find gz-math
gz_find_package(gz-math8 REQUIRED COMPONENTS eigen3)
set(GZ_MATH_VER ${gz-math8_VERSION_MAJOR})

#--------------------------------------
# Find gz-common
gz_find_package(gz-common6 REQUIRED
  COMPONENTS graphics events geospatial)
set(GZ_COMMON_VER ${gz-common6_VERSION_MAJOR})

#--------------------------------------
# Find gz-plugin
gz_find_package(gz-plugin3 REQUIRED COMPONENTS all)
set(GZ_PLUGIN_VER ${gz-plugin3_VERSION_MAJOR})

#--------------------------------------
# Find gz-utils
gz_find_package(gz-utils3 REQUIRED)
set(GZ_UTILS_VER ${gz-utils3_VERSION_MAJOR})

#--------------------------------------
# Find FreeImage
gz_find_package(FreeImage VERSION 3.9
  REQUIRED_BY optix
  PRIVATE_FOR optix)

#--------------------------------------
# Find OpenGL
# See CMP0072 for more details (cmake --help-policy CMP0072)
if ((NOT ${CMAKE_VERSION} VERSION_LESS 3.11) AND (NOT OpenGL_GL_PREFERENCE))
  set(OpenGL_GL_PREFERENCE "GLVND")
endif()

if (APPLE)
  gz_find_package(OpenGL
    REQUIRED_BY ogre ogre2
    PKGCONFIG gl)
else()
  gz_find_package(OpenGL REQUIRED
    COMPONENTS OpenGL
    OPTIONAL_COMPONENTS EGL
    REQUIRED_BY ogre ogre2
    PKGCONFIG gl)
endif()

if (OpenGL_FOUND)
  set(HAVE_OPENGL TRUE)
endif()

#find_package(ogre_vendor REQUIRED)
#set(GZ_RENDERING_HAVE_OGRE TRUE)

find_package(ogre_next_vendor REQUIRED)
set(GZ_RENDERING_HAVE_OGRE2 TRUE)
set(GZ_RENDERING_HAVE_VULKAN TRUE)


# Plugin install dirs
set(GZ_RENDERING_ENGINE_RELATIVE_INSTALL_DIR
  ${GZ_LIB_INSTALL_DIR}/gz-${GZ_DESIGNATION}-${PROJECT_VERSION_MAJOR}/engine-plugins
)
set(GZ_RENDERING_ENGINE_INSTALL_DIR
  ${CMAKE_INSTALL_PREFIX}/${GZ_RENDERING_ENGINE_RELATIVE_INSTALL_DIR})



#####################################
# Define compile-time default variables
if(MSVC)
  set(GZ_RENDERING_PLUGIN_PATH ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR})
else()
  set(GZ_RENDERING_PLUGIN_PATH ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR})
endif()

set(GZ_RENDERING_RELATIVE_RESOURCE_PATH ${GZ_DATA_INSTALL_DIR})
set(GZ_RENDERING_RESOURCE_PATH ${CMAKE_INSTALL_PREFIX}/${GZ_RENDERING_RELATIVE_RESOURCE_PATH})

#============================================================================
# Configure the build
#============================================================================
if (GZ_RENDERING_HAVE_OGRE)
  list(APPEND RENDERING_COMPONENTS ogre)
endif()

if (GZ_RENDERING_HAVE_OPTIX)
  list(APPEND RENDERING_COMPONENTS optix)
endif()

if (GZ_RENDERING_HAVE_OGRE2)
  list(APPEND RENDERING_COMPONENTS ogre2)
endif()

configure_file("${PROJECT_SOURCE_DIR}/cppcheck.suppress.in"
               ${PROJECT_BINARY_DIR}/cppcheck.suppress)

gz_configure_build(QUIT_IF_BUILD_ERRORS
    HIDE_SYMBOLS_BY_DEFAULT
    COMPONENTS ${RENDERING_COMPONENTS})

if (GZ_RENDERING_HAVE_OGRE2)
  # Must be done after gz_configure_build or else Terra
  # won't see GZ_ADD_fPIC_TO_LIBRARIES
  add_subdirectory(ogre2/src/terrain/Terra)
endif()

#============================================================================
# Create package information
#============================================================================
gz_create_packages()

#============================================================================
# Configure documentation
#============================================================================
configure_file(${CMAKE_SOURCE_DIR}/api.md.in ${CMAKE_BINARY_DIR}/api.md)
configure_file(${CMAKE_SOURCE_DIR}/tutorials.md.in ${CMAKE_BINARY_DIR}/tutorials.md)

gz_create_docs(
  API_MAINPAGE_MD "${CMAKE_BINARY_DIR}/api.md"
  TUTORIALS_MAINPAGE_MD "${CMAKE_BINARY_DIR}/tutorials.md"
  IMAGE_PATH_DIRS "${CMAKE_SOURCE_DIR}/tutorials/img"
  TAGFILES
    "${GZ-COMMON_DOXYGEN_TAGFILE} = ${GZ-COMMON_API_URL}"
    "${GZ-MATH_DOXYGEN_TAGFILE} = ${GZ-MATH_API_URL}"
)

file(COPY ${CMAKE_SOURCE_DIR}/tutorials/img/ DESTINATION ${CMAKE_BINARY_DIR}/doxygen/html/img/)
