####
# F prime CMakeLists.txt:
#
# SOURCE_FILES: combined list of source and autocoding files
# MOD_DEPS: (optional) module dependencies
####



set(SOURCE_FILES
  "${CMAKE_CURRENT_LIST_DIR}/instances.fpp"
  # Note: Uncomment when using Svc:TlmPacketizer
  #"${CMAKE_CURRENT_LIST_DIR}/LedBlinkerPackets.xml"
  "${CMAKE_CURRENT_LIST_DIR}/topology.fpp"
  "${CMAKE_CURRENT_LIST_DIR}/LedBlinkerTopology.cpp"
)
if (${CMAKE_SYSTEM_NAME} STREQUAL "RTEMS")
  set(MOD_DEPS
    Fw/Logger
    Os
    # These will use the RTEMS implementations when on RTEMS
    Drv/RTEMS/GR740/Network
    Drv/RTEMS/GR740/GpioDriver
    Drv/RTEMS/GR740/UartDriver
    Drv/RTEMS/GR740/SpiDriver
    Drv/RTEMS/GR740/I2cDriver
    Drv/TcpServer
    Drv/TcpClient
    Drv/Udp

  )
else()
  set(MOD_DEPS
    Fw/Logger
    Os
    Drv/TcpServer
    Drv/Udp
  )
endif()
register_fprime_module()
