####
# F prime CMakeLists.txt for RTEMS IP implementations
####

# Only build for RTEMS platform
if (NOT ${CMAKE_SYSTEM_NAME} STREQUAL "RTEMS")
    return()
endif()

# Define source files for server
set(TCP_SERVER_SOURCE_FILES
  "${CMAKE_CURRENT_LIST_DIR}/RtemsTcpServerAdapter.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/DefaultTcpServerAdapter.cpp"
)

# Define source files for client
set(TCP_CLIENT_SOURCE_FILES
  "${CMAKE_CURRENT_LIST_DIR}/RtemsTcpClientAdapter.cpp" 
  "${CMAKE_CURRENT_LIST_DIR}/DefaultTcpClientAdapter.cpp"
)

# Define source files for UDP
set(UDP_SOURCE_FILES
  "${CMAKE_CURRENT_LIST_DIR}/RtemsUdpAdapter.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/DefaultUdpAdapter.cpp"
)

# Network initialization
set(NETWORK_SOURCE_FILES
  "${CMAKE_CURRENT_LIST_DIR}/GR740NetworkInit.cpp"
)

# Common dependencies
set(IP_MOD_DEPS
  Fw/Logger
  Drv/Ip
)

# Create the individual modules first
set(SOURCE_FILES ${TCP_SERVER_SOURCE_FILES})
set(MOD_DEPS ${IP_MOD_DEPS})
set(MODULE_NAME "Drv_TcpServer_RTEMS")
register_fprime_module()

set(SOURCE_FILES ${TCP_CLIENT_SOURCE_FILES})
set(MOD_DEPS ${IP_MOD_DEPS})
set(MODULE_NAME "Drv_TcpClient_RTEMS")
register_fprime_module()

set(SOURCE_FILES ${UDP_SOURCE_FILES})
set(MOD_DEPS ${IP_MOD_DEPS})
set(MODULE_NAME "Drv_Udp_RTEMS")
register_fprime_module()

set(SOURCE_FILES ${NETWORK_SOURCE_FILES})
set(MOD_DEPS ${IP_MOD_DEPS})
set(MODULE_NAME "Drv_Network_RTEMS")
register_fprime_module()

# Now register them as implementations
register_fprime_implementation("Drv_TcpServer" "Drv_TcpServer_RTEMS")
register_fprime_implementation("Drv_TcpClient" "Drv_TcpClient_RTEMS")
register_fprime_implementation("Drv_Udp" "Drv_Udp_RTEMS")