# File: Os/RTEMS/CMakeLists.txt

####
# F prime CMakeLists.txt for Os/RTEMS:
#
# OS implementation for RTEMS
####
restrict_platforms(RTEMS)
add_custom_target("${FPRIME_CURRENT_MODULE}")

# Register all OSAL implementations for RTEMS
register_os_implementation(File RTEMS)
message(STATUS "Registered Os_File_RTEMS: ${Os_File_RTEMS}")
register_os_implementation(FileSystem RTEMS)
register_os_implementation(Directory RTEMS)
register_os_implementation(Console RTEMS)
register_os_implementation(Task RTEMS)
register_os_implementation(Mutex RTEMS)
register_os_implementation(Queue RTEMS)
register_os_implementation(Cpu RTEMS)
register_os_implementation(Memory RTEMS)
register_os_implementation(RawTime RTEMS)
register_os_implementation(Condition RTEMS)  # Add this line to register Condition implementation

set(FPRIME_OS_FILE_IMPL "Os_File_RTEMS" CACHE STRING "Os_File implementation for RTEMS" FORCE)