// ======================================================================
// \title Os/RTEMS/ConditionVariable.cpp
// \brief RTEMS implementation for Os::ConditionVariable using POSIX API
// ======================================================================
#include "Os/RTEMS/ConditionVariable.hpp"
#include "Os/Delegate.hpp"
#include <Fw/Types/Assert.hpp>
#include <pthread.h>
namespace Os {
namespace RTEMS {
namespace ConditionVariable {

// Constructor
RtemsConditionVariable::RtemsConditionVariable() {
    int status = pthread_cond_init(&m_handle.condition_id, NULL);
    FW_ASSERT(status == 0, static_cast<FwAssertArgType>(status));
}

// Destructor
RtemsConditionVariable::~RtemsConditionVariable() {
    int status = pthread_cond_destroy(&m_handle.condition_id);
    FW_ASSERT(status == 0, static_cast<FwAssertArgType>(status));
}

// Wait on condition_id variable
ConditionVariableInterface::Status RtemsConditionVariable::pend(Os::Mutex& mutex) {
    // Assuming Os::Mutex delegates to an Os::MutexInterface implementation
    Os::MutexInterface* mutex_if = mutex.m_delegate;  // Adjusted based on F´ pattern
    Os::RTEMS::Mutex::RTEMSMutex::MutexHandle* rtems_mutex_handle =
        static_cast<Os::RTEMS::Mutex::RTEMSMutex::MutexHandle*>(mutex_if->getHandle());

    int status = pthread_cond_wait(&m_handle.condition_id, &rtems_mutex_handle->mutex);
    if (status == 0) {
        return Status::OP_OK;
    } else {
        return Status::ERROR_OTHER;
    }
}

// Signal one thread
void RtemsConditionVariable::notify() {
    int status = pthread_cond_signal(&m_handle.condition_id);
    FW_ASSERT(status == 0, static_cast<FwAssertArgType>(status));
}

// Broadcast to all threads
void RtemsConditionVariable::notifyAll() {
    int status = pthread_cond_broadcast(&m_handle.condition_id);
    FW_ASSERT(status == 0, static_cast<FwAssertArgType>(status));
}

// Get handle to condition_id variable
ConditionVariableHandle* RtemsConditionVariable::getHandle() {
    return &m_handle;
}

} // namespace ConditionVariable
} // namespace RTEMS
} // namespace Os

namespace Os {
// Delegate factory function
ConditionVariableInterface* ConditionVariableInterface::getDelegate(ConditionVariableHandleStorage& aligned_new_memory) {
    return Os::Delegate::makeDelegate<ConditionVariableInterface, Os::RTEMS::ConditionVariable::RtemsConditionVariable>(aligned_new_memory);
}
} // namespace Os