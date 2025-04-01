# cmake/platform/RTEMS.cmake
# Platform file for RTEMS systems

# Define operating system
add_definitions(-DTGT_OS_TYPE_RTEMS)

# Configure compiler flags for RTEMS
set(CMAKE_C_FLAGS
  "${CMAKE_C_FLAGS} -Wno-error=pedantic -Wno-error=shadow -Wno-error=unused-variable -Wno-error=attributes"
)
set(CMAKE_CXX_FLAGS
  "${CMAKE_CXX_FLAGS} -Wno-error=pedantic -Wno-error=shadow -Wno-error=unused-variable -Wno-error=attributes -Wno-error=old-style-cast"
)

# Choose USE_BAREMETAL_SCHEDULER for RTEMS
if (NOT DEFINED FPRIME_USE_BAREMETAL_SCHEDULER)
   set(FPRIME_USE_BAREMETAL_SCHEDULER OFF)
   message(STATUS "Using RTEMS scheduler")
endif()

# Use the correct path formats for the implementations
choose_fprime_implementation(Os/File Os_File_Posix)
choose_fprime_implementation(Os/FileSystem Os_Posix_Shared)
choose_fprime_implementation(Os/Directory Os_Posix_Shared)
choose_fprime_implementation(Os/Console Os_Console_Posix)
choose_fprime_implementation(Os/Task Os_Task_Posix)
choose_fprime_implementation(Os/Mutex Os_Mutex_Posix)
choose_fprime_implementation(Os/RawTime Os_RawTime_Posix)
# Use RTEMS implementations for these
choose_fprime_implementation(Os/Cpu Os_Cpu_RTEMS)
choose_fprime_implementation(Os/Queue Os_Queue_RTEMS)
choose_fprime_implementation(Os/Memory Os_Memory_RTEMS)

# Set POSIX flag
set(FPRIME_USE_POSIX ON)

# Configure the platform types header
configure_file(
    "${CMAKE_CURRENT_LIST_DIR}/types/PlatformTypesRTEMS.h"
    "${CMAKE_BINARY_DIR}/PlatformTypes.h"
    COPYONLY
)
include_directories(${CMAKE_BINARY_DIR})

# Additional RTEMS-specific compile options
add_compile_options(
    -Wno-pedantic 
    -Wno-error=ignored-qualifiers 
    -Wno-error=old-style-cast 
    -Wno-error=sign-compare 
    -Wno-old-style-cast 
    -Wno-error=unused-variable 
    -Wno-sign-compare
    -Wno-error=shadow
    -Wno-shadow
)


