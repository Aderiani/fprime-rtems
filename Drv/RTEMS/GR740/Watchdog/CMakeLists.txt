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
  "${CMAKE_CURRENT_LIST_DIR}/GR740WatchdogDriver.fpp"
  "${CMAKE_CURRENT_LIST_DIR}/GR740WatchdogDriver.cpp"
)

# Dependencies
set(MOD_DEPS
  Fw/Logger
  Drv/ByteStreamDriverModel
  Os
)

register_fprime_module()