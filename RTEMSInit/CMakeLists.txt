
# Only build for RTEMS platform
if (NOT ${CMAKE_SYSTEM_NAME} STREQUAL "RTEMS")
    return()
endif()

set(SOURCE_FILES
    "${CMAKE_CURRENT_LIST_DIR}/rtems_init.c"
    "${CMAKE_CURRENT_LIST_DIR}/network_init.c"
    "${CMAKE_CURRENT_LIST_DIR}/rtems_cxx_support.cpp"
)

# Dependencies
set(MOD_DEPS
    Fw/Logger
    Os
)


set_source_files_properties(rtems_init.c PROPERTIES COMPILE_FLAGS "-Wall -Wextra")

# Add include directories for RTEMS specific headers
include_directories(SYSTEM 
    ${RTEMS_BSP_PATH}/lib/include
    ${RTEMS_BSP_PATH}/lib/include/grlib
)

# Register as a module
register_fprime_module()