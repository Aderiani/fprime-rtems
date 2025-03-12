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
register_os_implementation("Mutex;ConditionVariable" RTEMS)
register_os_implementation(Cpu RTEMS)
register_os_implementation(Memory RTEMS)
register_os_implementation(Queue RTEMS)
register_os_implementation(RawTime RTEMS)


set(FPRIME_OS_FILE_IMPL "Os_File_RTEMS" CACHE STRING "Os_File implementation for RTEMS" FORCE)
# # The remainder of the file isn't needed because register_os_implementation 
# # already does the work of setting up the libraries. If you need to add
# # custom functionality, use the following pattern:

# # Task implementation
# if (TARGET Os_Task_RTEMS)
#     target_sources(Os_Task_RTEMS PRIVATE
#         "${CMAKE_CURRENT_LIST_DIR}/Task.cpp"
#     )
#     target_link_libraries(Os_Task_RTEMS PUBLIC Fw_Types)
# endif()

# # Mutex implementation
# if (TARGET Os_Mutex_RTEMS)
#     target_sources(Os_Mutex_RTEMS PRIVATE
#         "${CMAKE_CURRENT_LIST_DIR}/Mutex.cpp"
#     )
#     target_link_libraries(Os_Mutex_RTEMS PUBLIC Fw_Types)
# endif()

# File implementation 
# if (TARGET Os_File_RTEMS)
#     target_sources(Os_File_RTEMS PRIVATE
#         "${CMAKE_CURRENT_LIST_DIR}/File.cpp"
#     )
#     target_link_libraries(Os_File_RTEMS PUBLIC Fw_Types)
# endif()

# # FileSystem implementation
# if (TARGET Os_FileSystem_RTEMS)
#     target_sources(Os_FileSystem_RTEMS PRIVATE
#         "${CMAKE_CURRENT_LIST_DIR}/FileSystem.cpp"
#     )
#     target_link_libraries(Os_FileSystem_RTEMS PUBLIC Fw_Types)
# endif()

# # Directory implementation
# if (TARGET Os_Directory_RTEMS)
#     target_sources(Os_Directory_RTEMS PRIVATE
#         "${CMAKE_CURRENT_LIST_DIR}/Directory.cpp"
#     )
#     target_link_libraries(Os_Directory_RTEMS PUBLIC Fw_Types)
# endif()

# # Queue implementation
# if (TARGET Os_Queue_RTEMS)
#     target_sources(Os_Queue_RTEMS PRIVATE
#         "${CMAKE_CURRENT_LIST_DIR}/Queue.cpp"
#     )
#     target_link_libraries(Os_Queue_RTEMS PUBLIC Fw_Types)
# endif()

# # Cpu implementation
# if (TARGET Os_Cpu_RTEMS)
#     target_sources(Os_Cpu_RTEMS PRIVATE
#         "${CMAKE_CURRENT_LIST_DIR}/Cpu.cpp"
#     )
#     target_link_libraries(Os_Cpu_RTEMS PUBLIC Fw_Types)
# endif()

# # Memory implementation
# if (TARGET Os_Memory_RTEMS)
#     target_sources(Os_Memory_RTEMS PRIVATE
#         "${CMAKE_CURRENT_LIST_DIR}/Memory.cpp"
#     )
#     target_link_libraries(Os_Memory_RTEMS PUBLIC Fw_Types)
# endif()

# # Console implementation
# if (TARGET Os_Console_RTEMS)
#     target_sources(Os_Console_RTEMS PRIVATE
#         "${CMAKE_CURRENT_LIST_DIR}/Console.cpp"
#     )
#     target_link_libraries(Os_Console_RTEMS PUBLIC Fw_Types)
# endif()

# # RawTime implementation
# if (TARGET Os_RawTime_RTEMS)
#     target_sources(Os_RawTime_RTEMS PRIVATE
#         "${CMAKE_CURRENT_LIST_DIR}/RawTime.cpp"
#     )
#     target_link_libraries(Os_RawTime_RTEMS PUBLIC Fw_Types)
# endif()

# # ConditionVariable implementation
# if (TARGET Os_ConditionVariable_RTEMS)
#     target_sources(Os_ConditionVariable_RTEMS PRIVATE
#         "${CMAKE_CURRENT_LIST_DIR}/ConditionVariable.cpp"
#     )
#     target_link_libraries(Os_ConditionVariable_RTEMS PUBLIC Fw_Types)
# endif()