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
  "${CMAKE_CURRENT_LIST_DIR}/RtemsTcpServerAdapter.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/RtemsTcpClientAdapter.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/GR740NetworkInit.cpp"
)

# Dependencies
set(MOD_DEPS
  Fw/Logger
  Drv/Ip
)

register_fprime_module()

# Register the RTEMS IP implementations
register_fprime_implementation("Drv_TcpServer" "Drv_TcpServer_RTEMS")
register_fprime_implementation("Drv_TcpClient" "Drv_TcpClient_RTEMS")