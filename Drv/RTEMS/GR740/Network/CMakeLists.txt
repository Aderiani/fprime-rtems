####
# F prime CMakeLists.txt:
#
# SOURCE_FILES: combined list of source and autocoding files
# MOD_DEPS: (optional) module dependencies
#
####

# Only build for RTEMS platform
if (NOT ${CMAKE_SYSTEM_NAME} STREQUAL "RTEMS")
  add_definitions(-D_GNU_SOURCE)
endif()


set(SOURCE_FILES
"${CMAKE_CURRENT_LIST_DIR}/GR740NetworkComponent.fpp"
"${CMAKE_CURRENT_LIST_DIR}/GR740NetworkComponent.cpp"
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

set_source_files_properties(
  NetworkComponent.cpp
  PROPERTIES
  COMPILE_FLAGS "-Wno-error=shadow -Wno-shadow -Wno-error=unused-variable -Wno-error=attributes"
)


register_fprime_module()

# target_link_libraries(Drv_RTEMS_GR740_Network PRIVATE grlib)
target_compile_options(Drv_RTEMS_GR740_Network PRIVATE -Wno-error=shadow)