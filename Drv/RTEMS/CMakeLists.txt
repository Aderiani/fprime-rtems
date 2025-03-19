####
# F prime CMakeLists.txt for RTEMS drivers:
#
# This directory contains drivers specific to RTEMS platforms,
# organized by board family.
####

# Only build RTEMS drivers when targeting RTEMS
if (NOT ${CMAKE_SYSTEM_NAME} STREQUAL "RTEMS")
    return()
endif()

message(STATUS "Building RTEMS drivers for F´")

# # Add the IP (networking) directory
# add_fprime_subdirectory("${CMAKE_CURRENT_LIST_DIR}/Ip/")

# Add the GR740 drivers
add_fprime_subdirectory("${CMAKE_CURRENT_LIST_DIR}/GR740/")

# Add the include directory
include_directories(SYSTEM "${CMAKE_CURRENT_LIST_DIR}/include/")