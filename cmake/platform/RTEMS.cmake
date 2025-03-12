# # cmake/platform/RTEMS.cmake
# # Platform file for RTEMS systems

# # Define operating system
# add_definitions(-DTGT_OS_TYPE_RTEMS)

# # Configure compiler flags
# set(CMAKE_C_FLAGS
#   "${CMAKE_C_FLAGS} -mcpu=leon3 -g -ffunction-sections -fdata-sections -Wall -Wmissing-prototypes -Wimplicit-function-declaration -Wstrict-prototypes -Wnested-externs"
# )
# set(CMAKE_CXX_FLAGS
#   "${CMAKE_CXX_FLAGS} -mcpu=leon3 -g -ffunction-sections -fdata-sections -Wall -fno-exceptions -fno-rtti"
# )

# # Find thread package (not needed if using baremetal scheduler)
# if (NOT DEFINED FPRIME_USE_BAREMETAL_SCHEDULER)
#    set(FPRIME_USE_BAREMETAL_SCHEDULER OFF)
#    message(STATUS "Using RTEMS scheduler")
# endif()

# # Include RTEMS system headers and platform types
# # include_directories(SYSTEM "${FPRIME_FRAMEWORK_PATH}/Os/RTEMS")


# cmake/platform/RTEMS.cmake
# Platform file for RTEMS systems

# Define operating system
add_definitions(-DTGT_OS_TYPE_RTEMS)

# Configure compiler flags
set(CMAKE_C_FLAGS
  "${CMAKE_C_FLAGS} -mcpu=leon3 -g -ffunction-sections -fdata-sections -Wall -Wmissing-prototypes -Wimplicit-function-declaration -Wstrict-prototypes -Wnested-externs"
)
set(CMAKE_CXX_FLAGS
  "${CMAKE_CXX_FLAGS} -mcpu=leon3 -g -ffunction-sections -fdata-sections -Wall -fno-exceptions -fno-rtti"
)

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

# Include RTEMS system headers and platform types if needed
# Uncomment and adjust the path if your RTEMS-specific headers are in a custom location
# include_directories(SYSTEM "${FPRIME_FRAMEWORK_PATH}/Os/RTEMS")
# include_directories(SYSTEM "${CMAKE_CURRENT_LIST_DIR}/types")

# Optional: Indicate RTEMS-specific features
set(FPRIME_USE_POSIX OFF)  # RTEMS isn’t POSIX-compliant by default