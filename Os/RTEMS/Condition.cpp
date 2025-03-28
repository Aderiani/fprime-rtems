// Os/RTEMS/Condition.cpp
#include "Os/RTEMS/ConditionVariable.hpp"
#include "Os/Delegate.hpp"
#include "Os/RTEMS/Mutex.hpp"
#include <Fw/Types/Assert.hpp>

namespace Os {
namespace RTEMS {
namespace ConditionVariable {



RtemsConditionVariable::RtemsConditionVariable() {
    pthread_condattr_t attr;
    pthread_condattr_init(&attr);
    int status = pthread_cond_init(&m_handle.condition, &attr);
    pthread_condattr_destroy(&attr);
    FW_ASSERT(status == 0, static_cast<FwAssertArgType>(status));
}

RtemsConditionVariable::Status RtemsConditionVariable::pend(Os::Mutex& mutex) {
    // Get the mutex handle
    Os::MutexHandle* mutex_handle = mutex.getHandle();
    auto* rtems_mutex_handle = static_cast<Os::RTEMS::Mutex::RTEMSMutexHandle*>(mutex_handle);
    
    // Use pthread_cond_wait with appropriate error handling
    int status = pthread_cond_wait(&m_handle.condition, &rtems_mutex_handle->posix_mutex);
    
    if (status != 0) {
        return RtemsConditionVariable::Status::ERROR_OTHER;
    }
    return RtemsConditionVariable::Status::OP_OK;
}

void RtemsConditionVariable::notify() {
    int status = pthread_cond_signal(&m_handle.condition);
    FW_ASSERT(status == 0, static_cast<FwAssertArgType>(status));
}

void RtemsConditionVariable::notifyAll() {
    int status = pthread_cond_broadcast(&m_handle.condition);
    FW_ASSERT(status == 0, static_cast<FwAssertArgType>(status));
}

} // namespace ConditionVariable
} // namespace RTEMS
} // namespace Os
