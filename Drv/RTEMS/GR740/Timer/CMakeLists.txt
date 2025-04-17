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
  "${CMAKE_CURRENT_LIST_DIR}/GR740TimerDriver.fpp"
  "${CMAKE_CURRENT_LIST_DIR}/GR740TimerDriver.cpp"
)

set_source_files_properties(
    GR740TimerDriver.cpp
    PROPERTIES
    COMPILE_FLAGS "-Wno-pedantic -Wno-error=pedantic -fpermissive"
)

# Dependencies
set(MOD_DEPS
  Fw/Logger
  Os
  Svc/Cycle
)

register_fprime_module()