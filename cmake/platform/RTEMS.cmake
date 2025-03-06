####
# RTEMS.cmake:
#
# Platform setup for RTEMS on GR740
####
include_guard()

# Enable small string optimization
set(FPRIME_STRING_OPT_SIZE "16" CACHE STRING "Small string optimization size")

# Set platform-specific compile flags
add_compile_definitions(TGT_OS_TYPE_RTEMS)
add_compile_definitions(_RTEMS)

# Turn off port serialization for RTEMS
# This is typically done for resource-constrained systems
set(FPRIME_ENABLE_TEXT_LOGGING ON CACHE BOOL "Enable text logging" FORCE)
set(FPRIME_ENABLE_AUTOCODER_UTS OFF CACHE BOOL "Enable autocoder unit tests" FORCE)

# Define the PlatformTypes.h template with RTEMS-specific settings
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/PlatformTypes.h.template" 
"#ifndef FPRIME_CONFIG_PLATFORMTYPES_H
#define FPRIME_CONFIG_PLATFORMTYPES_H

#include <stdint.h>
#include <sys/types.h>
#include <rtems.h>

/**
 * RTEMS Platform types for F′
 */

typedef int32_t         PlatformIntType;
#define PRI_PlatformIntType \"d\"

typedef uint32_t        PlatformUIntType;
#define PRI_PlatformUIntType \"u\"

typedef int32_t         PlatformIndexType;
#define PRI_PlatformIndexType \"d\"

typedef uint32_t        PlatformSizeType;
#define PRI_PlatformSizeType \"u\"

typedef uint32_t        PlatformPointerCastType;
#define PRI_PlatformPointerCastType \"u\"

typedef int32_t         PlatformAssertArgType;
#define PRI_PlatformAssertArgType \"d\"

#endif // FPRIME_CONFIG_PLATFORMTYPES_H
")

# Add the directory containing the custom PlatformTypes.h to the include path
include_directories(${CMAKE_CURRENT_BINARY_DIR})
configure_file(${CMAKE_CURRENT_BINARY_DIR}/PlatformTypes.h.template
               ${CMAKE_CURRENT_BINARY_DIR}/PlatformTypes.h
               @ONLY)

# Set platform specific link flags
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")
message(STATUS "Platform file for RTEMS included")