####
# RTEMSInit CMakeLists.txt:
#
# Build the RTEMS initialization module that serves as the entry point
# for the RTEMS application.
####

# Only build this module for RTEMS platform
if (NOT "${CMAKE_SYSTEM_NAME}" STREQUAL "RTEMS")
    return()
endif()

# Define the source files
set(SOURCE_FILES
    "${CMAKE_CURRENT_LIST_DIR}/rtems_init.c"
)

# Export the library for use in deployments
add_library(RTEMSInit STATIC ${SOURCE_FILES})

# Add include directories for RTEMS
target_include_directories(RTEMSInit PRIVATE
    ${RTEMS_PATH}/${CROSS_PREFIX}/include
    ${RTEMS_BSP_PATH}/lib/include
    ${RTEMS_BSP_PATH}/lib/include/bsp
    ${RTEMS_BSP_PATH}/lib/include/networking
)

# Disable warnings that would cause issues with RTEMS headers
target_compile_options(RTEMSInit PRIVATE
    -Wno-pedantic
    -Wno-shadow
    -Wno-error=shadow
    -Wno-error=unused-variable
    -Wno-unused-variable
    -Wno-error=format
    -Wno-implicit-function-declaration
    -Wno-error=implicit-function-declaration
    -Wno-error=nested-externs
    -Wno-nested-externs
    -Wno-pointer-compare
    -w
)

# Make the library available to the rest of the build
add_library(RTEMSInit::RTEMSInit ALIAS RTEMSInit)