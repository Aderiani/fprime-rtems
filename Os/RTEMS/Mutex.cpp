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
}

RTEMSMutex::~RTEMSMutex() {
    // Delete the mutex
    rtems_status_code status = rtems_semaphore_delete(this->m_handle.mutex_id);
    FW_ASSERT(status == RTEMS_SUCCESSFUL, static_cast<FwAssertArgType>(status));
}

MutexInterface::Status RTEMSMutex::take() {
    rtems_status_code status = rtems_semaphore_obtain(
        this->m_handle.mutex_id,
        RTEMS_WAIT,        // Wait option
        RTEMS_NO_TIMEOUT   // No timeout
    );
    
    if (status == RTEMS_SUCCESSFUL) {
        return Status::OP_OK;
    } else if (status == RTEMS_UNSATISFIED) {
        return Status::ERROR_BUSY;
    } else if (status == RTEMS_TIMEOUT) {
        return Status::ERROR_BUSY;
    } else {
        return Status::ERROR_OTHER;
    }
}

MutexInterface::Status RTEMSMutex::release() {
    rtems_status_code status = rtems_semaphore_release(this->m_handle.mutex_id);
    
    if (status == RTEMS_SUCCESSFUL) {
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