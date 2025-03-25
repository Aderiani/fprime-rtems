# cmake/platform/RTEMS.cmake
# Platform file for RTEMS systems

# Define operating system
add_definitions(-DTGT_OS_TYPE_RTEMS)

# Configure compiler flags and disable problematic warnings for RTEMS
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

# Choose RTEMS-specific implementations for Os interfaces
choose_fprime_implementation(Os/File Os_File_RTEMS)
choose_fprime_implementation(Os/Console Os_Console_RTEMS)
choose_fprime_implementation(Os/Task Os_Task_RTEMS)
choose_fprime_implementation(Os/Mutex Os_Mutex_RTEMS)
choose_fprime_implementation(Os/Queue Os_Queue_RTEMS)
choose_fprime_implementation(Os/RawTime Os_RawTime_RTEMS)
choose_fprime_implementation(Os/Cpu Os_Cpu_RTEMS)
choose_fprime_implementation(Os/Memory Os_Memory_RTEMS)
choose_fprime_implementation(Os/FileSystem Os_FileSystem_RTEMS)
choose_fprime_implementation(Os/Directory Os_Directory_RTEMS)
choose_fprime_implementation(Os/Condition Os_Condition_RTEMS)

# Configure the platform types header
configure_file(
    "${CMAKE_CURRENT_LIST_DIR}/types/PlatformTypesRTEMS.h"
    "${CMAKE_BINARY_DIR}/PlatformTypes.h"
    COPYONLY
)
include_directories(${CMAKE_BINARY_DIR})

# Additional RTEMS-specific compile options to make the build work
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

