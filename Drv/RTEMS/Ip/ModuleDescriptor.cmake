####
# This file registers the RTEMS IP implementations with the build system
####

# Only active when targeting RTEMS
if (NOT ${CMAKE_SYSTEM_NAME} STREQUAL "RTEMS")
    return()
endif()

# Define the implementations
add_library(Drv_TcpServer_RTEMS STATIC)
add_library(Drv_TcpClient_RTEMS STATIC)
add_library(Drv_Udp_RTEMS STATIC)

# Add sources
target_sources(Drv_TcpServer_RTEMS PRIVATE
  "${CMAKE_CURRENT_LIST_DIR}/RtemsTcpServerAdapter.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/DefaultTcpServerAdapter.cpp"
)

target_sources(Drv_TcpClient_RTEMS PRIVATE
  "${CMAKE_CURRENT_LIST_DIR}/RtemsTcpClientAdapter.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/DefaultTcpClientAdapter.cpp"
)

target_sources(Drv_Udp_RTEMS PRIVATE
  "${CMAKE_CURRENT_LIST_DIR}/RtemsUdpAdapter.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/DefaultUdpAdapter.cpp"
)

# Set dependencies
target_link_libraries(Drv_TcpServer_RTEMS PUBLIC Fw_Logger)
target_link_libraries(Drv_TcpClient_RTEMS PUBLIC Fw_Logger)
target_link_libraries(Drv_Udp_RTEMS PUBLIC Fw_Logger)

# Register implementations
register_fprime_implementation(Drv_TcpServer Drv_TcpServer_RTEMS)
register_fprime_implementation(Drv_TcpClient Drv_TcpClient_RTEMS)
register_fprime_implementation(Drv_Udp Drv_Udp_RTEMS)

# Create interface targets for headers
add_library(Drv_RTEMS_Ip_Headers INTERFACE)
target_include_directories(Drv_RTEMS_Ip_Headers INTERFACE "${CMAKE_CURRENT_LIST_DIR}")

# Link headers to implementations
target_link_libraries(Drv_TcpServer_RTEMS PUBLIC Drv_RTEMS_Ip_Headers)
target_link_libraries(Drv_TcpClient_RTEMS PUBLIC Drv_RTEMS_Ip_Headers)
target_link_libraries(Drv_Udp_RTEMS PUBLIC Drv_RTEMS_Ip_Headers)