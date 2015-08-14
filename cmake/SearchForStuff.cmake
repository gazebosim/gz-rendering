include (${project_cmake_dir}/Utils.cmake)
include (CheckCXXSourceCompiles)

include (${project_cmake_dir}/FindOS.cmake)
include (FindPkgConfig)

########################################
# Include man pages stuff
include (${project_cmake_dir}/Ronn2Man.cmake)
add_manpage_target()

########################################
# Temporarily include Gazebo stuff
find_package (gazebo REQUIRED gazebo_common gazebo_math)
include_directories (${GAZEBO_INCLUDE_DIRS})
link_directories (${GAZEBO_LIBRARY_DIRS})

########################################
# Find FreeImage
include (${project_cmake_dir}/FindFreeImage.cmake)
find_package(FreeImage QUIET)
if (NOT FreeImage_INCLUDE_DIRS)
  message(STATUS "Lookding for FreeImage - not found")
  BUILD_ERROR("Missing: FreeImage library.")
else()
  message(STATUS "Looking for FreeImage - found")
  include_directories(${FreeImage_INCLUDE_DIRS})
endif()

########################################
# Find CUDA
find_package(CUDA 5.5 QUIET)
if (NOT CUDA_FOUND)
  message(STATUS "Lookding for CUDA - not found")
  BUILD_ERROR("Missing: CUDA library.")
else()
  message(STATUS "Looking for CUDA - found")
  include_directories(${CUDA_INCLUDE_DIRS})
endif()

########################################
# Find OptiX
include (${project_cmake_dir}/FindOptiX.cmake)
find_package(OptiX QUIET)
if (NOT OptiX_INCLUDE)
  message(STATUS "Lookding for OptiX - not found")
  BUILD_ERROR("Missing: OptiX library.")
else()
  message(STATUS "Looking for OptiX - found")
  include_directories(${OptiX_INCLUDE})
endif()
