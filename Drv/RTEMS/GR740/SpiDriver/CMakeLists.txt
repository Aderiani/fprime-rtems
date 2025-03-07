####
# F prime CMakeLists.txt:
#
# SOURCE_FILES: combined list of source and autocoding files
# MOD_DEPS: (optional) module dependencies
#
####
set(SOURCE_FILES
  "${CMAKE_CURRENT_LIST_DIR}/GR740GpioDriver.fpp"
  "${CMAKE_CURRENT_LIST_DIR}/GR740GpioDriver.cpp"
)

set(MOD_DEPS
  Fw/Types
  Fw/Log
  Os
)

register_fprime_module()