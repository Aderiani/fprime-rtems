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
# Define the source files
# RTEMSInit/CMakeLists.txt
set(SOURCE_FILES
    "${CMAKE_CURRENT_LIST_DIR}/rtems_init.c"
    "${CMAKE_CURRENT_LIST_DIR}/board_init.c" 
    "${CMAKE_CURRENT_LIST_DIR}/rtems_cxx_support.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/stubs.c"
    "${CMAKE_CURRENT_LIST_DIR}/network_init.c"
    "${CMAKE_CURRENT_LIST_DIR}/config_leon3_drvmgr.c"
)

# Export the library for use in deployments
add_library(RTEMSInit STATIC ${SOURCE_FILES})
set_source_files_properties(rtems_init.c
    PROPERTIES 
    COMPILE_FLAGS "-Wno-error=unused-variable"
)



# Add include directories for RTEMS
target_include_directories(RTEMSInit PRIVATE
    ${RTEMS_PATH}/${CROSS_PREFIX}/include
    ${RTEMS_BSP_PATH}/lib/include
    ${RTEMS_BSP_PATH}/lib/include/bsp
    ${RTEMS_BSP_PATH}/lib/include/networking
)

if(EXISTS "${CMAKE_SYSROOT}/lib/include/grlib/l4stat.h")
    target_compile_definitions(Os_Cpu_RTEMS PRIVATE HAS_L4STAT_DRIVER)
    message(STATUS "L4STAT support enabled")
else()
    message(WARNING "L4STAT header not found, using CPU estimation")
endif()

# Disable warnings that would cause issues with RTEMS headers
target_compile_options(RTEMSInit PRIVATE
    -Wno-pedantic
    -Wno-shadow
    -Wno-error=shadow
    -Wno-error=unused-variable
    -Wno-unused-variable
    -Wno-error=format
    -Wno-error=implicit-function-declaration
    -Wno-pointer-compare
    -w
)

# Make the library available to the rest of the build
add_library(RTEMSInit::RTEMSInit ALIAS RTEMSInit)