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

# First, create the RTEMS implementations of the IP interfaces
add_library(Drv_TcpServer_RTEMS STATIC
  "${CMAKE_CURRENT_LIST_DIR}/RtemsTcpServerAdapter.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/DefaultTcpServerAdapter.cpp"
)

add_library(Drv_TcpClient_RTEMS STATIC
  "${CMAKE_CURRENT_LIST_DIR}/RtemsTcpClientAdapter.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/DefaultTcpClientAdapter.cpp"
)

add_library(Drv_Udp_RTEMS STATIC
  "${CMAKE_CURRENT_LIST_DIR}/RtemsUdpAdapter.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/DefaultUdpAdapter.cpp"
)

# Set dependencies
target_link_libraries(Drv_TcpServer_RTEMS PUBLIC Drv_Ip Fw_Logger)
target_link_libraries(Drv_TcpClient_RTEMS PUBLIC Drv_Ip Fw_Logger)
target_link_libraries(Drv_Udp_RTEMS PUBLIC Drv_Ip Fw_Logger)

# Then create the module with the GR740 network initialization
set(SOURCE_FILES
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
register_fprime_implementation("Drv_Udp" "Drv_Udp_RTEMS")