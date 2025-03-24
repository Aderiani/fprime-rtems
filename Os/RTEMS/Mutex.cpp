// ======================================================================
// \title Os/RTEMS/Mutex.cpp
// \brief RTEMS implementation for Os::Mutex
// ======================================================================
#include <Os/RTEMS/Mutex.hpp>
#include <Fw/Types/Assert.hpp>
#include <rtems.h>

namespace Os {
namespace RTEMS {
namespace Mutex {

RTEMSMutex::RTEMSMutex() {
    // Create a RTEMS mutex (semaphore used as mutex)
    rtems_status_code status = rtems_semaphore_create(
        rtems_build_name('F', 'P', 'M', 'X'),  // Name
        1,                                      // Initial count (1 for mutex)
        RTEMS_BINARY_SEMAPHORE | RTEMS_PRIORITY | RTEMS_INHERIT_PRIORITY,  // Attributes
        0,                                      // Priority ceiling (not used with INHERIT_PRIORITY)
        &this->m_handle.mutex_id              // Where to store ID
    );
    FW_ASSERT(status == RTEMS_SUCCESSFUL, static_cast<FwAssertArgType>(status));
    
    // Initialize the POSIX mutex for condition variables
    pthread_mutexattr_t attr;
    int result = pthread_mutexattr_init(&attr);
    FW_ASSERT(result == 0, static_cast<FwAssertArgType>(result));
    
    // Configure mutex attributes if needed
    result = pthread_mutexattr_setprotocol(&attr, PTHREAD_PRIO_INHERIT);
    FW_ASSERT(result == 0, static_cast<FwAssertArgType>(result));
    
    // Initialize the mutex with attributes
    result = pthread_mutex_init(&this->m_handle.posix_mutex, &attr);
    FW_ASSERT(result == 0, static_cast<FwAssertArgType>(result));
    
    // Destroy the attributes
    result = pthread_mutexattr_destroy(&attr);
    FW_ASSERT(result == 0, static_cast<FwAssertArgType>(result));
}

RTEMSMutex::~RTEMSMutex() {
    // Delete the RTEMS mutex
    rtems_status_code status = rtems_semaphore_delete(this->m_handle.mutex_id);
    FW_ASSERT(status == RTEMS_SUCCESSFUL, static_cast<FwAssertArgType>(status));
    
    // Destroy the POSIX mutex
    int result = pthread_mutex_destroy(&this->m_handle.posix_mutex);
    FW_ASSERT(result == 0, static_cast<FwAssertArgType>(result));
}

MutexInterface::Status RTEMSMutex::take() {
    // Take the RTEMS mutex
    rtems_status_code rtems_status = rtems_semaphore_obtain(
        this->m_handle.mutex_id,
        RTEMS_WAIT,        // Wait option
        RTEMS_NO_TIMEOUT   // No timeout
    );
    
    // Also take the POSIX mutex for consistency
    int posix_status = pthread_mutex_lock(&this->m_handle.posix_mutex);
    FW_ASSERT(posix_status == 0, static_cast<FwAssertArgType>(posix_status));
    
    if (rtems_status == RTEMS_SUCCESSFUL) {
        return Status::OP_OK;
    } else if (rtems_status == RTEMS_UNSATISFIED) {
        return Status::ERROR_BUSY;
    } else if (rtems_status == RTEMS_TIMEOUT) {
        return Status::ERROR_BUSY;
    } else {
        return Status::ERROR_OTHER;
    }
}

MutexInterface::Status RTEMSMutex::release() {
    // Release the RTEMS mutex
    rtems_status_code rtems_status = rtems_semaphore_release(this->m_handle.mutex_id);
    
    // Also release the POSIX mutex
    int posix_status = pthread_mutex_unlock(&this->m_handle.posix_mutex);
    FW_ASSERT(posix_status == 0, static_cast<FwAssertArgType>(posix_status));
    
    if (rtems_status == RTEMS_SUCCESSFUL) {
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