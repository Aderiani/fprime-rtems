####
# F prime CMakeLists.txt:
#
# SOURCE_FILES: combined list of source and autocoding files
# MOD_DEPS: (optional) module dependencies
#
####

# Only build for RTEMS platform
if (NOT ${CMAKE_SYSTEM_NAME} STREQUAL "RTEMS")
    return()
endif()

set(SOURCE_FILES
  "${CMAKE_CURRENT_LIST_DIR}/GpioDriver.fpp"
  "${CMAKE_CURRENT_LIST_DIR}/GpioDriver.cpp"
)

# Dependencies
set(MOD_DEPS
  Fw/Logger
  Os
  Drv/Interfaces  # Make sure this directory exists with GpioInterface.fppi
)

register_fprime_module()