####
# F prime CMakeLists.txt for GR740 RTEMS drivers:
#
# This directory contains drivers specific to the GR740 platform
# running RTEMS.
####

# Add all driver subdirectories
add_fprime_subdirectory("${CMAKE_CURRENT_LIST_DIR}/GpioDriver/")
add_fprime_subdirectory("${CMAKE_CURRENT_LIST_DIR}/I2cDriver/")
add_fprime_subdirectory("${CMAKE_CURRENT_LIST_DIR}/SpiDriver/")
add_fprime_subdirectory("${CMAKE_CURRENT_LIST_DIR}/UartDriver/")
add_fprime_subdirectory("${CMAKE_CURRENT_LIST_DIR}/Network/")