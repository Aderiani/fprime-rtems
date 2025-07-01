// ======================================================================
// \title Os/RTEMS/Task.cpp
// \brief RTEMS implementation for Os::Task
// ======================================================================
#include <Os/RTEMS/Task.hpp>
#include <Fw/Types/Assert.hpp>
#include <rtems.h>
#include <rtems/score/schedulerpriorityaffinitysmp.h>
#include <rtems/cpuuse.h>
#include <cstring>
#include <cstdio>  // Add this for printf

namespace Os {
namespace RTEMS {
namespace Task {

// Static CPU distribution counter
static int next_cpu_assignment = 0;

static int get_next_cpu() {
    int cpu = next_cpu_assignment;
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wdeprecated-declarations"
    next_cpu_assignment = (next_cpu_assignment + 1) % rtems_get_processor_count();
    #pragma GCC diagnostic pop
    return cpu;
}



// Static wrapper to match RTEMS task entry point signature
static rtems_task task_wrapper(rtems_task_argument arg) {
    // Recover the original task routine and argument
    struct {
        TaskInterface::taskRoutine routine;
        void* arg;
    } *params = reinterpret_cast<decltype(params)>(arg);

    // Call the original task routine
    params->routine(params->arg);

    // Free the parameters structure
    delete params;

    // Task must delete itself when complete
    rtems_task_delete(RTEMS_SELF);
}

void RTEMSTask::onStart() {}

TaskInterface::Status RTEMSTask::start(const Arguments& arguments) {
    FW_ASSERT(arguments.m_routine != nullptr);

    // We need to pass both the routine and argument to the wrapper
    struct TaskParams {
        TaskInterface::taskRoutine routine;
        void* arg;
    };
    
    // Create the parameters structure
    TaskParams* params = new TaskParams;
    params->routine = arguments.m_routine;
    params->arg = arguments.m_routine_argument;

    // Create RTEMS task
    rtems_name task_name;
    // Convert string name to rtems_name (4 chars)
    const char* name_str = arguments.m_name.toChar();
    task_name = rtems_build_name(
        name_str[0] ? name_str[0] : ' ',
        name_str[1] ? name_str[1] : ' ',
        name_str[2] ? name_str[2] : ' ',
        name_str[3] ? name_str[3] : ' '
    );
    
    // Set up task priority - convert from F' to RTEMS (higher values are higher priority in RTEMS)
    rtems_task_priority priority;
    if (arguments.m_priority == TASK_DEFAULT) {
        priority = 50;  // Default middle priority
    } else {
        // Map F' priority 0-255 to RTEMS 1-255 (invert since in RTEMS higher value = higher priority)
        priority = static_cast<rtems_task_priority>(255 - (arguments.m_priority > 254 ? 254 : arguments.m_priority));
    }

    // Set up stack size
    size_t stack_size = (arguments.m_stackSize != TASK_DEFAULT) ? 
        static_cast<size_t>(arguments.m_stackSize) : 
        static_cast<size_t>(RTEMS_MINIMUM_STACK_SIZE * 4);  // Default stack size

    // Create and start the task
    rtems_status_code status = rtems_task_create(
        task_name,
        priority,
        stack_size,
        RTEMS_DEFAULT_MODES,
        RTEMS_DEFAULT_ATTRIBUTES,
        &this->m_handle.task_id
    );

    if (status != RTEMS_SUCCESSFUL) {
        delete params;  // Clean up if task creation fails
        return TaskInterface::Status::ERROR_RESOURCES;
    }

    // Set CPU affinity for SMP
    #ifdef RTEMS_SMP
    {
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        
        // Distribute tasks across CPUs in round-robin fashion
        int target_cpu = get_next_cpu();
        CPU_SET(target_cpu, &cpuset);
        
        rtems_status_code affinity_status = rtems_task_set_affinity(
            this->m_handle.task_id, 
            sizeof(cpuset), 
            &cpuset
        );
        
        if (affinity_status == RTEMS_SUCCESSFUL) {
            printf("Task '%s' assigned to CPU %d\n", name_str, target_cpu);
        }
    }
    #endif

    // Start the task
    status = rtems_task_start(
        this->m_handle.task_id,
        task_wrapper,
        reinterpret_cast<rtems_task_argument>(params)
    );

    if (status != RTEMS_SUCCESSFUL) {
        rtems_task_delete(this->m_handle.task_id);
        delete params;  // Clean up if task start fails
        return TaskInterface::Status::UNKNOWN_ERROR;
    }

    return TaskInterface::Status::OP_OK;
}

TaskInterface::Status RTEMSTask::join() {
    // RTEMS doesn't have a direct equivalent to join, but we can check if the task exists
    rtems_status_code status = rtems_task_is_suspended(this->m_handle.task_id);
    if (status != RTEMS_SUCCESSFUL) {
        return TaskInterface::Status::JOIN_ERROR;
    }
    return TaskInterface::Status::OP_OK;
}

TaskHandle* RTEMSTask::getHandle() {
    return &this->m_handle;
}

void RTEMSTask::suspend(SuspensionType /*suspensionType*/) {
    rtems_status_code status = rtems_task_suspend(this->m_handle.task_id);
    FW_ASSERT(status == RTEMS_SUCCESSFUL, static_cast<FwAssertArgType>(status));
}

void RTEMSTask::resume() {
    rtems_status_code status = rtems_task_resume(this->m_handle.task_id);
    FW_ASSERT(status == RTEMS_SUCCESSFUL, static_cast<FwAssertArgType>(status));
}

TaskInterface::Status RTEMSTask::_delay(Fw::TimeInterval interval) {
    uint32_t microseconds = interval.getUSeconds() + interval.getSeconds() * 1000000;
    
    // Get ticks per second using the RTEMS macro (which returns a value)
    uint32_t ticks_per_second = rtems_clock_get_ticks_per_second();
    
    // Calculate ticks to delay
    uint32_t ticks = (microseconds * ticks_per_second) / 1000000;
    
    rtems_status_code status = rtems_task_wake_after(ticks);
    if (status != RTEMS_SUCCESSFUL) {
        return TaskInterface::Status::DELAY_ERROR;
    }
    
    return TaskInterface::Status::OP_OK;
}

TaskInterface::Status RTEMSTask::setCpuAffinity(int cpu_num) {
    #ifdef RTEMS_SMP
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    
    if (cpu_num < 0) {
        // Negative value means use all CPUs
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wdeprecated-declarations"
        int processor_count = rtems_get_processor_count();
        #pragma GCC diagnostic pop
        
        for (int i = 0; i < processor_count; i++) {
            CPU_SET(i, &cpuset);
        }
    } else {
        // Set specific CPU (with bounds checking)
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wdeprecated-declarations"
        int max_cpu = rtems_get_processor_count() - 1;
        #pragma GCC diagnostic pop
        
        int target_cpu = (cpu_num > max_cpu) ? max_cpu : cpu_num;
        CPU_SET(target_cpu, &cpuset);
    }
    
    rtems_status_code status = rtems_task_set_affinity(
        this->m_handle.task_id, 
        sizeof(cpuset), 
        &cpuset
    );
    
    if (status != RTEMS_SUCCESSFUL) {
        return TaskInterface::Status::UNKNOWN_ERROR;
    }
    #endif
    
    return TaskInterface::Status::OP_OK;
}

void RTEMSTask::printCpuStats() {
    #ifdef RTEMS_SMP
    printf("\n=== CPU Task Distribution ===\n");
    
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wdeprecated-declarations"
    printf("Total CPUs: %d\n", rtems_get_processor_count());
    printf("Current CPU: %d\n", rtems_get_current_processor());
    #pragma GCC diagnostic pop
    
    // This will print detailed CPU usage
    rtems_cpu_usage_report();
    #else
    printf("SMP not enabled\n");
    #endif
}

}  // namespace Task
}  // namespace RTEMS
}  // namespace Os