####
# F prime CMakeLists.txt for Os/RTEMS:
#
# OS implementation for RTEMS on GR740
####
restrict_platforms(RTEMS)
add_custom_target("${FPRIME_CURRENT_MODULE}")

# Register the various RTEMS implementations of OS APIs
register_os_implementation(Task RTEMS)
register_os_implementation(Mutex RTEMS)
register_os_implementation(Queue RTEMS)
register_os_implementation(RawTime RTEMS)
register_os_implementation(File RTEMS)
register_os_implementation(Directory RTEMS)
register_os_implementation(Cpu RTEMS)
register_os_implementation(Memory RTEMS)
register_os_implementation(Console RTEMS)


# Set the source files for each implementation
set(TASK_SOURCE_FILES
    "${CMAKE_CURRENT_LIST_DIR}/Task.cpp"
)

set(MUTEX_SOURCE_FILES
    "${CMAKE_CURRENT_LIST_DIR}/Mutex.cpp"
)

set(QUEUE_SOURCE_FILES
    "${CMAKE_CURRENT_LIST_DIR}/Queue.cpp"
)

set(FILE_SOURCE_FILES
    "${CMAKE_CURRENT_LIST_DIR}/File.cpp"
)

set(DIRECTORY_SOURCE_FILES
    "${CMAKE_CURRENT_LIST_DIR}/Directory.cpp"
)

# Set the source files for the CPU implementation
set(CPU_SOURCE_FILES
    "${CMAKE_CURRENT_LIST_DIR}/Cpu.cpp"
)

# Set the source files for the Memory implementation
set(MEMORY_SOURCE_FILES
    "${CMAKE_CURRENT_LIST_DIR}/Memory.cpp"
)

# Set the source files for the Console implementation
set(CONSOLE_SOURCE_FILES
    "${CMAKE_CURRENT_LIST_DIR}/Console.cpp"
)

# Set the source files for the RawTime implementation
set(RAWTIME_SOURCE_FILES
    "${CMAKE_CURRENT_LIST_DIR}/RawTime.cpp"
)



# Create targets for each implementation
add_library(Os_Task_RTEMS STATIC ${TASK_SOURCE_FILES})
add_library(Os_Mutex_RTEMS STATIC ${MUTEX_SOURCE_FILES})
add_library(Os_Queue_RTEMS STATIC ${QUEUE_SOURCE_FILES})
add_library(Os_File_RTEMS STATIC ${FILE_SOURCE_FILES})
add_library(Os_Directory_RTEMS STATIC ${DIRECTORY_SOURCE_FILES})
add_library(Os_Cpu_RTEMS STATIC ${CPU_SOURCE_FILES})
add_library(Os_Memory_RTEMS STATIC ${MEMORY_SOURCE_FILES})
add_library(Os_Console_RTEMS STATIC ${CONSOLE_SOURCE_FILES})
add_library(Os_RawTime_RTEMS STATIC ${RAWTIME_SOURCE_FILES})


# Create dependencies and linking
target_link_libraries(Os_Task_RTEMS PUBLIC Fw_Types)
target_link_libraries(Os_Mutex_RTEMS PUBLIC Fw_Types)
target_link_libraries(Os_Queue_RTEMS PUBLIC Fw_Types)
target_link_libraries(Os_IntervalTimer_RTEMS PUBLIC Fw_Types)
target_link_libraries(Os_File_RTEMS PUBLIC Fw_Types)
target_link_libraries(Os_Directory_RTEMS PUBLIC Fw_Types)
target_link_libraries(Os_Cpu_RTEMS PUBLIC Fw_Types)
target_link_libraries(Os_Memory_RTEMS PUBLIC Fw_Types)
target_link_libraries(Os_Console_RTEMS PUBLIC Fw_Types)
target_link_libraries(Os_RawTime_RTEMS PUBLIC Fw_Types)
