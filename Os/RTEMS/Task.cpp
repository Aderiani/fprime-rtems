// ======================================================================
// \title Os/RTEMS/Task.cpp
// \brief RTEMS implementation for Os::Task
// ======================================================================
#include <Os/RTEMS/Task.hpp>
#include <Fw/Types/Assert.hpp>
#include <rtems.h>
#include <cstring>

namespace Os {
namespace RTEMS {
namespace Task {

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

    // Define the structure type before using it in new
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
        priority = 255 - (arguments.m_priority > 254 ? 254 : arguments.m_priority);
    }

    // Set up stack size
    size_t stack_size = (arguments.m_stackSize != TASK_DEFAULT) ? 
        arguments.m_stackSize : 
        RTEMS_MINIMUM_STACK_SIZE * 4;  // Default stack size

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

}  // namespace Task
}  // namespace RTEMS
}  // namespace Os