####
# F prime CMakeLists.txt for GR740 Timer Driver:
#
# This timer driver uses GPTIMER hardware with ISR support
# for driving F' rate groups on GR740/RTEMS systems
####

set(SOURCE_FILES
    "${CMAKE_CURRENT_LIST_DIR}/GR740TimerDriver.fpp"
    "${CMAKE_CURRENT_LIST_DIR}/GR740TimerDriver.cpp"
)

# Platform restrictions - only for RTEMS/GR740
restrict_platforms("RTEMS" "gr740")

# Module dependencies
set(MOD_DEPS
    Fw/Time
    Fw/Types
    Fw/Logger
    Os
    Svc/Cycle
)

# Register the F' module
register_fprime_module()

# Link RTEMS Timer Library (TLIB) for GPTIMER support
if(CMAKE_SYSTEM_NAME STREQUAL "RTEMS")
    target_link_libraries(${FPRIME_CURRENT_MODULE} 
        PRIVATE 
        tlib    # RTEMS Timer Library for GPTIMER access
    )
    
    # Add compiler definitions for GPTIMER support
    target_compile_definitions(${FPRIME_CURRENT_MODULE} 
        PRIVATE 
        USE_GPTIMER_HARDWARE=1
    )
endif()

# # Unit test files
# set(UT_SOURCE_FILES
#     "${CMAKE_CURRENT_LIST_DIR}/GR740TimerDriver.fpp"
#     "${CMAKE_CURRENT_LIST_DIR}/test/ut/GR740TimerDriverTester.cpp"
#     "${CMAKE_CURRENT_LIST_DIR}/test/ut/main.cpp"
# )

# # Register unit tests (only if not cross-compiling)
# if(NOT CMAKE_CROSSCOMPILING)
#     register_fprime_ut()
# endif()