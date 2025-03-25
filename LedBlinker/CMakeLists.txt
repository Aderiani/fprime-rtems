#####
# 'LedBlinker' Deployment:
#
# This registers the 'LedBlinker' deployment to the build system. 
# Custom components that have not been added at the project-level should be added to 
# the list below.
#
#####

###
# Topology and Components
###
add_fprime_subdirectory("${CMAKE_CURRENT_LIST_DIR}/Top/")

# Add custom components to this specific deployment here
# add_fprime_subdirectory("${CMAKE_CURRENT_LIST_DIR}")

set(SOURCE_FILES
    "${CMAKE_CURRENT_LIST_DIR}/Main.cpp"
)

# Add RTEMSInit dependency for RTEMS builds
if (${CMAKE_SYSTEM_NAME} STREQUAL "RTEMS")
    set(MOD_DEPS ${FPRIME_CURRENT_MODULE}/Top RTEMSInit)
    
    # Extra link libraries needed for RTEMS
    set(FPRIME_LINK_LIBRARIES 
        ${FPRIME_LINK_LIBRARIES} 
        rtemscpu 
        rtemsbsp 
        c
        m
        gcc
    )
else()
    set(MOD_DEPS ${FPRIME_CURRENT_MODULE}/Top)
endif()

register_fprime_deployment()

# For RTEMS, we need to add special link flags to ensure all symbols are resolved
if (${CMAKE_SYSTEM_NAME} STREQUAL "RTEMS")
    # Get the target name
    get_property(TARGET_NAME TARGET ${FPRIME_CURRENT_MODULE} PROPERTY NAME)
    
    # Add custom link flags for the RTEMS target
    target_link_options(${TARGET_NAME} PRIVATE
        -Wl,--gc-sections 
        -Wl,--wrap=printf 
        -Wl,--wrap=puts 
        -Wl,--wrap=putchar
    )
    
    # Set the output executable name to include .exe extension
    set_target_properties(${TARGET_NAME} PROPERTIES
        OUTPUT_NAME ${TARGET_NAME}.exe
    )
    
    # Ensure RTEMSInit object files are included
    target_link_libraries(${TARGET_NAME} PRIVATE RTEMSInit)
    
    # Make sure the executable knows where to find symbols
    target_link_directories(${TARGET_NAME} PRIVATE
        ${RTEMS_BSP_PATH}/lib
        ${RTEMS_PATH}/${CROSS_PREFIX}/lib
    )
endif()