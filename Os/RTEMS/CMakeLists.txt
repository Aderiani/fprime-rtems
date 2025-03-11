# File: Os/RTEMS/CMakeLists.txt

####
# F prime CMakeLists.txt for Os/RTEMS:
#
# OS implementation for RTEMS
####
restrict_platforms(RTEMS)
add_custom_target("${FPRIME_CURRENT_MODULE}")

# Only register implementations if not already registered
# This helps avoid duplicate target errors
set(IMPLEMENTATIONS_NEEDED)
function(check_and_add_implementation IMPL)
    get_property(ALREADY_REGISTERED GLOBAL PROPERTY "${IMPL}_REGISTERED" SET)
    if (NOT ALREADY_REGISTERED)
        list(APPEND IMPLEMENTATIONS_NEEDED ${IMPL})
        set_property(GLOBAL PROPERTY "${IMPL}_REGISTERED" TRUE)
    endif()
endfunction()

# Check which implementations to register
check_and_add_implementation(Task)
check_and_add_implementation(Mutex)
check_and_add_implementation(Queue)
check_and_add_implementation(RawTime)
check_and_add_implementation(File)
check_and_add_implementation(FileSystem)
check_and_add_implementation(Directory)
check_and_add_implementation(Cpu)
check_and_add_implementation(Memory)
check_and_add_implementation(Console)

# Only register implementations if needed
foreach(IMPL IN LISTS IMPLEMENTATIONS_NEEDED)
    register_os_implementation(${IMPL} RTEMS)
endforeach()

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

if (NOT "File" IN_LIST IMPLEMENTATIONS_NEEDED)
    set(FILE_SOURCE_FILES
        "${CMAKE_CURRENT_LIST_DIR}/File.cpp"
    )
    add_library(Os_File_RTEMS STATIC ${FILE_SOURCE_FILES})
    target_link_libraries(Os_File_RTEMS PUBLIC Fw_Types)
endif()

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