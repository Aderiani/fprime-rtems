#include "Os/RTEMS/Condition.hpp"
#include "Os/Delegate.hpp"
#include "Os/RTEMS/Mutex.hpp"
#include <Fw/Types/Assert.hpp>
#include <cstdio>  // For printf

namespace Os {
namespace RTEMS {
namespace ConditionVariable {

RtemsConditionVariable::RtemsConditionVariable() {
    int status = pthread_cond_init(&m_handle.condition, NULL);
    FW_ASSERT(status == 0, static_cast<FwAssertArgType>(status));
}

RtemsConditionVariable::~RtemsConditionVariable() {
    pthread_cond_destroy(&m_handle.condition);
}

ConditionVariableInterface::Status RtemsConditionVariable::pend(Os::Mutex& mutex) {
    printf("Condition: Entering pend\n");
    fflush(stdout);
    Os::MutexHandle* mutex_handle = mutex.getHandle();
    auto* rtems_mutex_handle = static_cast<Os::RTEMS::Mutex::RTEMSMutexHandle*>(mutex_handle);
    int status = pthread_cond_wait(&m_handle.condition, &rtems_mutex_handle->posix_mutex);
    printf("Condition: Exited pend with status %d\n", status);
    fflush(stdout);
    if (status != 0) {
        return Status::ERROR_OTHER;
    }
    return Status::OP_OK;
}

void RtemsConditionVariable::notify() {
    printf("Condition: Notifying\n");
    fflush(stdout);
    int status = pthread_cond_signal(&m_handle.condition);
    FW_ASSERT(status == 0, static_cast<FwAssertArgType>(status));
}

void RtemsConditionVariable::notifyAll() {
    printf("Condition: Notifying all\n");
    fflush(stdout);
    int status = pthread_cond_broadcast(&m_handle.condition);
    FW_ASSERT(status == 0, static_cast<FwAssertArgType>(status));
}

} // namespace ConditionVariable
} // namespace RTEMS
} // namespace Os