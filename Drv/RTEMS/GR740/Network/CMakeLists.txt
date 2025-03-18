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
  "${CMAKE_CURRENT_LIST_DIR}/NetworkComponent.fpp"
  "${CMAKE_CURRENT_LIST_DIR}/NetworkComponent.cpp"
)

# Dependencies
set(MOD_DEPS
  Fw/Logger
  Fw/Cmd
  Fw/Tlm
  Fw/Prm
  Fw/Log
  Fw/Time
  Fw/Comp
  Fw/Port
)

register_fprime_module()