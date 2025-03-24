// Os/RTEMS/Condition.cpp
#include "Os/RTEMS/ConditionVariable.hpp"
#include "Os/Delegate.hpp"
#include "Os/RTEMS/Mutex.hpp"
#include <Fw/Types/Assert.hpp>

namespace Os {
namespace RTEMS {
namespace ConditionVariable {

// Constructor
RtemsConditionVariable::RtemsConditionVariable() {
    int status = pthread_cond_init(&m_handle.condition, nullptr);
    FW_ASSERT(status == 0, static_cast<FwAssertArgType>(status));
}

// Destructor
RtemsConditionVariable::~RtemsConditionVariable() {
    int status = pthread_cond_destroy(&m_handle.condition);
    FW_ASSERT(status == 0, static_cast<FwAssertArgType>(status));
}

// Wait on condition variable
ConditionVariableInterface::Status RtemsConditionVariable::pend(Os::Mutex& mutex) {
    // Get the mutex handle
    Os::MutexHandle* mutex_handle = mutex.getHandle();
    
    // Cast to RTEMS mutex handle
    auto* rtems_mutex_handle = static_cast<Os::RTEMS::Mutex::RTEMSMutexHandle*>(mutex_handle);
    
    // Wait on condition with the POSIX mutex
    int status = pthread_cond_wait(&m_handle.condition, &rtems_mutex_handle->posix_mutex);
    
    if (status == 0) {
        return Status::OP_OK;
    }
    return Status::ERROR_OTHER;
}

// Signal one thread
void RtemsConditionVariable::notify() {
    int status = pthread_cond_signal(&m_handle.condition);
    FW_ASSERT(status == 0, static_cast<FwAssertArgType>(status));
}

// Broadcast to all threads
void RtemsConditionVariable::notifyAll() {
    int status = pthread_cond_broadcast(&m_handle.condition);
    FW_ASSERT(status == 0, static_cast<FwAssertArgType>(status));
}

// Get handle to condition variable
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