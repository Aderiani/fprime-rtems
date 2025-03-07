// ======================================================================
// \title Os/RTEMS/DefaultTask.cpp
// \brief sets default Os::Task to RTEMS implementation via linker
// ======================================================================
#include "Os/Task.hpp"
#include "Os/RTEMS/Task.hpp"
#include "Os/Delegate.hpp"

namespace Os {
TaskInterface* TaskInterface::getDelegate(TaskHandleStorage& aligned_new_memory) {
    return Os::Delegate::makeDelegate<TaskInterface, Os::RTEMS::Task::RTEMSTask>(aligned_new_memory);
}
}