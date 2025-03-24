
# cmake/platform/RTEMS.cmake
# Platform file for RTEMS systems

# Define operating system
add_definitions(-DTGT_OS_TYPE_RTEMS)

# # Configure compiler flags
# set(CMAKE_C_FLAGS
#   "${CMAKE_C_FLAGS} -mcpu=leon3 -g -ffunction-sections -fdata-sections -Wall -Wmissing-prototypes -Wimplicit-function-declaration -Wstrict-prototypes -Wnested-externs"
# )
# set(CMAKE_CXX_FLAGS
#   "${CMAKE_CXX_FLAGS} -mcpu=leon3 -g -ffunction-sections -fdata-sections -Wall -fno-exceptions -fno-rtti"
# )


# Find thread package (not needed if using baremetal scheduler)
if (NOT DEFINED FPRIME_USE_BAREMETAL_SCHEDULER)
   set(FPRIME_USE_BAREMETAL_SCHEDULER OFF)
   message(STATUS "Using RTEMS scheduler")
endif()

# Choose RTEMS-specific implementations for Os interfaces
choose_fprime_implementation(Os/File Os/File/RTEMS)
choose_fprime_implementation(Os/Console Os/Console/RTEMS)
choose_fprime_implementation(Os/Task Os/Task/RTEMS)
choose_fprime_implementation(Os/Mutex Os/Mutex/RTEMS)
choose_fprime_implementation(Os/Queue Os/Queue/RTEMS)
choose_fprime_implementation(Os/RawTime Os/RawTime/RTEMS)
choose_fprime_implementation(Os/Cpu Os/Cpu/RTEMS)
choose_fprime_implementation(Os/Memory Os/Memory/RTEMS)
choose_fprime_implementation(Os/FileSystem Os/FileSystem/RTEMS)
choose_fprime_implementation(Os/Directory Os/Directory/RTEMS)
choose_fprime_implementation(Os/Condition Os/Condition/RTEMS)
# Choose RTEMS-specific implementations
# choose_fprime_implementation(Drv_TcpServer Drv_TcpServer)
# choose_fprime_implementation(Drv_TcpClient Drv_TcpClient)
# choose_fprime_implementation(Drv_Udp Drv_Udp)
# Include RTEMS system headers and platform types if needed
# Uncomment and adjust the path if your RTEMS-specific headers are in a custom location
# include_directories(SYSTEM "${FPRIME_FRAMEWORK_PATH}/Os/RTEMS")

# Set the platform types header
configure_file(
    "${CMAKE_CURRENT_LIST_DIR}/../platform/types/PlatformTypesRTEMS.h"
    "${CMAKE_BINARY_DIR}/PlatformTypes.h"
    COPYONLY
)
include_directories(${CMAKE_BINARY_DIR})
include_directories(SYSTEM "${CMAKE_CURRENT_LIST_DIR}/types")

# RTEMS-specific features
 # Add to the COMPAT_FLAGS line:
set(COMPAT_FLAGS "-Wno-pedantic -Wno-error=ignored-qualifiers -Wno-error=old-style-cast -Wno-error=sign-compare -Wno-old-style-cast -Wno-error=unused-variable -Wno-sign-compare")

