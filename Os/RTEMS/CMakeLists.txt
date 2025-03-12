# File: Os/RTEMS/CMakeLists.txt

# File: Os/RTEMS/CMakeLists.txt

####
# F prime CMakeLists.txt for Os/RTEMS:
#
# OS implementation for RTEMS
####
restrict_platforms(RTEMS)
add_custom_target("${FPRIME_CURRENT_MODULE}")

# Register all OSAL implementations for RTEMS
register_os_implementation("File;FileSystem;Directory" RTEMS)
register_os_implementation(Console RTEMS)
register_os_implementation(Task RTEMS)
register_os_implementation("Mutex" RTEMS)
register_os_implementation("ConditionVariable" RTEMS)
register_os_implementation(Cpu RTEMS)
register_os_implementation(Memory RTEMS)
register_os_implementation(Queue RTEMS)
register_os_implementation(RawTime RTEMS)

# The rest of your CMake file can remain...

# Set source files for each implementation only if implementations are needed
if (NOT "Task" IN_LIST IMPLEMENTATIONS_NEEDED)
    set(TASK_SOURCE_FILES
        "${CMAKE_CURRENT_LIST_DIR}/Task.cpp"
    )
    add_library(Os_Task_RTEMS STATIC ${TASK_SOURCE_FILES})
    target_link_libraries(Os_Task_RTEMS PUBLIC Fw_Types)
endif()

if (NOT "Mutex" IN_LIST IMPLEMENTATIONS_NEEDED)
    set(MUTEX_SOURCE_FILES
        "${CMAKE_CURRENT_LIST_DIR}/Mutex.cpp"
    )
    add_library(Os_Mutex_RTEMS STATIC ${MUTEX_SOURCE_FILES})
    target_link_libraries(Os_Mutex_RTEMS PUBLIC Fw_Types)
endif()

if (NOT "Queue" IN_LIST IMPLEMENTATIONS_NEEDED)
    set(QUEUE_SOURCE_FILES
        "${CMAKE_CURRENT_LIST_DIR}/Queue.cpp"
    )
    add_library(Os_Queue_RTEMS STATIC ${QUEUE_SOURCE_FILES})
    target_link_libraries(Os_Queue_RTEMS PUBLIC Fw_Types)
endif()


set(FILE_SOURCE_FILES
        "${CMAKE_CURRENT_LIST_DIR}/File.cpp"
)
add_library(Os_File_RTEMS STATIC ${FILE_SOURCE_FILES})
target_link_libraries(Os_File_RTEMS PUBLIC Fw_Types)

set(FILE_SOURCE_FILES
        "${CMAKE_CURRENT_LIST_DIR}/FileSystem.cpp"
)
add_library(Os_File_RTEMS STATIC ${FILE_SOURCE_FILES})
target_link_libraries(Os_File_RTEMS PUBLIC Fw_Types)


if (NOT "Directory" IN_LIST IMPLEMENTATIONS_NEEDED)
    set(DIRECTORY_SOURCE_FILES
        "${CMAKE_CURRENT_LIST_DIR}/Directory.cpp"
    )
    add_library(Os_Directory_RTEMS STATIC ${DIRECTORY_SOURCE_FILES})
    target_link_libraries(Os_Directory_RTEMS PUBLIC Fw_Types)
endif()

if (NOT "Cpu" IN_LIST IMPLEMENTATIONS_NEEDED)
    set(CPU_SOURCE_FILES
        "${CMAKE_CURRENT_LIST_DIR}/Cpu.cpp"
    )
    add_library(Os_Cpu_RTEMS STATIC ${CPU_SOURCE_FILES})
    target_link_libraries(Os_Cpu_RTEMS PUBLIC Fw_Types)
endif()

if (NOT "Memory" IN_LIST IMPLEMENTATIONS_NEEDED)
    set(MEMORY_SOURCE_FILES
        "${CMAKE_CURRENT_LIST_DIR}/Memory.cpp"
    )
    add_library(Os_Memory_RTEMS STATIC ${MEMORY_SOURCE_FILES})
    target_link_libraries(Os_Memory_RTEMS PUBLIC Fw_Types)
endif()

if (NOT "Console" IN_LIST IMPLEMENTATIONS_NEEDED)
    set(CONSOLE_SOURCE_FILES
        "${CMAKE_CURRENT_LIST_DIR}/Console.cpp"
    )
    add_library(Os_Console_RTEMS STATIC ${CONSOLE_SOURCE_FILES})
    target_link_libraries(Os_Console_RTEMS PUBLIC Fw_Types)
endif()

if (NOT "RawTime" IN_LIST IMPLEMENTATIONS_NEEDED)
    set(RAWTIME_SOURCE_FILES
        "${CMAKE_CURRENT_LIST_DIR}/RawTime.cpp"
    )
    add_library(Os_RawTime_RTEMS STATIC ${RAWTIME_SOURCE_FILES})
    target_link_libraries(Os_RawTime_RTEMS PUBLIC Fw_Types)
endif()