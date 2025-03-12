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

# Include RTEMS system headers and platform types
# include_directories(SYSTEM "${FPRIME_FRAMEWORK_PATH}/Os/RTEMS")