####
# F prime CMakeLists.txt:
#
# SOURCE_FILES: combined list of source and autocoding files
# MOD_DEPS: (optional) module dependencies
#
####
set(SOURCE_FILES
  "${CMAKE_CURRENT_LIST_DIR}/GpioInterface.fppi"
  "${CMAKE_CURRENT_LIST_DIR}/I2cInterface.fppi"
  "${CMAKE_CURRENT_LIST_DIR}/SpiInterface.fppi"
  "${CMAKE_CURRENT_LIST_DIR}/ByteStreamDriverInterface.fppi"
  "${CMAKE_CURRENT_LIST_DIR}/TickInterface.fppi"
  "${CMAKE_CURRENT_LIST_DIR}/Stub.cpp"
)

register_fprime_module()