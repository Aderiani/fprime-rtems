# Platform/RTEMS.cmake
# Minimal platform file for RTEMS to satisfy CMake's requirements

# Tell CMake we're cross-compiling
set(CMAKE_CROSSCOMPILING TRUE)

# Specify compilation as static libraries
set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")

# Basic default settings for RTEMS
set(RTEMS_BSP "${RTEMS_BSP}" CACHE STRING "RTEMS BSP in use")

# Don't try to find programs on the host when cross-compiling
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)