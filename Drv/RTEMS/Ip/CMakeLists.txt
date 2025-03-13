####
# F prime CMakeLists.txt for RTEMS IP implementations
####

# Only build for RTEMS platform
if (NOT ${CMAKE_SYSTEM_NAME} STREQUAL "RTEMS")
    return()
endif()

# Include the module descriptor
include("${CMAKE_CURRENT_LIST_DIR}/ModuleDescriptor.cmake")