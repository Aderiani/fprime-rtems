// Os/RTEMS/Mutex.cpp
#include <Os/RTEMS/Mutex.hpp>
#include <Fw/Types/Assert.hpp>
#include <rtems.h>
#include <cstdio>    // For printf
#include <errno.h>   // For EBUSY

namespace Os {
namespace RTEMS {
namespace Mutex {

RTEMSMutex::RTEMSMutex() {
    pthread_mutexattr_t attr;
    int result = pthread_mutexattr_init(&attr);
    FW_ASSERT(result == 0, static_cast<FwAssertArgType>(result));
    
    result = pthread_mutexattr_setprotocol(&attr, PTHREAD_PRIO_INHERIT);
    FW_ASSERT(result == 0, static_cast<FwAssertArgType>(result));
    
    result = pthread_mutex_init(&this->m_handle.posix_mutex, &attr);
    FW_ASSERT(result == 0, static_cast<FwAssertArgType>(result));
    
    result = pthread_mutexattr_destroy(&attr);
    FW_ASSERT(result == 0, static_cast<FwAssertArgType>(result));
}

RTEMSMutex::~RTEMSMutex() {
    int result = pthread_mutex_destroy(&this->m_handle.posix_mutex);
    FW_ASSERT(result == 0, static_cast<FwAssertArgType>(result));
}

MutexInterface::Status RTEMSMutex::take() {
    printf("Mutex: Taking\n");
    int status = pthread_mutex_lock(&this->m_handle.posix_mutex);
    printf("Mutex: Taken with status %d\n", status);
    if (status == 0) {
        return Status::OP_OK;
    } else if (status == EBUSY) {
        return Status::ERROR_BUSY;
    } else {
        return Status::ERROR_OTHER;  // Default return for unhandled cases
    }
}

MutexInterface::Status RTEMSMutex::release() {
    printf("Mutex: Releasing\n");
    int status = pthread_mutex_unlock(&this->m_handle.posix_mutex);
    printf("Mutex: Released with status %d\n", status);
    if (status == 0) {
        return Status::OP_OK;
    } else {
        return Status::ERROR_OTHER;
    }
}

MutexHandle* RTEMSMutex::getHandle() {
    return &this->m_handle;
}

}  // namespace Mutex
}  // namespace RTEMS
}  // namespace Os