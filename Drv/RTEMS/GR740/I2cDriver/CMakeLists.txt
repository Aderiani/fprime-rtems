####
# F prime CMakeLists.txt:
#
# SOURCE_FILES: combined list of source and autocoding files
# MOD_DEPS: (optional) module dependencies
#
####
set(SOURCE_FILES
  "${CMAKE_CURRENT_LIST_DIR}/GR740I2cDriver.fpp"
  "${CMAKE_CURRENT_LIST_DIR}/GR740I2cDriver.cpp"
)

set(MOD_DEPS
  Fw/Types
  Fw/Log
  Os
)

register_fprime_module()