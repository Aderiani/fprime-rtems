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
else()
    set(MOD_DEPS ${FPRIME_CURRENT_MODULE}/Top)
endif()

register_fprime_deployment()



# RTEMS-specific target modifications
if (${CMAKE_SYSTEM_NAME} STREQUAL "RTEMS")
    # Get the target name
    get_property(TARGET_NAME TARGET ${FPRIME_CURRENT_MODULE} PROPERTY NAME)
        
    # Set the output executable name
    set_target_properties(${TARGET_NAME} PROPERTIES 
        OUTPUT_NAME "${TARGET_NAME}"
    )
    
    # Link with RTEMSInit library
    target_link_libraries(${TARGET_NAME} PRIVATE RTEMSInit)
    
    # Link with required RTEMS libraries
    target_link_libraries(${TARGET_NAME} PRIVATE
        rtemscpu
        rtemsbsp
    )
endif()